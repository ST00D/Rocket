# Rocket

EVCC NASA Student Launch 




Current libraries:


Raspberry PI:

Library for serial com with arduino-
pip3 install pyserial 

tts-
sudo apt install espeak-ng


Arduino:

gps-
TinyGPS++.h

SDA, SCL definitions-
SoftwareWire.h
Wire.h

Coms for Pi-
SoftwareSerial.h
I2Cdev.h

accelerometer-
MPU6050.h


# Description

The Arduino R3 collects critical telemetry data using a Gouuu Tech GT-U7 GPS module and an MPU 6050 accelerometer, with the data continuously stored on the SD card shield. Upon landing, the impact velocity, GPS location, and STEMnaut survivability data (including payload temperature and G-forces withstood) are read by the Raspberry Pi 3B via USB serial communication. The data is converted to speech using a locally run TTS (text-to-speech) library and transmitted over the Baofeng UV-5R radio. The transmission uses the 3.5mm audio jack for audio output, with the radio’s VOX (voice-operated exchange) feature controlling the PTT (push-to-talk) function. Power is supplied by a 12,000 mAh portable charger, which provides 5V to the Raspberry Pi and Arduino for up to six hours. A FingerTech switch is connected between the portable charger and the Pi’s power input, allowing the electronics to be powered on or off as needed.





