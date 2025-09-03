// These examples are from the Electronics Cookbook by Simon Monk
//https://github.com/LSatan/SmartRC-CC1101-Driver-Lib
// mod by Little_S@tan
#include <ELECHOUSE_CC1101_SRC_DRV.h>

int gdo0;

void setup(){

#ifdef ESP32
gdo0 = 2;  // for esp32! GDO0 on GPIO pin 2.
#elif ESP8266
gdo0 = 5;  // for esp8266! GDO0 on pin 5 = D1.
#else
gdo0 = 6;  // for Arduino! GDO0 on pin 6.
#endif 
  
    Serial.begin(9600);
    
    if (ELECHOUSE_cc1101.getCC1101()){         // Check the CC1101 Spi connection.
    Serial.println("Connection OK");
    }else{
    Serial.println("Connection Error");
    }
  
    ELECHOUSE_cc1101.Init();               // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setGDO0(gdo0);       // set lib internal gdo pin (gdo0). Gdo2 not use for this example.
    ELECHOUSE_cc1101.setCCMode(1);       // set config for internal transmission mode.
    ELECHOUSE_cc1101.setModulation(0);  // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setMHZ(433.92);   // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
    ELECHOUSE_cc1101.setSyncMode(2);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
    ELECHOUSE_cc1101.setCrc(1);      // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.

    Serial.println("Rx Mode");
}
byte buffer[61] = {0};

void loop(){

    if (ELECHOUSE_cc1101.CheckReceiveFlag()){
    
    if (ELECHOUSE_cc1101.CheckCRC()){    //CRC Check. If "setCrc(false)" crc returns always OK!
    Serial.print("Rssi: ");
    Serial.println(ELECHOUSE_cc1101.getRssi());
    Serial.print("LQI: ");
    Serial.println(ELECHOUSE_cc1101.getLqi());
    
    int len = ELECHOUSE_cc1101.ReceiveData(buffer);
    buffer[len] = '\0';
    Serial.println((char *) buffer);
    for (int i = 0; i<len; i++){
    Serial.print(buffer[i]);
    Serial.print(",");
    }
    Serial.println();
    }
    }
}
