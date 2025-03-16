#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// GPS Configuration
#define RXPin 8 // Update based on your setup
#define TXPin 9 // Update based on your setup
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Variables for time tracking
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 500; // 1-second interval

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize GPS
  gpsSerial.begin(9600);
  Serial.println("GPS initialized.");
}

void loop() {
  // Handle GPS Data
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);
  }

  // Update Serial Monitor periodically
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();
    displaySpeed();
  }
}

// Display speed on the Serial Monitor
void displaySpeed() {
  if (gps.speed.isValid()) {
    float speedKmh = gps.speed.kmph(); // Speed in km/h
    float speedMps = gps.speed.mps(); // Speed in m/s

    // Print to Serial Monitor
    Serial.print("Speed: ");
    Serial.print(speedKmh, 2);
    Serial.print(" km/h (");
    Serial.print(speedMps, 2);
    Serial.println(" m/s)");
  } else {
    Serial.println("Speed data not available.");
  }
}
