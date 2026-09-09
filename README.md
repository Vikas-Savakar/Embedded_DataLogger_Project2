# Industrial Data Logger with SD Card and Real-Time Monitoring

## Objective

This ESP32 firmware integrates DHT22 temperature/humidity acquisition, an ADC process variable, a DS3231 real-time clock, SPI SD-card storage, UART diagnostics, status LEDs, and periodic scheduling. The key novelty is timestamped persistent CSV storage using both I2C and SPI peripherals in a structured logger.

This is a separate Project 2 and does not modify `Embedded_RTOS_Project1`.

## Features

- DHT22 temperature and humidity acquisition on GPIO4.
- 12-bit ADC process-variable measurement on GPIO34, scaled to 0–100%.
- DS3231 timestamping over I2C.
- CSV persistence to `datalog.csv` on an SD card over SPI.
- 5-second `millis()`-based scheduling.
- UART diagnostics at 115200 baud.
- Status LED on GPIO2 and error LED on GPIO15.
- Safe handling of invalid DHT22, ADC, RTC, SD, file-open, and write errors.
- No fabricated readings or timestamps: invalid cycles are logged as `INVALID` only when a real RTC timestamp and SD file are available.
- Modular functions separating initialization, acquisition, timestamping, and storage.

## Hardware

- ESP32 DevKit-compatible board
- DHT22 sensor
- DS3231 RTC module with a valid battery/time source
- SPI SD-card module and FAT-formatted microSD card
- Potentiometer or 0–3.3 V process-variable source
- LEDs and suitable current-limiting resistors

## Pin configuration

| Function | ESP32 pin | Interface |
|---|---:|---|
| DHT22 data | GPIO4 | Digital |
| Process variable | GPIO34 | ADC input |
| Status LED | GPIO2 | Digital output |
| Error LED | GPIO15 | Digital output |
| UART TX/RX | GPIO1 / GPIO3 | UART0 / USB serial |
| DS3231 SDA/SCL | GPIO21 / GPIO22 | I2C |
| SD CS | GPIO5 | SPI chip select |
| SD SCK | GPIO18 | SPI |
| SD MISO | GPIO19 | SPI |
| SD MOSI | GPIO23 | SPI |

Use 3.3 V-compatible wiring. GPIO34 is input-only, which is suitable for the ADC signal.

## Software environment

- Visual Studio Code (optional editor)
- Arduino CLI bundled with Arduino IDE
- Arduino ESP32 core **3.3.3**
- Target FQBN: `esp32:esp32:esp32` (ESP32 Dev Module)
- Wokwi VS Code extension/CLI for the separate simulation configuration

The project uses the same Arduino ESP32 environment as Project 1. It is not migrated to PlatformIO; the existing `platformio.ini` is preserved as prior work but is not used by the verified build.

## Dependencies

Already available on this PC and reused without installing duplicate toolchains:

- Adafruit DHT sensor library **1.4.7**
- Adafruit Unified Sensor **1.1.15**
- Adafruit RTClib **2.1.4** for DS3231
- ESP32 core libraries: `SD`, `SPI`, and `Wire`

The verified Arduino build resolved these libraries from the existing Arduino library folders and ESP32 core 3.3.3.

## System architecture

`setup()` initializes UART, GPIO/ADC, DHT22, I2C/DS3231, SPI/SD, and the CSV header. `loop()` uses elapsed `millis()` time to call `performLogging()` every 5000 ms. Each logging cycle reads and validates the DHT22 and ADC, obtains a timestamp only when the RTC is trusted, then appends one complete valid row and closes the file.

DHT and SD operations are synchronous. The scheduling loop is periodic and avoids `delay()` for normal operation, but the entire system should not be described as fully non-blocking.

If the RTC cannot be detected or reports lost power, timestamped records are refused. If the SD card is unavailable, the firmware remains alive and reports the fault. An invalid DHT22 or ADC cycle is not written.

## CSV format

The first line is exactly:

```text
Timestamp,Temperature_C,Humidity_Pct,ProcessValue_Pct,Status
```

Valid rows use this structure:

```text
YYYY-MM-DD HH:MM:SS,temperature,humidity,process_percentage,OK
```

