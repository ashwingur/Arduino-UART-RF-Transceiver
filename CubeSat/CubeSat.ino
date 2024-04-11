#include <SoftwareSerial.h>
#include "Zetaplus.h"


// Shutdown pin
const int PIN_SDN = 2;
const int PIN_TX = 4;
const int PIN_RX = 3;


const int BAUD_RATE = 19200;

SoftwareSerial mySerial(PIN_RX, PIN_TX);


void setup() {
  // PC serial monitor
  Serial.begin(9600);

  // RF Transceiver serial
  mySerial.begin(BAUD_RATE);

  pinMode(PIN_SDN, OUTPUT);
  digitalWrite(PIN_SDN, LOW);

  delay(1000);
  ATR(0, 17);
  delay(20);
  ATM(1);
  delay(20);
}

void loop() {

  Transmit(0, 16, "Hello from space");
  
  // Serial.println("Command Sent");
  delay(150);
  Serial.print("Received: ");
  int i = 0;
  while (mySerial.available()) {
    char receivedChar = mySerial.read();
    if (i == 2 || i == 3){
      Serial.print(receivedChar, DEC);
    } else {
      Serial.print(receivedChar);
    }
    i++;
  }
  Serial.println();
  delay(1000);
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
