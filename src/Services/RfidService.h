#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <Wire.h>
#include "Vendors/PN532.h" 

class RfidService {
public:
    // Base, I2C only
    void configure(uint8_t sda, uint8_t scl);
    bool begin();

    // Operations
    int  read(int cardBaudRate = 0);  // 0 = MIFARE, 1 = FeliCa
    int  write(int cardBaudRate = 0);
    int  write_ndef();
    int  erase();
    int  clone(bool checkSak = true);

    // Getters
    std::string uid() const;          // ex: "04 A2 1B ..."
    std::string sak() const;          // ex: "08"
    std::string atqa() const;         // ex: "00 04"
    std::string piccType() const;     // ex: "MIFARE 1K"

    // Setters
    void  setUid(const std::string& uidHex);
    void  setSak(const std::string& sakHex);
    void  setAtqa(const std::string& atqaHex);
    
    // Pages
    int  totalPages() const;
    int  dataPages() const;
    bool pageReadOk() const;
    std::string allPages() const;     // dump
    void loadDump(const std::string& dump);

    // Helpers
    std::vector<std::string> getTagTypes() const;
    std::vector<std::string> getMifareFamily() const;
    std::string statusMessage(int rc) const;
    void parseData();

private:
    uint8_t  _sda   = 1;
    uint8_t  _scl   = 2;
    bool     _configured = false;
    bool     _begun      = false;

    PN532 _rfid { PN532::CONNECTION_TYPE::I2C };
};
