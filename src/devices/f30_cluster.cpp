#include "timer.hpp"
#include "devices.hpp"
#include "crc/crc.hpp"

//
// Counters
//
static Counter counter_4bit(0, 14, 4);
static Counter counter_alive(0, 254, 8);
static Counter counter_acc(0, 14, 4, 4);

//
// Timers
//
static Timer timer_fast(20);
static Timer timer_mid(100);
static Timer timer_slow(500);

//
// Messages
//
static DBCMessage dbc_message_speed("Speed", 0x1A1, 4);
static DBCMessage dbc_message_rpm("RPM", 0x0F3, 7);
static DBCMessage dbc_message_drivemode("Drive Mode", 0x3A7, 6);
static DBCMessage dbc_message_backlight("Backlight Brightness", 0x202, 2);
static DBCMessage dbc_message_ignition("Ignition Status", 0x12F, 7);
static DBCMessage dbc_message_lights("Lights", 0x21A, 3);
static DBCMessage dbc_message_blinkers("Blinkers", 0x1F6, 2);
static DBCMessage dbc_message_alerts("Alerts", 0x5C0, 8);
static DBCMessage dbc_message_gear_automatic("Gear Automatic", 0x3FD, 4);
static DBCMessage dbc_message_fuel_level("Fuel Level", 0x349, 5);
static DBCMessage dbc_message_handbrake("Handbrake Status", 0x36F, 4);
static DBCMessage dbc_message_steering_wheel_buttons("Steering Wheel Buttons", 0x1EE, 2);
static DBCMessage dbc_message_oil("Oil Status", 0x3F9, 7);
static DBCMessage dbc_message_engine_temp("Engine Temp", 0x2C4, 7);

// OK status messages
static DBCMessage dbc_message_abs("ABS Status", 0x36E, 4);
static DBCMessage dbc_message_power_steering("Power Steering Status", 0x2A7, 4);
static DBCMessage dbc_message_airbag("Airbag Status", 0x19B, 7);
static DBCMessage dbc_message_seatbelt("Seatbelt Status", 0x297, 6);
static DBCMessage dbc_message_tpms("TPMS Status", 0x369, 4);

static DBCMessage dbc_message_alive("Alive Counter", 0x0D7, 2);
static DBCMessage dbc_message_acc("ACC Counter", 0x33B, 5);

//
// Signals
//

// Speed
static DBCSignal dbc_sig_speed_value = {
    .name = "speed-value",
    .start_bit = 8,
    .bit_length = 24,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x810000, // TODO create separate signal for last byte (is zero)
};

// RPM
static DBCSignal dbc_sig_rpm_value = {
    .name = "rpm-value",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x00, // Maps from 0x00 to 0x2B
};

static DBCSignal dbc_sig_rpm_calculated_gear = {
    .name = "rpm-calculated-gear",
    .start_bit = 32,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x0, // Drive
};

// Ignition
static DBCSignal dbc_sig_ignition_state = {
    .name = "ignition-state",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x08,
};

// Drive mode
// 1 - Traction
// 2 - Comfort
// 4 - Sport
// 5 - Sport+
// 6 - DSC off
// 7 - ECO Pro
static DBCSignal dbc_sig_drive_mode_state = {
    .name = "drive-mode-state",
    .start_bit = 24,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0,
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
    .factor = 1.0,
    .value = 0,
};
static DBCSignal dbc_sig_lights_main = {
    .name = "main-lights",
    .start_bit = 2,
    .bit_length = 1,
    .factor = 1.0,
    .value = 0,
};
static DBCSignal dbc_sig_lights_ffog = {
    .name = "front-fog",
    .start_bit = 5,
    .bit_length = 1,
    .factor = 1.0,
    .value = 0,
};
static DBCSignal dbc_sig_lights_rfog = {
    .name = "rear-fog",
    .start_bit = 6,
    .bit_length = 1,
    .factor = 1.0,
    .value = 0,
};

// Blinkers
static DBCSignal dbc_sig_blinkers_left = {
    .name = "blinker-left",
    .start_bit = 4,
    .bit_length = 1,
    .factor = 1.0,
    .value = 0,
};
static DBCSignal dbc_sig_blinkers_right = {
    .name = "blinker-right",
    .start_bit = 5,
    .bit_length = 1,
    .factor = 1.0,
    .value = 0,
};

