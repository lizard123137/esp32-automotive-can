#include <Arduino.h>
#include <mcp_can.h>

#include <cmath>

#include "crc/crc.hpp"
#include "dbc/dbc_message.hpp"
#include "dbc/dbc_signal.hpp"

#define SPI_CS_PIN 3

MCP_CAN CAN(SPI_CS_PIN);

uint8_t counter_4bit = 0;
unsigned long last_update_fast = 0;
unsigned long last_update_mid = 0;
unsigned long last_update_slow = 0;

DBCMessage dbc_message_ignition("IgnitionStatus", 0x12F, 7, CAN);
DBCMessage dbc_message_drivemode("DriveMode", 0x3A7, 7, CAN);
DBCMessage dbc_message_backlight("BacklightBrightness", 0x202, 2, CAN);
DBCMessage dbc_message_lights("Lights", 0x21A, 3, CAN);
DBCMessage dbc_message_blinkers("Blinkers", 0x1F6, 2, CAN);

 void setup() {
    Serial.begin(115200);
    pinMode(SPI_CS_PIN, OUTPUT);

    while (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
        delay(500);
    }
    CAN.setMode(MCP_NORMAL);

    // Messages setup

    dbc_message_ignition.use_counter = true;
    dbc_message_ignition.default_payload = {
        0x80, 0x08, 0xDD, 0xF1,
        0x01, 0x30, 0x06, 0x00
    };
    dbc_message_ignition.use_crc = true;
    dbc_message_ignition.crc_func = getCrc8;
    
    dbc_message_drivemode.use_counter = true;
    dbc_message_drivemode.default_payload = {
        0xF0, 0x00, 0x00, 0x00,
        0x11, 0xC0, 0x00, 0x00,
    };
    dbc_message_drivemode.crc_func = getCrc8;
    
    dbc_message_backlight.use_counter = false;
    dbc_message_backlight.default_payload = {
        0xFF, 0xFF,
    };
    dbc_message_backlight.use_crc = false;
    
    dbc_message_lights.use_counter = false;
    dbc_message_lights.default_payload = {
        0x00, 0xC0, 0xF7
    };
    dbc_message_lights.use_crc = false;

    dbc_message_blinkers.use_counter = false;
    dbc_message_blinkers.default_payload = {
        0x81, 0xF0 // TODO fix to support blinkers off
    };
    dbc_message_blinkers.use_crc = false;

    // Signals

    DBCSignal dbc_sig_ignition_state = {
        .name = "Ignition State",
        .start_bit = 8,
        .bit_length = 8,
        .factor = 1.0,
        .offset = 0.0,
        .value = 0x8A,
    };
    dbc_message_ignition.signals.push_back(dbc_sig_ignition_state);

    DBCSignal dbc_sig_drive_mode_state {
        .name = "Drive Mode State",
        .start_bit = 24,
        .bit_length = 8,
        .factor = 1.0,
        .offset = 0.0,
        .value = 5, // Sport+
    };
    dbc_message_drivemode.signals.push_back(dbc_sig_drive_mode_state);

    DBCSignal dbc_sig_backlight_brightness = {
        .name = "Backlight Brightness",
        .start_bit = 0,
        .bit_length = 8,
        .factor = 1.0,
        .offset = 0.0,
        .value = 253,
    };
    dbc_message_backlight.signals.push_back(dbc_sig_backlight_brightness);    

    DBCSignal dbc_sig_lights_highbeams = {
        .name = "HighBeam",
        .start_bit = 1,
        .bit_length = 1,
        .value = 1,
    };
    DBCSignal dbc_sig_lights_main = {
        .name = "MainLights",
        .start_bit = 2,
        .bit_length = 1,
        .value = 1,
    };
    DBCSignal dbc_sig_lights_ffog = {
        .name = "FrontFog",
        .start_bit = 5,
        .bit_length = 1,
        .value = 1,
    };
    DBCSignal dbc_sig_lights_rfog = {
        .name = "RearFog",
        .start_bit = 6,
        .bit_length = 1,
        .value = 1,
    };
    dbc_message_lights.signals.push_back(dbc_sig_lights_highbeams);
    dbc_message_lights.signals.push_back(dbc_sig_lights_main);
    dbc_message_lights.signals.push_back(dbc_sig_lights_ffog);
    dbc_message_lights.signals.push_back(dbc_sig_lights_rfog);

    DBCSignal dbc_sig_blinkers_left = {
        .name = "Blinker Left",
        .start_bit = 4,
        .bit_length = 1,
        .value = 1,
    };
    DBCSignal dbc_sig_blinkers_right = {
        .name = "Blinker Right",
        .start_bit = 5,
        .bit_length = 1,
        .value = 1,
    };
    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_left);
    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_right);
}

void loop() {
    unsigned long now = millis();

    if (now - last_update_fast >= 20) {
        last_update_fast = now;

        counter_4bit++;
        if (counter_4bit >= 14) counter_4bit = 0;
    
    }

    if (now - last_update_mid >= 100) {
        last_update_mid = now;

        dbc_message_ignition.sendMessage(0x44, counter_4bit);
    }

    if (now - last_update_slow >= 500) {
        last_update_slow = now;

        dbc_message_drivemode.sendMessage(0x4A, counter_4bit);
        dbc_message_backlight.sendMessage();
        dbc_message_lights.sendMessage();
        dbc_message_blinkers.sendMessage();
    }
}