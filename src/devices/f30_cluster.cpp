#include "timer.hpp"
#include "devices.hpp"
#include "crc/crc.hpp"

//
// Counters
//
static Counter counter_4bit(0, 14, 4);
static Counter counter_alive(0, 254, 8);

//
// Timers
//
static Timer timer_fast(20, &counter_4bit);
static Timer timer_mid(100, &counter_alive);
static Timer timer_slow(500);

//
// Messages
//
static DBCMessage dbc_message_speed("Speed", 0x1A1, 5);
static DBCMessage dbc_message_drivemode("DriveMode", 0x3A7, 7);
static DBCMessage dbc_message_backlight("BacklightBrightness", 0x202, 2);
static DBCMessage dbc_message_ignition("IgnitionStatus", 0x12F, 7);
static DBCMessage dbc_message_lights("Lights", 0x21A, 3);
static DBCMessage dbc_message_blinkers("Blinkers", 0x1F6, 2);

//
// Signals
//

// Speed
static DBCSignal dbc_sig_speed_value = {
    .name = "speed-value",
    .start_bit = 8,
    .bit_length = 18,
    .factor = 0.0,
    .offset = 0.0,
    .value = 0x400191,
};

// Ignition
static DBCSignal dbc_sig_ignition_state = {
    .name = "ignition-state",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x8A,
};

// Drive mode
static DBCSignal dbc_sig_drive_mode_state {
    .name = "drive-mode-state",
    .start_bit = 24,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 5, // Sport+
};

// Backlight
static DBCSignal dbc_sig_backlight_brightness = {
    .name = "backlight-brightness",
    .start_bit = 0,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 253,
};

// Lights
static DBCSignal dbc_sig_lights_highbeams = {
    .name = "high-beam",
    .start_bit = 1,
    .bit_length = 1,
    .value = 1,
};
static DBCSignal dbc_sig_lights_main = {
    .name = "main-lights",
    .start_bit = 2,
    .bit_length = 1,
    .value = 1,
};
static DBCSignal dbc_sig_lights_ffog = {
    .name = "front-fog",
    .start_bit = 5,
    .bit_length = 1,
    .value = 1,
};
static DBCSignal dbc_sig_lights_rfog = {
    .name = "rear-fog",
    .start_bit = 6,
    .bit_length = 1,
    .value = 1,
};

// Blinkers
static DBCSignal dbc_sig_blinkers_left = {
    .name = "blinker-left",
    .start_bit = 4,
    .bit_length = 1,
    .value = 1,
};
static DBCSignal dbc_sig_blinkers_right = {
    .name = "blinker-right",
    .start_bit = 5,
    .bit_length = 1,
    .value = 1,
};

//
// Scheduler
//
static Scheduler scheduler_f30_cluster;    

Scheduler& init_scheduler_f30_cluster() {
    //
    // Messages
    //
    dbc_message_speed.counter = &counter_4bit;
    dbc_message_speed.default_payload = { 0xC0, 0x00, 0x00, 0x81, 0x00 };
    dbc_message_speed.crc_func = getCrc8;

    dbc_message_ignition.counter = &counter_4bit;
    dbc_message_ignition.default_payload = { 0x80, 0x08, 0xDD, 0xF1, 0x01, 0x30, 0x06 };
    dbc_message_ignition.crc_func = getCrc8;
    
    dbc_message_drivemode.counter = &counter_4bit;
    dbc_message_drivemode.default_payload = { 0xF0, 0x00, 0x00, 0x00, 0x11, 0xC0, 0x00 };
    dbc_message_drivemode.crc_func = getCrc8;
    
    dbc_message_backlight.default_payload = { 0xFF, 0xFF };
    
    dbc_message_lights.default_payload = { 0x00, 0xC0, 0xF7 };
    // TODO fix to support blinkers off
    dbc_message_blinkers.default_payload = { 0x81, 0xF0 };

    //
    // Signals
    //
    dbc_message_speed.signals.push_back(dbc_sig_speed_value);

    dbc_message_ignition.signals.push_back(dbc_sig_ignition_state);

    dbc_message_drivemode.signals.push_back(dbc_sig_drive_mode_state);

    dbc_message_backlight.signals.push_back(dbc_sig_backlight_brightness);    

    dbc_message_lights.signals.push_back(dbc_sig_lights_highbeams);
    dbc_message_lights.signals.push_back(dbc_sig_lights_main);
    dbc_message_lights.signals.push_back(dbc_sig_lights_ffog);
    dbc_message_lights.signals.push_back(dbc_sig_lights_rfog);

    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_left);
    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_right);

    //
    // Timers
    //
    timer_fast.messages.push_back(&dbc_message_speed);

    timer_mid.messages.push_back(&dbc_message_ignition);

    timer_slow.messages.push_back(&dbc_message_drivemode);
    // timer_slow.messages.push_back(&dbc_message_blinkers);
    timer_slow.messages.push_back(&dbc_message_backlight);
    timer_slow.messages.push_back(&dbc_message_lights);

    //
    // Scheduler
    //
    scheduler_f30_cluster.timers.push_back(&timer_fast);
    scheduler_f30_cluster.timers.push_back(&timer_mid);
    scheduler_f30_cluster.timers.push_back(&timer_slow);

    return scheduler_f30_cluster;
}