// Parking brake
// 0x15 - Engaged
// 0x14 - Not engaged
static DBCSignal dbc_sig_handbrake = {
    .name = "handbrake",
    .start_bit = 24,
    .bit_length = 8,
    .factor = 1.0,
    .value = 0x14, // not engaged
};

// Steering wheel buttons
// byte 0 is the button signal
// 0x4C for something???
static DBCSignal dbc_sig_steering_wheel_buttons = {
    .name = "steering-wheel-buttons",
    .start_bit = 0,
    .bit_length = 8,
    .factor = 1.0,
    .value = 0x00, // No buttons pressed
};

/**
 * Alerts
 *
 * Alert payload structure:
 *     Byte 0: 0x40
 *     Byte 1: <alert type>
 *     Byte 2: 0x00
 *     Byte 3: <alert value>
 *     Bytes 4-7: 0xFF
 *
 * Alert types:
 *     0x0F: Door open
 *     0x18: Parking brake - Yellow
 *     0x47: Parking brake - Red
 *     0x22: Check engine
 *     0x24: DSC OFF
 *     0x23: DSC 1
 *     0xD7: DSC 2
 *     0x4D: Seatbelt
 * Alert values:
 *     0x29: True
 *     0x28: False
 */
static DBCSignal dbc_sig_alerts_type = {
    .name = "alert-type",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .value = 0x22, // Check engine
};
static DBCSignal dbc_sig_alerts_value = {
    .name = "alert-value",
    .start_bit = 24,
    .bit_length = 8,
    .factor = 1.0,
    .value = 0x28, // False
};

/**
 * Gear Automatic
 * 
 * Gears:
 *     0x01 - 0x09: 1-9
 *     0x20: P
 *     0x40: R
 *     0x60: N
 *     0x80: D
 *     0x81: DS
 */
static DBCSignal dbc_sig_gear_automatic = {
    .name = "gear-automatic",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .value = 0x20, // P
};

/*
 * Fuel Level is weird :/
 *
 * Apparently on some cars the values are in Liters, but for me it's not really working...
 * I have a theory that it's because my cluster comes from a USA import car, so it might be in gallons???
 *
 * Fuel level is mapped in the following way:
 *
 * out0 = 37    (0x25)
 * out50 = 18   (0x12)
 * out100 = 4   (0x04)
 *
 * If fuel below 50%:
 *      fuel_percent mapped between out0 and out50
 * If fuel above 50%:
 *      fuel_percent mapped between out50 and out100
 *
 * Final payload is:
 *    Byte 0: High byte of fuel level (unused)
 *    Byte 1: Low byte of fuel level
 *    Byte 2: High byte of fuel level (unused)
 *    Byte 3: Low byte of fuel level
 */
static DBCSignal dbc_sig_fuel_level_1 = {
    .name = "fuel-level-1",
    .start_bit = 0,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x18,
};
static DBCSignal dbc_sig_fuel_level_2 = {
    .name = "fuel-level-2",
    .start_bit = 24,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0.0,
    .value = 0x18,
};

// Oil Temperature range 0x64 - 0xC8
static DBCSignal dbc_sig_oil_temp = {
    .name = "oil-temp",
    .start_bit = 32,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0x32,
    .value = 0x64,
};

