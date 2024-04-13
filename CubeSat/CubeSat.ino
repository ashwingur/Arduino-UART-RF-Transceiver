#include <SoftwareSerial.h>
#include "credentials.h"

// Shutdown pin
const int PIN_SDN = 2;
const int PIN_TX = 4;
const int PIN_RX = 3;

// Default Microcontroller <-> Transceiver Baud Rate
const int BAUD_RATE = 19200;

// UART connection to transceiver
// SoftwareSerial rfSerial(PIN_RX, PIN_TX);

// Initialise Zetaplus transceiver to receive 64 byte packets on channel 1
// Sets transceiver to RX mode (Transmit still works in RX mode, it reverts back to RX)
// BAUD_RATE is from Arduino <-> Transceiver
// Transceiver <---> Transceiver is different
class Zetaplus{

  int baud_rate;
  SoftwareSerial rfSerial;

  enum MessageType : uint8_t {
        WOD = 1,
        SCIENCE = 2,
        COMMAND = 3,
        PONG = 4,
  };
  enum CommandType : uint8_t {
        REQUEST_WOD = 1,
        REQUEST_SCIENCE_CURRENT = 2,
        REQUEST_SCIENCE_IMAGE = 3,
        PING = 4,
        REQUEST_TIME = 5,
        SET_TIME = 6,
  };

  public:
  Zetaplus(uint32_t baud_rate): baud_rate(), rfSerial(PIN_RX, PIN_TX){
    this->baud_rate = baud_rate;
  }

  void InitialiseTransceiver(){
    Serial.println("Initialising Transceiver");
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
  void SendUserCommand(){
    if (Serial.available() > 0) { // Check if data is available to read
      String input = Serial.readStringUntil('\n'); // Read the input until newline character
      
      // Check the received input and perform actions accordingly
      if (input == "ping") {
        // Action for Option1
        Serial.println("Sending Ping");
        // Transmit(0, 4, "Ping");
        SendPing();
      } else if (input == "time") {
        // Action for Option2
        Serial.println("Time Command");
        Transmit(0, 4, "Time");
      } else {
        // Invalid option
        Serial.println("Invalid command");
      }
    } 
  }

  void SendPing(){
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::COMMAND;
    packet[5] = CommandType::PING;
    Transmit(0, 64, packet);
  }
  void SendPong(){
    byte packet[64];
    memset(packet, 0, 64);
    memcpy(packet, TARGET_ADDRESS, 4);
    packet[4] = MessageType::PONG;
    Transmit(0, 64, packet);
  }

  // Check if a new transmission has been received
  // This is the first packet of a new transmission, which will contain data about later packets if
  // the data cannot fit in 64 bytes
  void ReceiveNewTransmission(){
    if (rfSerial.available() > 0) { // Check if data is available to read
      Serial.print("RF avaialble: ");
      Serial.println(rfSerial.available());
      char byte1 = rfSerial.read();
      char byte2 = rfSerial.read();
      // Serial.print(byte1);
      // Serial.print(byte2);
      if (byte1 == '#' && byte2 == 'R') {
        // Read packet length
        uint8_t packetLength = rfSerial.read();
        
        // Read signal strength
        uint8_t RSSI = rfSerial.read();
        
        // Read data packets
        byte data[packetLength];
        for (int i = 0; i < packetLength; i++) {
          data[i] = rfSerial.read();
          // Allow the uart buffer to fill up. Without a delay we empty it too quick
          delayMicroseconds(200);
        }

        Serial.print("Packet Received - length: ");
        Serial.print(packetLength);
        Serial.print(", RSSI: ");
        Serial.println(RSSI);
        Serial.print("Data: ");
        PrintByteArray(data, packetLength);
        // Process the received data
        // ProcessCommand(packetLength, signalStrength, data);
        ProcessNewCommand(packetLength, data);
      }
    }
  }

  /*
    Process the first packet of a new transmission that was received
    Depending on the command contained in the packet, further packets will be expected
    and processed accordingly
  */
  void ProcessNewCommand(uint8_t packet_length, byte *data){
    // First 4 bytes is the address of the message
    // It should match with the current transceiver's ID
    if (!(data[0] == MY_ADDRESS[0] && 
        data[1] == MY_ADDRESS[1] && 
        data[2] == MY_ADDRESS[2] && 
        data[3] == MY_ADDRESS[3])){
      Serial.print("Received a message with incorrect ID: ");
      Serial.print((char)data[0]);
      Serial.print((char)data[1]);
      Serial.print((char)data[2]);
      Serial.println((char)data[3]);
      return;
    }

    Serial.println("Processing new command");
    
    // Next byte is the message type
    MessageType msgType = static_cast<MessageType>(data[4]);
    Serial.println(msgType);

    switch (msgType) {
    case WOD:
      Serial.println("Receiving WOD message");
      break;
    case SCIENCE:
      Serial.println("Receiving SCIENCE message");
      break;
    case COMMAND:
      Serial.println("Receiving COMMAND message");
      CommandType commandType = static_cast<CommandType>(data[5]);
      if (commandType == PING){
        // Send back a PONG
        Serial.println("Received Ping, sending back a Pong...");
        SendPong();
      }
      break;
    case PONG:
      Serial.println("Received PONG message");
      break;
    default:
      Serial.println("Unknown message type received");
      break;
    }


  }

  void PrintByteArray(byte* data, uint8_t length) {
    for (int i = 0; i < length; i++) {
      Serial.print((char) data[i]); // Cast byte to integer before printing
      // Serial.print(" "); // Add space between values
    }
    Serial.println(); // Print a newline character after printing the array
  }


  /*
    Operating Mode 
    1: RX mode
    2: Default state, fast switch between TX and RX
    3: Sleep
  */
  void ATM(uint8_t mode){
    // Operating
    rfSerial.write("ATM");
    rfSerial.write(mode);
  }

  /*
    Receive mode config
    Channel: 0-15
    Packet Length: 1-64
  */
  void ATR(uint8_t channel, uint8_t packet_length){
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
  void Transmit(uint8_t channel, uint8_t packet_length, byte *data){
    rfSerial.write("ATS");
    rfSerial.write((byte) channel);
    rfSerial.write(packet_length);
    for (size_t i = 0; i < packet_length; i++){
      rfSerial.write(data[i]);
    }
  }

  // RSSI Value
  void ATQ(){
    rfSerial.write("ATQ");
  }

  // Retrieve Currant Configuration and Settings
  void ATQuestion(){
    rfSerial.write("AT?");
  }

  // Enable command response. Doesnt seem to work??
  void ATC(bool enable){
    rfSerial.write(65);
    rfSerial.write(84);
    rfSerial.write(67);
    if (enable){
      rfSerial.write(1);
    } else {
      rfSerial.write((byte) 0);
    }
  }

};


// Create a zetaplus instance
Zetaplus zetaplus(BAUD_RATE);


void setup() {
  // PC serial monitor
  Serial.begin(9600);
  
  // Initialise transceiver to ready it for RX and TX
  zetaplus.InitialiseTransceiver();
}

void loop() {
  // Send a command if one was entered
  zetaplus.SendUserCommand();
  // Receive a command and process it
  zetaplus.ReceiveNewTransmission();
}




