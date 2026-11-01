#include <ArduinoJson.hpp>
#include <WiFi.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

#include "can.hpp"
#include "http/api.hpp"
#include "http/html.hpp"
#include "scheduler.hpp"

static constexpr unsigned long FRAMES_INTERVAL_MS = 250;
static constexpr unsigned long STATE_INTERVAL_MS = 1000;
static constexpr size_t MAX_BODY_SIZE = 256;

static AsyncWebSocket *websocket = nullptr;
static std::atomic<bool> can_online{false};

// can_manager.history is modified by loop() while the HTTP/WebSocket handlers run on the async_tcp task,
// so update_api() copies it here and the handlers only ever read the copy.
static std::mutex frames_mutex;
static std::vector<CANFrame> frames_snapshot;

//
// JSON payloads
//

static String frames_json() {
    std::vector<CANFrame> frames;
    {
        std::lock_guard<std::mutex> lock(frames_mutex);
        frames = frames_snapshot;
    }

    JsonDocument doc;
    doc["type"] = "frames";
    JsonArray frames_array = doc["frames"].to<JsonArray>();

    for (const auto& frame : frames) {
        JsonObject frame_obj = frames_array.add<JsonObject>();
        frame_obj["id"] = frame.id;
        frame_obj["dlc"] = frame.dlc;
        JsonArray data_array = frame_obj["data"].to<JsonArray>();
        for (uint8_t i = 0; i < frame.dlc && i < frame.data.size(); ++i) {
            data_array.add(frame.data[i]);
        }
    }

    String json;
    serializeJson(doc, json);
    return json;
}

static String state_json() {
    JsonDocument doc;
    doc["type"] = "state";
    doc["can"] = can_online.load();
    doc["uptime"] = millis();
    doc["heap"] = ESP.getFreeHeap();
    doc["clients"] = websocket->count();

    JsonObject net_obj = doc["net"].to<JsonObject>();
    if (WiFi.status() == WL_CONNECTED) {
        net_obj["mode"] = "STA";
        net_obj["ip"] = WiFi.localIP().toString();
        net_obj["rssi"] = WiFi.RSSI();
    } else {
        net_obj["mode"] = "AP";
        net_obj["ip"] = WiFi.softAPIP().toString();
    }

    JsonObject signals_obj = doc["signals"].to<JsonObject>();
    JsonArray tx_array = doc["tx"].to<JsonArray>();
    for (const auto& timer : current_scheduler.timers) {
        for (const auto& message : timer->messages) {
            for (const auto& signal : message->signals) {
                signals_obj[signal.name] = signal.value;
            }

            // Payload as built by getBytes(); sendMessage() prepends a CRC byte when crc_func is set
            JsonObject tx_obj = tx_array.add<JsonObject>();
            tx_obj["id"] = message->id;
            tx_obj["crc"] = static_cast<bool>(message->crc_func);
            JsonArray data_array = tx_obj["data"].to<JsonArray>();
            const auto bytes = message->getBytes();
            for (uint8_t i = 0; i < message->dlc && i < bytes.size(); ++i) {
                data_array.add(bytes[i]);
            }
        }
    }

    String json;
    serializeJson(doc, json);
    return json;
}

//
// Signal updates
//

// Applies {"<signal>": <number>, ...} to the scheduler. Unknown signals and non-numeric values are skipped.
// Returns the number of applied signals.
static size_t apply_values(JsonObjectConst values) {
    size_t applied = 0;
    for (JsonPairConst pair : values) {
        const JsonVariantConst value = pair.value();
        if ((value.is<double>() || value.is<bool>())
            && current_scheduler.updateSignalValue(pair.key().c_str(), value.as<double>()) == 0) {
            ++applied;
        }
    }
    return applied;
}

// Reads the PATCH value from ?value=, a form body (value=120), a JSON body ({"value": 120}) or a bare JSON number.
static bool read_value(AsyncWebServerRequest *request, double &value) {
    const AsyncWebParameter *param = request->getParam("value");
    if (param == nullptr) {
        param = request->getParam("value", true);
    }

    if (param != nullptr) {
        const char *text = param->value().c_str();
        char *end = nullptr;
        value = strtod(text, &end);
        return end != text && *end == '\0' && std::isfinite(value);
    }

    if (request->_tempObject == nullptr) {
        return false;
    }
    JsonDocument body;
    if (deserializeJson(body, static_cast<const char *>(request->_tempObject))) {
        return false;
    }
    JsonVariantConst variant = body.as<JsonVariantConst>();
    if (variant.is<JsonObjectConst>()) {
        variant = variant["value"];
    }
    value = variant.as<double>();
    return variant.is<double>() && std::isfinite(value);
}

