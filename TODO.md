# esp32-automotive-can

### Reading from CAN bus
- [ ] Reading packets from automotive canbus
- [ ] Reading OBD errors

### Writing to CAN bus
- [ ] Maybe parsing DBC files for data (or use a json with the same information)
- [x] Create debug logging system, with adjustable priority levels, to track data being sent

### GUI
- [ ] Reading
    - [ ] A generic table that shows the CAN frames
    - [ ] A separate interface for reading OBD errors
    - [ ] Maybe parsing the data based on a DBC file
- [ ] Writing
    - [ ] A set of generic inputs that can be added (sliders, bytes, booleans etc)
    - [ ] Loading and saving the writing layout from/to a file 