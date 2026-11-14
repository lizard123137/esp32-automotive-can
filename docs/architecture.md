# System architecture

The firmware is built around a small model borrowed from `.dbc` files:

- A `Scheduler` owns several `Timer` objects.
- Each timer periodically sends a list of `DBCMessage` objects.
- Each message is a default payload plus bit-packed `DBCSignal` fields, optionally with a rolling `Counter` and a CRC-8 checksum.

Clients only ever change signal values. The scheduler keeps re-encoding and re-sending the frames by itself.

## Technology stack

| Layer | Technology |
| --- | --- |
| Build system | PlatformIO, `espressif32` platform, Arduino framework, C++17 (`-std=gnu++17`) |
| CAN driver | `coryjfowler/mcp_can` |
| Web server | `esp32async/ESPAsyncWebServer` + `esp32async/AsyncTCP` (regex routes enabled with `-DASYNCWEBSERVER_REGEX`) |
| JSON | `bblanchon/ArduinoJson` v7 |
| Front end | Plain HTML, CSS and JavaScript embedded in flash (`PROGMEM`), with no CDN or external assets |
| Documentation | MkDocs with the Material theme |

## System block diagram

```mermaid
flowchart TB
    subgraph CLIENT["Web client"]
        UI["Dashboard SPA in the browser<br/>(served from html.hpp)"]
    end

    subgraph FW["XIAO ESP32-S3 firmware"]
        WEB["Wi-Fi STA or fallback AP · mDNS f30.local<br/>AsyncWebServer :80 · AsyncWebSocket /ws"]
        API["http/api.cpp<br/>REST routes · WebSocket protocol · update_api()"]
        SCH["Scheduler<br/>3 Timers · 3 Counters"]
        DBC["DBCMessage / DBCSignal<br/>bit packing + rolling counter"]
        CRC["getCrc8()<br/>SAE J1850 CRC-8, per-ID final XOR"]
        DRV["mcp_can driver"]
        RXM["CANManager<br/>RX history, 100 frames"]
    end

    subgraph HW["Hardware"]
        MCP["MCP2515 CAN controller<br/>+ TJA1050 transceiver"]
        KOMBI["BMW F30 instrument cluster"]
        PWR["12 V bench supply<br/>+ LM2596 buck converter (5 V)"]
    end

    UI <-->|"HTTP + WebSocket, JSON"| WEB
    WEB <--> API
    API -->|"updateSignalValue()"| SCH
    SCH -->|"Timer::run()"| DBC
    DBC --> CRC
    CRC -->|"sendMsgBuf()"| DRV
    DRV <-->|"SPI"| MCP
    MCP <-->|"CAN_H / CAN_L, 500 kbit/s"| KOMBI
    DRV -->|"readMsgBuf()"| RXM
    RXM -->|"snapshot every 250 ms"| API
    PWR -->|"12 V"| KOMBI
    PWR -->|"5 V"| MCP
```

