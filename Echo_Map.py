import numpy as np
import serial
import math

map = [[]]
baseline = 100
x, y = 1
old_position_marker = '0'

# Create an empty map 
def create_map_empty():
    global map
    map = np.array(baseline, baseline)

    for i in range(0, baseline):
        for j in range(0, baseline):
            map[i][j] = '0'

# Print map array with map details
def print_map():
    global map

    # print map  
    for i in range(0, baseline):
        for j in range(0, baseline):
            print(map[i][j], end=None)

        print("")

# Initialize device and optionally baseline
def init_serial(port="COM3", baud_rate=9600, bl=100):
    global device
    global baseline

    try:
        device = serial.Serial(port, baud_rate, timeout=.1)
        baseline = bl

    except Exception as error:
        print(error)
        return 0

# Get object location and update map    
def locate():
    # Verify device was found
    if device == 0:
        print("Device not found")
        return 0
    
    try:
        global map
        global baseline

        if device.in_waiting > 18:
            # Read data until newline
            data = device.readline().decode('utf-8').split(",")

            a = float(data[1]) - float(data[0]);        # d2
            b = float(data[0]);                         # d1
            c = baseline;                               # baseline
            d = math.sqrt(baseline*baseline);           # diagonal
            s = (a + b + c)/2;                          # semi-perimeter

            # validate distances
            distances_valid = True

            # d1 must be less than d2
            # d1 out-of-range
            # d2 out-of-range
            # these values must be positive
            if (a < 0) or (b > d) or (a > d) or ((s - a) < 0) or ((s - b) < 0) or ((s - c) < 0):
                distances_valid = False

            # apply Heron's formula
            if distances_valid:
                global old_position_marker

                # Replace object marker with old marker
                map[x][y] = old_position_marker

                # Calculate object position
                area = math.sqrt(s * (s - a) * (s - b) * (s - c))
                y = area * 2 / c
                x = math.sqrt(b * b - y * y)
                
                # Save old marker and replace with object marker in new position
                old_position_marker = map[x][y]                
                map[x][y] = '1'

                # display data and updated map
                print("\n------------------------")
                print_map()
                print("d1:", b, "x:", x) 
                print("d2:", a, "y:", y)

            else:
                # display data
                print("d1:", b, "x:", x) 
                print("d2:", a, "y:", y)

    except Exception as error:
        print(error)
        return 0

# Main function
try:
    init_serial()
    create_map_empty()
    
    while(True):
        locate()

except KeyboardInterrupt:
    print("\nstopping")
