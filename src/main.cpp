#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <mcp_can.h>
#include <WiFi.h>

#include <cmath>

#include "can.hpp"
#include "crc/crc.hpp"
#include "devices.hpp"
#include "dbc/dbc_message.hpp"
#include "dbc/dbc_signal.hpp"
#include "http/api.hpp"
#include "logger.hpp"
#include "secrets.hpp"
#include "timer.hpp"
#include "scheduler.hpp"

MCP_CAN CAN(SPI_CS_PIN);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

CANManager can_manager;
Scheduler current_scheduler;

void setup() {
    Serial.begin(9600);
    Logger::print(INFO, "UART", "Started serial communication");

    //
    // WiFi Setup
    //

    // Create a WiFi access point with credentials defined in secrets.hpp
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASS);
    Logger::print(INFO, "WiFi", "Access point IP: %s", WiFi.softAPIP().toString().c_str());

    // Connect to WiFi that was set in secrets.hpp
    // WiFi.mode(WIFI_STA);
    // WiFi.begin(WIFI_SSID, WIFI_PASS);
    // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    //     Logger::print(ERROR, "WiFi", "Failed to connect to wifi SSID: %s", WIFI_SSID);
    //     return;
    // }
    // Logger::print(INFO, "WiFi", "Received IP: %s", WiFi.localIP().toString().c_str());

    //
    // CAN Setup
    //
    pinMode(SPI_CS_PIN, OUTPUT);

    // IMPORTANT! Some older vehicles use 250kbps.
    while (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
        delay(500);
    }
    CAN.setMode(MCP_NORMAL);

    Logger::print(INFO, "CAN", "Initialized CAN");

    //
    // HTTP Setup
    //
    init_api(server, ws);
    
    //
    // Initialize device scheduler
    //
    current_scheduler = init_scheduler_f30_cluster();
}

void loop() {
    can_manager.readFrame();
    current_scheduler.step();
}