/*****************************************************************************
* This is a library for SPI EEPROMs like the 25LCxxx, 25Axxx, etc.
*
* In case you like it would be cool if you give it a star.
* 
* If you find bugs, please inform me!
* 
* Written by Wolfgang (Wolle) Ewald
*
* Here you find the detailed documentation:
* https://wolles-elektronikkiste.de/en/eeprom-part-3-external-spi-eeproms   (English)
* https://wolles-elektronikkiste.de/eeprom-teil-3-externe-spi-eeproms       (German)
*
*******************************************************************************/

#include "EEPROM_SPI_WE.h"

/************ Basic settings ************/
    
bool EEPROM_SPI_WE::init(uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin, uint8_t cs, uint8_t wp) {
    csPin = cs;
    wpPin = wp;

    _spi->begin(sckPin, misoPin, mosiPin, csPin);
    mySPISettings = SPISettings(spiClock, MSBFIRST, SPI_MODE0);
    pinMode(csPin, OUTPUT);
    digitalWrite(csPin, HIGH);
    contAddr = 0;
    smallEEPROM = false;
    memSize = 0;

    if (wpPin != 255) {
        pinMode(wpPin, OUTPUT);
        digitalWrite(wpPin, HIGH);
    }

    pageSize = 16;
    writeProtect(PROTECT_NONE);
    eepromWriteStatusReg(EEP_WREN);
    
    if (eepromReadStatusReg() & 0x02) {
        eepromWriteStatusReg(EEP_WRDI);
        return true;
    } else {
        return false;
    }
}

bool EEPROM_SPI_WE::probe() {
    eepromWriteStatusReg(EEP_WREN);
    
    if (eepromReadStatusReg() & 0x02) {
        eepromWriteStatusReg(EEP_WRDI);
        return true;
    } else {
        return false;
    }
}

void EEPROM_SPI_WE::setPageSize(eeprom_pageSize eps){
    pageSize = 16 << eps;
}

void EEPROM_SPI_WE::setMemorySize(eeprom_size_t size){
    memSize = size;
}

uint32_t EEPROM_SPI_WE::getMemorySize(){
    return memSize;
}

void EEPROM_SPI_WE::deepPowerDown(){
    eepromWriteStatusReg(EEP_DPD);
}

uint8_t EEPROM_SPI_WE::powerUpAndReadID(){
    uint8_t id = 0;
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(EEP_RDID); 
    _spi->transfer(0xFF); // dummy address
    _spi->transfer(0xFF);
    id = _spi->transfer(0x00);
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    while(isBusy()){}
    return id;
}

/************ read and write ************/

void EEPROM_SPI_WE::erasePage(uint32_t addr){
    eepromWriteEnable();
    
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(EEP_PE);
    if (memSize > EEPROM_KBITS_512) {
      _spi->transfer(static_cast<uint8_t>(addr>>16));
    }
    _spi->transfer(static_cast<uint8_t>(addr>>8));
    _spi->transfer(static_cast<uint8_t>(addr&0xFF));
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    
    while(isBusy()){}   
}

void EEPROM_SPI_WE::eraseSector(uint32_t addr){
    eepromWriteEnable();
    
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(EEP_SE);
    if (memSize > EEPROM_KBITS_512) {
      _spi->transfer(static_cast<uint8_t>(addr>>16));
    }
    _spi->transfer(static_cast<uint8_t>(addr>>8));
    _spi->transfer(static_cast<uint8_t>(addr&0xFF));
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    
    while(isBusy()){}
}

void EEPROM_SPI_WE::eraseCompleteEEPROM(){
    if (smallEEPROM){
        //No EEP_CE instruction set available on small eeproms
        for (uint32_t i = 0; i < memSize; i++) {
            write(i, 0); 
        }
    } else {
        eepromWriteEnable();
        _spi->beginTransaction(mySPISettings);
        digitalWrite(csPin, LOW);
        _spi->transfer(EEP_CE);
        digitalWrite(csPin, HIGH);
        _spi->endTransaction();
        while(isBusy()){}  
    }  
}

