#include "SubGhzService.h"
#include "driver/rmt.h"
#include <sstream>

// Base

bool SubGhzService::configure(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss,
                              uint8_t gdo0, float mhz, int paDbm)
{
    // Value assignment
    sck_ = sck; 
    miso_ = miso; 
    mosi_ = mosi; 
    ss_ = ss;
    gdo0_ = gdo0;
    mhz_ = mhz;
    paDbm_ = paDbm;
    ccMode_ = true;

    // Initialize CC1101
    ELECHOUSE_cc1101.setSpiPin(sck_, miso_, mosi_, ss_);
    ELECHOUSE_cc1101.setGDO0(gdo0_);
    ELECHOUSE_cc1101.Init();
    applyDefaultProfile();

    // Check if CC1101 is configured
    isConfigured_ = ELECHOUSE_cc1101.getCC1101();

    return isConfigured_;
}

void SubGhzService::tune(float mhz)
{
    if (!isConfigured_) return;
    mhz_ = mhz;
    ELECHOUSE_cc1101.SetRx(mhz_);
    delay(2);
}

int SubGhzService::measurePeakRssi(uint32_t holdMs)
{
    if (!isConfigured_) return -127;
    if (holdMs < 1) holdMs = 1;

    const unsigned long t0 = millis();
    int peak = -127;
    while (millis() - t0 < holdMs) {
        int r = ELECHOUSE_cc1101.getRssi();
        if (r > peak) peak = r;
        delay(1);
    }
    return peak;
}

std::vector<std::string> SubGhzService::getSupportedBand() const {
    return std::vector<std::string>(std::begin(kSubGhzScanBandNames), std::end(kSubGhzScanBandNames));
}

std::vector<float> SubGhzService::getSupportedFreq(const std::string& band) const {
    std::vector<float> out;

    int bandIdx = -1;
    for (int i = 0; i < 4; ++i) {
        if (band == kSubGhzScanBandNames[i]) {
            bandIdx = i;
            break;
        }
    }

    if (bandIdx < 0) {
        return out;
    }

    int start = kSubGhzRangeLimits[bandIdx][0];
    int end   = kSubGhzRangeLimits[bandIdx][1];

    out.reserve(static_cast<size_t>(end - start + 1));
    for (int i = start; i <= end; ++i) {
        out.push_back(kSubGhzFreqList[i]);
    }

    return out;
}

void SubGhzService::setScanBand(const std::string& s) {
    for (int i = 0; i < 4; ++i) {
        if (s == kSubGhzScanBandNames[i]) {
            scanBand_ = static_cast<SubGhzScanBand>(i);
            return;
        }
    }

    char* end = nullptr;
    long idx = std::strtol(s.c_str(), &end, 10);
    if (end != s.c_str()) {
        scanBand_ = static_cast<SubGhzScanBand>(idx);
    }
}

// Raw sniffer

bool SubGhzService::startRawSniffer(int pin)
{
    // Configure RMT
    rmt_config_t rxconfig;
    rxconfig.rmt_mode = RMT_MODE_RX;
    rxconfig.channel = RMT_RX_CHANNEL;
    rxconfig.gpio_num = (gpio_num_t)pin;
    rxconfig.clk_div = RMT_CLK_DIV;
    rxconfig.mem_block_num = 2;
    rxconfig.flags = 0;
    rxconfig.rx_config.idle_threshold = 3 * RMT_1MS_TICKS,
    rxconfig.rx_config.filter_ticks_thresh = 200 * RMT_1US_TICKS;
    rxconfig.rx_config.filter_en = true;

    // Install RMT driver
    if (rmt_config(&rxconfig) != ESP_OK) return false;
    if (rmt_driver_install(rxconfig.channel, 8192, 0) != ESP_OK) return false;

    // Get ring buffer handle
    if (rmt_get_ringbuf_handle(rxconfig.channel, &rb_) != ESP_OK) return false;
    rmt_rx_start(rxconfig.channel, true);

    return true;
}