// Engine Temperature range 0x64 - 0xC8
static DBCSignal dbc_sig_engine_temp = {
    .name = "engine-temp",
    .start_bit = 8,
    .bit_length = 8,
    .factor = 1.0,
    .offset = 0x32,
    .value = 0x64,
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
    dbc_message_speed.default_payload = { 0xC0, 0x00, 0x00, 0x81 };
    dbc_message_speed.crc_func = getCrc8;
    dbc_message_speed.final_xor = 0xA9;

    dbc_message_rpm.counter = &counter_4bit;
    dbc_message_rpm.default_payload = { 0x60, 0x00, 0xC0, 0xF0, 0x00, 0xFF, 0xFF };
    dbc_message_rpm.crc_func = getCrc8;
    dbc_message_rpm.final_xor = 0x7A;

    dbc_message_ignition.counter = &counter_4bit;
    dbc_message_ignition.default_payload = { 0x80, 0x08, 0xDD, 0xF1, 0x01, 0x30, 0x06 };
    dbc_message_ignition.crc_func = getCrc8;
    dbc_message_ignition.final_xor = 0x44;

    dbc_message_drivemode.counter = &counter_4bit;
    dbc_message_drivemode.default_payload = { 0xF0, 0x00, 0x00, 0x00, 0x11, 0xC0, 0x00 };
    dbc_message_drivemode.crc_func = getCrc8;
    dbc_message_drivemode.final_xor = 0x4A;

    dbc_message_alive.counter = &counter_alive;
    dbc_message_alive.default_payload = { 0x00, 0xFF };

    dbc_message_abs.counter = &counter_4bit;
    dbc_message_abs.default_payload = { 0xF0, 0xFE, 0xFF, 0x14 };
    dbc_message_abs.crc_func = getCrc8;
    dbc_message_abs.final_xor = 0xD8;

    dbc_message_power_steering.counter = &counter_4bit;
    dbc_message_power_steering.default_payload = { 0xF0, 0xFE, 0xFF, 0x14 };
    dbc_message_power_steering.crc_func = getCrc8;
    dbc_message_power_steering.final_xor = 0x9E;

    dbc_message_airbag.counter = &counter_4bit;
    dbc_message_airbag.default_payload = { 0x40, 0x40, 0x55, 0xFD, 0xFF, 0xFF, 0xFF };
    dbc_message_airbag.crc_func = getCrc8;
    dbc_message_airbag.final_xor = 0xFF;

    dbc_message_seatbelt.counter = &counter_4bit;
    dbc_message_seatbelt.default_payload = { 0xE0, 0xF1, 0xF0, 0xF2, 0xF2, 0xFE };
    dbc_message_seatbelt.crc_func = getCrc8;
    dbc_message_seatbelt.final_xor = 0x28;

    dbc_message_tpms.counter = &counter_4bit;
    dbc_message_tpms.default_payload = { 0xF0, 0xA2, 0xA0, 0xA0 };
    dbc_message_tpms.crc_func = getCrc8;
    dbc_message_tpms.final_xor = 0xC5;

    dbc_message_acc.counter = &counter_acc;
    dbc_message_acc.default_payload = { 0xF0, 0x5C, 0x70, 0x00, 0x00 };
    dbc_message_acc.crc_func = getCrc8;
    dbc_message_acc.final_xor = 0x6B;

    /*
     * According to openinverter.org there are two messages sent by GWS
     *
     * A status message with id 0x197 that goes like this:
     * byte 0: CRC
     * byte 1: Counter 4bit
     * byte 2: Lever position
     * byte 3: Park button pressed ? 0xD5 : 0xC0
     *
     * Lever position is mapped as follows:
     * 0x2E - UP 2
     * 0x1E - UP 1
     * 0x0E - Center middle
     * 0x3E - DOWN 1
     * 0x4E - DOWN 2
     * 0x5E - SIDE UP
     * 0x7E - Center side
     * 0x6E - SIDE DOWN
     *
     * A GWS appearence message with id 0x3FD that goes like this:
     * byte 0: CRC
     * byte 1: Counter 4bit
     * byte 2: Gear 
     * byte 3: Ignored
     * byte 4: Ignored
     *
     * Where gears are mapped as in the dbc_sig_gear_automatic signal above.
     */

    dbc_message_gear_automatic.counter = &counter_4bit;
    dbc_message_gear_automatic.default_payload = { 0x00, 0x00, 0xFC, 0xFF };
    dbc_message_gear_automatic.crc_func = getCrc8;
    dbc_message_gear_automatic.final_xor = 0xD6;

    dbc_message_handbrake.counter = &counter_4bit;
    dbc_message_handbrake.default_payload = { 0xF0, 0x38, 0x00, 0x00 };
    dbc_message_handbrake.crc_func = getCrc8;
    dbc_message_handbrake.final_xor = 0x17;

    dbc_message_oil.counter = &counter_4bit;
    dbc_message_oil.default_payload = { 0x10, 0x82, 0x4E, 0x7E, 0x00, 0x05, 0x89 };
    dbc_message_oil.crc_func = getCrc8;
    dbc_message_oil.final_xor = 0xF1;

    // No counter
    dbc_message_engine_temp.default_payload = { 0x3E, 0x00, 0x64, 0x64, 0x64, 0x01, 0xF1 };
    dbc_message_engine_temp.crc_func = getCrc8;
    dbc_message_engine_temp.final_xor = 0xB2;

    dbc_message_fuel_level.default_payload = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    dbc_message_backlight.default_payload = { 0xFF, 0xFF };
    dbc_message_lights.default_payload = { 0x00, 0xC0, 0xF7 };
    dbc_message_blinkers.default_payload = { 0x81, 0xF0 };
    dbc_message_alerts.default_payload = { 0x40, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF };

    //
    // Signals
    //
    dbc_message_speed.signals.push_back(dbc_sig_speed_value);

    dbc_message_rpm.signals.push_back(dbc_sig_rpm_value);
    dbc_message_rpm.signals.push_back(dbc_sig_rpm_calculated_gear);

    dbc_message_ignition.signals.push_back(dbc_sig_ignition_state);

    dbc_message_drivemode.signals.push_back(dbc_sig_drive_mode_state);

    dbc_message_backlight.signals.push_back(dbc_sig_backlight_brightness);    

    dbc_message_lights.signals.push_back(dbc_sig_lights_highbeams);
    dbc_message_lights.signals.push_back(dbc_sig_lights_main);
    dbc_message_lights.signals.push_back(dbc_sig_lights_ffog);
    dbc_message_lights.signals.push_back(dbc_sig_lights_rfog);

    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_left);
    dbc_message_blinkers.signals.push_back(dbc_sig_blinkers_right);

    dbc_message_alerts.signals.push_back(dbc_sig_alerts_type);
    dbc_message_alerts.signals.push_back(dbc_sig_alerts_value);

    dbc_message_handbrake.signals.push_back(dbc_sig_handbrake);

    dbc_message_steering_wheel_buttons.signals.push_back(dbc_sig_steering_wheel_buttons);

    dbc_message_gear_automatic.signals.push_back(dbc_sig_gear_automatic);

    dbc_message_fuel_level.signals.push_back(dbc_sig_fuel_level_1);
    dbc_message_fuel_level.signals.push_back(dbc_sig_fuel_level_2);

    dbc_message_oil.signals.push_back(dbc_sig_oil_temp);
    
    dbc_message_engine_temp.signals.push_back(dbc_sig_engine_temp);

    //
    // Timers
    //
    timer_fast.messages.push_back(&dbc_message_speed);
    timer_fast.messages.push_back(&dbc_message_rpm);

    timer_mid.messages.push_back(&dbc_message_ignition);
    timer_mid.messages.push_back(&dbc_message_abs);
    timer_mid.messages.push_back(&dbc_message_power_steering);
    timer_mid.messages.push_back(&dbc_message_airbag);
    timer_mid.messages.push_back(&dbc_message_seatbelt);
    timer_mid.messages.push_back(&dbc_message_alerts);
    timer_mid.messages.push_back(&dbc_message_alive);
    timer_mid.messages.push_back(&dbc_message_acc);
    timer_mid.messages.push_back(&dbc_message_tpms);
    timer_mid.messages.push_back(&dbc_message_oil);
    timer_mid.messages.push_back(&dbc_message_engine_temp);
    timer_mid.messages.push_back(&dbc_message_gear_automatic);
    timer_mid.messages.push_back(&dbc_message_fuel_level);
    timer_mid.messages.push_back(&dbc_message_handbrake);
    timer_mid.messages.push_back(&dbc_message_steering_wheel_buttons);

    timer_slow.messages.push_back(&dbc_message_drivemode);
    timer_slow.messages.push_back(&dbc_message_blinkers);
    timer_slow.messages.push_back(&dbc_message_backlight);
    timer_slow.messages.push_back(&dbc_message_lights);

    //
    // Counters
    //
    timer_fast.counters.push_back(&counter_4bit);
    timer_mid.counters.push_back(&counter_alive);
    timer_mid.counters.push_back(&counter_acc);

    //
    // Scheduler
    //
    scheduler_f30_cluster.timers.push_back(&timer_fast);
    scheduler_f30_cluster.timers.push_back(&timer_mid);
    scheduler_f30_cluster.timers.push_back(&timer_slow);

    return scheduler_f30_cluster;
}