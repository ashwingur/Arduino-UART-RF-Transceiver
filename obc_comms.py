import serial
import struct
import time


class OBCCommunication:

    def __init__(self, uart_port='/dev/ttyS4', baud_rate=19200, timeout=2, channel=0, packet_length=64, SSID="CUBE") -> None:
        try:
            self.ser = serial.Serial(uart_port, baud_rate, timeout=timeout)
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

        # Validate header byte of a new packet
        if data[:2] != "#R":
            return

        # Read packet length and signal strength
        packet_length, signal_strength = struct.unpack("BB", data[2:4])
        print(
            f'packet length: {packet_length}, signal_strength: {signal_strength}')

        # Now read the actual packet contents
        data = data[4:]

        # TODO

    '''
        Receive mode config
        Channel: 0-15
        Packet Length: 1-64
    '''

    def ATR(self, channel, packet_length):
        self.ser.write("ATR")
        self.ser.write(struct.pack("BB", channel, packet_length))

    '''
        Operating Mode
        1: RX mode
        2: Default state, fast switch between TX and RX
        3: Sleep
    '''

    def ATM(self, mode):
        self.ser.write("ATM")
        self.ser.write(struct.pack("B", mode))

        # Initialise transceiver

        # Define the UART port and baud rate
        # uart_port = '/dev/ttyS4'  # Change this according to your PocketBeagle configuration
        # baud_rate = 9600  # Change this according to your UART settings

        # try:
        #     # Open serial connection
        #     ser = serial.Serial(uart_port, baud_rate)
        #     print("Serial connection established")

        #     ser.write("hello world!\n".encode())

        #     # Reading from UART
        #     while True:
        #         data = ser.readline().decode().strip()
        #         print("Received:", data)

        # except serial.SerialException as e:
        #     print("Serial port error:", e)

        # finally:
        #     # Close serial connection
        #     if ser.is_open:
        #         ser.close()
        #         print("Serial connection closed")
