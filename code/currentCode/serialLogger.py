import serial


ser = serial.Serial('COM10', 115200, timeout=1)
ser.flush()

with open("flight_log.txt", "a") as file:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(f"Logging: {line}") 
            file.write(line + "\n")
