# ESP32 Automotive CAN & BMW F30 Cluster Controller

*Engineering report for the **IoT Devices** university course, by [Michał Gagoś and Kamil Kostka](#authors).*

## Abstract

Modern vehicles exchange state between electronic control units (ECUs) over the Controller Area Network (**CAN**). This project presents a low-cost, Wi-Fi-connected CAN interface built from an **ESP32-S3** microcontroller and an **MCP2515** CAN controller. It has two roles:

- a **passive traffic scanner**;
- a **periodic ECU simulator**. Its frames are specified declaratively, following the message/signal model of the industry-standard `.dbc` format, and include rolling counters and SAE J1850 CRC-8 checksums.

A self-hosted web dashboard and a REST/WebSocket API change any signal in real time. The system is demonstrated on a **BMW F30 (3 Series) instrument cluster** on a laboratory bench. There, 21 simulated messages drive the gauges, indicators and warning functions without the rest of the vehicle being present.

## Objectives

- Describe an automotive device declaratively, as timers, messages and signals, instead of hard-coded byte arrays.
- Let the firmware handle transmission timing, bit packing, rolling counters and checksums.
- Monitor and control the bus remotely and in real time over Wi-Fi, from any browser.
- Demonstrate the approach on real automotive hardware.

## Key capabilities

| Capability | Description | Implementation |
| --- | --- | --- |
| **CAN traffic scanner** | Receives every frame on the bus, with no acceptance filters. Keeps the last 100 frames and streams them to the browser and the REST API. | `CANManager` (`include/can.hpp`) |
| **DBC-based ECU simulator** | Sends 21 BMW F30 messages on three timers (20, 100 and 500 ms) with DBC-style signals, rolling counters and SAE J1850 CRC-8 checksums. | `Scheduler`, `Timer`, `DBCMessage`, `DBCSignal` |
| **Web dashboard** | A self-contained single-page app served from flash. Live control over a WebSocket, with HTTP fallback. | `include/http/html.hpp` |
| **REST + WebSocket API** | Reads received frames and the scheduler, and sets any signal by name. | `src/http/api.cpp` |
| **Networking** | Joins Wi-Fi as `http://f30.local/` via mDNS, or opens a fallback access point. | `src/main.cpp` |
| **Robust boot** | The web UI starts even without the MCP2515; CAN initialisation is retried every 5 s. | `src/main.cpp` |

On the F30 cluster the dashboard controls:

- the speedometer and rev counter;
- ignition, drive modes and the gear display;
- lights, turn indicators and illumination;
- check-control alerts and the parking brake;
- the fuel and temperature gauges and the steering wheel buttons.

The firmware also keeps the ABS, power-steering, airbag, seatbelt and TPMS (tyre pressure) warnings off. The full message and signal mapping is on the [BMW F30 CAN IDs](F30-can-ids.md) page.

## Live demo

![BMW F30 instrument cluster driven by the ESP32 on the bench](images/f30-cluster-working.gif)

## Hardware summary

| Qty | Component | Role |
| --- | --- | --- |
| 1 | Seeed Studio XIAO ESP32-S3 | Wi-Fi, web server, scheduler, SPI host |
| 1 | MCP2515 CAN module with TJA1050 transceiver | SPI-to-CAN controller and bus driver |
| 1 | BMW F30 instrument cluster | Device under test |
| 1 | 12 V bench power supply | Powers the cluster |
| 1 | LM2596 step-down converter, set to 5 V | Powers the XIAO and the CAN module |
| – | Connector harness, jumper wires, USB-C cable | Interconnect and programming |

The wiring schematic, pinouts and power-up procedure are in [Setup & Usage](setup.md#1-hardware-wiring).

## Repository structure

```text
esp32-automotive-can/
├── platformio.ini              # Board, build flags, library dependencies
├── mkdocs.yml                  # This documentation site
├── docs/                       # Documentation sources and images
├── include/
│   ├── can.hpp                 # CANFrame, CANManager (RX history), SPI_CS_PIN
│   ├── counter.hpp             # Counter (rolling counters)
│   ├── crc/crc.hpp             # getCrc8() – SAE J1850 CRC-8
│   ├── dbc/dbc_message.hpp     # DBCMessage – bit packing, counter, CRC, send
│   ├── dbc/dbc_signal.hpp      # DBCSignal
│   ├── timer.hpp               # Timer – periodic group of messages
│   ├── scheduler.hpp           # Scheduler – set of timers, signal lookup
│   ├── devices.hpp             # Device profile factory prototypes
│   ├── http/api.hpp            # init_api(), update_api()
│   ├── http/html.hpp           # Web dashboard (PROGMEM string)
│   ├── logger.hpp              # Logger
│   ├── obd2.hpp                # OBD-II PID constants
│   └── secrets.example.hpp     # Template for Wi-Fi credentials and CS pin
└── src/
    ├── main.cpp                # setup()/loop(): Wi-Fi, mDNS, CAN init, API, scheduler
    ├── scheduler.cpp
    ├── timer.cpp
    ├── http/api.cpp            # REST endpoints and WebSocket protocol
    └── devices/
        ├── f30_cluster.cpp     # BMW F30 cluster profile
        └── a4b7_radio.cpp      # Audi A4 B7 radio profile (stub)
```

## Authors

| Author | GitHub | LinkedIn |
| --- | --- | --- |
| **Michał Gagoś** | [:fontawesome-brands-github: lizard123137](https://github.com/lizard123137) | [:fontawesome-brands-linkedin: michal-gagos](https://www.linkedin.com/in/michal-gagos/) |
| **Kamil Kostka** | [:fontawesome-brands-github: KaloeSan](https://github.com/KaloeSan) | [:fontawesome-brands-linkedin: kamilkostka](https://www.linkedin.com/in/kamilkostka/) |

## Credits

- [CarCluster by r00li](https://github.com/r00li/CarCluster) helped us greatly with figuring out how to communicate with the cluster.
- The [openinverter.org wiki](https://openinverter.org/wiki/BMW_F-Series_Gear_Lever) documents the F-series gear selector (GWS) messages.
- [node-bmw-ref by kmalinich](https://github.com/kmalinich/node-bmw-ref) is a reference for BMW CAN IDs.

The project is released under the GNU General Public License v3.0.