Invalid-but-timestamped rows use the same columns with `INVALID` in the status field and `nan` for any unavailable numeric value. No example measurement is included as a generated data file, because measurements must come from real hardware.

## Build instructions

The verified build uses the installed Arduino CLI and does not modify Project 1:

```powershell
$cli = 'C:\Users\vikas\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
& $cli compile --fqbn esp32:esp32:esp32 --build-path build-final .
```

The resulting firmware and ELF files are placed in `build-final/` for Wokwi. No duplicate toolchain installation is required.

## Upload instructions

Connect a compatible ESP32 board by USB, identify its serial port, and upload with the installed Arduino CLI:

```powershell
$cli = 'C:\Users\vikas\AppData\Local\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe'
& $cli upload -p COMx --fqbn esp32:esp32:esp32 .
```

Replace `COMx` with the actual port. Upload and port access require a real board and USB driver; they were not tested here.

## Serial monitor

Use Arduino IDE's Serial Monitor or another serial terminal at **115200 baud**. The firmware prints actual runtime messages only when it is running on a board or simulator. The documented startup strings are expected formats, not claimed observations from this build-only verification.

## Hardware test procedure

1. Wire the DHT22, potentiometer, LEDs, DS3231, and SD module exactly as shown in the pin table.
2. Format a microSD card with a supported FAT filesystem and insert it.
3. Set the DS3231 clock using a separate trusted setup sketch or tool; this firmware deliberately does not invent a time after `lostPower()`.
4. Power the ESP32 and open the UART monitor at 115200 baud.
5. Confirm peripheral initialization messages and that the status LED is on with the error LED off.
6. Wait for the 5-second logging interval and inspect `datalog.csv` on the SD card.
7. Turn the potentiometer and confirm the process percentage changes between 0 and 100.
8. Disconnect or fault-test one peripheral only with power removed, then confirm the expected UART error and error LED behavior.
9. Restore the wiring/card/RTC and confirm recovery where applicable.

## Verification status

- **BUILD VERIFIED:** Arduino CLI compilation succeeded for `esp32:esp32:esp32` using ESP32 core 3.3.3; no warnings were reported.
- **NOT TESTED:** firmware upload, physical UART output, SD file creation, real DHT22 readings, real ADC movement, and RTC timestamps.
- **Requires real hardware:** all electrical behavior, peripheral communication, measured values, SD persistence, LED indications, and long-duration logging.

## Known limitations

- This firmware does not set the RTC automatically after a lost-power condition; the clock must be set deliberately.
- SD-card module wiring and voltage-level compatibility depend on the selected hardware.
- DHT22 acquisition and SD writes can take synchronous time; the schedule is periodic, not a hard real-time guarantee.
- GPIO1/GPIO3 are shared with the ESP32 USB/UART0 path and should not be repurposed while using the serial monitor.
- A separate `diagram.json` and `wokwi.toml` are included for ESP32, DHT22, potentiometer, microSD, and LEDs. Wokwi's supported built-in RTC is DS1307, so the diagram uses it only as an explicitly documented I2C/RTClib compatibility proxy; the firmware remains targeted at the required physical DS3231. Exact DS3231 behavior requires hardware verification.

## Troubleshooting

- **RTC not detected:** check SDA/SCL, common ground, module power, and I2C address/wiring.
- **RTC clock invalid:** set the DS3231 time, power-cycle, and retry.
- **SD initialization failed:** check CS/SPI wiring, card format, module power, and 3.3 V compatibility.
- **Invalid DHT22 reading:** check data wiring, power, ground, and the required pull-up arrangement.
- **ADC does not vary:** ensure GPIO34 receives a safe 0–3.3 V signal and shares ground with the ESP32.
- **No serial output:** select the correct USB serial port and use 115200 baud.
- **Build dependency failure:** confirm the Arduino ESP32 core and the listed libraries are installed, then rerun the Arduino CLI command from the build section.

## Internship discussion points

This project demonstrates ESP32 peripheral integration rather than only sensor display: ADC acquisition, DHT22 timing, I2C RTC timekeeping, SPI removable storage, CSV persistence, UART diagnostics, `millis()` scheduling, modular firmware, and explicit fault handling. It provides a clear progression from Project 1's monitoring behavior to timestamped, persistent industrial-style data logging.
