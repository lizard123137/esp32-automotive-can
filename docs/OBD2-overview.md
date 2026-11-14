# OBD2 Overview

!!! note "Scope"
    This page is protocol background only: the current firmware implements the BMW F30 cluster simulation profile rather than active OBD-II polling on `0x7DF`, and `include/obd2.hpp` defines only the constants listed below.

`OBD2` has several services/modes (naming varies):

- `$01` - **Current Data** - Real time data.
- `$02` - **Freeze Frame** - A snapshot of sensor data.
- `$03` - **Show Stored Codes** - A list of active `DTCs`.
- `$04` - **Clear Stored Codes** - Clears `DTCs`.
- `$07` - **Show Pending Codes** - A list of not yet confirmed `DTCs`.
- `$09` - **Request Vehicle Information** - Static Information about vehicle.
- `$0A` - **Show Permanent Codes** - A list of codes that can't be cleared.

When using a mode we specify the exact piece we want to influence by including a `Parameter ID (PID)`.

## Request and response frames

`OBD2` Requests are made with `CAN-ID` of `0x7DF` (`OBD2_ID_FUNCTIONAL` in `include/obd2.hpp`). This is a functional (broadcast) address that every emissions-related ECU listens to. ECUs answer on their own IDs, `0x7E8`–`0x7EF`.

A single-frame request carries the number of data bytes, the mode and the PID:

```text
7DF  8  02 01 0C 00 00 00 00 00     request: 2 bytes follow, mode $01, PID $0C (engine RPM)
7E8  8  04 41 0C 1A F8 00 00 00     response: 4 bytes follow, $41 = $01 + $40, PID $0C, A = 0x1A, B = 0xF8
```

The response mode is the request mode plus `0x40`. Here the engine speed is `(256 × A + B) / 4 = (256 × 26 + 248) / 4 = 1726 rpm`.

## PIDs defined in the firmware

| Constant | PID | Meaning | Formula (mode `$01`) |
| --- | --- | --- | --- |
| `OBD2_PID_SUPPORTED_1_20` | `0x00` | PIDs `$01`–`$20` supported | Bit field |
| `OBD2_PID_CALCULATED_ENGINE_LOAD` | `0x04` | Calculated engine load | `100 / 255 × A` % |
| `OBD2_PID_ENGINE_COOLANT_TEMP` | `0x05` | Engine coolant temperature | `A − 40` °C |
| `OBD2_PID_ENGINE_RPM` | `0x0C` | Engine speed | `(256A + B) / 4` rpm |
| `OBD2_PID_VEHICLE_SPEED` | `0x0D` | Vehicle speed | `A` km/h |
| `OBD2_PID_INTAKE_AIR_TEMP` | `0x0F` | Intake air temperature | `A − 40` °C |
| `OBD2_PID_MAF_AIR_FLOW_RATE` | `0x10` | Mass air flow rate | `(256A + B) / 100` g/s |
| `OBD2_PID_THROTTLE_POSITION` | `0x11` | Throttle position | `100 / 255 × A` % |
| `OBD2_PID_OBD_STANDARD_COMPLIANCE` | `0x1C` | OBD standard the vehicle conforms to | Enumeration |
| `OBD2_PID_RUNTIME_SINCE_ENGINE_START` | `0x1F` | Run time since engine start | `256A + B` s |
| `OBD2_PID_SUPPORTED_21_40` | `0x20` | PIDs `$21`–`$40` supported | Bit field |
| `OBD2_PID_FUEL_LEVEL_INPUT` | `0x2F` | Fuel tank level | `100 / 255 × A` % |
| `OBD2_PID_DIST_TRAVELED_SINCE_DTC_CLEARED` | `0x31` | Distance since codes cleared | `256A + B` km |
| `OBD2_PID_CONTROL_MODULE_VOLTAGE` | `0x42` | Control module voltage | `(256A + B) / 1000` V |
| `OBD2_PID_ENGINE_FUEL_RATE` | `0x5E` | Engine fuel rate | `(256A + B) / 20` L/h |

`A` and `B` are the first and second data bytes after the PID in the response.

`include/obd2.hpp` also declares `const char* obd2_pid_to_str(uint8_t pid);`, which has no implementation yet.

!!! tip "Bit rate"
    Most vehicles from 2008 onwards use CAN at 500 kbit/s on the OBD port, the rate `init_can()` configures. Some older vehicles use 250 kbit/s; see [Crystal and bit rate](setup.md#crystal-and-bit-rate).
