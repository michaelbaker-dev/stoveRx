# StoveRx

StoveRx is an IoT (Internet of Things) project for monitoring stove temperatures and environmental conditions remotely.

## Hardware Components
- Adafruit Feather M0 microcontroller board
- LoRa radio module for wireless communication
- LCD display for local information display
- WiFi connectivity module

## Main Functions
- Receives temperature data from a remote sensor (transmitter unit)
- Monitors:
  - Stove temperature
  - Outside temperature
  - Battery voltage and percentage
- Displays information on an LCD screen
- Sends data to Blynk cloud platform for remote monitoring
- Provides real-time RSSI (signal strength) readings

## Communication Features
- Uses LoRa radio (915 MHz frequency) to receive data from sensors
- Connects to WiFi to send data to Blynk cloud platform
- Displays status and readings on a 20x4 LCD screen
- Sends acknowledgments back to the transmitter

## Key Features
- Real-time temperature monitoring
- Battery level monitoring
- WiFi connectivity for cloud updates
- Local display of all sensor readings
- Automatic reconnection handling
- Signal strength monitoring

This is the receiver unit's codebase, which processes incoming sensor data and makes it available both locally on an LCD display and remotely through the Blynk IoT platform.