std::vector<std::string> SubGhzService::readRawPulses() {
    std::vector<std::string> result;
    if (!rb_) return result;

    // Receive raw pulses from RMT
    size_t rx_size = 0;
    rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb_, &rx_size, 0);
    if (!item) return result;

    size_t n = rx_size / sizeof(rmt_item32_t);
    uint32_t totalDuration = 0;
    for (size_t i = 0; i < n; i++) {
        totalDuration += item[i].duration0;
        totalDuration += item[i].duration1;
    }

    // Header
    std::ostringstream oss;
    oss << "[raw " << n << " pulses | freq=" << mhz_ << " MHz | dur=" 
        << totalDuration << " ticks]\r\n";

    // Pulse details
    int col = 0;
    for (size_t i = 0; i < n; i++) {
        oss << (item[i].level0 ? 'H' : 'L') << ":" << item[i].duration0
            << " | "
            << (item[i].level1 ? 'H' : 'L') << ":" << item[i].duration1
            << "   ";

        if (++col % 4 == 0) oss << "\r\n";
    }

    oss << "\n\r";

    result.push_back(oss.str());
    vRingbufferReturnItem(rb_, (void*)item);
    return result;
}

void SubGhzService::stopRawSniffer() {
    rmt_rx_stop(RMT_RX_CHANNEL);
    rmt_driver_uninstall(RMT_RX_CHANNEL);
    rb_ = nullptr;
}

// Profiles

bool SubGhzService::applyScanProfile(float dataRateKbps,
                                  float rxBwKhz,
                                  uint8_t modulation,
                                  bool packetMode)
{
    if (!isConfigured_) return false;

    ELECHOUSE_cc1101.setCCMode(packetMode ? 0 : 1);
    ELECHOUSE_cc1101.setModulation(modulation);
    ELECHOUSE_cc1101.setDRate(dataRateKbps);
    ELECHOUSE_cc1101.setRxBW(rxBwKhz);
    ELECHOUSE_cc1101.setSyncMode(0);
    ELECHOUSE_cc1101.setWhiteData(false);
    ELECHOUSE_cc1101.setCrc(false);
    ELECHOUSE_cc1101.setCRC_AF(false);
    ELECHOUSE_cc1101.setAdrChk(0);
    ELECHOUSE_cc1101.setLengthConfig(1);
    ELECHOUSE_cc1101.setPacketLength(0xFF);
    ELECHOUSE_cc1101.SetRx(mhz_);
    return true;
}

bool SubGhzService::applyDefaultProfile(float mhz) {
    if (!isConfigured_) return false;
    ELECHOUSE_cc1101.setPktFormat(0);   // PKT_FORMAT=0 (packet mode normal)
    ELECHOUSE_cc1101.setLengthConfig(1);// 1 = variable length
    ELECHOUSE_cc1101.setPacketLength(0xFF);
    ELECHOUSE_cc1101.setCCMode(0);      // RAW / serial out sur GDO0
    ELECHOUSE_cc1101.setMHZ(mhz);
    ELECHOUSE_cc1101.setModulation(2);  // OOK/ASK
    ELECHOUSE_cc1101.setDRate(4.8f);    // kbps
    ELECHOUSE_cc1101.setRxBW(135.0f);   // kHz
    ELECHOUSE_cc1101.setSyncMode(0);
    ELECHOUSE_cc1101.setWhiteData(false);
    ELECHOUSE_cc1101.setCrc(false);
    ELECHOUSE_cc1101.setCRC_AF(false);
    ELECHOUSE_cc1101.setAdrChk(0);

    return true;
}

bool SubGhzService::applySniffProfile(float mhz) {
    if (!isConfigured_) return false;
    // CC1101 en "raw/async" OOK
    ELECHOUSE_cc1101.setMHZ(mhz);
    ELECHOUSE_cc1101.setModulation(2);   // 2 = ASK/OOK
    ELECHOUSE_cc1101.setDRate(4.8);      // ~5 kbps
    ELECHOUSE_cc1101.setRxBW(135.0);     // 58–200
    ELECHOUSE_cc1101.setSyncMode(0);     //no sync
    ELECHOUSE_cc1101.setWhiteData(0);
    ELECHOUSE_cc1101.setCrc(0);
    ELECHOUSE_cc1101.setAdrChk(0);
    ELECHOUSE_cc1101.setDcFilterOff(true);
    ELECHOUSE_cc1101.setPktFormat(3);    // *** ASYNCHRONOUS SERIAL MODE ***
    ELECHOUSE_cc1101.SetRx(mhz);
    return true;
}
