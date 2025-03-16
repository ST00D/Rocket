#include <TinyGPS++.h>
#include <SoftwareWire.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"

#define SDA A4
#define SCL A5
#define BUZZER_PIN 3  // Define the buzzer pin

SoftwareWire myWire(SDA, SCL);
const int buzz =3;

// GPS Configuration
static const int TXPin = 9;       // TX of GPS to RXPin
static const int RXPin = 8;       // RX of GPS to TXPin
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// MPU6050 Configuration
MPU6050 sensor;

// Accelerometer Data
int16_t ax, ay, az, gx, gy, gz, temp;
float accX, accY, accZ, tempC;

// Timer for delays
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;
unsigned long lastBuzzTime = 0;
const unsigned long buzzInterval = 2000;  // 2 seconds

void setup() {
  Serial.begin(115200);
  setupGPS();
  setupAccelerometer();
  
  pinMode(BUZZER_PIN, OUTPUT);  
}

void loop() {
  handleGPS();
  if (millis() - lastSendTime >= sendInterval) {
    sendData();
    lastSendTime = millis();
  }

  // Buzzer logic - buzz every 2 seconds
  if (millis() - lastBuzzTime >= buzzInterval) {
    buzzBuzzer();
    lastBuzzTime = millis();
  }

  delay(10); // Minimal delay to prevent CPU overload
}

// Setup GPS Module
void setupGPS() {
  gpsSerial.begin(GPSBaud);
  Serial.println("GPS initialized.");
}

// Setup MPU6050 Accelerometer
void setupAccelerometer() {
  Wire.begin();
  sensor.initialize();
  if (!sensor.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1); // Halt execution
  }
  Serial.println("MPU6050 ready!");
}

// Handle GPS Data
void handleGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }
} 

// Read Accelerometer Data
void readAccelerometerData() {
  sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  temp = sensor.getTemperature();
  accX = (float)ax / 16384.0;
  accY = (float)ay / 16384.0;
  accZ = (float)az / 16384.0;
  tempC = (float)temp / 340.0 + 36.53;
}

// Display GPS Info
String getGPSInfo() {
  String gpsInfo = "";
  if (gps.location.isValid()) {
    gpsInfo += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
  } else {
    gpsInfo += "INVALID";
  }
  gpsInfo += " ";
  if (gps.date.isValid() && gps.time.isValid()) {
    gpsInfo += String(gps.date.month()) + "/" + String(gps.date.day()) + "/" + String(gps.date.year()) + " ";
    gpsInfo += (gps.time.hour() < 10 ? "0" : "") + String(gps.time.hour()) + ":" +
               (gps.time.minute() < 10 ? "0" : "") + String(gps.time.minute()) + ":" +
               (gps.time.second() < 10 ? "0" : "") + String(gps.time.second());
  } else {
    gpsInfo += "INVALID";
  }
  return gpsInfo;
}

// Send Data
void sendData() {
  readAccelerometerData();
  String gpsInfo = getGPSInfo();

  Serial.print("GPS: "); Serial.print(gpsInfo);
  Serial.print(" | ACC: X="); Serial.print(accX, 2);
  Serial.print(", Y="); Serial.print(accY, 2);
  Serial.print(", Z="); Serial.print(accZ, 2);
  Serial.print(" | TEMP: "); Serial.println(tempC, 2);
}

// Buzzer Function - Buzz for 2000s
//REMOVE COMMENT BEFORE FLIGHT
void buzzBuzzer() {
  //tone(buzz, 329);
  //digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer ON
  delay(2000);                      // Buzz for 200ms
  //digitalWrite(BUZZER_PIN, LOW);   // Turn buzzer OFF
}
