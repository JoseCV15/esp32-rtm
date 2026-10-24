# ESP32 Real-time Monitor

The goal of this project is to develop an embedded monitoring system that collects system metrics as well as sensor data providing diagnostic information through a serial port.

## Features
1. **LED Driver** - control the onboard or external LED through GPIO
2. **System Metrics** - monitor uptime and heap memory usage
3. **Disgnostic Shell** - command-line interface over the serial console


## Hardware



## Getting Started
1. Set up ESP-IDF
Install and configure ESP-IDF for your development environment
Open the ESP-IDF terminal and navigate to the project directory
2. Configure the project
3. Build the firmware
4. Flash the ESP32-S3
5. Open the serial monitor
Alternatively, flash the firmware and start the monitor in one command

## System Metrics
The system_metrics component is designed to collect:


## Architecture
The project follows a modular component-based design.
. Hardware abstraction: Hardware-specific operations are encapsulated in drivers
. Separation of concerns: Each component has a clearly defined responsibility
. Reusable interfaces: Public headers expose functionality to other components


## Learning Objectives
This project is intended to develop practical skills in
. Embedded C development
. ESP-IDF and FreeRTOS

