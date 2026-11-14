# esp32-automotive-can

This project uses an `ESP32-S3` to connect to a vehicle `CAN BUS`. It is able to scan the existing traffic, as well as to send its own frames. It was built for an `IoT Devices` collage course.

It is heavily inspired by the architecture of `.dbc` files. 
As a demonstration of it working it can interface with a `BMW F30` gauge cluster.

![Interfacing with F30 cluster](/docs/images/f30-cluster-working.gif)

Features that aren't yet implemented for the cluster:
- Fuel gauge
- Temp gauge isn't showing correctly

Features that the entire project could use:
- `OBD II` scanning
- Examples for interfacing with other automotive devices

A repository that helped us greatly with figuring out how to communicate with the cluster -  [CarCluster by r00li](https://github.com/r00li/CarCluster).