//Reads and prints TIME, LAT, LONG, VEL, ALT, AX, AY, AZ


#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"

// GPS Configuration
#define RXPin 8 // GPS RX to Arduino TX
#define TXPin 9 // GPS TX to Arduino RX
#define GPSBaud 9600

// MPU6050 Configuration
MPU6050 sensor;
int16_t ax, ay, az, gx, gy, gz, temp;
float accX, accY, accZ, tempC;

// Create instances
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Timing Variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 100; // 100 ms

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize GPS
  gpsSerial.begin(GPSBaud);
  Serial.println("GPS initialized.");

  // Initialize MPU6050
  Wire.begin();
  sensor.initialize();
  if (!sensor.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1);
  }
  Serial.println("MPU6050 ready!");
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

void readAccelerometerData() {
  sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  temp = sensor.getTemperature();
  accX = (float)ax / 16384.0;
  accY = (float)ay / 16384.0;
  accZ = (float)az / 16384.0;
  tempC = (float)temp / 340.0 + 36.53;
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

  // Altitude
  if (gps.altitude.isValid()) {
    output += " | ALT: " + String(gps.altitude.meters(), 2) + " m";
  } else {
    output += " | ALT: INVALID";
  }

  // Velocity
  if (gps.speed.isValid()) {
    output += " | VEL: " + String(gps.speed.mps(), 2) + " m/s";
  } else {
    output += " | VEL: INVALID";
  }

  // Accelerometer
  readAccelerometerData();
  output += " | AX: " + String(accX, 2) + ", AY: " + String(accY, 2) + ", AZ: " + String(accZ, 2);

  // Temperature
  output += " | TEMP: " + String(tempC, 2) + " C";

  Serial.println(output);
}
