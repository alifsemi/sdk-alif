import serial
import time
import sys
ser_1_port = "/dev/ttyAlifHEDUT1"
ser_2_port = "/dev/ttyAlifHEDUT2"

def configure_serial(port):
    count = 1
    while count < 4:
        print(f"Connecting device ({port}), attempt {count}")
        try:
            return serial.Serial(
                port=port,
                baudrate=115200,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                bytesize=serial.EIGHTBITS,
                timeout=1
            )
        except serial.SerialException as e:
            print(f"wait for device), attempt {count}")
            time.sleep(5)
        count+=1
    print(f"Error opening serial connection ({port}) after 3 attempts.")
    return None

def read_output(ser, device_name):

    if ser is None:
        print(f" {device_name}: Serial connection is not available.")
        sys.exit(99)
    lines = ser.readlines()
    ser.write(b"\nhelp\n")
    time.sleep(1)
    lines = ser.readlines()
    for line in lines:
        if b"Available commands" in line:
            print(f" {device_name}: Great, device responded correctly!")
            return

    print(f" {device_name}: No devices up\n")
    sys.exit(99)
ser_1 = configure_serial(ser_1_port)
ser_2 = configure_serial(ser_2_port)
if ser_1:
    read_output(ser_1, "Board 1")
    ser_1.close()
if ser_2:
    read_output(ser_2, "Board 2")
    ser_2.close()