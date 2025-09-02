#pragma once

#include <Arduino.h>
#include <vector>
#include <cstdint>
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "Data/SugGhzFreqs.h"

#define RMT_RX_CHANNEL RMT_CHANNEL_6
#define RMT_CLK_DIV 80
#define RMT_1US_TICKS (80000000 / RMT_CLK_DIV / 1000000)
#define RMT_1MS_TICKS (RMT_1US_TICKS * 1000)

class SubGhzService {
public:
    // Configure CC1101
    bool configure(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss, uint8_t gdo0,
                   float mhz = 433.92f, // default 433.92mhz
                   int paDbm = 10); // TX power, max 10
    
    // Base
    void tune(float mhz);
    int measurePeakRssi(uint32_t holdMs);
    std::vector<std::string> getSupportedBand() const;
    std::vector<float> getSupportedFreq(const std::string& band) const;
    void setScanBand(const std::string& bandName);

    // RMT raw sniffer
    bool startRawSniffer(int pin);
    std::vector<std::string> readRawPulses();
    void stopRawSniffer();
    
    // Profiles
    bool applyDefaultProfile(float mhz = 433.92f);
    bool applySniffProfile(float mhz);
    bool applyScanProfile(float dataRateKbps = 4.8f,
                          float rxBwKhz      = 200.0f,
                          uint8_t modulation = 2,    // 2 = OOK/ASK
                          bool packetMode    = true);

private:
    bool    isConfigured_ = false;
    uint8_t sck_ = 0, miso_ = 0, mosi_ = 0, ss_ = 0;
    uint8_t gdo0_ = 0;
    float   mhz_ = 433.92f;
    int     paDbm_ = 10;
    bool    ccMode_ = false;
    SubGhzScanBand scanBand_ = SubGhzScanBand::Band387_464;
    RingbufHandle_t rb_ = nullptr;
};
