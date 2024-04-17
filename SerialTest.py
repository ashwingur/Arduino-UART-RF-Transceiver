'''
For intercepting the serial communication of the arduino to the PC. This will allow
us to process the data and save to excel/image files as required
'''

import time
import serial
import threading
import struct
import csv

COM_PORT = "COM4"
BAUD_RATE = 115200

# Function to continuously read data from serial port


def serial_read(serial_port):
    additional_packets = b''
    reading_additional_data = False
    while True:
        # data = serial_port.readline().decode().strip()
        data = serial_port.readline()
        if data:
            if data.decode().strip() == "<WOD Message>":
                reading_additional_data = True
            elif data.decode().strip() == "<WOD Message/>":
                reading_additional_data = False
                process_wod(additional_packets)
                additional_packets = b''
            elif reading_additional_data:
                additional_packets += data
            # for byte in data:
            #     print(f'{int(byte)},',end='')
            # print()
            print(f'Decoded and stripped: {data.decode().strip()}\n')

# Function to continuously send data to serial port


def serial_write(serial_port):
    while True:
        message = input("")
        if not message:
            break
        serial_port.write(message.encode())


def process_wod(packets):
    chunk_size = 64
    data_format = '<HB' + 'B'*61
    contents = b''
    # WOD fits in 5 packets
    # We want to loop through and extract the actual data (we dont really need packets remaining and datatype here, but its there for debugging)
    for i in range(0, 5):
        contents += packets[chunk_size*i+3:(chunk_size*(i+1))]
        # packet = packets[chunk_size*i:(chunk_size*(i+1))]
        # unpacked_data = struct.unpack(data_format, packet)
        # print(f'Packets remaining: {unpacked_data[0]}, Msg type: {unpacked_data[1]}, data: {unpacked_data[2:64]}')

    # Parse contents. 4 byte unsigned int, then 8x32 bytes unsigned chars
    # Since there is extra space left on the last packet, truncate that so we can unpack exactly what we need
    unpacked_data = struct.unpack('<I' + 'B'*8*32, contents[0:8*32+4])
    time = unpacked_data[0]
    print(time)
    # Write to a csv file
    with open('wod.csv', 'w', newline='') as csvfile:
        header = ['time', 'mode', 'bat_voltage', 'bat_current', 'bus_3v_current',
                  'bus_5v_current', 'temp_comm', 'temp_eps', 'temp_battery']
        writer = csv.writer(csvfile)
        writer.writerow(header)
        for i in range(0, 32):
            # Increment by 1 minute for each iteration
            time_column = [time + 60*i]
            data_row = [unpacked_data[8*i + j + 1] for j in range(8)]
            # Check the mode value. It should only be 0 or 1
            # If it is another value then set it to -1 because it's invalid
            if data_row[0] > 1:
                data_row[0] = -1

            # mode            = unpacked_data[8*i + 1]
            # bat_voltage     = unpacked_data[8*i + 2]
            # bat_current     = unpacked_data[8*i + 3]
            # bus_3v_current  = unpacked_data[8*i + 4]
            # bus_5v_current  = unpacked_data[8*i + 5]
            # temp_comm       = unpacked_data[8*i + 6]
            # temp_eps        = unpacked_data[8*i + 7]
            # temp_battery    = unpacked_data[8*i + 8]
            # print(f"{mode}, {bat_voltage}, {bat_current}, {bus_3v_current}, {bus_5v_current}, {temp_comm}, {temp_eps}, {temp_battery}")
            writer.writerow(time_column + data_row)


# Open serial port
ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)

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
