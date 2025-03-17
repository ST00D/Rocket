import serial
import time
import os

# Define the serial port and baud rate
port = "/dev/ttyACM0"
baud_rate = 9600

try:
    ser = serial.Serial(port, baud_rate, timeout=1)
    print("Connected to {}".format(ser.name))
except serial.SerialException as e:
    print("Serial exception: {}".format(e))
    exit()
except Exception as e:
    print("Error: {}".format(e))
    exit()

# Function to speak the text using eSpeak NG
def speak(text):
    command = 'espeak-ng "{}"'.format(text)
    os.system(command)

# Function to get the latest data
def get_latest_data():
    # Flush any buffered input to prevent backlog
    ser.flushInput()  
    time.sleep(0.1)   # Small delay to allow fresh data to arrive
    line = ser.readline().decode('utf-8').strip()
    return line

# Main loop: Read and speak data every 10 seconds
try:
    while True:
        # Get the latest reading
        data = get_latest_data()
        if data:
            print("Received:", data)
            speak("The accelerations are: {}".format(data))
        
        # Wait for the next cycle
        time.sleep(10)
except KeyboardInterrupt:
    print("Exiting...")
finally:
    try:
        ser.close()  # Safely close the serial port
    except:
        pass