The buck converter also powers the XIAO with 5 V (see [Power](setup.md#power)).

## Data paths

### Transmit path (browser to cluster)

1. A WebSocket `set` message or a `PATCH /api/signals/{name}` request (see the [API reference](api.md#websocket-protocol)) reaches its handler in `src/http/api.cpp`, which runs on the AsyncTCP task. The handler calls `Scheduler::updateSignalValue(name, value)` for every entry and immediately broadcasts a new `state` message.
2. `updateSignalValue()` goes through the timers, their messages and their signals, and overwrites the `value` of the first signal with that name. Nothing is sent yet.
3. `loop()` calls `Scheduler::step()`, which calls `Timer::run()` on each timer. When a timer's interval has passed, it calls `DBCMessage::sendMessage()` on each of its messages and then increments its counters.
4. `DBCMessage::getBytes()` starts from `default_payload` and writes each signal's raw value, then the rolling counter (see [Frame encoding](#frame-encoding)).
5. If `crc_func` is set, `sendMessage()` computes `getCrc8(payload, dlc, final_xor)`, shifts the payload right by one byte and puts the CRC in byte 0, so the frame is `dlc + 1` bytes long.
6. `CAN.sendMsgBuf(id, 0, len, bytes)` from `mcp_can` loads one of the MCP2515's transmit buffers over SPI. The MCP2515 handles arbitration and sends the standard 11-bit frame at 500 kbit/s through the TJA1050 transceiver.

### Receive path (bus to browser)

1. The MCP2515 receives every frame on the bus (`MCP_ANY` turns off acceptance filtering).
2. `CANManager::readFrame()` runs once per `loop()` iteration. It polls `CAN.checkReceive()`, reads one frame with `CAN.readMsgBuf()` and appends it to `can_manager.history`. Once the history holds 100 frames, the oldest one is dropped first.
3. Every 250 ms, `update_api()` compares `history` with `frames_snapshot`. If they differ, it copies `history` into the snapshot while holding `frames_mutex`.
4. The snapshot is what the WebSocket `frames` message and `GET /api/read` return.

!!! note "A controller never receives its own frames"
    The MCP2515 does not receive the frames it sends, so the dashboard's **Received** table only shows traffic from other nodes. On our bench, those are the [frames the cluster sends](F30-cluster.md#packets-sent-by-the-cluster).

## Frame encoding

`DBCMessage::getBytes()` implements the encoding half of a `.dbc` signal definition:

```text
raw = (uint64_t)((value - offset) / factor)        // truncated, unsigned
bit i of raw  ->  payload bit (start_bit + i)      // i = 0 .. bit_length-1
payload bit n ->  byte n / 8, bit n % 8            // little-endian (Intel) bit order
```

- Bits that fall beyond `dlc` are dropped, and so are signals with `bit_length` 0 or above 64.
- The counter is written the same way at `counter_start_bit`. That is 0 for every F30 message, so the counter sits in the low bits of payload byte 0.
- The CRC is computed over the first `dlc` payload bytes after the counter has been written.
- `sendMessage()` then puts the CRC in front of the payload. All bit positions in the firmware therefore refer to the payload: for a message with a CRC, payload byte *n* is byte *n + 1* on the bus.

### Worked example: speed `0x1A1`

The dashboard encodes 50 km/h as `speed-value = (0x91 << 16) | trunc(50 * 64.01) = 0x910C80` (9505920). The [signal reference](F30-can-ids.md#signal-reference) explains this encoding.

| Step | Bytes (hex) |
| --- | --- |
| `default_payload` (dlc 4) | `C0 00 00 81` |
| after `speed-value` (start bit 8, 24 bits) | `C0 80 0C 91` |
| after counter (4 bits at bit 0, value 0) | `C0 80 0C 91` |
| CRC = `getCrc8(payload, 4, 0xA9)` | `EF` |
| **Frame on the bus** (DLC 5) | **`EF C0 80 0C 91`** |

On the next cycle the counter is 1, so byte 1 becomes `C1` and the CRC changes with it.

## Scheduling and timing

`src/devices/f30_cluster.cpp` builds one `Scheduler` with three timers:

| Timer | Interval | Messages | Counters it increments |
| --- | --- | --- | --- |
| `timer_fast` | 20 ms | 2 | `counter_4bit` |
| `timer_mid` | 100 ms | 15 | `counter_alive`, `counter_acc` |
| `timer_slow` | 500 ms | 4 | none |

Which timer sends which message is shown in the *Period* column of the [implemented messages](F30-can-ids.md#implemented-messages) table. The counters themselves are defined under [Rolling counter](F30-cluster.md#rolling-counter).

A counter only advances when the timer it is registered with (`Timer::counters`) fires. Messages on other timers just read its current value. For example, `counter_4bit` advances every 20 ms, while ignition (100 ms) and drive mode (500 ms) send whatever value it has at that moment.

`Timer::run()` fires when `millis() > last_timestamp + interval`. The real period is therefore `interval + 1` ms, plus however long `loop()` takes to come back to the timer. `loop()` never blocks: CAN reads are non-blocking and the web server runs in its own task.

In total the firmware sends about 250 frames/s. With bit stuffing that is roughly 25-30 kbit/s, or about 5-6 % of the 500 kbit/s bus.

## Concurrency and synchronization

Two FreeRTOS tasks share state:

- The Arduino `loopTask` (core 1 by default) runs `loop()`: CAN reception, the scheduler (all transmissions) and `update_api()` (periodic publishing).
- The AsyncTCP task (`async_tcp`) runs every HTTP request handler and WebSocket event callback registered in `init_api()`.

```mermaid
flowchart TB
    LOOP["Arduino loopTask<br/>loop():<br/>CANManager::readFrame()<br/>Scheduler::step()<br/>update_api()"]
    TCP["AsyncTCP task<br/>HTTP handlers<br/>WebSocket callbacks"]

    HIST[("can_manager.history<br/>loop task only")]
    SNAP[("frames_snapshot<br/>std::mutex frames_mutex")]
    FLAG[("can_online<br/>std::atomic bool")]
    SIG[("DBCSignal::value<br/>plain double, no lock")]
    WSQ[("WebSocket send queues<br/>locked by the library")]

    LOOP <-->|"append / compare"| HIST
    LOOP -->|"copy every 250 ms"| SNAP
    SNAP -->|"copy under lock"| TCP
    LOOP -->|"store"| FLAG
    FLAG -->|"load"| TCP
    TCP -->|"updateSignalValue()"| SIG
    SIG -->|"read in getBytes()"| LOOP
    LOOP -->|"textAll()"| WSQ
    TCP -->|"text() / textAll()"| WSQ
```

| Shared data | Writer | Readers | Protection |
| --- | --- | --- | --- |
| `can_manager.history` | `loopTask` (`readFrame`) | `loopTask` (`update_api`) | None needed, only the loop task touches it |
| `frames_snapshot` | `loopTask` (`update_api`) | AsyncTCP (`frames_json()`) | `std::mutex frames_mutex`. Readers copy the vector under the lock and serialise it after releasing it. |
| `can_online` | `loopTask` (`update_api`) | AsyncTCP (`state_json()`) | `std::atomic<bool>` |
| `DBCSignal::value` | AsyncTCP (`updateSignalValue`) | `loopTask` (`getBytes`), AsyncTCP (`state_json`) | None (see [Known limitations](#known-limitations)) |
| WebSocket send queues | Both tasks (`textAll`, `text`) | AsyncTCP | Locking inside ESPAsyncWebServer / AsyncTCP |

JSON parsing and serialisation run on the AsyncTCP task, and `loop()` copies the RX history at most four times per second instead of locking it for every received frame. Both choices keep slow work away from the timing-sensitive loop.

## Module responsibilities

| Module | Files | Responsibility |
| --- | --- | --- |
| Application entry | `src/main.cpp` | Defines the global objects (`CAN`, `server`, `ws`, `can_manager`, `current_scheduler`). `setup()` brings up the network ([details](setup.md#network-behaviour-at-boot)), starts the MCP2515 without blocking, starts the API and loads the F30 scheduler. `loop()` reads CAN, steps the scheduler, retries CAN init every 5 s while it is down, and calls `update_api()`. |
| CAN I/O | `include/can.hpp` | Header-only, there is no `can.cpp`. Holds the default `SPI_CS_PIN`, `extern MCP_CAN CAN`, `CANFrame`, and `CANManager` with its 100-frame RX history. |
| DBC model | `include/dbc/dbc_signal.hpp`, `include/dbc/dbc_message.hpp` | `DBCSignal` and `DBCMessage`. `getBytes()` encodes the payload; `sendMessage()` adds the CRC and sends the frame. |
| Checksum | `include/crc/crc.hpp` | 256-entry lookup table and `getCrc8()` (algorithm under [Checksum](F30-cluster.md#checksum)) |
| Scheduling | `include/counter.hpp`, `include/timer.hpp`, `src/timer.cpp`, `include/scheduler.hpp`, `src/scheduler.cpp` | `Counter` is a wrapping counter with a configurable range and step. `Timer` sends its messages and increments its counters. `Scheduler` steps all timers and looks signals up by name. |
| Device profiles | `include/devices.hpp`, `src/devices/f30_cluster.cpp`, `src/devices/a4b7_radio.cpp` | Factory functions that build a configured `Scheduler`: the [BMW F30 cluster](F30-can-ids.md) and the Audi A4 B7 radio (stub) |
| Web API | `include/http/api.hpp`, `src/http/api.cpp` | `init_api()` registers the HTTP routes and `/ws`, and `update_api()` publishes from `loop()` (see the [API reference](api.md)) |
| Web dashboard | `include/http/html.hpp` | The single-page app as one `PROGMEM` string. It turns controls into signal values, runs the WebSocket client with HTTP fallback, and shows the telemetry and CAN monitor. |
| Logging | `include/logger.hpp` | `Logger::print(level, module, fmt, ...)` with `ERROR`, `INFO` and `DEBUG` levels. The active level is `INFO`. |
| OBD-II | `include/obd2.hpp` | Functional request ID and PID constants ([OBD-II](OBD2-overview.md)) |
| Configuration | `include/secrets.hpp` (git-ignored), `include/secrets.example.hpp`, `platformio.ini` | Wi-Fi credentials and the CS pin; board, build flags and libraries |

## Design decisions

- A device is described as data (timers, messages and signals). Adding one means writing a single factory function, and the web API finds its signals through the scheduler without any changes.
- Clients send signal values by name and never raw frames. Counters, CRCs and timing always come from the firmware, so a client cannot send a frame with a bad checksum or break the timing the cluster expects.
- CAN start-up does not block. A missing or badly wired MCP2515 should not stop us from debugging over Wi-Fi, so CAN init is retried in the background and all CAN calls are skipped while `can_ready` is false. Without a responding controller, SPI reads return garbage, such as DLC values up to 15.
- The dashboard is compiled into flash. It works on the fallback access point with no internet and needs no separate filesystem image.
- Every accepted change triggers a `state` broadcast with the re-encoded payloads. The transmit table in every open browser updates within milliseconds instead of waiting for the next 1 s status message.

## Known limitations

- The fuel mapping is empirical and may depend on the cluster's market version (see [Fuel level](F30-can-ids.md#fuel-level)). The temperature gauge does not show values correctly yet.
- OBD-II is not implemented. Only the background notes and the PID constants exist (see [OBD-II](OBD2-overview.md)).
- Only the F30 profile is implemented. The Audi A4 B7 radio profile is an empty stub.
- `loop()` polls the MCP2515. The interrupt pin is not used and no hardware acceptance filters are set.
- `Logger::print` formats messages, but its `Serial.printf` call is commented out, so nothing gets printed.
- Signals can only be packed as unsigned, little-endian (Intel) bit fields. The `is_signed` and `is_little_endian` fields of `DBCSignal` are not used yet.
- Signal values are not synchronised between the two tasks. The AsyncTCP task writes them (plain `double` fields) and the loop task reads them without a lock. The scheduler's structure never changes after `setup()`, so the worst case is one frame carrying a value that is one update old. A torn read of an 8-byte `double` is possible in theory on the 32-bit Xtensa core, but it has no practical effect here. A production version should add a lock or store the raw value in an atomic integer.

## Resource usage

Build of the current `main` for `seeed_xiao_esp32s3`:

| Resource | Used | Available | Share |
| --- | --- | --- | --- |
| RAM (static) | 56,396 B | 327,680 B | 17.2 % |
| Flash (application) | 1,693,346 B | 3,342,336 B | 50.7 % |
