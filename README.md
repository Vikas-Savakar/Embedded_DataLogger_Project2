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

1. Install [VS Code](https://code.visualstudio.com/) with the **PlatformIO** and **Wokwi** extensions.
2. Clone and build:
   ```sh
   git clone https://github.com/Vikas-Savakar/Embedded_DataLogger_Project2.git
   cd Embedded_DataLogger_Project2
   pio run
   ```
3. Open `diagram.json` in VS Code and start the Wokwi simulator.

## Status

Simulation-based project, not tested on physical hardware.
