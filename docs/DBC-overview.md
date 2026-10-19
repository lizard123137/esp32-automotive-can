A `DBC` file contains the information necessary for decoding a CAN frame of a given ID.

A `DBC` file is composed of `messages` and `signals`.

### Messages
`BO_` is used to define a message:
``` dbc
BO_ <Message_ID> <Message_Name>: <Message_Len> <Sender_Name>
BO_ 256 EngineData: 8 EngineECU
```

### Signals
`SG_` is used to define a signal:
``` dbc
SG_ <Name> : <Start_Bit>|<Bit_Len>@<Sign><Endian> (<Scale>|<Offset>) [<Range_From>|<Range_To>] "<Unit>" <Receiver_Name>
SG_ EngineSpeed : 16:16@1+ (0.25,0) [0|16383.75] "rpm" EngineDisplay
```

`Signals` define each field that is sent as a part of a `Message`.

A rolling counter is a signal (a part of the frame).