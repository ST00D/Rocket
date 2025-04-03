#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "SparkFunMPL3115A2.h"
#include <SD.h>
#include <SPI.h>

// GPS Configuration
#define RXPin 9 // GPS RX to Arduino TX
#define TXPin 8 // GPS TX to Arduino RX
#define GPSBaud 9600

// SD Card Configuration
#define CS_PIN 4 // Chip Select pin for the SD card

// MPU6050 Configuration
MPU6050 mpu;
MPL3115A2 myPressure;

// Create Instances
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Timing Variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 500; // 100 ms
const unsigned long sdWriteInterval = 500; // Write every 3 seconds
unsigned long lastSDWriteTime = 0;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize SD card
  pinMode(CS_PIN, OUTPUT);
  if (!SD.begin(CS_PIN)) {
    Serial.println(F("SD card initialization failed!"));
    while (1);
  }
  Serial.println(F("SD card initialized."));

  // Initialize GPS
  gpsSerial.begin(GPSBaud);
  Serial.println(F("GPS initialized."));

  // Initialize MPU6050
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println(F("MPU6050 connection failed!"));
    while (1);
  }
  Serial.println(F("MPU6050 ready!"));

  // Initialize MPL3115A2
  myPressure.begin(); 
  myPressure.setModeAltimeter();
  myPressure.setOversampleRate(7);
  myPressure.enableEventFlags();
}

void loop() {
  unsigned long currentTime = millis();

  // Handle GPS Data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Update data and log every 100 ms
  if (currentTime - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentTime;

    // Generate and display output
    String output = generateOutput();
    Serial.println(output);

    // Write to SD card every 3 seconds
    if (currentTime - lastSDWriteTime >= sdWriteInterval) {
      lastSDWriteTime = currentTime;
      writeToSD(output);
    }
  }
}

void readSensorData(float &ax_g, float &ay_g, float &az_g, float &tempC, float &altitude) {
  // Read altitude from MPL3115A2
  altitude = myPressure.readAltitudeFt();

  // Read raw acceleration from MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  
  // Convert raw values to 'g'
  ax_g = ax / 16384.0;
  ay_g = ay / 16384.0;
  az_g = az / 16384.0;

  // Read temperature from MPU6050
  tempC = mpu.getTemperature() / 340.0 + 36.53;
}

String generateOutput() {
  String output = "TIME: INVALID";

  // Time
  if (gps.time.isValid()) {
    output = "TIME: " + String(gps.time.hour()) + ":" +
             String(gps.time.minute()) + ":" +
             String(gps.time.second());
  }

  // Coordinates
  if (gps.location.isValid()) {
    output += " | COORD: " + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  } else {
    output += " | COORD: INVALID";
  }

  // Velocity
  if (gps.speed.isValid()) {
    output += " | VEL: " + String(gps.speed.mps(), 2) + " m/s";
  } else {
    output += " | VEL: INVALID";
  }

  // Variables to hold sensor data
  float accX, accY, accZ, tempC, altitude;

  // Read sensor data
  readSensorData(accX, accY, accZ, tempC, altitude);

  // Append sensor data to output
  output += " | AX: " + String(accX, 2) + ", AY: " + String(accY, 2) + ", AZ: " + String(accZ, 2);
  output += " | ALT: " + String(altitude, 2) + " ft";
  output += " | TEMP: " + String(tempC, 2) + " C";

  return output;
}

void writeToSD(const String& data) {
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    Serial.println(F("Data written to SD card."));
  } else {
    Serial.println(F("Error opening data.txt"));
  }
}