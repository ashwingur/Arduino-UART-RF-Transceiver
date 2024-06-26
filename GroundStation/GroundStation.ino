#include <SoftwareSerial.h>
#include "credentials.h"

// Shutdown pin
const int PIN_SDN = 2;
const int PIN_TX = 4;
const int PIN_RX = 3;

// Default Microcontroller <-> Transceiver Baud Rate
const int BAUD_RATE = 19200;

long current_time = 0;

// UART connection to transceiver
// SoftwareSerial rfSerial(PIN_RX, PIN_TX);

// Initialise Zetaplus transceiver to receive 64 byte packets on channel 1
// Sets transceiver to RX mode (Transmit still works in RX mode, it reverts back to RX)
// BAUD_RATE is from Arduino <-> Transceiver
// Transceiver <---> Transceiver is different
class Zetaplus
{

  int baud_rate;
  SoftwareSerial rfSerial;

  enum MessageType : uint8_t
  {
    WOD = 1,
    SCIENCE_IMAGE = 2,
    SCIENCE_THERMO_AND_CURRENT = 3,
    GROUND_STATION_COMMAND = 4,
    TIME = 5,
    PONG = 6,
    DEBUG = 7,
  };
  enum CommandType : uint8_t
  {
    REQUEST_WOD = 1,
    REQUEST_SCIENCE_IMAGE = 2,
    REQUEST_SCIENCE_THERMO_AND_CURRENT = 3,
    SEND_PING = 4,
    REQUEST_TIME = 5,
    SET_TIME = 6,
    SET_OPERATING_MODE = 7,
    CLEAR_STORAGE_DATA = 8,
    ACTIVATE_PAYLOAD_STRIKING_MECHANISM = 9,
    PERFORM_SCIENCE_MEASUREMENT = 10
  };

public:
  Zetaplus(uint32_t baud_rate) : baud_rate(), rfSerial(PIN_RX, PIN_TX)
  {
    this->baud_rate = baud_rate;
  }

  void InitialiseTransceiver()
  {
    Serial.println();
    Serial.print("Initialising Transceiver, my Address is ");
    Serial.println(MY_ADDRESS);
    // RF Transceiver serial
    rfSerial.begin(baud_rate);

    pinMode(PIN_SDN, OUTPUT);
    digitalWrite(PIN_SDN, LOW);

    // Set to receive mode on channel 0 with packet sizes of 64
    delay(100);
    ATR(0, 64);
    delay(20);
    ATM(1);
    delay(20);
  }

  // Send a user command typed in from the PC
  void SendUserCommand()
  {
    if (Serial.available() > 0)
    {                                              // Check if data is available to read
      String input = Serial.readStringUntil('\n'); // Read the input until newline character

      // Check the received input and perform actions accordingly
      if (input == "ping")
      {
        Serial.println("Sending Ping");
        SendPing();
      }
      else if (input == "wod")
      {
        Serial.println("Requesting WOD");
        RequestWOD();
      }
      else if (input == "clearstorage")
      {
        Serial.println("Sending a clear storage command");
        SendClearStorage();
      }
      else if (input == "payloadstrike")
      {
        Serial.println("Sending a payload strike command");
        SendPayloadStrike();
      }
      else if (input == "sciencemeasurement")
      {
        Serial.println("Sending a science measurement command");
        SendScienceMeasurementCommand();
      }
      else if (input == "gettime")
      {
        Serial.println("Requesting Time");
        RequestTime();
      }
      else if (input.indexOf("settime") != -1)
      {
        // Send the time
        char strBuffer[50];
        long timestamp;

        sscanf(input.c_str(), "%s %ld", strBuffer, &timestamp);
        Serial.print("Setting time to: ");
        Serial.println(timestamp);
        SetTime(timestamp);
      }
      else if (input.indexOf("getimg") != -1)
      {
        char strBuffer[50];
        long timestamp;
        short camera_number;
        int resume_packet;
        int packets_to_send;
        sscanf(input.c_str(), "%s %ld %d %d %d", strBuffer, &timestamp, &camera_number, &resume_packet, &packets_to_send);
        Serial.print("gettimage command, timestamp:");
        Serial.print(timestamp);
        Serial.print(", camera num: ");
        Serial.print(camera_number);
        Serial.print(", resume packet: ");
        Serial.print(resume_packet);
        Serial.print(", packets to send: ");
        Serial.println(packets_to_send);
        RequestScienceImage(timestamp, camera_number, resume_packet, packets_to_send);
      }
      else if (input.indexOf("getsciencereading") != -1)
      {
        // Send the time
        char strBuffer[50];
        long timestamp;

        sscanf(input.c_str(), "%s %ld", strBuffer, &timestamp);
        Serial.println("Requesting science reading...");
        RequestScienceReading(timestamp);
      }
      else
      {
        // Invalid option
        Serial.print("Invalid command: ");
        Serial.println(input);
      }
    }
  }

