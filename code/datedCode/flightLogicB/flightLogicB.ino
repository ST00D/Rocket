#include <Wire.h>
#include "SparkFunMPL3115A2.h"
#include <MPU6050.h>

// Create instances of the sensor objects
MPL3115A2 myPressure;
MPU6050 mpu;

// Variables to hold flight status and starting altitude
String flightStatus = "waiting";
float startingAltitude = 0;
bool hasStarted = false;

// Threshold in feet for considering that we've returned to the original altitude
const float ALT_THRESHOLD = 3.0;

// Track how long we've been within the threshold for landing
unsigned long landingStartTime = 0;
const unsigned long LANDING_TIME_REQUIRED = 3000; // 3 seconds

// Variables to track the highest total acceleration
float maxAT = 0;
unsigned long maxATStartTime = 0;
float maxATDuration = 0; // Changed to float for precision display in seconds

void setup() {
  Wire.begin();        // Join I2C bus
  Serial.begin(9600);  // Start serial for output

  // Initialize MPL3115A2
  myPressure.begin(); 
  myPressure.setModeAltimeter();
  myPressure.setOversampleRate(7);
  myPressure.enableEventFlags();

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1); // Halt if the MPU6050 is not connected
  }

  Serial.println("Sensors initialized");
}

void loop() {
  // Read altitude from MPL3115A2
  float altitude = myPressure.readAltitudeFt();
  
  // Set starting altitude once
  if (!hasStarted) {
    startingAltitude = altitude;
    hasStarted = true;
  }
  
  // Check if altitude has increased by 10 feet from the starting altitude
  if ((altitude - startingAltitude) >= 10.0 && flightStatus == "waiting") {
    flightStatus = "flight";
  }

  // LANDING LOGIC:
  // If we are currently in flight, check whether we've returned close to the 
  // original altitude (within ALT_THRESHOLD) for at least 3 seconds.
  if (flightStatus == "flight") {
    float altitudeDifference = fabs(altitude - startingAltitude);
    if (altitudeDifference <= ALT_THRESHOLD) {
      if (landingStartTime == 0) {
        landingStartTime = millis();
      }
      else if ((millis() - landingStartTime) >= LANDING_TIME_REQUIRED) {
        flightStatus = "landed";
        
        // Ensure minimum duration of 0.5 seconds is recorded
        if (maxATDuration < 0.5) {
          maxATDuration = 0.5;
        }
        
        Serial.print("Landed. Max Acceleration: ");
        Serial.print(maxAT, 2);
        Serial.print(" g for ");
        Serial.print(maxATDuration);
        Serial.println(" seconds.");
        return; // Exit the loop
      }
    } else {
      landingStartTime = 0;
    }
  }
  
  // Read temperature from MPL3115A2
  float temperature = myPressure.readTempF();
  
  // Read raw acceleration from MPU6050
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  
  // Convert raw values to 'g'
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  // Calculate total acceleration magnitude and round to nearest 0.5
  float aT = sqrt(ax_g * ax_g + ay_g * ay_g + az_g * az_g);
  aT = round(aT * 2) / 2.0; // Round to nearest half G

  // Track maximum total acceleration and its duration
  if (aT > maxAT) {
    maxAT = aT;
    maxATStartTime = millis();
    maxATDuration = 0; // Reset duration if a new max is found
  } else if (aT == maxAT) {
    maxATDuration = (millis() - maxATStartTime) / 1000.0; // Calculate duration in seconds
  }

  // Serial output for altitude, status, and acceleration data
  Serial.print("Altitude (ft): ");
  Serial.print(altitude, 2);
  Serial.print("  Status: ");
  Serial.print(flightStatus);
  Serial.print("  Temp (F): ");
  Serial.print(temperature, 2);
  Serial.print("  Accel (g): X=");
  Serial.print(ax_g, 2);
  Serial.print(" Y=");
  Serial.print(ay_g, 2);
  Serial.print(" Z=");
  Serial.print(az_g, 2);
  Serial.print(" Total=");
  Serial.print(aT, 2); // Output total acceleration magnitude
  Serial.print(" MaxAT=");
  Serial.print(maxAT, 2);
  Serial.print(" for ");
  Serial.print(maxATDuration >= 0.5 ? maxATDuration : 0.5); // Ensure minimum display of 0.5 seconds
  Serial.println(" s");

  delay(1000); // Wait 1 second between readings
}