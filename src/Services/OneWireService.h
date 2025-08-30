#pragma once

#include <OneWire.h>
#include <vector>
#include "Models/ByteCode.h"

#define DS2431_FAMILY 0x2D
#define DS2433_FAMILY 0x23
#define DS28EC20_FAMILY 0x43

class OneWireService {
public:
    OneWireService();

    void configure(uint8_t pin);
    void close();
    bool reset();
    void write(uint8_t data);
    void writeBytes(const uint8_t* data, uint8_t len);
    uint8_t read();
    void readBytes(uint8_t* buffer, uint8_t length);
    void skip();
    void select(const uint8_t rom[8]);
    uint8_t crc8(const uint8_t* data, uint8_t len);
    void resetSearch();
    bool search(uint8_t* rom);
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);

    // RW1990
    void writeRw1990(uint8_t pin, uint8_t* data, size_t len);

    #ifndef DEVICE_M5STICK

    #include "OneWireNg_CurrentPlatform.h"

    // EEPROM
    void configureEeprom(uint8_t pin);
    void closeEeprom();
    bool getEepromModelInfos(uint8_t* romId, std::string& model, uint16_t& size, uint8_t& pageSize);

    // EEPROM DS24/28
    bool eeprom2431Probe(uint8_t* outId = nullptr);
    uint8_t eeprom2431ReadByte(uint16_t address);
    std::vector<uint8_t> eeprom2431Dump(uint16_t startAddress, uint16_t length);
    bool eeprom2431WriteRow(uint8_t rowAddr, const uint8_t* rowData, bool checkDataIntegrity = true);

    #else

    // Dummy implementations, no space left on the M5STICK for the EEPROM lib
    inline void configureEeprom(uint8_t) {}
    inline void closeEeprom() {}
    inline bool getEepromModelInfos(uint8_t*, std::string&, uint16_t&, uint8_t&) { return false; }
    inline bool eeprom2431Probe(uint8_t* = nullptr) { return false; }
    inline uint8_t eeprom2431ReadByte(uint16_t) { return 0xFF; }
    inline std::vector<uint8_t> eeprom2431Dump(uint16_t, uint16_t len) { return std::vector<uint8_t>(len, 0xFF); }
    inline bool eeprom2431WriteRow(uint8_t, const uint8_t*, bool = true) { return false; }

    #endif

private:
    OneWire* oneWire = nullptr;
    uint8_t oneWirePin = 0;
    
    #ifndef DEVICE_M5STICK

    OneWireNg* owEeprom = nullptr;
    OneWireNg::Id eepromId = {};

    #endif
};
