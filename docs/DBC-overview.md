# DBC Files

A `DBC` file contains the information necessary for decoding a CAN frame of a given ID.

A `DBC` file is composed of `messages` and `signals`.

## Messages

`BO_` is used to define a message:

```text
BO_ <Message_ID> <Message_Name>: <Message_Len> <Sender_Name>
BO_ 256 EngineData: 8 EngineECU
```

## Signals

`SG_` is used to define a signal:

```text
SG_ <Name> : <Start_Bit>|<Bit_Len>@<Sign><Endian> (<Scale>,<Offset>) [<Range_From>|<Range_To>] "<Unit>" <Receiver_Name>
SG_ EngineSpeed : 16|16@1+ (0.25,0) [0|16383.75] "rpm" EngineDisplay
```

In the example, `EngineSpeed` starts at bit 16 and is 16 bits long. `@1` means little-endian (Intel) byte order and `+` means unsigned. Its physical value is `raw × 0.25 + 0`, in rpm.

`Signals` define each field that is sent as a part of a `Message`.

A rolling counter is a signal (a part of the frame).

## How this project maps DBC concepts

The firmware does not parse `.dbc` files. Device profiles describe the same information directly in C++:

| DBC concept | Firmware equivalent |
| --- | --- |
| `BO_` message ID, name, length | `DBCMessage(name, id, dlc)` |
| `SG_` signal | `DBCSignal` in `DBCMessage::signals` |
| Start bit and bit length | `start_bit`, `bit_length` |
| `@1+` little-endian, unsigned | The only encoding implemented |
| `(<Scale>,<Offset>)` | `factor`, `offset`: `raw = (value − offset) / factor` |
| Range, unit, receiver | Not modelled |
| Rolling counter signal | `Counter` attached to `DBCMessage::counter` at `counter_start_bit` |
| Checksum signal | `DBCMessage::crc_func` + `final_xor`; the CRC is prepended as byte 0 |
| Message cycle time | The `Timer` the message is added to |

Firmware bit positions refer to the payload *before* the CRC is prepended (see [Frame encoding](architecture.md#frame-encoding)). They are therefore 8 lower than in a DBC description of the frame on the bus. For example, the speed message `0x1A1` would be described in a DBC file roughly like this (illustrative; DBC names cannot contain `-`):

```text
BO_ 417 Speed: 5 ESP32
 SG_ Speed_CRC : 0|8@1+ (1,0) [0|255] "" KOMBI
 SG_ Speed_Counter : 8|4@1+ (1,0) [0|14] "" KOMBI
 SG_ Speed_Value : 16|24@1+ (1,0) [0|16777215] "" KOMBI
```

The C++ classes are documented in the [API reference](api.md#c-firmware-api), and every F30 message defined this way is listed in [BMW F30 CAN IDs](F30-can-ids.md).
