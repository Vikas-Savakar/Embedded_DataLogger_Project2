# Industrial Data Logger (ESP32)

ESP32 firmware that logs temperature, humidity and an analog process value to a microSD card as CSV, with a timestamp from a real-time clock. Built for the Wokwi simulator, no hardware needed.

## What it does

- Reads a DHT22 (temperature, humidity) and a potentiometer (ADC) every 5 seconds
- Timestamps each reading with an RTC over I2C
- Appends rows to `datalog.csv` on a microSD card over SPI
- Checks readings against valid ranges and reports sensor, RTC or SD faults
- Prints status over UART at 115200 baud, and shows errors on an LED

| Item | Details |
|---|---|
| MCU | ESP32 DevKit |
| Sensor / input | DHT22 on GPIO4, potentiometer on GPIO34 |
| RTC | I2C (SDA 21, SCL 22) |
| Storage | microSD over SPI (CS 5) |
| Outputs | Status LED (GPIO2), error LED (GPIO15) |

## Run it

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (includes Arduino CLI) and VS Code with the **Wokwi** extension.
2. In Arduino IDE, install the **esp32** board package and these libraries: *DHT sensor library*, *Adafruit Unified Sensor*, *RTClib*.
3. Clone and build the firmware for the simulator:
   ```sh
   git clone https://github.com/Vikas-Savakar/Embedded_DataLogger_Project2.git
   cd Embedded_DataLogger_Project2
   arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build-final .
   ```
4. Open `diagram.json` in VS Code and start the Wokwi simulator.

PlatformIO also works: `pio run` builds the same firmware, for uploading to a real ESP32.

## Status

Simulation-based project, not tested on physical hardware.
