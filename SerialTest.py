'''
For intercepting the serial communication of the arduino to the PC. This will allow
us to process the data and save to excel/image files as required
'''

import time
import serial
import threading

# Function to continuously read data from serial port
def serial_read(serial_port):
    while True:
        data = serial_port.readline().decode().strip()
        if data:
            print("Received:", data)

# Function to continuously send data to serial port
def serial_write(serial_port):
    while True:
        message = input("")
        if not message:
            break
        serial_port.write(message.encode())

# Open serial port
ser = serial.Serial('COM7', 115200, timeout=1)

# Start reading and writing threads
read_thread = threading.Thread(target=serial_read, args=(ser,), daemon=True)
read_thread.start()

write_thread = threading.Thread(target=serial_write, args=(ser,), daemon=True)
write_thread.start()

# Keep the main thread alive
try:
    
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting...")
    ser.close()

