#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <mcp_can.h>
#include <WiFi.h>

#include <cmath>

#include "crc/crc.hpp"
#include "devices.hpp"
#include "dbc/dbc_message.hpp"
#include "dbc/dbc_signal.hpp"
#include "logger.hpp"
#include "secrets.hpp"
#include "timer.hpp"
#include "scheduler.hpp"

MCP_CAN CAN(SPI_CS_PIN);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Scheduler scheduler;

void setup() {
    Serial.begin(9600);
    Logger::print(INFO, "UART", "Started serial communication");

    //
    // WiFi Setup
    //
    // Log error on disconnect
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
            Logger::print(ERROR, "WiFi", "WiFi disconnected. Reason: %d", info.wifi_sta_disconnected.reason);
        }
    });

    // Connect to WiFi that was set in secrets.hpp
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Logger::print(ERROR, "WiFi", "Failed to connect to wifi SSID: %s", WIFI_SSID);
        return;
    }
    Logger::print(INFO, "WiFi", "Received IP: %s", WiFi.localIP().toString().c_str());

    //
    // CAN Setup
    //
    pinMode(SPI_CS_PIN, OUTPUT);

    while (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
        delay(500);
    }
    CAN.setMode(MCP_NORMAL);

    Logger::print(INFO, "CAN", "Initialized CAN");

    //
    // HTTP Setup
    //
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Logger::print(INFO, "WebSocket", "Client connected: %u", client->id());
        } else if (type == WS_EVT_DISCONNECT) {
            Logger::print(INFO, "WebSocket", "Client disconnected: %u", client->id());
        } else if (type == WS_EVT_DATA) {
            Logger::print(DEBUG, "WebSocket", "Received data from client: %u", client->id());
        }
    });
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", R"rawliteral(
            <!DOCTYPE html>
            <html>
                <head>
                    <title>ESP32 Automotive CAN</title>
                    <script>
                        const ws = new WebSocket("ws"//${location.host}/ws);
                        ws.onmessage = (event) => {
                            console.log("Received message from server:", event.data);
                        };
                    </script>
                </head>
                <body>
                    <h1>ESP32 Automotive CAN</h1>
                    <p>Welcome to the ESP32 Automotive CAN web interface!</p>
                </body>
            </html>
        )rawliteral");
    });

    server.begin();

    //
    // Initialize device scheduler
    //
    scheduler = init_scheduler_f30_cluster();
}

void loop() {
    scheduler.step();
}