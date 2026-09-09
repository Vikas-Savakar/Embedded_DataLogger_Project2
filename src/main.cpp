#include <Arduino.h>
#include <DHT.h>
#include <RTClib.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

namespace Pins {
constexpr uint8_t DHT_DATA = 4;
constexpr uint8_t PROCESS_ADC = 34;
constexpr uint8_t STATUS_LED = 2;
constexpr uint8_t ERROR_LED = 15;
constexpr uint8_t UART_TX = 1;
constexpr uint8_t UART_RX = 3;
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;
constexpr uint8_t SD_CS = 5;
constexpr uint8_t SD_SCK = 18;
constexpr uint8_t SD_MISO = 19;
constexpr uint8_t SD_MOSI = 23;
}  // namespace Pins

namespace Config {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t LOG_INTERVAL_MS = 5000;
constexpr uint16_t ADC_FULL_SCALE = 4095;
constexpr float MIN_TEMPERATURE_C = -40.0f;
constexpr float MAX_TEMPERATURE_C = 80.0f;
constexpr float MIN_HUMIDITY_PCT = 0.0f;
constexpr float MAX_HUMIDITY_PCT = 100.0f;
constexpr char LOG_FILE[] = "/datalog.csv";
constexpr char CSV_HEADER[] =
    "Timestamp,Temperature_C,Humidity_Pct,ProcessValue_Pct,Status";
}  // namespace Config

DHT dht(Pins::DHT_DATA, DHT22);
RTC_DS3231 rtc;
SPIClass sdSpi(VSPI);

bool rtcReady = false;
bool sdReady = false;
bool errorActive = false;
uint32_t lastLogTime = 0;

struct Measurement {
  float temperatureC = NAN;
  float humidityPct = NAN;
  float processValuePct = NAN;
};

void setIndicators(bool error) {
  errorActive = error;
  digitalWrite(Pins::STATUS_LED, error ? LOW : HIGH);
  digitalWrite(Pins::ERROR_LED, error ? HIGH : LOW);
}

void reportError(const char *message) {
  Serial.print("[ERROR] ");
  Serial.println(message);
  setIndicators(true);
}

void initializeSensors() {
  pinMode(Pins::STATUS_LED, OUTPUT);
  pinMode(Pins::ERROR_LED, OUTPUT);
  setIndicators(true);

  pinMode(Pins::PROCESS_ADC, INPUT);
  analogReadResolution(12);
  dht.begin();
  Serial.println("[BOOT] DHT22 and ADC initialised");
}

void initializeRTC() {
  Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
  Serial.println("[BOOT] Initialising DS3231 RTC over I2C");

  if (!rtc.begin()) {
    rtcReady = false;
    reportError("RTC not detected");
    return;
  }

  // A lost-power clock may contain stale time. Do not create misleading rows.
  if (rtc.lostPower()) {
    rtcReady = false;
    reportError("RTC clock is invalid; set the DS3231 time before logging");
    return;
  }

  rtcReady = true;
  Serial.println("[BOOT] RTC initialised");
}

bool writeCsvHeader() {
  File file = SD.open(Config::LOG_FILE, FILE_APPEND);
  if (!file) {
    reportError("Unable to open datalog.csv for header");
    return false;
  }

  bool headerWritten = true;
  if (file.size() == 0) {
    headerWritten = file.println(Config::CSV_HEADER) > 0;
  }
  file.close();

  if (!headerWritten) {
    reportError("Unable to write CSV header");
  }
  return headerWritten;
}

void initializeSD() {
  Serial.println("[BOOT] Initialising SD card over SPI");
  sdSpi.begin(Pins::SD_SCK, Pins::SD_MISO, Pins::SD_MOSI, Pins::SD_CS);

  if (!SD.begin(Pins::SD_CS, sdSpi)) {
    sdReady = false;
    reportError("SD card initialisation failed");
    return;
  }

  if (!writeCsvHeader()) {
    sdReady = false;
    return;
  }

  sdReady = true;
  Serial.println("[BOOT] SD card ready; datalog.csv checked");
}

bool readMeasurement(Measurement &measurement) {
  measurement.temperatureC = dht.readTemperature();
  measurement.humidityPct = dht.readHumidity();

  const bool dhtValid =
      !isnan(measurement.temperatureC) &&
      !isnan(measurement.humidityPct) &&
      measurement.temperatureC >= Config::MIN_TEMPERATURE_C &&
      measurement.temperatureC <= Config::MAX_TEMPERATURE_C &&
      measurement.humidityPct >= Config::MIN_HUMIDITY_PCT &&
      measurement.humidityPct <= Config::MAX_HUMIDITY_PCT;

  const int adcValue = analogRead(Pins::PROCESS_ADC);
  const bool adcValid = adcValue >= 0 && adcValue <= Config::ADC_FULL_SCALE;
  if (adcValid) {
    measurement.processValuePct =
        constrain((adcValue * 100.0f) / Config::ADC_FULL_SCALE, 0.0f, 100.0f);
  } else {
    measurement.processValuePct = NAN;
  }

  if (!dhtValid) {
    reportError("Invalid DHT22 reading");
  }
  if (!adcValid) {
    reportError("Invalid ADC reading");
  }
  return dhtValid && adcValid;
}

bool getTimestamp(char *buffer, size_t bufferLength) {
  if (!rtcReady) {
    reportError("RTC unavailable; record refused to prevent false timestamp");
    return false;
  }

  const DateTime now = rtc.now();
  snprintf(buffer, bufferLength, "%04d-%02d-%02d %02d:%02d:%02d", now.year(),
           now.month(), now.day(), now.hour(), now.minute(), now.second());
  return true;
}

bool appendMeasurement(const Measurement &measurement, const char *timestamp,
                       const char *status) {
  if (!sdReady) {
    reportError("SD card unavailable; record not written");
    return false;
  }

  File file = SD.open(Config::LOG_FILE, FILE_APPEND);
  if (!file) {
    reportError("Unable to open datalog.csv for append");
    return false;
  }

  const String row = String(timestamp) + "," + String(measurement.temperatureC, 1) +
                     "," + String(measurement.humidityPct, 1) + "," +
                     String(measurement.processValuePct, 1) + "," + status;
  const size_t bytesWritten = file.println(row);
  file.close();

  if (bytesWritten == 0) {
    reportError("CSV write failed");
    return false;
  }

  Serial.print("[LOG] ");
  Serial.print(row);
  Serial.println(" (written)");
  return true;
}

void performLogging() {
  Measurement measurement;
  char timestamp[20] = {};
  const bool measurementValid = readMeasurement(measurement);

  if (!getTimestamp(timestamp, sizeof(timestamp))) {
    return;
  }

  const char *status = measurementValid ? "OK" : "INVALID";
  if (!appendMeasurement(measurement, timestamp, status)) {
    return;
  }

  setIndicators(!measurementValid);
}

void setup() {
  Serial.begin(Config::SERIAL_BAUD);
  delay(500);
  Serial.println("[BOOT] Industrial Data Logger with SD Card");

  initializeSensors();
  initializeRTC();
  initializeSD();

  if (rtcReady && sdReady) {
    setIndicators(false);
    Serial.println("[BOOT] Initialisation complete. Logging every 5 seconds.");
  } else {
    Serial.println("[BOOT] Initialisation incomplete. Fault handling remains active.");
  }

  lastLogTime = millis();
}

void loop() {
  if (millis() - lastLogTime >= Config::LOG_INTERVAL_MS) {
    lastLogTime = millis();
    performLogging();
  }
}
