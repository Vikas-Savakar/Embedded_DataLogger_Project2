# Industrial Data Logger — ESP32

Embedded firmware project that acquires temperature, humidity, and an analog process variable, timestamps measurements with an RTC, and stores structured CSV records on a microSD card.

## Overview

| Item | Details |
|---|---|
| MCU | ESP32 DevKit |
| Firmware | C++ / Arduino ESP32 core 3.3.3 |
| Storage | FAT-formatted microSD over SPI |
| RTC | DS3231 over I2C |
| Sensor | DHT22 |
| Scheduling | `millis()`-based 5-second logging interval |
| Diagnostics | UART at 115200 baud |

## What it demonstrates

- Embedded C++ firmware organization
- DHT22 temperature/humidity acquisition
- ADC process-variable measurement and scaling
- I2C RTC integration and timestamp validation
- SPI SD-card initialization and CSV persistence
- Periodic scheduling without `delay()` in the main logging loop
- Explicit handling of invalid sensor data and peripheral failures
- UART diagnostics and status/error indication

## System flow

```text
DHT22 + ADC
     │
     ▼
  ESP32 MCU
     │
     ├── I2C ──► DS3231 RTC ──► timestamp
     │
     ├── SPI ──► microSD ─────► datalog.csv
     │
     └── UART ─► serial diagnostics
```

Each logging cycle reads and validates the inputs, obtains a trusted RTC timestamp, formats one CSV record, and appends it to the SD card. Invalid measurements are never presented as valid data.

## Pin configuration

| Function | ESP32 | Interface |
|---|---:|---|
| DHT22 data | GPIO4 | Digital |
| Process variable | GPIO34 | ADC |
| Status LED | GPIO2 | Digital output |
| Error LED | GPIO15 | Digital output |
| UART TX/RX | GPIO1 / GPIO3 | UART0 |
| DS3231 SDA/SCL | GPIO21 / GPIO22 | I2C |
| SD CS | GPIO5 | SPI |
| SD SCK | GPIO18 | SPI |
| SD MISO | GPIO19 | SPI |
| SD MOSI | GPIO23 | SPI |

Use 3.3 V-compatible connections and a suitable FAT-formatted microSD card.

## CSV format

```text
Timestamp,Temperature_C,Humidity_Pct,ProcessValue_Pct,Status
```

Valid records use `OK`. A cycle with invalid sensor/ADC data is marked `INVALID` only when a trusted RTC timestamp and writable SD file are available.

## Build

The verified build uses Arduino CLI with the ESP32 Arduino core. Generated build output is intentionally excluded from the repository.

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32 .
```

Dependencies:

- Adafruit DHT sensor library 1.4.7
- Adafruit Unified Sensor 1.1.15
- Adafruit RTClib 2.1.4
- ESP32 `SD`, `SPI`, and `Wire` libraries

## Hardware setup

1. Connect the DHT22, potentiometer/ADC source, DS3231, LEDs, and SD module using the pin map above.
2. Ensure the DS3231 has a valid time source and is not reporting lost power.
3. Insert a FAT-formatted microSD card.
4. Upload the firmware to an ESP32 DevKit-compatible board.
5. Open a serial monitor at 115200 baud.
6. Confirm initialization, then inspect `datalog.csv` on the SD card after multiple logging intervals.

## Verification status

- **Build:** verified successfully for `esp32:esp32:esp32` with ESP32 core 3.3.3.
- **Simulation:** normal logging flow verified in Wokwi using its DS1307 component as an explicitly documented RTC compatibility proxy.
- **Physical hardware:** not yet verified.
- **Real SD filesystem inspection:** not claimed as completed.
- **Sensor/ADC variation and fault-injection cases:** implemented but not claimed as runtime-tested unless reproduced on hardware.

## Wokwi

`diagram.json` and `wokwi.toml` contain the simulation configuration. Wokwi's built-in RTC component is DS1307; it is used only as a simulation proxy. The firmware remains designed for the required physical DS3231.

## Repository structure

```text
Embedded_DataLogger_Project2/
├── src/
│   └── main.cpp          # Firmware
├── diagram.json          # Wokwi circuit
├── wokwi.toml            # Wokwi configuration
├── .gitignore
└── README.md
```

Generated files, IDE metadata, screenshots, logs, and `datalog.csv` are intentionally excluded from version control.

## Scope

This project focuses on embedded firmware architecture and peripheral integration. It is a development/training project, not a production-certified industrial data logger.

## Author

**Vikas Savakar**  
B.Tech Electrical and Electronics Engineering — REVA University
