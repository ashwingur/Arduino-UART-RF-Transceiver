import serial
import struct
import time
from enum import Enum, unique

@unique
class MessageType(Enum):
    WOD = 1
    SCIENCE_IMAGE = 2
    SCIENCE_THERMO_AND_CURRENT = 3
    GROUND_STATION_COMMAND = 4
    TIME = 5
    PONG = 6

@unique
class CommandType(Enum):
    REQUEST_WOD = 1
    REQUEST_SCIENCE_IMAGE = 2
    REQUEST_SCIENCE_THERMO_AND_CURRENT = 3
    SEND_PING = 4
    REQUEST_TIME = 5
    SET_TIME = 6
    SET_OPERATING_MODE = 7
    CLEAR_STORAGE_DATA = 8
    ACTIVATE_PAYLOAD_STRIKING_MECHANISM = 9
    PERFORM_SCIENCE_MEASUREMENT = 10



class OBCCommunication:
    def __init__(self, uart_port='/dev/ttyS4', baud_rate=19200, timeout=2, channel=0, packet_length=64, SSID="CUBE") -> None:
        try:
            self.ser = serial.Serial(uart_port, baud_rate, timeout=timeout)
            self.SSID = SSID
            self.packet_length = packet_length
            self.channel = channel
        except serial.SerialException as e:
            print("Serial port error:", e)
        finally:
            # Initialise transceiver
            # Set to receive mode on channel 0 with packet sizes of 64
            self.ATR(channel, packet_length)
            time.sleep(0.02)
            self.ATM(1)
            time.sleep(0.02)

    def __del__(self):
        self.ser.close()

    # Check if we've received a ground station command and process accordingly
    def receiveTransmission(self):
        if self.ser.in_waiting == 0:
            return

        data = self.ser.read(self.ser.in_waiting)
        print(data.decode(errors='ignore').strip())

        if len(data) < 64:
            return

        # Validate header byte of a new packet
        if data[:2] != "#R":
            return

        # Read packet length and signal strength
        packet_length, signal_strength = struct.unpack("BB", data[2:4])
        print(
            f'packet length: {packet_length}, signal_strength: {signal_strength}')

        ### Now read the actual packet contents
        data = data[4:] # Trim out the header stuf from zetaplus
        # Check the target address matches the CubeSat's address
        target_address = data[:4].decode(errors="ignore")
        if target_address != self.SSID:
            return

        msg_type = struct.unpack("<B", data[4:5])[0]

        # As we are the cubesat, we should only be accepting command message types from the ground station
        if msg_type != MessageType.GROUND_STATION_COMMAND.value:
            return
        
        cmd_type = struct.unpack("<B", data[5:6])[0]

        if cmd_type == CommandType.REQUEST_WOD.value:
            print("CommandType is REQUEST_WOD")

        elif cmd_type == CommandType.REQUEST_SCIENCE_IMAGE.value:
            print("CommandType is REQUEST_SCIENCE_IMAGE")
        elif cmd_type == CommandType.REQUEST_SCIENCE_THERMO_AND_CURRENT.value:
            print("CommandType is REQUEST_SCIENCE_THERMO_AND_CURRENT")
        elif cmd_type == CommandType.SEND_PING.value:
            print("CommandType is SEND_PING")
            self.CMD_ping(data)
        elif cmd_type == CommandType.REQUEST_TIME.value:
            print("CommandType is REQUEST_TIME")
        elif cmd_type == CommandType.SET_TIME.value:
            print("CommandType is SET_TIME")
        elif cmd_type == CommandType.SET_OPERATING_MODE.value:
            print("CommandType is SET_OPERATING_MODE")
        elif cmd_type == CommandType.CLEAR_STORAGE_DATA.value:
            print("CommandType is CLEAR_STORAGE_DATA")
        elif cmd_type == CommandType.ACTIVATE_PAYLOAD_STRIKING_MECHANISM.value:
            print("CommandType is ACTIVATE_PAYLOAD_STRIKING_MECHANISM")
        elif cmd_type == CommandType.PERFORM_SCIENCE_MEASUREMENT.value:
            print("CommandType is PERFORM_SCIENCE_MEASUREMENT")
        else:
            print("Unknown CommandType")

    
    ''' For all CMD functions, bytes contains all the data AFTER the command type, ie any additional arguments'''

    def CMD_request_wod(self, data: bytes):
        pass

    def CMD_request_science_image(self, data: bytes):
        pass

    def CMD_request_science_reading(self, data: bytes):
        pass

    def CMD_ping(self, data: bytes):
        response_contents = struct.pack('B', MessageType.PONG.value)
        self.downlink_header_packet(response_contents)

    def CMD_request_time(self, data: bytes):
        pass

    def CMD_set_time(self, data: bytes):
        pass

    def CMD_set_operating_mode(self, data: bytes):
        pass

    def CMD_clear_storage_data(self, data: bytes):
        pass

    def CMD_activate_payload_strike(self, data: bytes):
        pass

    def CMD_perform_science_measurement(self, data: bytes):
        pass

    # All contents to be sent in the header packet AFTER SSID
    def downlink_header_packet(self, contents: bytes):
        data = (b'USYD' + contents).ljust(self.packet_length, b'\x00')
        self.ser.write("ATS")
        self.ser.write(struct.pack("BB", self.channel, self.packet_length))
        self.ser.write(data)

        

    def downlink_information_packets(self, contents: bytes):
        pass
        
    def downlink_information_packet(self, contents: bytes):
        pass


    def ATR(self, channel, packet_length):
        '''
            Receive mode config
            Channel: 0-15
            Packet Length: 1-64
        '''
        self.ser.write("ATR")
        self.ser.write(struct.pack("BB", channel, packet_length))


    def ATM(self, mode):
        '''
            Operating Mode
            1: RX mode
            2: Default state, fast switch between TX and RX
            3: Sleep
        '''
        self.ser.write("ATM")
        self.ser.write(struct.pack("B", mode))

