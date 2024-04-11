#define _SS_MAX_RX_BUFF 128
#include <SoftwareSerial.h>

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
        Transmit(0, 4, "Ping");
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

  // Check if a new transmission has been received
  // This is the first packet of a new transmission, which will contain data about later packets if
  // the data cannot fit in 64 bytes
  void ReceiveNewTransmission(){
    if (rfSerial.available() > 0) { // Check if data is available to read
      Serial.print("RF avaialble: ");
      Serial.println(rfSerial.available());
      char byte1 = rfSerial.read();
      char byte2 = rfSerial.read();
      Serial.print(byte1);
      Serial.print(byte2);
      // Serial.println(rfSerial.read(), );
      // Serial.println(rfSerial.read());
      // Serial.println(rfSerial.read());
      // Read incoming data until the starting pattern "#R" is found
      // String input = rfSerial.readStringUntil('#');
      // Serial.println(input);
      if (byte1 == '#' && byte2 == 'R') {
        // Read packet length
        uint8_t packetLength = rfSerial.read();
        
        // Read signal strength
        uint8_t RSSI = rfSerial.read();
        
        // Read data packets
        byte data[packetLength];
        for (int i = 0; i < packetLength; i++) {
          data[i] = rfSerial.read();
          // Allow the 
          delayMicroseconds(200);
        }

        Serial.print("Packet Received - length: ");
        Serial.print(packetLength);
        Serial.print(", RSSI: ");
        Serial.println(RSSI);
        Serial.print("Data: ");
        PrintByteArray(data, packetLength);
        // Process the received data
        // processCommand(packetLength, signalStrength, data);
      }
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



Zetaplus zetaplus(BAUD_RATE);


void setup() {
  // PC serial monitor
  Serial.begin(9600);
  
  // InitialiseTransceiver(BAUD_RATE);
  zetaplus.InitialiseTransceiver();
}

void loop() {
  // Send a command if one was entered
  zetaplus.SendUserCommand();
  // Receive a command and process it
  zetaplus.ReceiveNewTransmission();
}