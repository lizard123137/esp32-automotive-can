# OBD2 Overview

`OBD2` has several services/modes (naming varies):
- `$01` - **Current Data** - Real time data.
- `$02` - **Freeze Frame** - A snapshot of sensor data.
- `$03` - **Show Stored Codes** - A list of active `DTCs`.
- `$04` - **Clear Stored Codes** - Clears `DTCs`.
- `$07` - **Show Pending Codes** - A list of not yet confirmed `DTCs`.
- `$09` - **Request Vehicle Information** - Static Information about vehicle.
- `$0A` - **Show Permanent Codes** - A list of codes that can't be cleared.

When using a mode we specify the exact piece we want to influence by including a `Parameter ID (PID)`.

`OBD2` Requests are made with `CAN-ID` of `0x7DF`.