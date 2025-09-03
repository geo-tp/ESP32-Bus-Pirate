/*
  Simple example for repeating decimal code for cc1101
  
  https://github.com/sui77/rc-switch/
  https://github.com/LSatan/SmartRC-CC1101-Driver-Lib
  ----------------------------------------------------------
  Mod by Little Satan. Have Fun!
  ----------------------------------------------------------
*/
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

int pinRx; // int for Receive pin.
int pinTx; // int for Transmit pin.

RCSwitch mySwitch = RCSwitch();

int number = 15;  // set number of transmission repetitions.
int TIME = 3000; // set delay befor repeat. For direct repetition after receive set 0.

void setup() {
  Serial.begin(9600);

#ifdef ESP32
pinRx = 4; pinTx = 2;  // for esp32! Receiver on GPIO pin 4. Transmit on GPIO pin 2.
#elif ESP8266
pinRx = 4; pinTx = 5;  // for esp8266! Receiver on pin 4 = D2. Transmit on pin 5 = D1.
#else
pinRx = 0; pinTx = 6;  // for Arduino! Receiver on interrupt 0 => that is pin #2. Transmit on pin 6.
#endif   

  if (ELECHOUSE_cc1101.getCC1101()){       // Check the CC1101 Spi connection.
  Serial.println("Connection OK");
  }else{
  Serial.println("Connection Error");
  }

//CC1101 Settings:                (Settings with "//" are optional!)
  ELECHOUSE_cc1101.Init();            // must be set to initialize the cc1101!
//ELECHOUSE_cc1101.setRxBW(812.50);  // Set the Receive Bandwidth in kHz. Value from 58.03 to 812.50. Default is 812.50 kHz.
//ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!
  ELECHOUSE_cc1101.setMHZ(433.92); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  
  ELECHOUSE_cc1101.SetRx();       // set Receive on
  mySwitch.enableReceive(pinRx); // Receiver on

}
void loop() {

  if (mySwitch.available()){
    
    Serial.print("Received ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.print( mySwitch.getReceivedProtocol() );
    Serial.print(" Delay: ");    
    Serial.println( mySwitch.getReceivedDelay() );

    delay(TIME);   
    mySwitch.disableReceive();                                                        // Receiver off
    mySwitch.enableTransmit(pinTx);                                                   // Transmit on
    ELECHOUSE_cc1101.SetTx();                                                         // set Transmit on

    Serial.println("Transmit");

    mySwitch.setRepeatTransmit(number);                                               // transmission repetitions.
    mySwitch.setProtocol(mySwitch.getReceivedProtocol());                             // send Received Protocol
    mySwitch.setPulseLength(mySwitch.getReceivedDelay());                             // send Received Delay
    mySwitch.send(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength());      // send Received value/bits
  
    ELECHOUSE_cc1101.SetRx();                                                         // set Receive on
    mySwitch.disableTransmit();                                                       // set Transmit off
    mySwitch.enableReceive(pinRx);                                                    // Receiver on

    Serial.println("Receive");
    
    mySwitch.resetAvailable();

   }
}