void EEPROM_SPI_WE::write(uint32_t addr, uint8_t val){
    if (read(addr) != val) 
        writeEEPROM(addr, &val, 1);
    while(isBusy()){}
}

uint8_t EEPROM_SPI_WE::read(uint32_t addr){
    uint8_t byteVal;
    readEEPROM(addr, &byteVal, 1);
    return byteVal;
}

void EEPROM_SPI_WE::writeProtect(eeprom_writeProtect sectors){
    eepromWriteEnable();
    
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(EEP_WRSR);
    _spi->transfer((sectors << 2) | 0x80);
    digitalWrite(csPin, HIGH); 
    _spi->endTransaction();
    while(isBusy()){}
}

void EEPROM_SPI_WE::protectStatusRegister(bool protect){
    uint8_t statusReg = eepromReadStatusReg();
    if(!protect && (wpPin != 255)){
        digitalWrite(wpPin, HIGH);
    }
    else if(protect && (wpPin != 255)){
        statusReg |= 0x80;
        eepromWriteEnable();
        eepromWriteStatusReg(statusReg);
        digitalWrite(wpPin, LOW);
    }
}

uint32_t EEPROM_SPI_WE::putString(uint32_t addr, String &strToWrite){
    int strLen = strToWrite.length() + 1; 
    writeEEPROM(addr, (uint8_t*)strToWrite.c_str(), strLen);
    return addr + strLen;
}

void EEPROM_SPI_WE::getString(uint32_t addr, String &strToRead){
    if(strToRead.length()){
        strToRead.remove(0,strToRead.length());
    }
    uint8_t tmp = 65; // dummy
    uint8_t *tmpPtr;
    tmpPtr = &tmp;
    while(tmp != 0){
        readEEPROM(addr, tmpPtr, 1);
        if(tmp != 0){
            strToRead += static_cast<char>(tmp);
        }
        addr++;
    }
}

uint8_t EEPROM_SPI_WE::eepromReadStatusReg(){
    uint8_t statusReg = 0;
    
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(EEP_RDSR); 
    statusReg = _spi->transfer(0x00); 
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    
    return statusReg;
}

bool EEPROM_SPI_WE::isBusy(){
    delayMicroseconds(50);
    uint8_t statusReg = eepromReadStatusReg();
    return (statusReg & 0x01);
}

void EEPROM_SPI_WE::setSPIClockSpeed(unsigned long clock){
    mySPISettings = SPISettings(clock, MSBFIRST, SPI_MODE0);
}

void EEPROM_SPI_WE::setSmallEEPROM(){
    smallEEPROM = true;
}

/************************************************ 
     Private Functions
*************************************************/

void EEPROM_SPI_WE::eepromWriteEnable(){
    eepromWriteStatusReg(EEP_WREN); 
}

void EEPROM_SPI_WE::eepromWriteStatusReg(uint8_t cmd){
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    _spi->transfer(cmd); 
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    
    while(isBusy()){}
}

void EEPROM_SPI_WE::writeEEPROM(uint32_t addr, const uint8_t *buf, uint16_t sizeOfBuf){
    uint16_t noOfBytesStillToWrite = sizeOfBuf;
    uint16_t arrayIndex = 0;
    
    while((noOfBytesStillToWrite != 0)){
        uint16_t chunk = noOfBytesStillToWrite;
        uint16_t positionInPage = addr % pageSize;
        uint16_t spaceLeftInPage = pageSize - positionInPage;
        
        if(spaceLeftInPage < noOfBytesStillToWrite){
            chunk = spaceLeftInPage;
        }
                
        eepromWriteEnable();
        
        _spi->beginTransaction(mySPISettings);
        digitalWrite(csPin, LOW);
        if(!smallEEPROM){
            _spi->transfer(EEP_WRITE);
	    if (memSize > EEPROM_KBITS_512) {
	      _spi->transfer(static_cast<uint8_t>(addr>>16));
	    }
            _spi->transfer(static_cast<uint8_t>(addr>>8));
            _spi->transfer(static_cast<uint8_t>(addr&0xFF));
        }
        else{
            if(addr>>8){
                if(addr>>9){
                    _spi->transfer(EEP_READ | EEP_A8 | EEP_A9);
                }
                else{
                    _spi->transfer(EEP_READ | EEP_A8);
                }
            }
            else {
                _spi->transfer(EEP_WRITE);
            }
            _spi->transfer(static_cast<uint8_t>(addr)); 
        }
        
        for(uint16_t i=arrayIndex; i<(arrayIndex + chunk); i++){
            _spi->transfer(static_cast<uint8_t>(buf[i]));
        }
        
        digitalWrite(csPin, HIGH);
        _spi->endTransaction();
        
        addr += chunk;
        arrayIndex += chunk;
        noOfBytesStillToWrite -= chunk;
        while(isBusy()){}       
    }
}

