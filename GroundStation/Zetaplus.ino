
// // Initialise Zetaplus transceiver to receive 64 byte packets on channel 1
// // Sets transceiver to RX mode (Transmit still works in RX mode, it reverts back to RX)
// // BAUD_RATE is from Arduino <-> Transceiver
// // Transceiver <---> Transceiver is different
// class Zetaplus{

//   int baud_rate;

//   public:
//   Zetaplus(uint32_t baud_rate){
//     this->baud_rate = baud_rate;
//   }

//   void InitialiseTransceiver(){
//     // RF Transceiver serial
//     mySerial.begin(baud_rate);

//     pinMode(PIN_SDN, OUTPUT);
//     digitalWrite(PIN_SDN, LOW);

//     delay(1000);
//     ATR(0, 64);
//     delay(20);
//     ATM(1);
//     delay(20);
//   }


//   /*
//     Operating Mode 
//     1: RX mode
//     2: Default state, fast switch between TX and RX
//     3: Sleep
//   */
//   void ATM(uint8_t mode){
//     // Operating
//     mySerial.write("ATM");
//     mySerial.write(mode);
//   }

//   /*
//     Receive mode config
//     Channel: 0-15
//     Packet Length: 1-64
//   */
//   void ATR(uint8_t channel, uint8_t packet_length){
//     mySerial.write("ATR");
//     mySerial.write(channel);
//     mySerial.write(packet_length);
//   }

//   /*
//     Transmit a data packet
//     Channel: 0-15
//     Packet Length: 1-64
//     Data: 1-64 bytes of data corresponding to packet length
//   */
//   void Transmit(uint8_t channel, uint8_t packet_length, byte *data){
//     mySerial.write("ATS");
//     mySerial.write((byte) channel);
//     mySerial.write(packet_length);
//     for (size_t i = 0; i < packet_length; i++){
//       mySerial.write(data[i]);
//     }
//   }

//   // RSSI Value
//   void ATQ(){
//     mySerial.write("ATQ");
//   }

//   // Retrieve Currant Configuration and Settings
//   void ATQuestion(){
//     mySerial.write("AT?");
//   }

//   // Enable command response. Doesnt seem to work??
//   void ATC(bool enable){
//     mySerial.write(65);
//     mySerial.write(84);
//     mySerial.write(67);
//     if (enable){
//       mySerial.write(1);
//     } else {
//       mySerial.write((byte) 0);
//     }
//   }

// };