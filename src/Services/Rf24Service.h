#pragma once

#include <Arduino.h>
#include <RF24.h>
#include <vector>
#include <string>

class Rf24Service {
public:
    // Configuration
    bool configure(
        uint8_t csnPin,
        uint8_t cePin,
        uint8_t sckPin,
        uint8_t misoPin,
        uint8_t mosiPin,
        uint32_t spiSpeed = 10000000
    );

    void initRx();

    // Base
    void setChannel(uint8_t channel);              // 0..125
    uint8_t getChannel();
    void setDataRate(rf24_datarate_e rate);        // RF24_250KBPS, RF24_1MBPS, RF24_2MBPS
    void setCrcLength(rf24_crclength_e length);    // RF24_CRC_DISABLED, RF24_CRC_8, RF24_CRC_16
    void powerUp();
    void powerDown(bool hard = false);
    void setPowerLevel(rf24_pa_dbm_e level);       // RF24_PA_MIN .. RF24_PA_MAX
    void setPowerMax();

    // Communication
    void openWritingPipe(const uint64_t address);  
    void openReadingPipe(uint8_t number, const uint64_t address);
    void startListening();
    void stopListening();

    // IO
    bool send(const void* buf, uint8_t len);
    bool receive(void* buf, uint8_t len);
    bool available();

    // Utils
    bool isChipConnected();
    void flushTx();
    void flushRx();
    bool testCarrier();
    bool testRpd();
    void printDetails();

private:
    RF24* radio_ = nullptr;
    bool isInitialized = false;
    uint8_t cePin_ = 0;
    uint8_t csnPin_ = 0;
    uint8_t sckPin_ = 0;
    uint8_t misoPin_ = 0;
    uint8_t mosiPin_ = 0;
    uint32_t spiSpeed_ = 10000000;
};