void EEPROM_SPI_WE::continuousPutEnable(uint32_t addr){
    contAddr = addr;
    eepromWriteEnable();
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    if(!smallEEPROM){
        _spi->transfer(EEP_WRITE);
	if (memSize > EEPROM_KBITS_512) {
	  _spi->transfer(static_cast<uint8_t>(addr>>16));
	}
        _spi->transfer(static_cast<uint8_t>(addr>>8));
        _spi->transfer(static_cast<uint8_t>(addr&0xFF));
    }
    else{
        if(addr>>8){
                _spi->transfer(EEP_WRITE | EEP_A8);
        }
        else {
            _spi->transfer(EEP_WRITE);
        }               
        _spi->transfer(static_cast<uint8_t>(addr)); 
    }
}

void EEPROM_SPI_WE::continuousPutDisable(){
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
    while(isBusy());
}

void EEPROM_SPI_WE::continuousWriteEEPROM(const uint8_t *buf, uint16_t sizeOfBuf){
    uint16_t noOfBytesStillToWrite = sizeOfBuf;
    uint16_t arrayIndex = 0;
    
    while((noOfBytesStillToWrite != 0)){
        uint16_t chunk = noOfBytesStillToWrite;
        uint16_t positionInPage = contAddr % pageSize;
        uint16_t spaceLeftInPage = pageSize - positionInPage;
        bool pageEnd = false;
        
        if(spaceLeftInPage < noOfBytesStillToWrite){
            chunk = spaceLeftInPage;
            pageEnd = true;
        }
               
        for(uint16_t i=arrayIndex; i<(arrayIndex + chunk); i++){
            _spi->transfer(static_cast<uint8_t>(buf[i]));
        }
        
        contAddr += chunk;
        arrayIndex += chunk;
        noOfBytesStillToWrite -= chunk;
        if(pageEnd){
            digitalWrite(csPin, HIGH);
            _spi->endTransaction();
            while(isBusy()){}
            continuousPutEnable(contAddr);
            pageEnd = false;
        }
    }
}

void EEPROM_SPI_WE::readEEPROM(uint32_t addr, uint8_t *buf, uint16_t sizeOfBuf){    
    _spi->beginTransaction(mySPISettings);
    digitalWrite(csPin, LOW);
    if(!smallEEPROM){
        _spi->transfer(EEP_READ);
	if (memSize > EEPROM_KBITS_512) {
	  _spi->transfer(static_cast<uint8_t>(addr>>16));
	}
        _spi->transfer(static_cast<uint8_t>(addr>>8));
        _spi->transfer(static_cast<uint8_t>(addr&0xFF));
    }
    else{
        if(addr>>8){
            if(addr>>9){
                _spi->transfer(EEP_READ | EEP_A8 | EEP_A9);
            }
            else{
                _spi->transfer(EEP_READ | EEP_A8);
            }
        }
        else {
            _spi->transfer(EEP_READ);
        }               
        _spi->transfer(static_cast<uint8_t>(addr)); 
    }
    for(uint16_t i=0; i<sizeOfBuf; i++){
        buf[i] = _spi->transfer(0x00);
    }
    digitalWrite(csPin, HIGH);
    _spi->endTransaction();
}