void init_api(AsyncWebServer& server, AsyncWebSocket& ws) {
    websocket = &ws;

    //
    // WebSocket Setup
    //
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Logger::print(INFO, "WebSocket", "Client connected: %u", client->id());
            client->text(state_json());
            client->text(frames_json());
            return;
        }
        if (type == WS_EVT_DISCONNECT) {
            Logger::print(INFO, "WebSocket", "Client disconnected: %u", client->id());
            return;
        }
        if (type != WS_EVT_DATA) {
            return;
        }

        // Control messages are tiny, so only complete single-frame text messages are handled
        const AwsFrameInfo *info = static_cast<AwsFrameInfo *>(arg);
        if (!info->final || info->index != 0 || info->len != len || info->opcode != WS_TEXT) {
            return;
        }

        JsonDocument doc;
        if (deserializeJson(doc, reinterpret_cast<const char *>(data), len)) {
            return;
        }

        if (doc["type"] == "sync") {
            client->text(state_json());
            client->text(frames_json());
            return;
        }

        // {"signal": "speed-value", "value": 120} is handled as a batch of one
        if (const char *signal_name = doc["signal"]) {
            doc["values"][signal_name] = doc["value"];
        }

        // Every dashboard gets the new values and re-encoded payloads right away, not on the next 1 s tick
        if (apply_values(doc["values"].as<JsonObjectConst>()) > 0) {
            websocket->textAll(state_json());
        }
    });
    server.addHandler(&ws);

    //
    // HTTP Endpoints Setup
    //
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", html);
    });

    server.on("/api/read", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", frames_json());
    });

    server.on("/api/scheduler", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        JsonArray timers_array = doc["timers"].to<JsonArray>();

        for (const auto& timer : current_scheduler.timers) {
            JsonObject timer_obj = timers_array.add<JsonObject>();
            timer_obj["interval"] = timer->interval;

            JsonArray messages_array = timer_obj["messages"].to<JsonArray>();
            for (const auto& message : timer->messages) {
                JsonObject message_obj = messages_array.add<JsonObject>();
                message_obj["id"] = message->id;
                message_obj["name"] = message->name;
                message_obj["counter"] = message->counter ? message->counter->value : -1;
                message_obj["dlc"] = message->dlc;

                JsonArray payload_array = message_obj["default_payload"].to<JsonArray>();
                for (uint8_t byte : message->default_payload) {
                    payload_array.add(byte);
                }

                JsonArray signals_array = message_obj["signals"].to<JsonArray>();
                for (const auto& signal : message->signals) {
                    JsonObject signal_obj = signals_array.add<JsonObject>();
                    signal_obj["name"] = signal.name;
                    signal_obj["start_bit"] = signal.start_bit;
                    signal_obj["bit_length"] = signal.bit_length;
                    signal_obj["factor"] = signal.factor;
                    signal_obj["offset"] = signal.offset;
                    signal_obj["value"] = signal.value;
                }
            }
        }

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server.on("^\\/api\\/signals\\/([\\w-]+)$", HTTP_PATCH, [](AsyncWebServerRequest *request) {
        const String signal_name = request->pathArg(0);

        double value;
        if (!read_value(request, value)) {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing or invalid 'value' parameter\"}");
            return;
        }
        if (current_scheduler.updateSignalValue(signal_name.c_str(), value) != 0) {
            request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Signal not found\"}");
            return;
        }

        Logger::print(INFO, "API", "Signal %s set to %f", signal_name.c_str(), value);
        websocket->textAll(state_json());
        request->send(200, "application/json", "{\"status\":\"success\"}");
    }, nullptr, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        // JSON bodies arrive here (form bodies are parsed into params by the server); the request frees _tempObject
        if (total == 0 || total > MAX_BODY_SIZE || index + len > total) {
            return;
        }
        if (index == 0 && request->_tempObject == nullptr) {
            request->_tempObject = calloc(total + 1, 1);
        }
        if (request->_tempObject != nullptr) {
            memcpy(static_cast<uint8_t *>(request->_tempObject) + index, data, len);
        }
    });

    server.begin();
}

// Called from loop(): streams new CAN frames every 250 ms and the state (telemetry + TX payloads) every second.
void update_api(bool can_ready) {
    can_online = can_ready;
    if (websocket == nullptr) {
        return;
    }

    static unsigned long last_frames = 0;
    static unsigned long last_state = 0;
    const unsigned long now = millis();

    if (now - last_frames >= FRAMES_INTERVAL_MS) {
        last_frames = now;

        // Bytes past the DLC are never written by CANManager, so they are not compared
        const auto same_frame = [](const CANFrame& a, const CANFrame& b) {
            const size_t len = std::min<size_t>(a.dlc, a.data.size());
            return a.id == b.id && a.dlc == b.dlc && std::equal(a.data.begin(), a.data.begin() + len, b.data.begin());
        };

        bool changed;
        {
            std::lock_guard<std::mutex> lock(frames_mutex);
            changed = !std::equal(frames_snapshot.begin(), frames_snapshot.end(),
                                  can_manager.history.begin(), can_manager.history.end(), same_frame);
            if (changed) {
                frames_snapshot = can_manager.history;
            }
        }

        if (changed && websocket->count() > 0) {
            websocket->textAll(frames_json());
        }
    }

    if (now - last_state >= STATE_INTERVAL_MS) {
        last_state = now;
        websocket->cleanupClients();
        if (websocket->count() > 0) {
            websocket->textAll(state_json());
        }
    }
}
