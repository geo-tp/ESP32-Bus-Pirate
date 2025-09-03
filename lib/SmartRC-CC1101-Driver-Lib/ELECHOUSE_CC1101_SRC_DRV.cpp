/*
  ELECHOUSE_CC1101.cpp - CC1101 module library
  Copyright (c) 2010 Michael.
    Author: Michael, <www.elechouse.com>
    Version: November 12, 2010

  This library is designed to use CC1101/CC1100 module on Arduino platform.
  CC1101/CC1100 module is an useful wireless module.Using the functions of the
  library, you can easily send and receive data by the CC1101/CC1100 module.
  Just have fun!
  For the details, please refer to the datasheet of CC1100/CC1101.
----------------------------------------------------------------------------------------------------------------
cc1101 Driver for RC Switch. Mod by Little Satan. With permission to modify and publish Wilson Shen (ELECHOUSE).
----------------------------------------------------------------------------------------------------------------
*/
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include <Arduino.h>

/****************************************************************/
#define   WRITE_BURST       0x40            //write burst
#define   READ_SINGLE       0x80            //read single
#define   READ_BURST        0xC0            //read burst
#define   BYTES_IN_RXFIFO   0x7F            //byte number in RXfifo
#define   max_modul 6

byte modulation = 2;
byte frend0;
byte chan = 0;
int pa = 12;
byte last_pa;
byte SCK_PIN;
byte MISO_PIN;
byte MOSI_PIN;
byte SS_PIN;
byte GDO0;
byte GDO2;
byte SCK_PIN_M[max_modul];
byte MISO_PIN_M[max_modul];
byte MOSI_PIN_M[max_modul];
byte SS_PIN_M[max_modul];
byte GDO0_M[max_modul];
byte GDO2_M[max_modul];
byte gdo_set=0;
bool __spi = 0;
bool ccmode = 0;
float MHz = 433.92;
byte m4RxBw = 0;
byte m4DaRa;
byte m2DCOFF;
byte m2MODFM;
byte m2MANCH;
byte m2SYNCM;
byte m1FEC;
byte m1PRE;
byte m1CHSP;
byte pc1PQT;
byte pc1CRC_AF;
byte pc1APP_ST;
byte pc1ADRCHK;
byte pc0WDATA;
byte pc0PktForm;
byte pc0CRC_EN;
byte pc0LenConf;
byte trxstate = 0;
byte clb1[2]= {24,28};
byte clb2[2]= {31,38};
byte clb3[2]= {65,76};
byte clb4[2]= {77,79};

/****************************************************************/
uint8_t PA_TABLE[8]     {0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00};
//                       -30  -20  -15  -10   0    5    7    10
uint8_t PA_TABLE_315[8] {0x12,0x0D,0x1C,0x34,0x51,0x85,0xCB,0xC2,};             //280 - 348
uint8_t PA_TABLE_433[8] {0x12,0x0E,0x1D,0x34,0x60,0x84,0xC8,0xC0,};             //387 - 464
//                        -30  -20  -15  -10  -6    0    5    7    10   12
uint8_t PA_TABLE_868[10] {0x03,0x17,0x1D,0x26,0x37,0x50,0x86,0xCD,0xC5,0xC0,};  //779 - 899.99
//                        -30  -20  -15  -10  -6    0    5    7    10   11
uint8_t PA_TABLE_915[10] {0x03,0x0E,0x1E,0x27,0x38,0x8E,0x84,0xCC,0xC3,0xC0,};  //900 - 928

/****************************************************************
*FUNCTION NAME:setBeginEndLogic
*FUNCTION     :set a new state to Begin/End logic, in this logic SPI class uses SPI.begin(...) and SPI.end() logic for backwards compatibility
*INPUT        :state: true or false
*OUTPUT       :none
****************************************************************/
  void ELECHOUSE_CC1101::setBeginEndLogic(bool state) {
    _begin_end_logic = state;
  }


/****************************************************************
*FUNCTION NAME:getBeginEndLogic
*FUNCTION     :get the state to Begin/End logic, in this logic SPI class uses SPI.begin(...) and SPI.end() logic for backwards compatibility
*INPUT        :state: true or false
*OUTPUT       :none
****************************************************************/
  bool ELECHOUSE_CC1101::getBeginEndLogic() {
    return _begin_end_logic;
  }

/****************************************************************
*FUNCTION NAME:setSPIinstance
*FUNCTION     :Set the SPI Instance to use if needed to share SPI Bus with other devices
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSPIinstance(SPIClass* sspi)
{
  // Set the SPI instance to use if needed to share SPI Bus with other devices
  cc_spi=sspi;
}
/****************************************************************
*FUNCTION NAME:getSPIinstance
*FUNCTION     :Gives the SPI instance in use by CC1101, if none was set, will set the library SPI instance
*INPUT        :none
*OUTPUT       :none
****************************************************************/
SPIClass* ELECHOUSE_CC1101::getSPIinstance()
{
  if(cc_spi==nullptr) {
    cc_spi=&_cc_spi;
  }
  return cc_spi;
}

