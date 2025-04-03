#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "SparkFunMPL3115A2.h"

// GPS Configuration
#define RXPin 9 // GPS RX to Arduino TX
#define TXPin 8 // GPS TX to Arduino RX
#define GPSBaud 9600

// MPU6050 Configuration
MPU6050 mpu;

MPL3115A2 myPressure;

// Create instances
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Timing Variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 250; // 100 ms

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize GPS
  gpsSerial.begin(GPSBaud);
  Serial.println("GPS initialized.");

  // Initialize MPU6050
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }
  Serial.println("MPU6050 ready!");

  myPressure.begin(); 
  myPressure.setModeAltimeter();
  myPressure.setOversampleRate(7);
  myPressure.enableEventFlags();
}

void loop() {
  unsigned long currentTime = millis();

  // Handle GPS Data
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }

  // Update data and display every 100 ms
  if (currentTime - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentTime;
    displayData();
  }
}

void readSensorData(float &ax_g, float &ay_g, float &az_g, float &tempC, float &altitude, float &tempF) {
  // Read altitude from MPL3115A2
  altitude = myPressure.readAltitudeFt();

  // Read temperature from MPL3115A2
 // tempF = myPressure.readTempF();

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

void displayData() {
  String output = "";

  // Time
  if (gps.time.isValid()) {
    output += "TIME: " + String(gps.time.hour() < 10 ? "0" : "") + String(gps.time.hour()) + ":" +
              String(gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + ":" +
              String(gps.time.second() < 10 ? "0" : "") + String(gps.time.second());
  } else {
    output += "TIME: INVALID";
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
  float accX, accY, accZ, tempC, altitude, tempF;

  // Read sensor data
  readSensorData(accX, accY, accZ, tempC, altitude, tempF);

  // Append sensor data to output
  output += " | AX: " + String(accX, 2) + ", AY: " + String(accY, 2) + ", AZ: " + String(accZ, 2);
  output += " | ALT: " + String(altitude, 2) + " ft";
  output += " | TEMP (C): " + String(tempC, 2);
  //output += " | TEMP (F): " + String(tempF, 2);

  Serial.println(output);
}