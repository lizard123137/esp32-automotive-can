#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
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

// If the network from secrets.hpp can't be joined, the ESP32 opens its own access point instead.
static constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 8000;
static constexpr const char *MDNS_HOSTNAME = "f30";
static constexpr const char *FALLBACK_AP_SSID = "BMW_F30_Cluster";
static constexpr const char *FALLBACK_AP_PASS = "12345678";

// CAN init must not block boot: without the MCP2515 attached WiFi and the web UI still have to come up.
static constexpr int CAN_INIT_ATTEMPTS = 3;
static constexpr unsigned long CAN_RETRY_INTERVAL_MS = 5000;
static bool can_ready = false;
static unsigned long can_last_attempt = 0;

static bool init_can() {
    // IMPORTANT! Some older vehicles use 250kbps.
    if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) != CAN_OK) {
        return false;
    }
    CAN.setMode(MCP_NORMAL);
    return true;
}

void setup() {
    Serial.begin(9600);
    Logger::print(INFO, "UART", "Started serial communication");

    //
    // WiFi Setup
    //

    // Connect to WiFi that was set in secrets.hpp (template: secrets.example.hpp)
    WiFi.setHostname(MDNS_HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    if (WiFi.waitForConnectResult(WIFI_CONNECT_TIMEOUT_MS) == WL_CONNECTED) {
        Logger::print(INFO, "WiFi", "WiFi connected, IP: %s", WiFi.localIP().toString().c_str());

        if (MDNS.begin(MDNS_HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            Logger::print(INFO, "mDNS", "Dashboard available at http://%s.local", MDNS_HOSTNAME);
        } else {
            Logger::print(ERROR, "mDNS", "Failed to start mDNS responder");
        }
    } else {
        Logger::print(ERROR, "WiFi", "Failed to connect to wifi SSID: %s, starting access point %s", WIFI_SSID, FALLBACK_AP_SSID);

        // Stop the station's reconnect attempts, their channel scans would disrupt clients of the fallback AP
        WiFi.disconnect();
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(FALLBACK_AP_SSID, FALLBACK_AP_PASS);
        Logger::print(INFO, "WiFi", "Access point IP: %s", WiFi.softAPIP().toString().c_str());
    }

    //
    // CAN Setup
    //
    pinMode(SPI_CS_PIN, OUTPUT);

    for (int attempt = 0; attempt < CAN_INIT_ATTEMPTS && !can_ready; ++attempt) {
        can_ready = init_can();
        if (!can_ready) {
            delay(500);
        }
    }
    can_last_attempt = millis();

    if (can_ready) {
        Logger::print(INFO, "CAN", "Initialized CAN");
    } else {
        Logger::print(ERROR, "CAN", "MCP2515 not responding, continuing without CAN (retrying every %lu ms)", CAN_RETRY_INTERVAL_MS);
    }

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
    // Without a responding MCP2515 the SPI reads are garbage (DLC up to 15), so skip all CAN traffic until init succeeds.
    if (can_ready) {
        can_manager.readFrame();
        current_scheduler.step();
    } else if (millis() - can_last_attempt >= CAN_RETRY_INTERVAL_MS) {
        can_last_attempt = millis();
        can_ready = init_can();
        if (can_ready) {
            Logger::print(INFO, "CAN", "Initialized CAN");
        }
    }

    update_api(can_ready);
}