/****************************************************************
*FUNCTION NAME:SpiStart
*FUNCTION     :spi communication start
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SpiStart(void)
{
  //End transaction to ensure openin a new session with the right SPISettings
  digitalWrite(SS_PIN, HIGH);
  cc_spi->endTransaction();
  if(_begin_end_logic) cc_spi->end();
  if(_begin_end_logic) cc_spi->begin(SCK_PIN,MISO_PIN,MOSI_PIN,SS_PIN);
  digitalWrite(SS_PIN, LOW);
  cc_spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));


}
/****************************************************************
*FUNCTION NAME:SpiEnd
*FUNCTION     :spi communication disable
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SpiEnd(void)
{
  // disable SPI
  cc_spi->endTransaction();
  digitalWrite(SS_PIN, HIGH);
  if(_begin_end_logic) cc_spi->end();

}
/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0,GDO2 pin for serial pinmode.
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void ELECHOUSE_CC1101::GDO_Set (void)
{
	pinMode(GDO0, OUTPUT);
	pinMode(GDO2, INPUT);
}
/****************************************************************
*FUNCTION NAME: GDO_Set()
*FUNCTION     : set GDO0 for internal transmission mode.
*INPUT        : none
*OUTPUT       : none
****************************************************************/
void ELECHOUSE_CC1101::GDO0_Set (void)
{
  pinMode(GDO0, INPUT);
}

/****************************************************************
*FUNCTION NAME:checkMISO()
*FUNCTION     :check MISO Pin for errors
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::checkMISO (void)
{
  // This function does not with IDF>=5, it delays the communication and makes the CC1101 not responding.
  // So avoid using it in this situation
  #if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0))
  unsigned long timer = millis();
  while(digitalRead(MISO_PIN)){
    if(millis()-timer>1000) {
      log_e("CC1101: Fail checking MISO pin, check your connections");
      digitalWrite(SS_PIN, HIGH);
      return false;
    }
  }
  #endif
  return true;
}
/****************************************************************
*FUNCTION NAME:Reset
*FUNCTION     :CC1101 reset //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::Reset (void)
{
  SpiStart();
	digitalWrite(SS_PIN, LOW);
	delay(1);
	digitalWrite(SS_PIN, HIGH);
	delay(1);
	digitalWrite(SS_PIN, LOW);
  if(!checkMISO()) return false;
  cc_spi->transfer(CC1101_SRES);
  if(!checkMISO()) return false;
	digitalWrite(SS_PIN, HIGH);
  return true;
}
/****************************************************************
*FUNCTION NAME:Init
*FUNCTION     :CC1101 initialization
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::Init(void)
{
  // check if SPI Pins are set
  setSpi();
  pinMode(SS_PIN, OUTPUT);
  // If there were no different SPI Instance, use This lib instance
  if(cc_spi==nullptr || _begin_end_logic) {
    DEBUG_CC1101("CC1101: Null pointer SPI instance or Begin/end");
    _begin_end_logic=true;
    cc_spi=&_cc_spi;
    cc_spi->begin(SCK_PIN,MISO_PIN,MOSI_PIN,SS_PIN);
    delay(1);
  } else { DEBUG_CC1101("CC1101: Using other instance"); }

  SpiStart();                   //Start SPI Transaction
  digitalWrite(SS_PIN, HIGH);
  if(!Reset()) return false;                      //CC1101 reset
  RegConfigSettings();          //CC1101 register config
  SpiEnd();                     //Stops SPI Transaction
  return true;
}
/****************************************************************
*FUNCTION NAME:SpiWriteReg
*FUNCTION     :CC1101 write data to register
*INPUT        :addr: register address; value: register value
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::SpiWriteReg(byte addr, byte value)
{
  SpiStart();
  if(!checkMISO()) return false;
  cc_spi->transfer(addr);
  cc_spi->transfer(value);
  SpiEnd();
  DEBUG_CC1101("Write reg addr: 0x" + String(addr,HEX) + "=0x" + String(value,HEX));
  return true;
}
/****************************************************************
*FUNCTION NAME:SpiWriteBurstReg
*FUNCTION     :CC1101 write burst data to register
*INPUT        :addr: register address; buffer:register value array; num:number to write
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::SpiWriteBurstReg(byte addr, byte *buffer, byte num)
{
  byte i, temp;
  SpiStart();
  temp = addr | WRITE_BURST;
  if(!checkMISO()) return false;
  DEBUG_CC11012("\nWrite burst addr: 0x" + String(temp,HEX) + "=");
  cc_spi->transfer(temp);
  for (i = 0; i < num; i++)
  {
  cc_spi->transfer(buffer[i]);
  DEBUG_CC11012(" 0x" + String(buffer[i],HEX));
  }
  DEBUG_CC1101()
  SpiEnd();
  return true;
}
/****************************************************************
*FUNCTION NAME:SpiStrobe
*FUNCTION     :CC1101 Strobe
*INPUT        :strobe: command; //refer define in CC1101.h//
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::SpiStrobe(byte strobe)
{
  SpiStart();
  if(!checkMISO()) return false;
  cc_spi->transfer(strobe);
  SpiEnd();
  DEBUG_CC1101("Write Strobe: 0x" + String(strobe,HEX));
  return true;
}
/****************************************************************
*FUNCTION NAME:SpiReadReg
*FUNCTION     :CC1101 read data from register
*INPUT        :addr: register address
*OUTPUT       :register value
****************************************************************/
byte ELECHOUSE_CC1101::SpiReadReg(byte addr)
{
  byte temp, value;
  SpiStart();
  temp = addr| READ_SINGLE;
  if(!checkMISO()) return 0x00;
  cc_spi->transfer(temp);
  value=cc_spi->transfer(0);
  SpiEnd();
  DEBUG_CC1101("Reading addr: 0x" + String(addr,HEX) + " = 0x" + String(value,HEX));
  return value;
}

