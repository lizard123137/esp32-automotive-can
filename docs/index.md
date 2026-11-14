# ESP32-S3 Automotive Canbus

A project created by two students who wanted to make something cool for their `IoT Devices` course.

## Overview

The goal of this project is to create a flexible interface for interacting with modern automotive hardware.

It lets the user create loops that can send packets at specified intervals. It also has support for `CRC` functions and in-frame counters.

## Getting Started

This project uses `PlatformIO` as its framework.

First off you would need to wire your `ESP32-S3` devkit to the device you want to interface with. We have a wiring example for the `F30 cluster` [here](wiring.md).

To run it install the `VScode` extension and click the `Upload and Monitor` button in the newly added `PlatformIO` tab.

## Contributing

If you want to contribute to the project feel free to open a pull request with the features you want added. Implementations for new devices are welcome.

To add a new device just create a new file with the device name as its name in the `src/devices` folder and implement the communication. A good reference will be the `f30_cluster.cpp` file.