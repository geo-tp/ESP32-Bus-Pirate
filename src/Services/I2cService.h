#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <vector>
#include "Models/ByteCode.h"
#include <SparkFun_External_EEPROM.h>

class I2cService {
public:
    // Base
    void configure(uint8_t sda, uint8_t scl, uint32_t frequency = 100000);
    void beginTransmission(uint8_t address);
    void write(uint8_t data);
    bool endTransmission(bool sendStop = true);
    uint8_t requestFrom(uint8_t address, uint8_t quantity, bool sendStop = true);
    int read();
    bool available() const;
    bool end() const;
    bool isReadableDevice(uint8_t addr, uint8_t startReg);

    // I2C Bit bang
    void i2cBitBangDelay(uint32_t delayUs);
    void i2cBitBangSetLevel(uint8_t pin, bool level);
    void i2cBitBangSetOutput(uint8_t pin);
    void i2cBitBangSetInput(uint8_t pin);
    void i2cBitBangWriteBit(uint8_t scl, uint8_t sda, bool bit, uint32_t delayUs);
    void i2cBitBangWriteByte(uint8_t scl, uint8_t sda, uint8_t data, uint32_t delayUs, bool& ack);
    void i2cBitBangReadByte(uint8_t scl, uint8_t sda, uint32_t delayUs, bool nackLast);
    void i2cBitBangStartCondition(uint8_t scl, uint8_t sda, uint32_t delayUs);
    void i2cBitBangStopCondition(uint8_t scl, uint8_t sda, uint32_t delayUs);
    bool i2cBitBangRecoverBus(uint8_t scl, uint8_t sda, uint32_t freqHz);

    // Slave
    void beginSlave(uint8_t address, uint8_t sda, uint8_t scl, uint32_t freq = 100000);
    void endSlave();
    void setSlaveResponse(const uint8_t* data, size_t len);
    std::vector<std::string> getSlaveLog();
    void clearSlaveLog();

    // Glitch
    void rapidStartStop(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void floodRandom(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void floodStart(uint8_t address, uint32_t freqHz, uint8_t scl, uint8_t sda);
    void overReadAttack(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void invalidRegisterRead(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void simulateClockStretch(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void sclSdaGlitch(uint8_t sclPin, uint8_t sdaPin);
    void randomClockPulseNoise(uint8_t sclPin, uint8_t sdaPin, uint32_t freqHz);
    void glitchAckInjection(uint8_t address, uint32_t freqHz, uint8_t sclPin, uint8_t sdaPin);
    void injectRandomGlitch(uint8_t sclPin, uint8_t sdaPin, uint32_t freqHz);

    // Instructions
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);

    // EEPROM
    bool initEeprom(uint16_t chipSizeKb = 512, uint8_t addr=0x50);
    bool eepromWriteByte(uint16_t address, uint8_t value);
    uint8_t eepromReadByte(uint16_t address);
    bool eepromPutString(uint32_t address, const std::string& str);
    bool eepromGetString(uint32_t address, std::string& outStr);
    uint32_t eepromLength();
    uint32_t eepromGetMemorySize();
    uint16_t eepromPageSize();
    uint8_t  eepromWriteTimeMs();
    uint8_t  eepromAddressBytes();
    bool     eepromIsConnected();
    bool     eepromIsBusy();
    void     eepromErase(uint8_t fill = 0xFF);
    bool     eepromDetectMemorySize();
    uint8_t  eepromDetectAddressBytes();
    uint16_t eepromDetectPageSize();
    uint8_t  eepromDetectWriteTime(uint8_t testCount = 8);


private:
    static std::vector<std::string> slaveLog;
    static portMUX_TYPE slaveLogMux;
    static uint8_t slaveResponseBuffer[16];
    static size_t slaveResponseLength;
    static I2cService* activeSlaveInstance;
    ExternalEEPROM eeprom;
    static void onSlaveReceive(int len);
    static void onSlaveRequest();


};