/****************************************************************
*FUNCTION NAME:SpiReadBurstReg
*FUNCTION     :CC1101 read burst data from register
*INPUT        :addr: register address; buffer:array to store register value; num: number to read
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::SpiReadBurstReg(byte addr, byte *buffer, byte num)
{
  byte i,temp;
  SpiStart();
  temp = addr | READ_BURST;
  if(!checkMISO()) return false;
  DEBUG_CC11012("\nReading addr: 0x" + String(temp,HEX) + " = ");
  cc_spi->transfer(temp);
  for(i=0;i<num;i++)
  {
  buffer[i]=cc_spi->transfer(0);
  DEBUG_CC11012(" " + String(buffer[i],HEX));
  }
  DEBUG_CC1101()
  SpiEnd();
  return true;
}

/****************************************************************
*FUNCTION NAME:SpiReadStatus
*FUNCTION     :CC1101 read status register
*INPUT        :addr: register address
*OUTPUT       :status value
****************************************************************/
byte ELECHOUSE_CC1101::SpiReadStatus(byte addr)
{
  byte value,temp;
  SpiStart();
  temp = addr | READ_BURST;
  if(!checkMISO()) return 0x00;
  cc_spi->transfer(temp);
  value=cc_spi->transfer(0);
  SpiEnd();
  DEBUG_CC1101("Reading Reg addr: 0x" + String(addr,HEX) + " = 0x" + String(value,HEX));
  return value;
}
/****************************************************************
*FUNCTION NAME:SPI pin Settings
*FUNCTION     :Set Spi pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSpi(void){
  if (__spi == 0){
  #if defined __AVR_ATmega168__ || defined __AVR_ATmega328P__
  SCK_PIN = 13; MISO_PIN = 12; MOSI_PIN = 11; SS_PIN = 10;
  #elif defined __AVR_ATmega1280__ || defined __AVR_ATmega2560__
  SCK_PIN = 52; MISO_PIN = 50; MOSI_PIN = 51; SS_PIN = 53;
  #elif ESP8266
  SCK_PIN = 14; MISO_PIN = 12; MOSI_PIN = 13; SS_PIN = 15;
  #elif ESP32
  SCK_PIN = 18; MISO_PIN = 19; MOSI_PIN = 23; SS_PIN = 5;
  #else
  SCK_PIN = 13; MISO_PIN = 12; MOSI_PIN = 11; SS_PIN = 10;
  #endif
}
}
/****************************************************************
*FUNCTION NAME:COSTUM SPI
*FUNCTION     :set costum spi pins.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSpiPin(byte sck, byte miso, byte mosi, byte ss){
  __spi = 1;
  SCK_PIN = sck;
  MISO_PIN = miso;
  MOSI_PIN = mosi;
  SS_PIN = ss;
}
/****************************************************************
*FUNCTION NAME:COSTUM SPI
*FUNCTION     :set costum spi pins.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::addSpiPin(byte sck, byte miso, byte mosi, byte ss, byte modul){
  __spi = 1;
  SCK_PIN_M[modul] = sck;
  MISO_PIN_M[modul] = miso;
  MOSI_PIN_M[modul] = mosi;
  SS_PIN_M[modul] = ss;
}
/****************************************************************
*FUNCTION NAME:GDO Pin settings
*FUNCTION     :set GDO Pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setGDO(byte gdo0, byte gdo2){
GDO0 = gdo0;
GDO2 = gdo2;  
GDO_Set();
}
/****************************************************************
*FUNCTION NAME:GDO0 Pin setting
*FUNCTION     :set GDO0 Pin
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setGDO0(byte gdo0){
GDO0 = gdo0;
GDO0_Set();
}
/****************************************************************
*FUNCTION NAME:GDO Pin settings
*FUNCTION     :add GDO Pins
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::addGDO(byte gdo0, byte gdo2, byte modul){
GDO0_M[modul] = gdo0;
GDO2_M[modul] = gdo2;  
gdo_set=2;
GDO_Set();
}
/****************************************************************
*FUNCTION NAME:add GDO0 Pin
*FUNCTION     :add GDO0 Pin
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::addGDO0(byte gdo0, byte modul){
GDO0_M[modul] = gdo0;
gdo_set=1;
GDO0_Set();
}
/****************************************************************
*FUNCTION NAME:set Modul
*FUNCTION     :change modul
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setModul(byte modul){
  SCK_PIN = SCK_PIN_M[modul];
  MISO_PIN = MISO_PIN_M[modul];
  MOSI_PIN = MOSI_PIN_M[modul];
  SS_PIN = SS_PIN_M[modul];
  if (gdo_set==1){
  GDO0 = GDO0_M[modul];
  }
  else if (gdo_set==2){
  GDO0 = GDO0_M[modul];
  GDO2 = GDO2_M[modul];
  }
}
/****************************************************************
*FUNCTION NAME:CCMode
*FUNCTION     :Format of RX and TX data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setCCMode(bool s){
ccmode = s;
if (ccmode == 1){
if (!SpiWriteReg(CC1101_IOCFG2, 0x0B)) return;
SpiWriteReg(CC1101_IOCFG0,      0x06);
SpiWriteReg(CC1101_PKTCTRL0,    0x05);
SpiWriteReg(CC1101_MDMCFG3,     0xF8);
SpiWriteReg(CC1101_MDMCFG4,11+m4RxBw);
}else{
if (!SpiWriteReg(CC1101_IOCFG2, 0x0D)) return;
SpiWriteReg(CC1101_IOCFG0,      0x0D);
SpiWriteReg(CC1101_PKTCTRL0,    0x32);
SpiWriteReg(CC1101_MDMCFG3,     0x93);
SpiWriteReg(CC1101_MDMCFG4, 7+m4RxBw);
}
setModulation(modulation);
}
/****************************************************************
*FUNCTION NAME:Modulation
*FUNCTION     :set CC1101 Modulation
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setModulation(byte m){
if (m>4){m=4;}
modulation = m;
Split_MDMCFG2();
switch (m)
{
case 0: m2MODFM=0x00; frend0=0x10; break; // 2-FSK
case 1: m2MODFM=0x10; frend0=0x10; break; // GFSK
case 2: m2MODFM=0x30; frend0=0x11; break; // ASK
case 3: m2MODFM=0x40; frend0=0x10; break; // 4-FSK
case 4: m2MODFM=0x70; frend0=0x10; break; // MSK
}
if (!SpiWriteReg(CC1101_MDMCFG2, m2DCOFF + m2MODFM + m2MANCH + m2SYNCM)) return;
SpiWriteReg(CC1101_FREND0,   frend0);
setPA(pa);
}
/****************************************************************
*FUNCTION NAME:PA Power
*FUNCTION     :set CC1101 PA Power
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setPA(int p)
{
int a;
pa = p;

if (MHz >= 280 && MHz <= 348){
if (pa <= -30){a = PA_TABLE_315[0];}
else if (pa > -30 && pa <= -20){a = PA_TABLE_315[1];}
else if (pa > -20 && pa <= -15){a = PA_TABLE_315[2];}
else if (pa > -15 && pa <= -10){a = PA_TABLE_315[3];}
else if (pa > -10 && pa <= 0){a = PA_TABLE_315[4];}
else if (pa > 0 && pa <= 5){a = PA_TABLE_315[5];}
else if (pa > 5 && pa <= 7){a = PA_TABLE_315[6];}
else if (pa > 7){a = PA_TABLE_315[7];}
last_pa = 1;
}
else if (MHz >= 378 && MHz <= 464){
if (pa <= -30){a = PA_TABLE_433[0];}
else if (pa > -30 && pa <= -20){a = PA_TABLE_433[1];}
else if (pa > -20 && pa <= -15){a = PA_TABLE_433[2];}
else if (pa > -15 && pa <= -10){a = PA_TABLE_433[3];}
else if (pa > -10 && pa <= 0){a = PA_TABLE_433[4];}
else if (pa > 0 && pa <= 5){a = PA_TABLE_433[5];}
else if (pa > 5 && pa <= 7){a = PA_TABLE_433[6];}
else if (pa > 7){a = PA_TABLE_433[7];}
last_pa = 2;
}
else if (MHz >= 779 && MHz <= 899.99){
if (pa <= -30){a = PA_TABLE_868[0];}
else if (pa > -30 && pa <= -20){a = PA_TABLE_868[1];}
else if (pa > -20 && pa <= -15){a = PA_TABLE_868[2];}
else if (pa > -15 && pa <= -10){a = PA_TABLE_868[3];}
else if (pa > -10 && pa <= -6){a = PA_TABLE_868[4];}
else if (pa > -6 && pa <= 0){a = PA_TABLE_868[5];}
else if (pa > 0 && pa <= 5){a = PA_TABLE_868[6];}
else if (pa > 5 && pa <= 7){a = PA_TABLE_868[7];}
else if (pa > 7 && pa <= 10){a = PA_TABLE_868[8];}
else if (pa > 10){a = PA_TABLE_868[9];}
last_pa = 3;
}
else if (MHz >= 900 && MHz <= 928){
if (pa <= -30){a = PA_TABLE_915[0];}
else if (pa > -30 && pa <= -20){a = PA_TABLE_915[1];}
else if (pa > -20 && pa <= -15){a = PA_TABLE_915[2];}
else if (pa > -15 && pa <= -10){a = PA_TABLE_915[3];}
else if (pa > -10 && pa <= -6){a = PA_TABLE_915[4];}
else if (pa > -6 && pa <= 0){a = PA_TABLE_915[5];}
else if (pa > 0 && pa <= 5){a = PA_TABLE_915[6];}
else if (pa > 5 && pa <= 7){a = PA_TABLE_915[7];}
else if (pa > 7 && pa <= 10){a = PA_TABLE_915[8];}
else if (pa > 10){a = PA_TABLE_915[9];}
last_pa = 4;
}
if (modulation == 2){
PA_TABLE[0] = 0;
PA_TABLE[1] = a;
}else{
PA_TABLE[0] = a;
PA_TABLE[1] = 0;
}
SpiWriteBurstReg(CC1101_PATABLE,PA_TABLE,8);
}
/****************************************************************
*FUNCTION NAME:Frequency Calculator
*FUNCTION     :Calculate the basic frequency.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setMHZ(float mhz){
byte freq2 = 0;
byte freq1 = 0;
byte freq0 = 0;

MHz = mhz;

for (bool i = 0; i==0;){
if (mhz >= 26){
mhz-=26;
freq2+=1;
}
else if (mhz >= 0.1015625){
mhz-=0.1015625;
freq1+=1;
}
else if (mhz >= 0.00039675){
mhz-=0.00039675;
freq0+=1;
}
else{i=1;}
}
if (freq0 > 255){freq1+=1;freq0-=256;}

if (!SpiWriteReg(CC1101_FREQ2, freq2)) return;
SpiWriteReg(CC1101_FREQ1, freq1);
SpiWriteReg(CC1101_FREQ0, freq0);

Calibrate();
}
/****************************************************************
*FUNCTION NAME:Calibrate
*FUNCTION     :Calibrate frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Calibrate(void){

if (MHz >= 280 && MHz <= 348){
SpiWriteReg(CC1101_FSCTRL0, map(MHz, 280, 348, clb1[0], clb1[1]));
if (MHz < 322.88){SpiWriteReg(CC1101_TEST0,0x0B);}
else{
SpiWriteReg(CC1101_TEST0,0x09);
int s = ELECHOUSE_cc1101.SpiReadStatus(CC1101_FSCAL2);
if (s<32){SpiWriteReg(CC1101_FSCAL2, s+32);}
if (last_pa != 1){setPA(pa);}
}
}
else if (MHz >= 378 && MHz <= 464){
SpiWriteReg(CC1101_FSCTRL0, map(MHz, 378, 464, clb2[0], clb2[1]));
if (MHz < 430.5){SpiWriteReg(CC1101_TEST0,0x0B);}
else{
SpiWriteReg(CC1101_TEST0,0x09);
int s = ELECHOUSE_cc1101.SpiReadStatus(CC1101_FSCAL2);
if (s<32){SpiWriteReg(CC1101_FSCAL2, s+32);}
if (last_pa != 2){setPA(pa);}
}
}
else if (MHz >= 779 && MHz <= 899.99){
SpiWriteReg(CC1101_FSCTRL0, map(MHz, 779, 899, clb3[0], clb3[1]));
if (MHz < 861){SpiWriteReg(CC1101_TEST0,0x0B);}
else{
SpiWriteReg(CC1101_TEST0,0x09);
int s = ELECHOUSE_cc1101.SpiReadStatus(CC1101_FSCAL2);
if (s<32){SpiWriteReg(CC1101_FSCAL2, s+32);}
if (last_pa != 3){setPA(pa);}
}
}
else if (MHz >= 900 && MHz <= 928){
SpiWriteReg(CC1101_FSCTRL0, map(MHz, 900, 928, clb4[0], clb4[1]));
SpiWriteReg(CC1101_TEST0,0x09);
int s = ELECHOUSE_cc1101.SpiReadStatus(CC1101_FSCAL2);
if (s<32){SpiWriteReg(CC1101_FSCAL2, s+32);}
if (last_pa != 4){setPA(pa);}
}
}
/****************************************************************
*FUNCTION NAME:Calibration offset
*FUNCTION     :Set calibration offset
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setClb(byte b, byte s, byte e){
if (b == 1){
clb1[0]=s;
clb1[1]=e;
}
else if (b == 2){
clb2[0]=s;
clb2[1]=e;
}
else if (b == 3){
clb3[0]=s;
clb3[1]=e;
}
else if (b == 4){
clb4[0]=s;
clb4[1]=e;
}
}
/****************************************************************
*FUNCTION NAME:getCC1101
*FUNCTION     :Test Spi connection and return 1 when true.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::getCC1101(void){
  setSpi();
  byte val=SpiReadStatus(0x31);
  if (val>0){
    DEBUG_CC1101("getCC1101 result: 0x" + String(val,HEX) + " -> Ok");
    return 1;
  }else{
    DEBUG_CC1101("getCC1101 result: 0x" + String(val,HEX) + " -> Not Found");
    return 0;
}
}
/****************************************************************
*FUNCTION NAME:getMode
*FUNCTION     :Return the Mode. Sidle = 0, TX = 1, Rx = 2.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
byte ELECHOUSE_CC1101::getMode(void){
return trxstate;
}
/****************************************************************
*FUNCTION NAME:Set Sync_Word
*FUNCTION     :Sync Word
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSyncWord(byte sh, byte sl){
if(!SpiWriteReg(CC1101_SYNC1, sh)) return;
SpiWriteReg(CC1101_SYNC0, sl);
}
/****************************************************************
*FUNCTION NAME:Set ADDR
*FUNCTION     :Address used for packet filtration. Optional broadcast addresses are 0 (0x00) and 255 (0xFF).
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setAddr(byte v){
  if(!SpiWriteReg(CC1101_ADDR, v)) return;
}
/****************************************************************
*FUNCTION NAME:Set PQT
*FUNCTION     :Preamble quality estimator threshold
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setPQT(byte v){
Split_PKTCTRL1();
pc1PQT = 0;
if (v>7){v=7;}
pc1PQT = v*32;
SpiWriteReg(CC1101_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set CRC_AUTOFLUSH
*FUNCTION     :Enable automatic flush of RX FIFO when CRC is not OK
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setCRC_AF(bool v){
Split_PKTCTRL1();
pc1CRC_AF = 0;
if (v==1){pc1CRC_AF=8;}
SpiWriteReg(CC1101_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set APPEND_STATUS
*FUNCTION     :When enabled, two status bytes will be appended to the payload of the packet
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setAppendStatus(bool v){
Split_PKTCTRL1();
pc1APP_ST = 0;
if (v==1){pc1APP_ST=4;}
SpiWriteReg(CC1101_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set ADR_CHK
*FUNCTION     :Controls address check configuration of received packages
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setAdrChk(byte v){
Split_PKTCTRL1();
pc1ADRCHK = 0;
if (v>3){v=3;}
pc1ADRCHK = v;
SpiWriteReg(CC1101_PKTCTRL1, pc1PQT+pc1CRC_AF+pc1APP_ST+pc1ADRCHK);
}
/****************************************************************
*FUNCTION NAME:Set WHITE_DATA
*FUNCTION     :Turn data whitening on / off.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setWhiteData(bool v){
Split_PKTCTRL0();
pc0WDATA = 0;
if (v == 1){pc0WDATA=64;}
SpiWriteReg(CC1101_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set PKT_FORMAT
*FUNCTION     :Format of RX and TX data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setPktFormat(byte v){
Split_PKTCTRL0();
pc0PktForm = 0;
if (v>3){v=3;}
pc0PktForm = v*16;
SpiWriteReg(CC1101_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set CRC
*FUNCTION     :CRC calculation in TX and CRC check in RX
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setCrc(bool v){
Split_PKTCTRL0();
pc0CRC_EN = 0;
if (v==1){pc0CRC_EN=4;}
SpiWriteReg(CC1101_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set LENGTH_CONFIG
*FUNCTION     :Configure the packet length
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setLengthConfig(byte v){
Split_PKTCTRL0();
pc0LenConf = 0;
if (v>3){v=3;}
pc0LenConf = v;
SpiWriteReg(CC1101_PKTCTRL0, pc0WDATA+pc0PktForm+pc0CRC_EN+pc0LenConf);
}
/****************************************************************
*FUNCTION NAME:Set PACKET_LENGTH
*FUNCTION     :Indicates the packet length
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setPacketLength(byte v){
SpiWriteReg(CC1101_PKTLEN, v);
}
/****************************************************************
*FUNCTION NAME:Set DCFILT_OFF
*FUNCTION     :Disable digital DC blocking filter before demodulator
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setDcFilterOff(bool v){
Split_MDMCFG2();
m2DCOFF = 0;
if (v==1){m2DCOFF=128;}
SpiWriteReg(CC1101_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set MANCHESTER
*FUNCTION     :Enables Manchester encoding/decoding
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setManchester(bool v){
Split_MDMCFG2();
m2MANCH = 0;
if (v==1){m2MANCH=8;}
SpiWriteReg(CC1101_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set SYNC_MODE
*FUNCTION     :Combined sync-word qualifier mode
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSyncMode(byte v){
Split_MDMCFG2();
m2SYNCM = 0;
if (v>7){v=7;}
m2SYNCM=v;
SpiWriteReg(CC1101_MDMCFG2, m2DCOFF+m2MODFM+m2MANCH+m2SYNCM);
}
/****************************************************************
*FUNCTION NAME:Set FEC
*FUNCTION     :Enable Forward Error Correction (FEC)
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setFEC(bool v){
Split_MDMCFG1();
m1FEC=0;
if (v==1){m1FEC=128;}
SpiWriteReg(CC1101_MDMCFG1, m1FEC+m1PRE+m1CHSP);
}
/****************************************************************
*FUNCTION NAME:Set PRE
*FUNCTION     :Sets the minimum number of preamble bytes to be transmitted.
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setPRE(byte v){
Split_MDMCFG1();
m1PRE=0;
if (v>7){v=7;}
m1PRE = v*16;
SpiWriteReg(CC1101_MDMCFG1, m1FEC+m1PRE+m1CHSP);
}
/****************************************************************
*FUNCTION NAME:Set Channel
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setChannel(byte ch){
chan = ch;
SpiWriteReg(CC1101_CHANNR,   chan);
}
/****************************************************************
*FUNCTION NAME:Set Channel spacing
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setChsp(float f){
Split_MDMCFG1();
byte MDMCFG0 = 0;
m1CHSP = 0;
if (f > 405.456543){f = 405.456543;}
if (f < 25.390625){f = 25.390625;}
for (int i = 0; i<5; i++){
if (f <= 50.682068){
f -= 25.390625;
f /= 0.0991825;
MDMCFG0 = f;
float s1 = (f - MDMCFG0) *10;
if (s1 >= 5){MDMCFG0++;}
i = 5;
}else{
m1CHSP++;
f/=2;
}
}
if(!SpiWriteReg(19,m1CHSP+m1FEC+m1PRE)) return;
SpiWriteReg(20,MDMCFG0);
}
/****************************************************************
*FUNCTION NAME:Set Receive bandwidth
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setRxBW(float f){
Split_MDMCFG4();
int s1 = 3;
int s2 = 3;
for (int i = 0; i<3; i++){
if (f > 101.5625){f/=2; s1--;}
else{i=3;}
}
for (int i = 0; i<3; i++){
if (f > 58.1){f/=1.25; s2--;}
else{i=3;}
}
s1 *= 64;
s2 *= 16;
m4RxBw = s1 + s2;
SpiWriteReg(16,m4RxBw+m4DaRa);
}
/****************************************************************
*FUNCTION NAME:Set Data Rate
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setDRate(float d){
Split_MDMCFG4();
float c = d;
byte MDMCFG3 = 0;
if (c > 1621.83){c = 1621.83;}
if (c < 0.0247955){c = 0.0247955;}
m4DaRa = 0;
for (int i = 0; i<20; i++){
if (c <= 0.0494942){
c = c - 0.0247955;
c = c / 0.00009685;
MDMCFG3 = c;
float s1 = (c - MDMCFG3) *10;
if (s1 >= 5){MDMCFG3++;}
i = 20;
}else{
m4DaRa++;
c = c/2;
}
}
if(!SpiWriteReg(16,  m4RxBw+m4DaRa)) return;
SpiWriteReg(17,  MDMCFG3);
}
/****************************************************************
*FUNCTION NAME:Set Devitation
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setDeviation(float d){
float f = 1.586914;
float v = 0.19836425;
int c = 0;
if (d > 380.859375){d = 380.859375;}
if (d < 1.586914){d = 1.586914;}
for (int i = 0; i<255; i++){
f+=v;
if (c==7){v*=2;c=-1;i+=8;}
if (f>=d){c=i;i=255;}
c++;
}
SpiWriteReg(21,c);
}
/****************************************************************
*FUNCTION NAME:Split PKTCTRL0
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Split_PKTCTRL1(void){
int calc = SpiReadStatus(7);
pc1PQT = 0;
pc1CRC_AF = 0;
pc1APP_ST = 0;
pc1ADRCHK = 0;
for (bool i = 0; i==0;){
if (calc >= 32){calc-=32; pc1PQT+=32;}
else if (calc >= 8){calc-=8; pc1CRC_AF+=8;}
else if (calc >= 4){calc-=4; pc1APP_ST+=4;}
else {pc1ADRCHK = calc; i=1;}
}
}
/****************************************************************
*FUNCTION NAME:Split PKTCTRL0
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Split_PKTCTRL0(void){
int calc = SpiReadStatus(8);
pc0WDATA = 0;
pc0PktForm = 0;
pc0CRC_EN = 0;
pc0LenConf = 0;
for (bool i = 0; i==0;){
if (calc >= 64){calc-=64; pc0WDATA+=64;}
else if (calc >= 16){calc-=16; pc0PktForm+=16;}
else if (calc >= 4){calc-=4; pc0CRC_EN+=4;}
else {pc0LenConf = calc; i=1;}
}
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG1
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Split_MDMCFG1(void){
int calc = SpiReadStatus(19);
m1FEC = 0;
m1PRE = 0;
m1CHSP = 0;
int s2 = 0;
for (bool i = 0; i==0;){
if (calc >= 128){calc-=128; m1FEC+=128;}
else if (calc >= 16){calc-=16; m1PRE+=16;}
else {m1CHSP = calc; i=1;}
}
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG2
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Split_MDMCFG2(void){
int calc = SpiReadStatus(18);
m2DCOFF = 0;
m2MODFM = 0;
m2MANCH = 0;
m2SYNCM = 0;
for (bool i = 0; i==0;){
if (calc >= 128){calc-=128; m2DCOFF+=128;}
else if (calc >= 16){calc-=16; m2MODFM+=16;}
else if (calc >= 8){calc-=8; m2MANCH+=8;}
else{m2SYNCM = calc; i=1;}
}
}
/****************************************************************
*FUNCTION NAME:Split MDMCFG4
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::Split_MDMCFG4(void){
int calc = SpiReadStatus(16);
m4RxBw = 0;
m4DaRa = 0;
for (bool i = 0; i==0;){
if (calc >= 64){calc-=64; m4RxBw+=64;}
else if (calc >= 16){calc -= 16; m4RxBw+=16;}
else{m4DaRa = calc; i=1;}
}
}
/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC1101 register config //details refer datasheet of CC1101/CC1100//
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::RegConfigSettings(void)
{
    if(!SpiWriteReg(CC1101_FSCTRL1,  0x06)) return;
    setCCMode(ccmode);
    setMHZ(MHz);

    SpiWriteReg(CC1101_MDMCFG1,  0x02);
    SpiWriteReg(CC1101_MDMCFG0,  0xF8);
    SpiWriteReg(CC1101_CHANNR,   chan);
    SpiWriteReg(CC1101_DEVIATN,  0x47);
    SpiWriteReg(CC1101_FREND1,   0x56);
    SpiWriteReg(CC1101_MCSM0 ,   0x18);
    SpiWriteReg(CC1101_FOCCFG,   0x16);
    SpiWriteReg(CC1101_BSCFG,    0x1C);
    SpiWriteReg(CC1101_AGCCTRL2, 0xC7);
    SpiWriteReg(CC1101_AGCCTRL1, 0x00);
    SpiWriteReg(CC1101_AGCCTRL0, 0xB2);
    SpiWriteReg(CC1101_FSCAL3,   0xE9);
    SpiWriteReg(CC1101_FSCAL2,   0x2A);
    SpiWriteReg(CC1101_FSCAL1,   0x00);
    SpiWriteReg(CC1101_FSCAL0,   0x1F);
    SpiWriteReg(CC1101_FSTEST,   0x59);
    SpiWriteReg(CC1101_TEST2,    0x81);
    SpiWriteReg(CC1101_TEST1,    0x35);
    SpiWriteReg(CC1101_TEST0,    0x09);
    SpiWriteReg(CC1101_PKTCTRL1, 0x04);
    SpiWriteReg(CC1101_ADDR,     0x00);
    SpiWriteReg(CC1101_PKTLEN,   0x00);
}
/****************************************************************
*FUNCTION NAME:SetTx
*FUNCTION     :set CC1101 send data
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SetTx(void)
{
  if (!SpiStrobe(CC1101_SIDLE)) return;
  SpiStrobe(CC1101_STX); // start send
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:SetRx
*FUNCTION     :set CC1101 to receive state
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SetRx(void)
{
  if (!SpiStrobe(CC1101_SIDLE)) return;
  SpiStrobe(CC1101_SRX); // start receive
  trxstate=2;
}
/****************************************************************
*FUNCTION NAME:SetTx
*FUNCTION     :set CC1101 send data and change frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SetTx(float mhz)
{
  if(!SpiStrobe(CC1101_SIDLE)) return;
  setMHZ(mhz);
  SpiStrobe(CC1101_STX);        //start send
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:SetRx
*FUNCTION     :set CC1101 to receive state and change frequency
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SetRx(float mhz)
{
  if(!SpiStrobe(CC1101_SIDLE)) return;
  setMHZ(mhz);
  SpiStrobe(CC1101_SRX);        //start receive
  trxstate=2;
}
/****************************************************************
*FUNCTION NAME:RSSI Level
*FUNCTION     :Calculating the RSSI Level
*INPUT        :none
*OUTPUT       :none
****************************************************************/
int ELECHOUSE_CC1101::getRssi(void)
{
int rssi;
rssi=SpiReadStatus(CC1101_RSSI);
if (rssi >= 128){rssi = (rssi-256)/2-74;}
else{rssi = (rssi/2)-74;}
return rssi;
}
/****************************************************************
*FUNCTION NAME:LQI Level
*FUNCTION     :get Lqi state
*INPUT        :none
*OUTPUT       :none
****************************************************************/
byte ELECHOUSE_CC1101::getLqi(void)
{
byte lqi;
lqi=SpiReadStatus(CC1101_LQI);
return lqi;
}
/****************************************************************
*FUNCTION NAME:SetSres
*FUNCTION     :Reset CC1101
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSres(void)
{
  SpiStrobe(CC1101_SRES);
  trxstate=0;
}
/****************************************************************
*FUNCTION NAME:setSidle
*FUNCTION     :set Rx / TX Off
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::setSidle(void)
{
  SpiStrobe(CC1101_SIDLE);
  trxstate=0;
}
/****************************************************************
*FUNCTION NAME:goSleep
*FUNCTION     :set cc1101 Sleep on
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::goSleep(void){
  trxstate=0;
  if(!SpiStrobe(0x36)) return;//Exit RX / TX, turn off frequency synthesizer and exit
  SpiStrobe(0x39);//Enter power down mode when CSn goes high.
}
/****************************************************************
*FUNCTION NAME:Char direct SendData
*FUNCTION     :use CC1101 send data
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SendData(char *txchar)
{
int len = strlen(txchar);
byte chartobyte[len];
for (int i = 0; i<len; i++){chartobyte[i] = txchar[i];}
SendData(chartobyte,len);
}
/****************************************************************
*FUNCTION NAME:SendData
*FUNCTION     :use CC1101 send data
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SendData(byte *txBuffer,byte size)
{
  if(!SpiWriteReg(CC1101_TXFIFO,size)) return;
  SpiWriteBurstReg(CC1101_TXFIFO,txBuffer,size);      //write data to send
  SpiStrobe(CC1101_SIDLE);
  SpiStrobe(CC1101_STX);                  //start send
    while (!digitalRead(GDO0));               // Wait for GDO0 to be set -> sync transmitted
    while (digitalRead(GDO0));                // Wait for GDO0 to be cleared -> end of packet
  SpiStrobe(CC1101_SFTX);                 //flush TXfifo
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:Char direct SendData
*FUNCTION     :use CC1101 send data without GDO
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SendData(char *txchar,int t)
{
int len = strlen(txchar);
byte chartobyte[len];
for (int i = 0; i<len; i++){chartobyte[i] = txchar[i];}
SendData(chartobyte,len,t);
}
/****************************************************************
*FUNCTION NAME:SendData
*FUNCTION     :use CC1101 send data without GDO
*INPUT        :txBuffer: data array to send; size: number of data to send, no more than 61
*OUTPUT       :none
****************************************************************/
void ELECHOUSE_CC1101::SendData(byte *txBuffer,byte size,int t)
{
  if(!SpiWriteReg(CC1101_TXFIFO,size)) return;
  SpiWriteBurstReg(CC1101_TXFIFO,txBuffer,size);      //write data to send
  SpiStrobe(CC1101_SIDLE);
  SpiStrobe(CC1101_STX);                  //start send
  delay(t);
  SpiStrobe(CC1101_SFTX);                 //flush TXfifo
  trxstate=1;
}
/****************************************************************
*FUNCTION NAME:Check CRC
*FUNCTION     :none
*INPUT        :none
*OUTPUT       :none
****************************************************************/
bool ELECHOUSE_CC1101::CheckCRC(void){
byte lqi=SpiReadStatus(CC1101_LQI);
bool crc_ok = bitRead(lqi,7);
if (crc_ok == 1){
return 1;
}else{
SpiStrobe(CC1101_SFRX);
SpiStrobe(CC1101_SRX);
return 0;
}
}
/****************************************************************
*FUNCTION NAME:CheckRxFifo
*FUNCTION     :check receive data or not
*INPUT        :none
*OUTPUT       :flag: 0 no data; 1 receive data
****************************************************************/
bool ELECHOUSE_CC1101::CheckRxFifo(int t){
if(trxstate!=2){SetRx();}
if(SpiReadStatus(CC1101_RXBYTES) & BYTES_IN_RXFIFO){
delay(t);
return 1;
}else{
return 0;
}
}
/****************************************************************
*FUNCTION NAME:CheckReceiveFlag
*FUNCTION     :check receive data or not
*INPUT        :none
*OUTPUT       :flag: 0 no data; 1 receive data
****************************************************************/
byte ELECHOUSE_CC1101::CheckReceiveFlag(void)
{
  if(trxstate!=2){SetRx();}
	if(digitalRead(GDO0))			//receive data
	{
		while (digitalRead(GDO0));
		return 1;
	}
	else							// no data
	{
		return 0;
	}
}
/****************************************************************
*FUNCTION NAME:ReceiveData
*FUNCTION     :read data received from RXfifo
*INPUT        :rxBuffer: buffer to store data
*OUTPUT       :size of data received
****************************************************************/
byte ELECHOUSE_CC1101::ReceiveData(byte *rxBuffer)
{
	byte size;
	byte status[2];

	if(SpiReadStatus(CC1101_RXBYTES) & BYTES_IN_RXFIFO)
	{
		size=SpiReadReg(CC1101_RXFIFO);
		SpiReadBurstReg(CC1101_RXFIFO,rxBuffer,size);
		SpiReadBurstReg(CC1101_RXFIFO,status,2);
		SpiStrobe(CC1101_SFRX);
    SpiStrobe(CC1101_SRX);
		return size;
	}
	else
	{
		SpiStrobe(CC1101_SFRX);
    SpiStrobe(CC1101_SRX);
 		return 0;
	}
}
ELECHOUSE_CC1101 ELECHOUSE_cc1101;
