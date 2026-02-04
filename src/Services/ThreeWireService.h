#pragma once

#include <Arduino.h>
#include <vector>
#include <string>

extern "C" {
    #include "93Cx6.h"
}

class ThreeWireService {
public:
    void configure(uint8_t cs, uint8_t sk, uint8_t di, uint8_t doPin, int16_t model = 66, bool org8 = false);
    void end();

    uint16_t read16(uint16_t addr);
    uint8_t read8(uint16_t addr);
    void write16(uint16_t addr, uint16_t value);
    void write8(uint16_t addr, uint8_t value);
    void writeAll(uint16_t value);

    void erase(uint16_t addr);
    void eraseAll();

    std::vector<uint8_t> dump8();
    std::vector<uint16_t> dump16();

    void writeEnable();
    void writeDisable();
    bool isWriteEnabled();

    std::vector<std::string> getSupportedModels() const;
    int resolveModelId(const std::string& modelStr) const;
private:
    EEPROM_T eeprom;
    uint16_t eepromSizeBytes = 0;
    int16_t eepromOrgMode = EEPROM_MODE_16BIT;
};
