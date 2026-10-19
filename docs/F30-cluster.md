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

In open-source code I was able to find that interfaces with the cluster it is updated in the following way:
- `Every 20ms` - Speed, RPM
- `Every 100ms` - Ignition, Basic Drive Info, Gear, Fuel, Parking Brake, Distance, Alerts, Acc
- `Every 500ms` - Lights, Blinkers, Backlight Brightness, Drive Mode