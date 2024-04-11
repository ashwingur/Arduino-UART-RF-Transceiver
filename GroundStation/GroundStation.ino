#include <SoftwareSerial.h>

// Shutdown pin
const int PIN_SDN = 2;
const int PIN_TX = 4;
const int PIN_RX = 3;

// Default Microcontroller <-> Transceiver Baud Rate
const int BAUD_RATE = 19200;

// UART connection to transceiver
// SoftwareSerial mySerial(PIN_RX, PIN_TX);

// Initialise Zetaplus transceiver to receive 64 byte packets on channel 1
// Sets transceiver to RX mode (Transmit still works in RX mode, it reverts back to RX)
// BAUD_RATE is from Arduino <-> Transceiver
// Transceiver <---> Transceiver is different
class Zetaplus{

  int baud_rate;
  SoftwareSerial mySerial;

  public:
  Zetaplus(uint32_t baud_rate): baud_rate(), mySerial(PIN_RX, PIN_TX){
    this->baud_rate = baud_rate;
  }

  void InitialiseTransceiver(){
    Serial.println("Initialising Transceiver");
    // RF Transceiver serial
    mySerial.begin(baud_rate);

    pinMode(PIN_SDN, OUTPUT);
    digitalWrite(PIN_SDN, LOW);

    delay(100);
    ATR(0, 64);
    delay(20);
    ATM(1);
    delay(20);
  }

  // Process command from PC serial
  void ProcessUserCommand(){
    if (Serial.available() > 0) { // Check if data is available to read
    String input = Serial.readStringUntil('\n'); // Read the input until newline character
    
    // Check the received input and perform actions accordingly
    if (input == "Option1") {
      // Action for Option1
      Serial.println("Option1 selected");
    } else if (input == "Option2") {
      // Action for Option2
      Serial.println("Option2 selected");
    } else if (input == "Option3") {
      // Action for Option3
      Serial.println("Option3 selected");
    } else if (input == "Option4") {
      // Action for Option4
      Serial.println("Option4 selected");
    } else if (input == "Option5") {
      // Action for Option5
      Serial.println("Option5 selected");
    } else {
      // Invalid option
      Serial.println("Invalid option");
    }
  }
  }


  /*
    Operating Mode 
    1: RX mode
    2: Default state, fast switch between TX and RX
    3: Sleep
  */
  void ATM(uint8_t mode){
    // Operating
    mySerial.write("ATM");
    mySerial.write(mode);
  }

  /*
    Receive mode config
    Channel: 0-15
    Packet Length: 1-64
  */
  void ATR(uint8_t channel, uint8_t packet_length){
    mySerial.write("ATR");
    mySerial.write(channel);
    mySerial.write(packet_length);
  }

  /*
    Transmit a data packet
    Channel: 0-15
    Packet Length: 1-64
    Data: 1-64 bytes of data corresponding to packet length
  */
  void Transmit(uint8_t channel, uint8_t packet_length, byte *data){
    mySerial.write("ATS");
    mySerial.write((byte) channel);
    mySerial.write(packet_length);
    for (size_t i = 0; i < packet_length; i++){
      mySerial.write(data[i]);
    }
  }

  // RSSI Value
  void ATQ(){
    mySerial.write("ATQ");
  }

  // Retrieve Currant Configuration and Settings
  void ATQuestion(){
    mySerial.write("AT?");
  }

  // Enable command response. Doesnt seem to work??
  void ATC(bool enable){
    mySerial.write(65);
    mySerial.write(84);
    mySerial.write(67);
    if (enable){
      mySerial.write(1);
    } else {
      mySerial.write((byte) 0);
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

  zetaplus.Transmit(0, 17, "Hello from ground");
  
  // // Serial.println("Command Sent");
  // delay(150);
  // Serial.print("Received: ");
  // while (mySerial.available()) {
  //   char receivedChar = mySerial.read();
  //   Serial.print(receivedChar);
  // }
  // Serial.println();

  delay(1000);
}




