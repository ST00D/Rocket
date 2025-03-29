def extract_value(line, key):
    try:
        start = line.index(key) + len(key) + 1
        end = line.index('|', start)
        return line[start:end].strip()
    except ValueError:
        return '0'

def main():
    input_file_path = r"C:\Users\stuar\Documents\RocketTeam\flightData\data.txt"
    output_file_path = r"C:\Users\stuar\Documents\RocketTeam\flightData\formatted_data.txt"

    try:
        with open(input_file_path, 'r') as file:
            lines = file.readlines()

        with open(output_file_path, 'w') as file:
            file.write("TIME:\n")
            for line in lines:
                if "TIME" in line:
                    time = line.split("|")[0].split(":")[1].strip()
                    file.write(f"{time}\n")
            
            file.write("\nCOORD:\n")
            for line in lines:
                coord = extract_value(line, "COORD")
                file.write(f"{coord}\n")

            file.write("\nVEL:\n")
            for line in lines:
                vel = extract_value(line, "VEL")
                if 'm/s' not in vel and vel != '0':
                    vel = '0'
                file.write(f"{vel}\n")

            file.write("\nALT:\n")
            for line in lines:
                alt = extract_value(line, "ALT")
                if 'm' not in alt and alt != '0':
                    alt = '0'
                file.write(f"{alt}\n")

            file.write("\nTEMP:\n")
            for line in lines:
                temp = extract_value(line, "TEMP")
                if 'C' not in temp and temp != '0':
                    temp = '0'
                file.write(f"{temp}\n")

            file.write("\nAX:\n")
            for line in lines:
                ax = extract_value(line, "AX")
                file.write(f"{ax}\n")

            file.write("\nAY:\n")
            for line in lines:
                ay = extract_value(line, "AY")
                file.write(f"{ay}\n")

            file.write("\nAZ:\n")
            for line in lines:
                az = extract_value(line, "AZ")
                file.write(f"{az}\n")

        print(f"Data has been successfully written to {output_file_path}")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()