  void SendPing()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::SEND_PING;
    Transmit(0, 64, packet);
  }

  void SendPong()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::PONG;
    Transmit(0, 64, packet);
  }

  void SendClearStorage()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::CLEAR_STORAGE_DATA;
    Transmit(0, 64, packet);
  }

  void SendPayloadStrike()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::ACTIVATE_PAYLOAD_STRIKING_MECHANISM;
    Transmit(0, 64, packet);
  }

  void SendScienceMeasurementCommand()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::PERFORM_SCIENCE_MEASUREMENT;
    Transmit(0, 64, packet);
  }

  void SendDebug(String msg)
  {
    int str_length = msg.length() > 59 ? 59 : msg.length();
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::DEBUG;
    // memcpy(packet + 5, msg, str_length);
    msg.getBytes(packet + 5, str_length);
    Transmit(0, 64, packet);
  }

  void RequestWOD()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::REQUEST_WOD;
    Transmit(0, 64, packet);
  }

  // Just send fake WOD for testing for now
  void SendWOD()
  {
    // Send first data packet, containing header info
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::WOD;
    Transmit(0, 64, packet);

    // Now send the actual data content packets
    // For (260 bytes) it will take 260/61 = 5 additional packets
    int bytes = 260;
    int n_additional_packets = 5;
    for (int i = 0; i < n_additional_packets; i++)
    {
      // MAKE SURE THIS DELAY IS LESS THAN THE ITERATION TIME TO PROCESS EACH PACKET IN
      // ProcessNewPackets()
      delay(100);
      uint16_t remaining_packets = n_additional_packets - i - 1;
      memcpy(packet + 0, &remaining_packets, sizeof(uint16_t));
      packet[2] = MessageType::WOD;
      // Put some random crap in the data contents
      for (int k = 0; k < 61; k++)
      {
        if (bytes > 0)
        {
          packet[3 + k] = k;
          bytes--;
        }
        else
        {
          packet[3 + k] = 0x00;
        }
      }
      Transmit(0, 64, packet);
      Serial.print("Sending wod addtional packet ");
      Serial.println(i);
    }
  }

  void RequestTime()
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::REQUEST_TIME;
    Transmit(0, 64, packet);
  }

  void SendTime(uint32_t time_to_send)
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::TIME;
    // uint32_t current_time = 766772417; // Using some random testvalue for now
    memcpy(packet + 5, &time_to_send, 4);
    Transmit(0, 64, packet);
  }

  void SetTime(long timestamp)
  {
    current_time = timestamp;
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::SET_TIME;
    memcpy(packet + 6, &timestamp, 4);
    Transmit(0, 64, packet);
  }

  void RequestScienceImage(long timestamp,
                           short camera_number,
                           int resume_packet,
                           int packets_to_send)
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::REQUEST_SCIENCE_IMAGE;
    memcpy(packet + 6, &camera_number, 1);
    memcpy(packet + 7, &resume_packet, 2);
    memcpy(packet + 9, &resume_packet, 2);
    memcpy(packet + 11, &timestamp, 4);
    Transmit(0, 64, packet);
  }

  void ProcessScienceImageRequest(byte *packet)
  {
    long timestamp;
    short camera_number;
    int resume_packet;
    int packets_to_send;

    memcpy(&camera_number, packet + 6, sizeof(short));
    memcpy(&resume_packet, packet + 7, sizeof(int));
    memcpy(&packets_to_send, packet + 9, sizeof(int));
    memcpy(&timestamp, packet + 11, sizeof(long));

    // Print the data to serial in a single line
    Serial.print("Camera Number: ");
    Serial.print(camera_number);
    Serial.print(", Resume Packet: ");
    Serial.print(resume_packet);
    Serial.print(", Packets to Send: ");
    Serial.print(packets_to_send);
    Serial.print(", Timestamp: ");
    Serial.println(timestamp);

    // Send a dummy image
    TransmitDummyImage(camera_number);
  }

  void TransmitDummyImage(short camera_number)
  {
    // Send first data packet, containing header info
    const int img_width = 61;
    const int img_height = 61;
    const int start_packet_number = 0;
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::SCIENCE_IMAGE;
    memcpy(packet + 5, &camera_number, 1);
    memcpy(packet + 6, &img_width, 2);
    memcpy(packet + 8, &img_height, 2);
    memcpy(packet + 10, &current_time, 4);
    memcpy(packet + 14, &start_packet_number, 2);
    Transmit(0, 64, packet);

    // Now send the actual data content packets
    // For 50*50=2500 bytes it will take 2500/61 = 41 additional packets
    int n_additional_packets = 68;
    for (int i = 0; i < n_additional_packets; i++)
    {
      // MAKE SURE THIS DELAY IS LESS THAN THE ITERATION TIME TO PROCESS EACH PACKET IN
      // ProcessNewPackets()
      delay(100);
      uint16_t remaining_packets = n_additional_packets - i - 1;
      memcpy(packet + 0, &remaining_packets, sizeof(uint16_t));
      packet[2] = MessageType::SCIENCE_IMAGE;
      // Put some alternating black n white pixel values
      for (int k = 0; k < 61; k++)
      {
        packet[3 + k] = 255 - 4 * k;
      }
      Transmit(0, 64, packet);
      Serial.print("Sending wod addtional packet ");
      Serial.println(i);
    }
  }

  // For thermocouple and current reading
  void RequestScienceReading(long timestamp)
  {
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::GROUND_STATION_COMMAND;
    packet[5] = CommandType::REQUEST_SCIENCE_THERMO_AND_CURRENT;
    memcpy(packet + 6, &timestamp, 4);
    Transmit(0, 64, packet);
  }
  // Check if a new transmission has been received
  // This is the first packet of a new transmission, which will contain data about later packets if
  // the data cannot fit in 64 bytes
  void ReceiveNewTransmission()
  {
    if (rfSerial.available() > 0)
    { // Check if data is available to read
      // Add a delay so we give some time for the buffer to fill up
      // Transmission rate is much slower than clock speed
      delay(10);
      char byte1 = rfSerial.read();
      char byte2 = rfSerial.read();
      if (byte1 == '#' && byte2 == 'R')
      {
        // Read packet length
        uint8_t packetLength = rfSerial.read();

        // Read signal strength
        uint8_t RSSI = rfSerial.read();

        // Read data packets
        byte data[packetLength];
        for (int i = 0; i < packetLength; i++)
        {
          // Allow the uart buffer to fill up. Without a delay we empty it too quick
          delayMicroseconds(110);
          data[i] = rfSerial.read();
        }

        Serial.print("New Received - length:");
        Serial.print(packetLength);
        Serial.print(", RSSI:");
        Serial.print(RSSI);
        Serial.println(", Data:");
        PrintByteArray(data, packetLength);
        Serial.println();
        // Process the received data
        ProcessNewMessage(data);
      }
    }
  }

  /*
    Process the first packet of a new transmission that was received
    Depending on the command contained in the packet, further packets will be expected
    and processed accordingly
  */
  void ProcessNewMessage(byte *data)
  {
    // First 4 bytes is the address of the message
    // It should match with the current transceiver's ID
    if (!(data[0] == MY_ADDRESS[0] &&
          data[1] == MY_ADDRESS[1] &&
          data[2] == MY_ADDRESS[2] &&
          data[3] == MY_ADDRESS[3]))
    {
      Serial.print("Received a message with incorrect ID: ");
      Serial.print((char)data[0]);
      Serial.print((char)data[1]);
      Serial.print((char)data[2]);
      Serial.println((char)data[3]);
      return;
    }

    // Next byte is the message type
    MessageType msgType = static_cast<MessageType>(data[4]);

    if (msgType == MessageType::WOD)
    {
      Serial.println("Receiving WOD message");
      ProcessAdditionalPackets("WOD Message");
    }
    else if (msgType == MessageType::SCIENCE_IMAGE)
    {
      Serial.println("Receiving SCIENCE IMAGE message");
      ProcessAdditionalPackets("Science Image");
    }
    else if (msgType == MessageType::SCIENCE_THERMO_AND_CURRENT)
    {
      Serial.println("Receiving SCIENCE THERMO AND CURRENT reading message");
    }
    else if (msgType == MessageType::GROUND_STATION_COMMAND)
    {
      Serial.println("Receiving GROUND STATION COMMAND message");
      CommandType commandType = static_cast<CommandType>(data[5]);
      if (commandType == CommandType::SEND_PING)
      {
        // Send back a PONG
        Serial.println("Received Ping, sending back a Pong...");
        SendPong();
      }
      else if (commandType == CommandType::REQUEST_WOD)
      {
        Serial.println("Received WOD request, sending WOD...");
        SendWOD();
      }
      else if (commandType == CommandType::REQUEST_TIME)
      {
        Serial.println("Received gettime request, sending time...");
        SendTime(current_time);
      }
      else if (commandType == CommandType::SET_TIME)
      {
        Serial.println("Received settime request, setting time...");
        current_time = 0;
        for (int i = 0; i < 4; i++)
        {
          current_time |= ((long int)data[i + 6]) << (i * 8);
        }

        Serial.print("Set time to ");
        Serial.println(current_time);
      }
      else if (commandType == CommandType::REQUEST_SCIENCE_IMAGE)
      {
        Serial.println("Received science image request:");
        ProcessScienceImageRequest(data);
      }
      else if (commandType == CommandType::REQUEST_SCIENCE_THERMO_AND_CURRENT)
      {
        Serial.println("Received science thermo and current request");
      }
      else
      {
        Serial.println("Unknown or unimplemented ground station command");
      }
    }
    else if (msgType == MessageType::TIME)
    {
      // The result is already printed out to the ground station computer, dont need anything here unless for debug
      Serial.println("Received current time from CubeSat");
    }
    else if (msgType == MessageType::PONG)
    {
      Serial.println("Received PONG message");
    }
    else if (msgType == MessageType::DEBUG)
    {
      Serial.println("Received DEBUG message");
      PrintString(data + 5, 59);
    }
    else
    {
      Serial.println("Unknown message type received");
    }
  }

  /*
    The first packet has been received. This function is called if additional data packets
    relating to the first packet message are expected.

    These additional datapackets will be of the following format:
    2 bytes - u_int16, number of additional packets to be sent
    1 byte - Message type (WOD, science, command)
    61 bytes - Actual data contents

    The data contents will be printed to serial. An external program can intercept and
    process this serial data into readable output through CSVs etc.

    The input is a string corresponding to the message type expected. This is arbitrary
    and will be printed in the serial output for easier parsing by an external program
    (so we know when the additional packets stop and end)
  */
  void ProcessAdditionalPackets(char *messageName)
  {
    Serial.print("<");
    Serial.print(messageName);
    Serial.println(">");
    unsigned long function_start_time = millis();
    // Add a 50ms timeout between additional datapackets. If this is exceeded then return
    unsigned long timeout = 1000;
    unsigned long time_since_last_packet = 0;
    while (time_since_last_packet < timeout)
    {
      unsigned long start_time = millis();
      if (rfSerial.available() > 0)
      {
        delay(10);
        char starting_byte1 = rfSerial.read();
        char starting_byte2 = rfSerial.read();
        if (!(starting_byte1 == '#' && starting_byte2 == 'R'))
        {
          Serial.println("Invalid starting bytes in additional packets");
          return;
        }

        // Read packet length
        uint8_t packetLength = rfSerial.read();
        // Read signal strength
        uint8_t RSSI = rfSerial.read();

        // Number of packets remaining
        // uint8_t byte1 = rfSerial.read();
        // uint8_t byte2 = rfSerial.read();

        // // Little endian
        // uint16_t remaining_packets = (byte2 << 8) | byte1;

        // MessageType msgType = static_cast<MessageType>(rfSerial.read());

        // Read the data
        byte data[64];
        for (int i = 0; i < 64; i++)
        {
          // Allow the uart buffer to fill up. Without a delay we empty it too quick
          delayMicroseconds(200);
          data[i] = rfSerial.read();
        }
        // Serial.print(remaining_packets);
        // Remaining packets as byte1 and byte2
        // Serial.print(byte1);
        // Serial.print(byte2);
        // Serial.print(msgType);
        PrintByteArray(data, 64);
        // Serial.print("Remaining packets: ");
        // Serial.print(remaining_packets);
        // Serial.print(", Message type: ");
        // Serial.print(msgType);
        // Serial.print(", Data: ");
        // PrintByteArray(data, 61);
        // Serial.print("Iteration took ");
        // Serial.println(millis() - start_time);
        time_since_last_packet = 0;
      }
      time_since_last_packet += millis() - start_time;
    }
    Serial.println();
    Serial.print("<");
    Serial.print(messageName);
    Serial.println("/>");
    Serial.print("Finished processing all additional packets. Time taken: ");
    Serial.print((millis() - function_start_time) / 1000.0);
    Serial.println("s");
  }

  void PrintByteArray(byte *data, uint8_t length)
  {
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)data[i]); // Cast byte to integer before printing
    }
  }

  void PrintString(byte *data, uint8_t length)
  {
    for (int i = 0; i < length; i++)
    {
      Serial.print((char)data[i]);
    }
    Serial.println();
  }

  /*
    Operating Mode
    1: RX mode
    2: Default state, fast switch between TX and RX
    3: Sleep
  */
  void ATM(uint8_t mode)
  {
    // Operating
    rfSerial.write("ATM");
    rfSerial.write(mode);
  }

  /*
    Receive mode config
    Channel: 0-15
    Packet Length: 1-64
  */
  void ATR(uint8_t channel, uint8_t packet_length)
  {
    rfSerial.write("ATR");
    rfSerial.write(channel);
    rfSerial.write(packet_length);
  }

  /*
    Transmit a data packet
    Channel: 0-15
    Packet Length: 1-64
    Data: 1-64 bytes of data corresponding to packet length
  */
  void Transmit(uint8_t channel, uint8_t packet_length, byte *data)
  {
    rfSerial.write("ATS");
    rfSerial.write((byte)channel);
    rfSerial.write(packet_length);
    for (size_t i = 0; i < packet_length; i++)
    {
      rfSerial.write(data[i]);
    }
  }

  // RSSI Value
  void ATQ()
  {
    rfSerial.write("ATQ");
  }

  // Retrieve Currant Configuration and Settings
  void ATQuestion()
  {
    rfSerial.write("AT?");
  }

  // Enable command response. Doesnt seem to work??
  void ATC(bool enable)
  {
    rfSerial.write(65);
    rfSerial.write(84);
    rfSerial.write(67);
    if (enable)
    {
      rfSerial.write(1);
    }
    else
    {
      rfSerial.write((byte)0);
    }
  }
};

// Create a zetaplus instance
Zetaplus zetaplus(BAUD_RATE);

void setup()
{
  // PC serial monitor
  Serial.begin(115200);

  // Initialise transceiver to ready it for RX and TX
  zetaplus.InitialiseTransceiver();
  zetaplus.SendDebug("Hello worldddddd");
}

void loop()
{
  // Send a command if one was entered
  zetaplus.SendUserCommand();
  // Receive a command and process it
  zetaplus.ReceiveNewTransmission();
}
