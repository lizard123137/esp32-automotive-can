# esp32-automotive-can

### Reading from CAN bus
- [ ] Reading packets from automotive canbus
- [ ] Reading OBD errors
- [ ] Maybe use INT pin of MCP2515 to inform program about receiving data

### Writing to CAN bus
- [ ] Maybe parsing DBC files for data (or use a json with the same information)
- [x] Create debug logging system, with adjustable priority levels, to track data being sent
- [ ] Refactor timers to be interpreted as signals

### GUI
- [ ] Reading
    - [ ] A generic table that shows the CAN frames
    - [ ] A separate interface for reading OBD errors
    - [ ] Maybe parsing the data based on a DBC file
    - [ ] Use WS for live updates about received data
- [ ] Writing
    - [ ] A set of generic inputs that can be added (sliders, bytes, booleans etc)
    - [ ] Loading and saving the writing layout from/to a file 

### Other
- [ ] Maybe we should store the actual website in LittleFS
- [ ] Research where we can use PROGMEM to save on SRAM memory