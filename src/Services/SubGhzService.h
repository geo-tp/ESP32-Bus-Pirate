#pragma once

#include <Arduino.h>
#include <vector>
#include <cstdint>
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/ringbuf.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "Data/SugGhzFreqs.h"
#include "Transformers/SubGhzTransformer.h"

#define RMT_RX_CHANNEL RMT_CHANNEL_6
#define RMT_TX_CHANNEL RMT_CHANNEL_5
#define RMT_CLK_DIV 80
#define RMT_1US_TICKS (80000000 / RMT_CLK_DIV / 1000000)
#define RMT_1MS_TICKS (RMT_1US_TICKS * 1000)
#define RMT_BUFFER_SIZE 8192

#define TEMBED_CC1101_SW0 48
#define TEMBED_CC1101_SW1 47

class SubGhzService {
public:
    // Configure CC1101
    bool configure(SPIClass& spi, uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss, uint8_t gdo0,
                   float mhz = 433.92f, // default 433.92mhz
                   int paDbm = 10); // TX power, max 12
    
    // Base
    void tune(float mhz);
    int measurePeakRssi(uint32_t holdMs);
    std::vector<std::string> getSupportedBand() const;
    std::vector<float> getSupportedFreq(const std::string& band) const;
    void setScanBand(const std::string& bandName);

    // RMT raw sniffer
    bool startRawSniffer(int pin);
    std::pair<std::string, size_t> readRawPulses();
    std::vector<rmt_item32_t> readRawFrame();
    bool isSnifferOverflowing() const;
    void drainSniffer();
    void stopRawSniffer();

    // Raw send
    bool startTxBitBang();
    bool stopTxBitBang();
    bool sendRawFrame(int pin,
                      const std::vector<rmt_item32_t>& items,
                      uint32_t tick_per_us = RMT_1US_TICKS);
    bool sendRandomBurst(int pin);
    bool sendRawPulse(int pin, int duration);
    bool sendRcSwitch_(uint64_t key, uint16_t bits, int te_us, int proto, int repeat);
    bool sendPrinceton_(uint64_t key, uint16_t bits, int te_us);
    bool sendBinRaw_(const std::vector<uint8_t>& bytes, int te_us, int bits, bool msb_first = true, bool invert = false);
    bool sendTimingsOOK_(const std::vector<int32_t>& timings); 
    bool sendRawTimings(const std::vector<int32_t>& timings);
    bool sendTimingsRawSigned_(const std::vector<int32_t>& timings);
    bool send(const SubGhzFileCommand& cmd);

    // Profiles
    bool applyDefaultProfile(float mhz = 433.92f);
    bool applySniffProfile(float mhz);
    bool applyRawSendProfile(float mhz);
    bool applyPresetByName(const std::string& name, float mhz);
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
    uint8_t rfSw0_ = TEMBED_CC1101_SW0;
    uint8_t rfSw1_ = TEMBED_CC1101_SW1;
    uint8_t rfSel_ = 2; //  uses 0/1/2 as selections

    // Tembed S3 CC1101 specific
    void initTembed();
    void selectRfPathFor(float mhz);
};
