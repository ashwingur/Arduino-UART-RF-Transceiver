'''
For intercepting the serial communication of the arduino to the PC. This will allow
us to process the data and save to excel/image files as required
'''

import time
import serial
import threading
import struct
import csv
from datetime import datetime, timedelta
from PIL import Image

COM_PORT = "COM3"
BAUD_RATE = 115200

# Function to continuously read data from serial port
def serial_read(serial_port):
    while True:
        # data = serial_port.readline().decode().strip()
        data = serial_port.readline()
        if data:
            if "New Received" in data.decode(errors="ignore").strip():
                # Next line contains a new command data
                data_contents = serial_port.readline()
                process_header_data_contents(serial_port, data_contents)
            print(f'Arduino: {data.decode(errors="ignore").strip()}')

# Function to continuously send data to serial port
def serial_write(serial_port):
    while True:
        message = input("")
        if not message:
            continue
        if "settime" in message:
            args = message.split()
            if len(args) == 1:
                # automatically pass in the current timestamp value
                timestamp = current_time_to_seconds()
            elif len(args) == 2:
                # parse the given timestamp value and send it
                timestamp = parse_datetime_to_seconds(args[1])
            else:
                print("Invalid gettime command")
                continue
            # print(f'timestamp is: {timestamp}')
            serial_port.write("settime ".encode())
            serial_port.write(str(timestamp).encode())
            serial_port.write(b'\n')
        elif "getimage" in message:
            args = message.split()
            # If we are given 4 arguments, then use live timestamp
            # Otherwise use the given timestamp
            if len(args) == 4:
                timestamp = current_time_to_seconds()
                camera = args[1]
                resume_packet_number = args[2]
                packets_to_send = args[3]
            elif len(args) == 5:
                timestamp = parse_datetime_to_seconds(args[1])
                camera = args[2]
                resume_packet_number = args[3]
                packets_to_send = args[4]
            else:
                print("Incorrect getimage args provided")
                continue
            
            if camera == 'left':
                camera_num = 0
            elif camera == 'right':
                camera_num = 1
            else:
                print(f"Camera should be 'left' or 'right' but '{camera}' was provided")
                continue
            print(f'{camera}: {camera_num}')

            serial_port.write(f'getimage {timestamp} {camera_num} {resume_packet_number} {packets_to_send}\n'.encode())

        else:
            serial_port.write(message.encode())


def process_header_data_contents(serial_port: serial.Serial, data):
    # first 4 bytes are the USYD callsign which has already been verified by the arduino
    # skip these 4 bytes
    data = data[4:]

    # Check what the message type is
    msg_type = struct.unpack("<B", data[:1])[0]

    # Process accordingly to the message type (defined by MessageType enum in the arduino script)
    if msg_type == 1:
        # WOD
        additional_packets = b''
        data = serial_port.readline() # This is an additional print in the arduino
        data = serial_port.readline()
        if data.decode().strip() == "<WOD Message>":
            data = serial_port.readline()
            iterations = 0
            while iterations < 10: # So we dont go into infinite loop in case cubesat sends bad data
                # Read all the wod packets, there should be 5
                data = serial_port.readline()
                if data.decode().strip() == "<WOD Message/>":
                    break
                    
                additional_packets += data
                iterations += 1
            process_wod(additional_packets)

    elif msg_type == 2:
        # SCIENCE_IMAGE
        print("Science image received")
        # Process the header data
        camera_number = struct.unpack("<B", data[1:2])[0]
        img_width, img_height = struct.unpack("<hh", data[2:6])
        img_time = struct.unpack("<I", data[6:10])[0]
        start_packet_number = struct.unpack("<h", data[10:12])[0]
        print(f'camera number: {camera_number}, img dimensions: {img_width}x{img_height}, time_taken: {parse_seconds_to_datetime(img_time)}, start packet number: {start_packet_number}')

        # Now process the information packets
        image_bytes = process_image_content_stream(serial_port)
        print(f'len bytes: {len(image_bytes)}')
        # There may be some excess bytes in the final packet, be sure to ignore those
        image_bytes = image_bytes[:img_width*img_height]
        image = reconstructImage(image_bytes, img_width, img_height)
        save_and_display_image(image, f"images/{img_time}_{'left' if camera_number == 0 else 'right'}")


    elif msg_type == 3:
        # SCIENCE_THERMO_AND_CURRENT
        print("Science thermocouple and current received")
        pass
    elif msg_type == 4:
        # GROUND_STATION_COMMAND (this should never be sent as we are the ground station)
        print("Ground station command received, CubeSat should not be sending one. Ignoring the rest of the packet")
    elif msg_type == 5:
        # TIME
        # the next 4 bytes represent a uint32t time value
        time = struct.unpack("<I", data[1:5])[0]
        print(f"Current CubeSat time: {parse_seconds_to_datetime(time)}")
        pass
    elif msg_type == 6:
        #PONG
        print("Pong received!")
        pass
    else:
        print("Invalid message type received, ignoring the rest of the packet.")

