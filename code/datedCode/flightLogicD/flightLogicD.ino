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

// Instances
MPU6050 mpu;
MPL3115A2 myPressure;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Timing Variables
unsigned long lastUpdateTime = 0;
const unsigned long updateInterval = 250; // 100 ms

// Flight Logic Variables
String flightStatus = "waiting";
float startingAltitude = 0;
bool hasStarted = false;
const float ALT_THRESHOLD = 3.0;
unsigned long landingStartTime = 0;
const unsigned long LANDING_TIME_REQUIRED = 3000; // 3 seconds
float maxAT = 0;
unsigned long maxATStartTime = 0;
float maxATDuration = 0;

void setup() {
  Serial.begin(115200);

  gpsSerial.begin(GPSBaud);
  Serial.println("GPS initialized.");

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
    handleFlightLogic(); // Call the flight logic handler
    displayData();
  }
}

void handleFlightLogic() {
  float altitude = myPressure.readAltitudeFt();

  if (!hasStarted) {
    startingAltitude = altitude;
    hasStarted = true;
  }

  if ((altitude - startingAltitude) >= 10.0 && flightStatus == "waiting") {
    flightStatus = "flight";
  }

  if (flightStatus == "flight") {
    float altitudeDifference = fabs(altitude - startingAltitude);
    if (altitudeDifference <= ALT_THRESHOLD) {
      if (landingStartTime == 0) {
        landingStartTime = millis();
      }
      else if ((millis() - landingStartTime) >= LANDING_TIME_REQUIRED) {
        flightStatus = "landed";
        if (maxATDuration < 0.5) {
          maxATDuration = 0.5;
        }
        float lastVelocity = gps.speed.isValid() ? gps.speed.mps() : -1.0;
        Serial.print("Landed. Max Acceleration: ");
        Serial.print(maxAT, 2);
        Serial.print(" g for ");
        Serial.print(maxATDuration);
        Serial.print(" seconds. Last Velocity: ");
        if(lastVelocity >= 0) {
          Serial.print(lastVelocity, 2);
          Serial.println(" m/s.");
        } else {
          Serial.println("INVALID.");
        }
        delay(9999); // Pause after landing statement
        return;
      }
    } else {
      landingStartTime = 0;
    }
  }

  float accX, accY, accZ;
  readSensorData(accX, accY, accZ);

  float aT = sqrt(accX * accX + accY * accY + accZ * accZ);
  aT = round(aT * 2) / 2.0;

  if (aT > maxAT) {
    maxAT = aT;
    maxATStartTime = millis();
    maxATDuration = 0;
  } else if (aT == maxAT) {
    maxATDuration = (millis() - maxATStartTime) / 1000.0;
  }
}

void readSensorData(float &ax_g, float &ay_g, float &az_g) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  ax_g = ax / 16384.0;
  ay_g = ay / 16384.0;
  az_g = az / 16384.0;
}

void displayData() {
  String output = "";

  if (gps.time.isValid()) {
    output += "TIME: " + String(gps.time.hour() < 10 ? "0" : "") + String(gps.time.hour()) + ":" +
              String(gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + ":" +
              String(gps.time.second() < 10 ? "0" : "") + String(gps.time.second());
  } else {
    output += "TIME: INVALID";
  }

  if (gps.location.isValid()) {
    output += " | COORD: " + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  } else {
    output += " | COORD: INVALID";
  }

  if (gps.speed.isValid()) {
    output += " | VEL: " + String(gps.speed.mps(), 2) + " m/s";
  } else {
    output += " | VEL: INVALID";
  }

  float accX, accY, accZ, altitude;
  readSensorData(accX, accY, accZ);

  altitude = myPressure.readAltitudeFt();

  float aT = sqrt(accX * accX + accY * accY + accZ * accZ);
  aT = round(aT * 2) / 2.0;

  output += " | AX: " + String(accX, 2) + ", AY: " + String(accY, 2) + ", AZ: " + String(accZ, 2);
  output += " | ALT: " + String(altitude, 2) + " ft";
  output += " | Status: " + flightStatus;
  output += " | Total Accel: " + String(aT, 2) + " g";
  output += " | MaxAT: " + String(maxAT, 2);
  output += " for " + String(maxATDuration >= 0.5 ? maxATDuration : 0.5) + " s";

  Serial.println(output);
}