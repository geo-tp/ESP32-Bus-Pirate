#include "RfidService.h"

// Base

void RfidService::configure(uint8_t sda, uint8_t scl) {
    _sda = sda;
    _scl = scl;
    _configured = true;
}

bool RfidService::begin() {
    if (!_configured) {
        return false;
    }

    if (_begun) {
        return true;
    }

    _begun = _rfid.begin(_sda, _scl);
    return _begun;
}

// Operations

int RfidService::read(int cardBaudRate)      { return _rfid.read(cardBaudRate); }
int RfidService::write(int cardBaudRate)     { return _rfid.write(cardBaudRate); }
int RfidService::write_ndef()                { return _rfid.write_ndef(); }
int RfidService::erase()                     { return _rfid.erase(); }
int RfidService::clone(bool checkSak)        { return _rfid.clone(checkSak); }

// Getters

std::string RfidService::uid() const         { return std::string(_rfid.printableUID.uid.c_str()); }
std::string RfidService::sak() const         { return std::string(_rfid.printableUID.sak.c_str()); }
std::string RfidService::atqa() const        { return std::string(_rfid.printableUID.atqa.c_str()); }
std::string RfidService::piccType() const    { return std::string(_rfid.printableUID.picc_type.c_str()); }
std::string RfidService::allPages() const    { return std::string(_rfid.strAllPages.c_str()); }

// Setters

void RfidService::setUid(const std::string& uidHex) {
    // Fill printable fields
    _rfid.printableUID.uid  = uidHex.c_str();
}

void RfidService::setSak(const std::string& sakHex) {
    _rfid.printableUID.sak = sakHex.c_str();     // e.g. "08"
}

void RfidService::setAtqa(const std::string& atqaHex) {
    _rfid.printableUID.atqa = atqaHex.c_str();    // e.g. "00 04"
}

// Pages

int  RfidService::totalPages() const         { return _rfid.totalPages; }
int  RfidService::dataPages() const          { return _rfid.dataPages; }
bool RfidService::pageReadOk() const {
    return _rfid.pageReadSuccess && (_rfid.pageReadStatus == RFIDInterface::SUCCESS);
}

// Helpers

std::string RfidService::statusMessage(int rc) const {
    return std::string(_rfid.statusMessage(rc).c_str());
}

void RfidService::loadDump(const std::string& dump) {
    if (!dump.empty() && dump.back() == '\n') {
        _rfid.strAllPages = dump.c_str();
    } else {
        _rfid.strAllPages = (dump + "\n").c_str();
    }
}

void RfidService::parseData() {
    _rfid.parse_data();
}

std::vector<std::string> RfidService::getTagTypes() const {
    return { " MIFARE / ISO14443A", " FeliCa" };
}

std::vector<std::string> RfidService::getMifareFamily() const {
    return  { " MIFARE Classic (16 bytes)", " NTAG/Ultralight (4 bytes)" };
}


