# BMW F30 CAN IDs

Automotive CAN buses are described by `.dbc` files. Each ID and its payload is described within.

This page lists every message and signal the firmware sends. The checksum and counter rules behind the CRC and Counter columns are on the [BMW F30 instrument cluster](F30-cluster.md) page.

## Implemented messages

All 21 messages the firmware sends are listed below.

Payload is `DBCMessage::dlc`. For messages with a CRC, the frame on the bus is one byte longer and payload byte *n* is bus byte *n + 1* (see [Frame encoding](architecture.md#frame-encoding)). The ABS, power steering, airbag, seatbelt and TPMS messages report "OK", so the cluster keeps those warning lights off.

| ID | Name in code | Payload | Bus DLC | CRC `final_xor` | Counter | Period | Default payload |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `0x1A1` | Speed | 4 | 5 | `0xA9` | `counter_4bit` | 20 ms | `C0 00 00 81` |
| `0x0F3` | RPM | 7 | 8 | `0x7A` | `counter_4bit` | 20 ms | `60 00 C0 F0 00 FF FF` |
| `0x12F` | Ignition Status | 7 | 8 | `0x44` | `counter_4bit` | 100 ms | `80 08 DD F1 01 30 06` |
| `0x36E` | ABS Status | 4 | 5 | `0xD8` | `counter_4bit` | 100 ms | `F0 FE FF 14` |
| `0x2A7` | Power Steering Status | 4 | 5 | `0x9E` | `counter_4bit` | 100 ms | `F0 FE FF 14` |
| `0x19B` | Airbag Status | 7 | 8 | `0xFF` | `counter_4bit` | 100 ms | `40 40 55 FD FF FF FF` |
| `0x297` | Seatbelt Status | 6 | 7 | `0x28` | `counter_4bit` | 100 ms | `E0 F1 F0 F2 F2 FE` |
| `0x5C0` | Alerts | 8 | 8 | none | none | 100 ms | `40 00 00 00 FF FF FF FF` |
| `0x0D7` | Alive Counter | 2 | 2 | none | `counter_alive` | 100 ms | `00 FF` |
| `0x33B` | ACC Counter | 5 | 6 | `0x6B` | `counter_acc` | 100 ms | `F0 5C 70 00 00` |
| `0x369` | TPMS Status | 4 | 5 | `0xC5` | `counter_4bit` | 100 ms | `F0 A2 A0 A0` |
| `0x3F9` | Oil Status | 7 | 8 | `0xF1` | `counter_4bit` | 100 ms | `10 82 4E 7E 00 05 89` |
| `0x2C4` | Engine Temp | 7 | 8 | `0xB2` | none | 100 ms | `3E 00 64 64 64 01 F1` |
| `0x3FD` | Gear Automatic | 4 | 5 | `0xD6` | `counter_4bit` | 100 ms | `00 00 FC FF` |
| `0x349` | Fuel Level | 5 | 5 | none | none | 100 ms | `00 00 00 00 00` |
| `0x36F` | Handbrake Status | 4 | 5 | `0x17` | `counter_4bit` | 100 ms | `F0 38 00 00` |
| `0x1EE` | Steering Wheel Buttons | 2 | 2 | none | none | 100 ms | `00 00` |
| `0x3A7` | Drive Mode | 6 | 7 | `0x4A` | `counter_4bit` | 500 ms | `F0 00 00 00 11 C0` |
| `0x1F6` | Blinkers | 2 | 2 | none | none | 500 ms | `81 F0` |
| `0x202` | Backlight Brightness | 2 | 2 | none | none | 500 ms | `FF FF` |
| `0x21A` | Lights | 3 | 3 | none | none | 500 ms | `00 C0 F7` |

### Not implemented yet

| ID | Function |
| --- | --- |
| `0x289` | Cruise control |
| unknown | TODO distance traveled |
| `0x197` | Gear selector status, see [GWS](#gear-selector-gws) |

## Signal reference

These are all 21 signals and the values the cluster understands. Every signal uses `factor = 1`. Values marked "dashboard convention" are put together by the web dashboard; the firmware just sends whatever value it gets.

| Signal | Message | Start bit / length | Default | Encoding |
| --- | --- | --- | --- | --- |
| `speed-value` | `0x1A1` | 8 / 24 | `0x810000` | Bits 0-15: speed * 64.01 (raw per km/h). Bits 16-23: `0x81` standing still, `0x91` moving (dashboard convention). |
| `rpm-value` | `0x0F3` | 8 / 8 | `0x00` | `0x00` to `0x2B` moves the rev counter from 0 to 6900 rpm |
| `rpm-calculated-gear` | `0x0F3` | 32 / 8 | `0x00` | 0 for P/D, 1 for N, 2 for R, manual gear + 4 in M/S (dashboard convention) |
| `ignition-state` | `0x12F` | 8 / 8 | `0x08` | `0x08` off, `0x8A` on |
| `drive-mode-state` | `0x3A7` | 24 / 8 | `0` | 1 Traction, 2 Comfort, 4 Sport, 5 Sport+, 6 DSC off, 7 ECO Pro |
| `gear-automatic` | `0x3FD` | 8 / 8 | `0x20` | `0x01` to `0x09` gears 1-9, `0x20` P, `0x40` R, `0x60` N, `0x80` D, `0x81` DS |
| `backlight-brightness` | `0x202` | 0 / 8 | `253` | Illumination level. The dashboard offers 0-253. |
| `high-beam` | `0x21A` | 1 / 1 | `0` | 1 = on |
| `main-lights` | `0x21A` | 2 / 1 | `0` | 1 = on |
| `front-fog` | `0x21A` | 5 / 1 | `0` | 1 = on |
| `rear-fog` | `0x21A` | 6 / 1 | `0` | 1 = on |
| `blinker-left` | `0x1F6` | 4 / 1 | `0` | 1 = on (bit value `0x10`) |
| `blinker-right` | `0x1F6` | 5 / 1 | `0` | 1 = on (bit value `0x20`). Both on = hazard `0x30`. |
| `alert-type` | `0x5C0` | 8 / 8 | `0x22` | Check-control message ID, see [Alerts](#alerts) |
| `alert-value` | `0x5C0` | 24 / 8 | `0x28` | `0x29` active, `0x28` inactive |
| `handbrake` | `0x36F` | 24 / 8 | `0x14` | `0x15` engaged, `0x14` not engaged |
| `fuel-level-1` | `0x349` | 0 / 8 | `0x18` | Fuel scale, see [Fuel level](#fuel-level) |
| `fuel-level-2` | `0x349` | 24 / 8 | `0x18` | Same value as `fuel-level-1` |
| `oil-temp` | `0x3F9` | 32 / 8 | `0x64` | Value range `0x64` to `0xC8`. Offset `0x32`, so raw = value - 50. |
| `engine-temp` | `0x2C4` | 8 / 8 | `0x64` | Value range `0x64` to `0xC8`. Offset `0x32`, so raw = value - 50. |
| `steering-wheel-buttons` | `0x1EE` | 0 / 8 | `0x00` | Byte 0 is the button signal; `0x00` = no buttons pressed. `0x4C` does something we haven't identified yet. |

### Alerts

Alert payload structure (`0x5C0`, no CRC):

| Byte | Content |
| --- | --- |
| 0 | `0x40` |
| 1 | Alert type (`alert-type`) |
| 2 | `0x00` |
| 3 | Alert value (`alert-value`): `0x29` true, `0x28` false |
| 4-7 | `0xFF` |

The firmware carries one alert type at a time. The dashboard remembers the ON/OFF state of each type and offers the types below. Any other type byte can be typed in by hand.

| Type | Alert | Type | Alert |
| --- | --- | --- | --- |
| `0x0E` | Door open FR | `0x3E` | Speeding - red |
| `0x0F` | Door open FL | `0x47` | Parking brake - red |
| `0x10` | Door open RL | `0x4D` | Seatbelt |
| `0x11` | Door open RR | `0x4E` | Speeding - yellow |
| `0x12` | Hood open | `0x58` | Driving light FL |
| `0x13` | Trunk open | `0x59` | Driving light FR |
| `0x18` | Parking brake - yellow | `0x67` | Gearbox overheat - yellow |
| `0x1B` | Low oil - white | `0x68` | Gearbox overheat - red |
| `0x1C` | Low oil - orange | `0x69` | Clutch overheat - red |
| `0x20` | Fuel cap open | `0x74` | Turn signal RL |
| `0x22` | Check engine | `0x77` | Turn signal FR |
| `0x23` | DSC 1 | `0x7A` | Turn signal FL |
| `0x24` | DSC OFF | `0x7D` | Turn signal RR |
| `0x27` | Engine overheat - red | `0xD7` | DSC 2 |

### Fuel level

Fuel level is weird. On some cars the values are apparently in liters, but that doesn't work for our cluster. Our theory is that it comes from a USA import car, so the values might be in gallons.

| Fuel | Raw value |
| --- | --- |
| 0 % (`out0`) | 37 (`0x25`) |
| 50 % (`out50`) | 18 (`0x12`) |
| 100 % (`out100`) | 4 (`0x04`) |

Below 50 % the percentage is mapped linearly between `out0` and `out50`, and above 50 % between `out50` and `out100`. The dashboard does this mapping and writes the same raw value into both fuel signals.

!!! note "Byte layout"
    The comment in `f30_cluster.cpp` describes the payload as byte 0: high byte (unused), byte 1: low byte, byte 2: high byte (unused), byte 3: low byte. In the code, `fuel-level-1` actually writes payload byte 0 (start bit 0) and `fuel-level-2` writes byte 3 (start bit 24).

## Gear selector (GWS)

In the F30 there is a module called `Getriebe-Wahl-Schalter (GWS)` that is responsible for shifting gears. The module communication is described [here](https://openinverter.org/wiki/BMW_F-Series_Gear_Lever). According to openinverter.org, the GWS sends two messages.

### Status message `0x197`

The firmware does not send this message.

| Byte | Content |
| --- | --- |
| 0 | CRC |
| 1 | 4-bit counter |
| 2 | Lever position |
| 3 | Park button: `0xD5` pressed, `0xC0` released |

| Lever position | Value |
| --- | --- |
| Up 2 | `0x2E` |
| Up 1 | `0x1E` |
| Centre middle | `0x0E` |
| Down 1 | `0x3E` |
| Down 2 | `0x4E` |
| Side up | `0x5E` |
| Centre side | `0x7E` |
| Side down | `0x6E` |

### Appearance message `0x3FD`

The firmware sends this message as *Gear Automatic*.

| Byte | Content |
| --- | --- |
| 0 | CRC |
| 1 | 4-bit counter |
| 2 | Gear, mapped as in `gear-automatic` above |
| 3 | Ignored |
| 4 | Ignored |

## Open questions

- It's also apparently required for steady RPM to send the oil state? The firmware always sends `0x3F9`.
- We noted `0x0D7` as "Alive count???" and still haven't fully confirmed what it is for. The firmware sends it with an 8-bit counter in byte 0.
