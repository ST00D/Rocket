//STORES TIME, LAT, LONG, VEL, AX, AY, AZ, TEMP CONSISTANTLY

//ALTITUDE COMMENTED OUT OR ELSE IT BREAK

#include <TinyGPS++.h> 
#include <SoftwareSerial.h> 
#include <Wire.h> 
#include "I2Cdev.h" 
#include "MPU6050.h" 
#include <SD.h> 
#include <SPI.h> 

// GPS Configuration 
#define RXPin 8 // GPS RX to Arduino TX 
#define TXPin 9 // GPS TX to Arduino RX 
#define GPSBaud 9600 

// SD Card Configuration 
#define CS_PIN 4 // Chip Select pin for the SD card 

// MPU6050 Configuration 
MPU6050 sensor; 

// Variables for Sensor Data 
int16_t ax, ay, az, gx, gy, gz, temp; 
float accX, accY, accZ, tempC; 

// Create Instances 
TinyGPSPlus gps; 
SoftwareSerial gpsSerial(RXPin, TXPin); 

// Timing Variables 
unsigned long lastUpdateTime = 0; 
const unsigned long updateInterval = 500; // 100 ms 
const unsigned long sdWriteInterval = 3000; // Write every 3 seconds 
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
  sensor.initialize(); 
  if (!sensor.testConnection()) { 
    Serial.println(F("MPU6050 connection failed!")); 
    while (1); 
  } 
  Serial.println(F("MPU6050 ready!")); 
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

    // Read sensor data 
    readAccelerometerData(); 

    // Generate output 
    String output = generateOutput(); 
    Serial.println(output); 

    // Write to SD card every 3 seconds 
    if (currentTime - lastSDWriteTime >= sdWriteInterval) { 
      lastSDWriteTime = currentTime; 
      writeToSD(output); 
    } 
  } 
} 

void readAccelerometerData() { 
  sensor.getMotion6(&ax, &ay, &az, &gx, &gy, &gz); 
  accX = (float)ax / 16384.0; 
  accY = (float)ay / 16384.0; 
  accZ = (float)az / 16384.0; 
  temp = sensor.getTemperature(); 
  tempC = (float)temp / 340.0 + 36.53; 
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

  // Altitude
  // if (gps.altitude.isValid()) {
  //   output += " | ALT: " + String(gps.altitude.meters(), 2) + " m";
  // } else {
  //   output += " | ALT: INVALID";
  // }

  // Velocity
  if (gps.speed.isValid()) {
    output += " | VEL: " + String(gps.speed.mps(), 2) + " m/s";
  } else {
    output += " | VEL: INVALID";
  }

  // Accelerometer
  output += " | AX: " + String(accX, 2);
  output += " | AY: " + String(accY, 2);
  output += " | AZ: " + String(accZ, 2);

  // Temperature
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