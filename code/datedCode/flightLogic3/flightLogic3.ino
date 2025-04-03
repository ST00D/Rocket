//reliably stores all flight data on sd card in two strings 


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
const unsigned long updateInterval = 250; // 250 ms
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

  // Update data and log
  if (currentTime - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentTime;

    // Generate and display output
    generateOutputAndWrite();
  }
}

void readSensorData(float &ax_g, float &ay_g, float &az_g, float &tempC, float &altitude) {
  // Read altitude from MPL3115A2
  altitude = myPressure.readAltitudeFt();

  // Read temperature from MPL3115A2
  tempC = myPressure.readTemp();

  // Read raw acceleration from MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  // Convert raw values to 'g'
  ax_g = (float)ax / 16384.0;
  ay_g = (float)ay / 16384.0;
  az_g = (float)az / 16384.0;
}

void generateOutputAndWrite() {
  String output = "";
  String secondLine = "";

  // Time
  if (gps.time.isValid()) {
    output = "TIME: " + String(gps.time.hour()) + ":" +
             String(gps.time.minute()) + ":" +
             String(gps.time.second());
  } else {
    output = "TIME: INVALID";
  }

  // Coordinates
  if (gps.location.isValid()) {
    output += " | COORD: " + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  } else {
    output += " | COORD: INVALID";
  }

  // Velocity
  if (gps.speed.isValid()) {
    secondLine = "VEL: " + String(gps.speed.mps(), 2) + " m/s";
  } else {
    secondLine = "VEL: INVALID";
  }

  // Variables to hold sensor data
  float accX, accY, accZ, tempC, altitude;

  // Read sensor data
  readSensorData(accX, accY, accZ, tempC, altitude);

  // Append accelerometer data
  secondLine += " | AX: " + String(accX, 2);
  secondLine += " | AY: " + String(accY, 2);
  secondLine += " | AZ: " + String(accZ, 2);

  // Append altitude and temperature
  secondLine += " | ALT: " + String(altitude, 2) + " ft";
  secondLine += " | TEMP: " + String(tempC, 2) + " C";

  // Print to Serial Monitor
  Serial.println(output);
  Serial.println(secondLine);

  // Write to SD Card
  writeToSD(output);
  writeToSD(secondLine);
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