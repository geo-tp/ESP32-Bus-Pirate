/**
   Copyright (c) 2011, 2012, 2013 Research In Motion Limited.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

// Uncomment just _one_ line below depending on how your breakout or shield
// is connected to the Arduino:

// Use this line for a breakout with a SPI connection:
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

// Use this line for a breakout with a hardware SPI connection.  Note that
// the PN532 SCK, MOSI, and MISO pins need to be connected to the Arduino's
// hardware SPI SCK, MOSI, and MISO pins.  On an Arduino Uno these are
// SCK = 13, MOSI = 11, MISO = 12.  The SS line can be any digital IO pin.
//Adafruit_PN532 nfc(PN532_SS);

// Or use this line for a breakout or shield with an I2C connection:
//Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// PWM LED will be on the following PINs.
#define R_PIN (9)
#define G_PIN (10)
#define B_PIN (11)

// Initial values of RGB.
uint8_t r = 0x00;
uint8_t g = 0x00;
uint8_t b = 0x7f;

/**
 * Write the current color to the output pins.
 */
void showColor() {
  analogWrite(R_PIN,r);
  analogWrite(G_PIN,g);
  analogWrite(B_PIN,b);
}


void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // for Leonardo/Micro/Zero

  pinMode(R_PIN,OUTPUT);
  pinMode(G_PIN,OUTPUT);
  pinMode(B_PIN,OUTPUT);
  showColor();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("Did not find the shield - locking up");
    while (true) {
    }
  }

  Serial.print("Found chip PN5");
  Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata>>8) & 0xFF, DEC);

  nfc.begin();
}

uint8_t message[4];

void loop(void) {
  uint8_t i;

  // Serial.println("Listening...");
  if (nfc.inListPassiveTarget()) {
    // Serial.println("Something's there...");
    while(true) {
      message[0] = 1;
      message[1] = r;
      message[2] = g;
      message[3] = b;
      uint8_t responseLength = sizeof(message);
      if (nfc.inDataExchange(message,sizeof(message),message,&responseLength)) {
        uint8_t command = message[0];
        switch(command) {
        case 0:
          // NOP
          break;
        case 1:
          if (responseLength==4) {
            r = message[1];
            g = message[2];
            b = message[3];
            showColor();
            //Serial.print("Read a color: ");
            //for (i=0; i<3; ++i) {
            //  Serial.print(message[i+1],HEX);
            //  Serial.print(' ');
            //}
            //Serial.println();
          }
          else {
            //Serial.println("Doesn't seem to be a color...");
          }
          break;
        default:
          //Serial.print("Unknown command ");
          //Serial.println(message[0]);
          ;
        }
        delay(10);
      }
      else {
        //Serial.println("It's gone...");
        break;
      }
    }
  }
  else {
    //Serial.print("Trying again...");
  }
}

