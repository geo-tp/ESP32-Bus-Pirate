#pragma once

#include <vector>
#include <atomic>
#include <deque>
#include <mutex>
#include <Arduino.h>
#include <EEPROM_SPI_WE.h>
#include <SPI.h>
#include <Data/FlashDatabase.h>
#include <Models/ByteCode.h>

class SpiService {
public:
    // Base
    void configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency = 1000000);
    void end();
    void beginTransaction();
    void endTransaction();
    uint8_t transfer(uint8_t data);

    // Flash
    std::string readFlashID();
    void readFlashIdRaw(uint8_t* buffer);
    void readFlashData(uint32_t address, uint8_t* buffer, size_t length);
    uint32_t calculateFlashCapacity(uint8_t code);
    void eraseFlashSector(uint32_t address, uint32_t freq);
    void enableFlashWrite(uint32_t freq);
    void waitForFlashWriteComplete(uint32_t freq);
    void writeFlashPage(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq);
    void writeFlashPatch(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq);

    // EEPROM
    bool initEeprom(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint16_t pageSize, uint32_t memSize, uint16_t wp=999, bool small=false);
    bool probeEeprom();
    bool writeEeprom(uint32_t address, uint8_t value);
    uint8_t readEeprom(uint32_t address);
    bool writeEepromBuffer(uint32_t address, const uint8_t* data, size_t len);
    bool readEepromBuffer(uint32_t address, uint8_t* buffer, size_t len);
    bool writeEepromInt(uint32_t address, int32_t value);
    int32_t readEepromInt(uint32_t address);
    bool writeEepromFloat(uint32_t address, float value);
    float readEepromFloat(uint32_t address);
    bool writeEepromString(uint32_t address, const std::string& str);
    bool readEepromString(uint32_t address, std::string& str);
    void eraseEepromChip();
    void eraseEepromSector(uint32_t address);
    void eraseEepromPage(uint32_t address);
    void closeEeprom();

    // Slave
    void startSlave(int sclk, int miso, int mosi, int cs);
    void stopSlave(int sclk, int miso, int mosi, int cs);
    bool isSlave() const;
    std::vector<std::vector<uint8_t>> getSlaveData();

    // Instructions
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
private:
    uint8_t csPin;
    uint32_t spiFrequency = 1000000;
    EEPROM_SPI_WE eeprom = EEPROM_SPI_WE(&SPI, SPI_CS_PIN, 999, 8000000);
    bool eepromInitialized = false;
    uint32_t eepromFrequency = 8000000;

};


