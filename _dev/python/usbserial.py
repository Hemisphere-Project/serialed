import serial

with serial.Serial('/dev/ttyUSB0', 921600, timeout=2) as ser:
    while ser.is_open:
        line = ser.readline()
        print(line)
