The F30 gauge cluster uses `SAE J1850 CRC8` as its checksum algorithm.

It also seems to be using a `4-bit counter` that increments every `20ms` and resets to `0` once it hits `14`. This counter is used to modify some of the data sent. The properties that use it are:
- `Ignition status`
- `Speed`
- `RPM`
- `Transmission Gear`
- `Basic Drive Info`
- `Parking brake`
- `Distance Traveled`
- `Drive mode`

Properties that don't use the counter:
- `Fuel`
- `Blinkers`
- `Lights`
- `Backlight Brightness`
- `Alerts`
- `Acc`

In open-source code that interfaces with the cluster it is updated in the following way:
- `Every 20ms` - Speed, RPM
- `Every 100ms` - Ignition, Basic Drive Info, Gear, Fuel, Parking Brake, Distance, Alerts, Acc
- `Every 500ms` - Lights, Blinkers, Backlight Brightness, Drive Mode

### Packets sent by cluster
As a side note the gauge cluster seems to be called `KOMBI` in the `dbc` files.

The gauge cluster seems to also be sending some packets:


#### `0x1B3`
```
0x1B3    8    FC F6 01 F6 56 FF 82 79
0x1B3    8    FC F6 01 F5 56 FF 82 78
0x1B3    8    FC F6 01 F4 56 FF 82 77
0x1B3    8    FC F6 01 F3 56 FF 82 76
0x1B3    8    FC F6 01 F2 56 FF 82 75
0x1B3    8    FC F6 01 F1 56 FF 82 74
0x1B3    8    FC F6 01 F0 56 FF 82 73
0x1B3    8    FC F6 01 FE 56 FF 82 81
0x1B3    8    FC F6 01 FD 56 FF 82 80
0x1B3    8    FC F6 01 FC 56 FF 82 7F
0x1B3    8    FC F6 01 FB 56 FF 82 7E
0x1B3    8    FC F6 01 FA 56 FF 82 7D
0x1B3    8    FC 61 00 F9 56 FF 82 E5
0x1B3    8    FC 61 00 F8 56 FF 82 E4
0x1B3    8    FC 61 00 F7 56 FF 82 E3
0x1B3    8    FC 61 00 F6 56 FF 82 E2
0x1B3    8    FC 61 00 F5 56 FF 82 E1
0x1B3    8    FC 61 00 F4 56 FF 82 E0
0x1B3    8    FC 61 00 F3 56 FF 82 DF
0x1B3    8    FC 61 00 F2 56 FF 82 DE
0x1B3    8    FC 61 00 F1 56 FF 82 DD
0x1B3    8    FC 61 00 F0 56 FF 82 DC
0x1B3    8    FC 61 00 FE 56 FF 82 EA
0x1B3    8    FC 61 00 FD 56 FF 82 E9
0x1B3    8    FC 61 00 FC 56 FF 82 E8
0x1B3    8    FC 61 00 FB 56 FF 82 E7
0x1B3    8    FC 61 00 FA 56 FF 82 E6
0x1B3    8    FC 61 00 F9 56 FF 82 E5
0x1B3    8    FC 61 00 F8 56 FF 82 E4
0x1B3    8    FC 61 00 F7 56 FF 82 E3
0x1B3    8    FC 61 00 F6 56 FF 82 E2
0x1B3    8    FC 61 00 F5 56 FF 82 E1
0x1B3    8    FC 61 00 F4 56 FF 82 E0
0x1B3    8    FC 61 00 F3 56 FF 82 DF
0x1B3    8    FC 61 00 F2 56 FF 82 DE
0x1B3    8    FC 61 00 F1 56 FF 82 DD
0x1B3    8    FC 61 00 F0 56 FF 82 DC
0x1B3    8    FC 61 00 FE 56 FF 82 EA

```

#### `0x2C5`
The default payload bytes are:
`xx xx 00 10 3F 70 DC`

