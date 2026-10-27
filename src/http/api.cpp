#include <ArduinoJson.hpp>

#include "can.hpp"
#include "http/api.hpp"
#include "http/html.hpp"
#include "scheduler.hpp"

void init_api(AsyncWebServer& server, AsyncWebSocket& ws) {
    //
    // WebSocket Setup
    //
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Logger::print(INFO, "WebSocket", "Client connected: %u", client->id());
        } else if (type == WS_EVT_DISCONNECT) {
            Logger::print(INFO, "WebSocket", "Client disconnected: %u", client->id());
        } else if (type == WS_EVT_DATA) {
            // Handle incoming data from clients if needed
        }
    });
    server.addHandler(&ws);

    //
    // GET Endpoints Setup
    //

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", html);
    });

    server.on("api/read", HTTP_GET, [](AsyncWebServerRequest *request) {
        // TODO live update the read table via websocket
        JsonDocument doc;
        JsonArray frames_array = doc.createNestedArray("frames");

        for (const auto& frame : can_manager.history) {
            JsonObject frame_obj = frames_array.createNestedObject();
            frame_obj["id"] = frame.id;
            frame_obj["dlc"] = frame.dlc;
            JsonArray data_array = frame_obj.createNestedArray("data");

            for (uint8_t byte : frame.data) {
                data_array.add(byte);
            }
        }

        String json;
        serializeJson(doc, json);

        request->send(200, "application/json", json);
    });

    server.on("/api/scheduler", HTTP_GET, [](AsyncWebServerRequest *request) {
        JsonDocument doc;
        JsonArray timers_array = doc.createNestedArray("timers");

        for (const auto& timer : current_scheduler.timers) {
            JsonObject timer_obj = timers_array.createNestedObject();
            timer_obj["interval"] = timer->interval;

            JsonArray messages_array = timer_obj.createNestedArray("messages");
            for (const auto& message : timer->messages) {
                JsonObject message_obj = messages_array.createNestedObject();
                message_obj["id"] = message->id;
                message_obj["name"] = message->name;
                message_obj["counter"] = message->counter ? message->counter->value : -1;
                message_obj["dlc"] = message->dlc;
                JsonArray payload_array = message_obj.createNestedArray("default_payload");

                for (uint8_t byte : message->default_payload) {
                    payload_array.add(byte);
                }

                JsonArray signals_array = message_obj.createNestedArray("signals");
                for (const auto& signal : message->signals) {
                    JsonObject signal_obj = signals_array.createNestedObject();
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

    //
    // PATCH Endpoints Setup
    //

    server.on("^\\/api\\/signals\\/(S+)$", HTTP_PATCH, [](AsyncWebServerRequest *request) {
        std::string signal_name(request->pathArg(0).c_str());
        Logger::print(INFO, "API", "Received PATCH request for signal: %s", signal_name.c_str());

        if (request->hasParam("value", true)) {
            String value_str = request->getParam("value", true)->value();
            double new_value = value_str.toDouble();
            Logger::print(INFO, "API", "New value for signal %s: %f", signal_name.c_str(), new_value);

            // Update the signal value in the scheduler
            int result = current_scheduler.updateSignalValue(signal_name, new_value);
            if (result == 0) {
                request->send(200, "application/json", "{\"status\":\"success\"}");
            } else {
                request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Signal not found\"}");
            }
        } else {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing 'value' parameter\"}");
        }
    });
    
    server.begin();
}