def process_image_content_stream(serial_port: serial.Serial) -> bytes:
    image_bytes = b''

    data = serial_port.readline() # This is an additional print in the arduino
    data = serial_port.readline()
    if data.decode().strip() == "<Science Image>":
        data = serial_port.read(64)
        while True:
            # First 2 bytes are the packets remaining
            packets_remaining = struct.unpack("<H", data[:2])
            print(f'packets remaining: {packets_remaining}')
            data = serial_port.read(64)
            print(f'image data: {data}')
            if len(data) < 64 or data.decode(errors='ignore').strip() == "<Science Image/>":
                break
            image_bytes += data[3:]
    print("Finished reading all image bytes")
    return image_bytes

def reconstructImage(bytes: bytes, width: int, height: int):
    # Create an empty image with the specified width and height
    image = Image.new("L", (width, height))
    # Load the byte stream into the image
    image.putdata(bytes)
    return image

def save_and_display_image(image, file_name):
    # Save the image as PNG
    image.save(file_name + ".png")
    image.show()    


def process_wod(packets):
    print("Processing WOD...")
    chunk_size = 64
    contents = b''
    # WOD fits in 5 packets
    # We want to loop through and extract the actual data (we dont really need packets remaining and datatype here, but its there for debugging)
    for i in range(0, 5):
        contents += packets[chunk_size*i+3:(chunk_size*(i+1))]

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
        print("WOD saved to wod.csv")

def current_time_to_seconds():
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Get the current time
    current_time = datetime.utcnow()
    # Calculate the time difference
    time_difference = current_time - reference_epoch
    # Convert the time difference to seconds
    seconds_since_epoch = int(time_difference.total_seconds())
    return seconds_since_epoch

def parse_seconds_to_datetime(seconds, sydney_zone=True):
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Add the given number of seconds to the reference epoch
    target_datetime = reference_epoch + timedelta(seconds=seconds)

    # Convert to sydney time if requested
    if sydney_zone:
        # Set timezone to Sydney
        target_datetime += timedelta(seconds=10*3600)

    # Format the datetime object
    formatted_datetime = target_datetime.strftime("%I:%M:%S%p %dth %B %Y")  # Example: 6:39PM 26th April 2024
    return formatted_datetime

def parse_datetime_to_seconds(datetime_str, sydney_zone=True):
    # Parse the datetime string into a datetime object
    dt = datetime.strptime(datetime_str, "%d-%m-%Y-%H-%M-%S")
    # Reference epoch (01/01/2000 00:00:00 UTC)
    reference_epoch = datetime(2000, 1, 1, 0, 0, 0)
    # Calculate the time difference
    time_difference = dt - reference_epoch
    # Convert to sydney time if requested
    if sydney_zone:
        # Convert sydney to utc + 0
        time_difference -= timedelta(seconds=10*3600)
    # Convert the time difference to seconds
    seconds_since_epoch = int(time_difference.total_seconds())
    return seconds_since_epoch

# Open serial port
ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=5)

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