```
0x2C5    7    47 F2 00 10 3F 70 DC
0x2C5    7    48 FE 00 10 3F 70 DC
0x2C5    7    8D F1 00 10 3F 70 DC
0x2C5    7    CB F0 00 10 3F 70 DC
0x2C5    7    82 FD 00 10 3F 70 DC
0x2C5    7    C4 FC 00 10 3F 70 DC
0x2C5    7    0B FB 00 10 3F 70 DC
0x2C5    7    4D FA 00 10 3F 70 DC
0x2C5    7    87 F9 00 10 3F 70 DC
0x2C5    7    C1 F8 00 10 3F 70 DC
0x2C5    7    04 F7 00 10 3F 70 DC
0x2C5    7    42 F6 00 10 3F 70 DC
0x2C5    7    88 F5 00 10 3F 70 DC
0x2C5    7    CE F4 00 10 3F 70 DC
0x2C5    7    01 F3 00 10 3F 70 DC
0x2C5    7    47 F2 00 10 3F 70 DC
0x2C5    7    8D F1 00 10 3F 70 DC
0x2C5    7    CB F0 00 10 3F 70 DC
0x2C5    7    48 FE 00 10 3F 70 DC
0x2C5    7    82 FD 00 10 3F 70 DC
0x2C5    7    C4 FC 00 10 3F 70 DC
0x2C5    7    0B FB 00 10 3F 70 DC
0x2C5    7    4D FA 00 10 3F 70 DC
0x2C5    7    87 F9 00 10 3F 70 DC
0x2C5    7    C1 F8 00 10 3F 70 DC
0x2C5    7    04 F7 00 10 3F 70 DC
0x2C5    7    42 F6 00 10 3F 70 DC
0x2C5    7    88 F5 00 10 3F 70 DC
0x2C5    7    CE F4 00 10 3F 70 DC
0x2C5    7    01 F3 00 10 3F 70 DC
0x2C5    7    47 F2 00 10 3F 70 DC
0x2C5    7    8D F1 00 10 3F 70 DC
0x2C5    7    CB F0 00 10 3F 70 DC
0x2C5    7    48 FE 00 10 3F 70 DC
0x2C5    7    82 FD 00 10 3F 70 DC
0x2C5    7    C4 FC 00 10 3F 70 DC
0x2C5    7    0B FB 00 10 3F 70 DC
0x2C5    7    4D FA 00 10 3F 70 DC
0x2C5	 7	  87 F9 00 10 3F 70 DC

```

#### `0x2CA` Outside temperature
According to [this repo](https://github.com/kmalinich/node-bmw-ref) this is the **outside temperature**.

```
0x2CA    2    FF FF
0x2CA    2    FF FF
0x2CA    2    FF FF
0x2CA    2    FF FF
```

#### `0x2F7`
```
0x2F7    6    12 58 49 09 11 F2
0x2F7    6    12 58 49 09 11 F2
0x2F7    6    12 58 49 09 11 F2
0x2F7    6    12 58 49 09 11 F2
```

#### `0x328`
Relative time ???
```
0x328    6    2F AA 00 00 3C 25
0x328    6    2E AA 00 00 3C 25
0x328    6    2D AA 00 00 3C 25
0x328    6    2C AA 00 00 3C 25
```

#### `0x393`
```
0x393    4    3F 32 25 FD
```

#### `0x560`
```
0x560    8    00 00 00 00 55 55 00 60
0x560    8    00 00 00 00 55 55 00 60
0x560    8    00 00 00 00 55 55 00 60
0x560    8    00 00 00 00 55 55 00 60
0x560    8    00 00 00 00 55 55 00 60
0x560    8    00 00 00 00 55 55 00 60
```

#### `0x5E0`
```
0x5E0    8    11 03 00 02 FF FF 00 00
0x5E0    8    11 03 00 01 FF FF 00 00
0x5E0    8    8C 01 FF FF FF FF FF FF
0x5E0    8    8C 00 02 FF 00 00 00 FF
```