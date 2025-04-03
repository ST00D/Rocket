import serial
import time
import os

# Define the serial port and baud rate
port = "/dev/ttyACM0"
baud_rate = 115200

try:
    ser = serial.Serial(port, baud_rate, timeout=1)
    print("Connected to {}".format(ser.name))
except serial.SerialException as e:
    print("Serial exception: {}".format(e))
    exit()
except Exception as e:
    print("Error: {}".format(e))
    exit()

# Function to speak the text using espeak-ng
def speak(text):
    # Use --stdout piped to aplay to ensure proper audio output
    command = 'espeak-ng "{}" --stdout | aplay'.format(text)
    os.system(command)

# Function to get the latest data from the serial port
def get_latest_data():
    ser.flushInput()  
    time.sleep(0.1)  # Small delay to allow fresh data to arrive
    line = ser.readline().decode('utf-8', errors='ignore').strip()
    return line

# Main loop: continuously check for final landing transmissions
try:
    while True:
        data = get_latest_data()
        if data:
            print("Received:", data)
            # Check if the data starts with the marker callsign "KK7VPV"
            if data.startswith("KK7VPV"):
                # When landing is detected, speak the data
                speak("The landing transmission is: {}".format(data))
        time.sleep(0.1)  # Reduce delay if needed for responsiveness
except KeyboardInterrupt:
    print("Exiting...")
finally:
    try:
        ser.close()  # Safely close the serial port
    except Exception:
        pass
