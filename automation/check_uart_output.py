import serial
import time

ser_1_port = "/dev/ttyAlifHEDUT1"
ser_2_port = "/dev/ttyAlifHEDUT2"

def configure_serial(port):
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
        print(f"Error opening serial connection ({port}): {e}")
        return None

def read_output(ser, device_name):

    if ser is None:
        print(f" {device_name}: Serial connection is not available.")
        return
    ser.write(b"help\n") 
    time.sleep(1)  
    lines = ser.readlines() 
    for line in lines:
        if b"Available commands" in line:  
            print(f" {device_name}: Great, device responded correctly!")             
            return  

    print(f" {device_name}: No devices up\n")

ser_1 = configure_serial(ser_1_port)
ser_2 = configure_serial(ser_2_port)
if ser_1:
    read_output(ser_1, "Board 1")
    ser_1.close()  
if ser_2:
    read_output(ser_2, "Board 2")
    ser_2.close() 
