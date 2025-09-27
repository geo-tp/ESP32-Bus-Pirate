#include "SubGhzService.h"
#include "driver/rmt.h"
#include <sstream>

// Base

bool SubGhzService::configure(SPIClass& spi, uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss,
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


    // SPI instance
    #ifdef DEVICE_TEMBEDS3CC1101

    initTembed();
    ELECHOUSE_cc1101.setSPIinstance(&spi);

    #elif defined(DEVICE_M5STICK) || defined(DEVICE_CARDPUTER)

    SPI.end();
    delay(10);
    SPI.begin(sck_, miso_, mosi_, ss_);
    ELECHOUSE_cc1101.setSPIinstance(&SPI);

    #else

    ELECHOUSE_cc1101.setSPIinstance(&spi);

    #endif

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

    #ifdef DEVICE_TEMBEDS3CC1101

    selectRfPathFor(mhz_);

    #endif
 
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

bool SubGhzService::startRawSniffer(int pin) {
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
    if (rmt_driver_install(rxconfig.channel, 16 * 1024, 0) != ESP_OK) return false;

    // Get ring buffer handle
    if (rmt_get_ringbuf_handle(rxconfig.channel, &rb_) != ESP_OK) return false;
    rmt_rx_start(rxconfig.channel, true);

    return true;
}

void SubGhzService::stopRawSniffer() {
    rmt_rx_stop(RMT_RX_CHANNEL);
    rmt_driver_uninstall(RMT_RX_CHANNEL);
    rb_ = nullptr;
    ELECHOUSE_cc1101.setSidle();
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

std::vector<rmt_item32_t> SubGhzService::readRawFrame() {
    std::vector<rmt_item32_t> frame;
    if (!rb_) return frame;

    size_t rx_size = 0;
    rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb_, &rx_size, 0);
    if (!item) return frame;

    size_t n = rx_size / sizeof(rmt_item32_t);
    frame.assign(item, item + n);

    vRingbufferReturnItem(rb_, (void*)item);
    return frame;
}

bool SubGhzService::sendRawFrame(int pin, const std::vector<rmt_item32_t>& items, uint32_t /*tick_per_us_ignored*/) {
    if (!isConfigured_ || items.empty()) return false;


    if (!startTxBitBang()) return false;  // configure gdo0_

    // Stabilisation
    gpio_set_level((gpio_num_t)pin, 0);
    esp_rom_delay_us(200);

    for (const auto& it : items) {
        // phase 0
        gpio_set_level((gpio_num_t)pin, it.level0 ? 1 : 0);
        uint32_t us0 = it.duration0;
        if (us0) esp_rom_delay_us(us0);

        // phase 1
        gpio_set_level((gpio_num_t)pin, it.level1 ? 1 : 0);
        uint32_t us1 = it.duration1;
        if (us1) esp_rom_delay_us(us1);
    }

    // Stop
    gpio_set_level((gpio_num_t)pin, 0);
    esp_rom_delay_us(200);

    return true;
}

bool SubGhzService::startTxBitBang() {
    if (!isConfigured_) return false;

    gpio_config_t io{};
    io.pin_bit_mask = (1ULL << gdo0_);
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;
    return gpio_config(&io) == ESP_OK;
}

bool SubGhzService::stopTxBitBang() {
    if (!isConfigured_) return false;

    // Force low
    gpio_set_direction((gpio_num_t)gdo0_, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)gdo0_, 0);
    delay(1);

    // Reset to input
    gpio_config_t io{};
    io.pin_bit_mask  = (1ULL << gdo0_);
    io.mode          = GPIO_MODE_INPUT; 
    io.pull_up_en    = GPIO_PULLUP_DISABLE;
    io.pull_down_en  = GPIO_PULLDOWN_ENABLE;
    io.intr_type     = GPIO_INTR_DISABLE;

    ELECHOUSE_cc1101.setSidle();

    return gpio_config(&io) == ESP_OK;
}

bool SubGhzService::sendRawPulse(int pin, int duration) {
    if (!isConfigured_) return false;

    if (duration < 0) {
        gpio_set_level((gpio_num_t)pin, 0);
        delayMicroseconds(-duration);
    } else {
        gpio_set_level((gpio_num_t)pin, 1);
        delayMicroseconds(duration);
    }
    return true;
}

bool SubGhzService::sendRandomBurst(int pin)
{
    if (!isConfigured_) return false;

    constexpr int ITEMS_PER_BURST = 256;   // Number of items
    constexpr int MEAN_US         = 200;   // Mean duration in µs per phase
    constexpr int JITTER_PCT      = 30;    // around the mean

    // Calcul bornes jitter
    int j     = (MEAN_US * JITTER_PCT) / 100;
    int minUs = MEAN_US - j; if (minUs < 1) minUs = 1;
    int maxUs = MEAN_US + j; if (maxUs < minUs) maxUs = minUs;

    auto rnd_between = [&](int lo, int hi) -> uint32_t {
        uint32_t span = static_cast<uint32_t>(hi - lo + 1);
        return static_cast<uint32_t>(lo) + (esp_random() % span);
    };

    // Initial random level
    int level = (esp_random() & 1) ? 1 : 0;

    // Bit-bang
    for (int i = 0; i < ITEMS_PER_BURST; ++i) {
        // phase 0
        gpio_set_level((gpio_num_t)pin, level);
        level ^= 1;
        esp_rom_delay_us(rnd_between(minUs, maxUs));

        // phase 1
        gpio_set_level((gpio_num_t)pin, level);
        level ^= 1;
        esp_rom_delay_us(rnd_between(minUs, maxUs));
    }

    // Reset
    gpio_set_level((gpio_num_t)pin, 0);
    return true;
}

// Profiles

bool SubGhzService::applyScanProfile(float dataRateKbps,
                                  float rxBwKhz,
                                  uint8_t modulation,
                                  bool packetMode)
{
    if (!isConfigured_) return false;
    ELECHOUSE_cc1101.setSidle();
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
    ELECHOUSE_cc1101.setSidle();
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
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.setMHZ(mhz);
    ELECHOUSE_cc1101.setModulation(2);      // 2 = ASK/OOK
    ELECHOUSE_cc1101.setDRate(50);         // 50 kbps
    ELECHOUSE_cc1101.setRxBW(135.0f);     // 58–200
    ELECHOUSE_cc1101.setSyncMode(0);     // no sync
    ELECHOUSE_cc1101.setWhiteData(0);
    ELECHOUSE_cc1101.setCrc(0);
    ELECHOUSE_cc1101.setAdrChk(0);
    ELECHOUSE_cc1101.setDcFilterOff(true);
    ELECHOUSE_cc1101.setPktFormat(3);    // *** ASYNCHRONOUS SERIAL MODE ***
    ELECHOUSE_cc1101.SetRx(mhz);
    return true;
}

bool SubGhzService::applyRawSendProfile(float mhz) {
    if (!isConfigured_) return false;
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.setMHZ(mhz);
    ELECHOUSE_cc1101.setModulation(2);   // OOK/ASK
    ELECHOUSE_cc1101.setDRate(50);
    ELECHOUSE_cc1101.setRxBW(135.0f);
    ELECHOUSE_cc1101.setSyncMode(0);
    ELECHOUSE_cc1101.setWhiteData(0);
    ELECHOUSE_cc1101.setCrc(0);
    ELECHOUSE_cc1101.setAdrChk(0);
    ELECHOUSE_cc1101.setDcFilterOff(true);
    ELECHOUSE_cc1101.setPktFormat(3);    // ASYNC SERIAL MODE (GDO0 = data)
    ELECHOUSE_cc1101.SetTx();
    return true;
}

bool SubGhzService::applyPresetByName(const std::string& name, float mhz) {
    if (!isConfigured_) return false;
    ELECHOUSE_cc1101.setSidle();
    ELECHOUSE_cc1101.setMHZ(mhz);

    // Default
    ELECHOUSE_cc1101.setWhiteData(0);
    ELECHOUSE_cc1101.setCrc(0);
    ELECHOUSE_cc1101.setCRC_AF(0);
    ELECHOUSE_cc1101.setAdrChk(0);
    ELECHOUSE_cc1101.setSyncMode(0);

    if (name == "FuriHalSubGhzPresetOok270Async") {
        ELECHOUSE_cc1101.setModulation(2); // OOK
        ELECHOUSE_cc1101.setRxBW(270.0f);
        ELECHOUSE_cc1101.setDRate(10.0f);
        ELECHOUSE_cc1101.setPktFormat(3); // async serial
        ELECHOUSE_cc1101.SetTx();
        return true;
    }
    if (name == "FuriHalSubGhzPresetOok650Async") {
        ELECHOUSE_cc1101.setModulation(2);
        ELECHOUSE_cc1101.setRxBW(650.0f);
        ELECHOUSE_cc1101.setDRate(10.0f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }
    if (name == "FuriHalSubGhzPreset2FSKDev238Async") {
        ELECHOUSE_cc1101.setModulation(0); // 2-FSK
        ELECHOUSE_cc1101.setDeviation(2.380f);
        ELECHOUSE_cc1101.setRxBW(238.0f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }
    if (name == "FuriHalSubGhzPreset2FSKDev476Async") {
        ELECHOUSE_cc1101.setModulation(0);
        ELECHOUSE_cc1101.setDeviation(47.607f);
        ELECHOUSE_cc1101.setRxBW(476.0f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }
    if (name == "FuriHalSubGhzPresetMSK99_97KbAsync") {
        ELECHOUSE_cc1101.setModulation(4); // MSK
        ELECHOUSE_cc1101.setDeviation(47.607f);
        ELECHOUSE_cc1101.setDRate(99.97f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }
    if (name == "FuriHalSubGhzPresetGFSK9_99KbAsync") {
        ELECHOUSE_cc1101.setModulation(1); // GFSK
        ELECHOUSE_cc1101.setDeviation(19.043f);
        ELECHOUSE_cc1101.setDRate(9.996f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }

    // RcSwitch:
    char* end=nullptr;
    long proto = std::strtol(name.c_str(), &end, 10);
    if (end != name.c_str()) {
        ELECHOUSE_cc1101.setModulation(2); // OOK
        ELECHOUSE_cc1101.setRxBW(270.0f);
        ELECHOUSE_cc1101.setDRate(10.0f);
        ELECHOUSE_cc1101.setPktFormat(3);
        ELECHOUSE_cc1101.SetTx();
        return true;
    }

    // Fallback: OOK async
    return applyRawSendProfile(mhz);
}

bool SubGhzService::sendTimingsOOK_(const std::vector<int32_t>& timings) {
    if (!startTxBitBang()) return false;
    int level = 1; // start HIGH
    for (int32_t us : timings) {
        gpio_set_level((gpio_num_t)gdo0_, level);
        if (us > 0) esp_rom_delay_us((uint32_t)us);
        level ^= 1; // alternate
    }
    gpio_set_level((gpio_num_t)gdo0_, 0);
    stopTxBitBang();
    return true;
}

static inline void appendPair(std::vector<int32_t>& v, int hi_us, int lo_us) {
    if (hi_us>0) v.push_back(hi_us);
    if (lo_us>0) v.push_back(lo_us);
}

bool SubGhzService::sendRcSwitch_(uint64_t key, uint16_t bits, int te_us, int proto, int repeat) {
    if (!bits) return false;
    if (te_us <= 0) te_us = 350;      // défaut
    if (repeat <= 0) repeat = 10;

    // Usual protocols
    int sync_hi=0, sync_lo=0, zero_hi=0, zero_lo=0, one_hi=0, one_lo=0;

    if (proto == 1) {            // 350us, sync 1:31, zero 1:3, one 3:1
        sync_hi = te_us * 1;  sync_lo = te_us * 31;
        zero_hi = te_us * 1;  zero_lo = te_us * 3;
        one_hi  = te_us * 3;  one_lo  = te_us * 1;
    } else if (proto == 2) {     // 650us, sync 1:10, zero 1:2, one 2:1
        sync_hi = te_us * 1;  sync_lo = te_us * 10;
        zero_hi = te_us * 1;  zero_lo = te_us * 2;
        one_hi  = te_us * 2;  one_lo  = te_us * 1;
    } else {                     // “11” commonly used: 350us, sync 1:23, zero 1:2, one 2:1
        proto   = 11;
        sync_hi = te_us * 1;  sync_lo = te_us * 23;
        zero_hi = te_us * 1;  zero_lo = te_us * 2;
        one_hi  = te_us * 2;  one_lo  = te_us * 1;
    }

    std::vector<int32_t> timings;
    timings.reserve((bits * 2 + 4) * repeat);

    for (int r = 0; r < repeat; ++r) {
        // sync
        appendPair(timings, sync_hi, sync_lo);

        // bits MSB->LSB
        for (int i = bits - 1; i >= 0; --i) {
            bool b = (key >> i) & 1ULL;
            if (b) appendPair(timings, one_hi, one_lo);
            else   appendPair(timings, zero_hi, zero_lo);
        }
        // inter-frame gap
    }

    return sendTimingsOOK_(timings);
}

bool SubGhzService::sendPrinceton_(uint64_t key, uint16_t bits, int te_us) {
    // Canon PT2262: TE≈350us, 1=1h3l, 0=3h1l, sync=1h31l
    return sendRcSwitch_(key, bits, te_us>0?te_us:350, /*proto*/1, /*repeat*/10);
}

bool SubGhzService::sendBinRaw_(const std::vector<uint8_t>& bytes,
                                int te_us,
                                int bits /*ignored in ref mode*/,
                                bool /*msb_first unused in ref mode*/,
                                bool /*invert unused in ref mode*/) {
    if (bytes.empty()) return false;
    if (te_us <= 0) te_us = 100;

    // Impl. de référence : idle LOW, envoi depuis la FIN de la chaîne binaire,
    // donc: parcourir les octets à rebours, et dans chaque octet LSB->MSB.
    const int total_bits = int(bytes.size()) * 8;
    // // Si tu veux respecter "Bit:", décommente:
    // int limit_bits = (bits > 0 && bits < total_bits) ? bits : total_bits;
    const int limit_bits = total_bits; // référence = envoie tout

    if (!startTxBitBang()) return false;

    // Idle bas
    gpio_set_level((gpio_num_t)gdo0_, 0);

    int sent = 0;

    // Octets depuis la fin
    for (int bi = int(bytes.size()) - 1; bi >= 0 && sent < limit_bits; --bi) {
        uint8_t b = bytes[bi];

        // Bits LSB -> MSB
        for (int i = 0; i < 8 && sent < limit_bits; ++i, ++sent) {
            bool one = (b >> i) & 0x01; // LSB-first
            gpio_set_level((gpio_num_t)gdo0_, one ? 1 : 0);
            esp_rom_delay_us((uint32_t)te_us);
        }
    }

    // Idle bas en fin de trame
    gpio_set_level((gpio_num_t)gdo0_, 0);

    stopTxBitBang();
    return true;
}

bool SubGhzService::sendRawTimings(const std::vector<int32_t>& timings) {
    return sendTimingsRawSigned_(timings);
}

bool SubGhzService::sendTimingsRawSigned_(const std::vector<int32_t>& timings) {
    if (!startTxBitBang()) return false;

    // Idle LOW
    gpio_set_level((gpio_num_t)gdo0_, 0);

    for (int32_t t : timings) {
        if (t == 0) continue;
        bool high = (t > 0);
        uint32_t us = (uint32_t)(high ? t : -t);
        gpio_set_level((gpio_num_t)gdo0_, high ? 1 : 0);
        if (us) esp_rom_delay_us(us);
    }

    // Return to idle LOW
    gpio_set_level((gpio_num_t)gdo0_, 0);
    stopTxBitBang();
    return true;
}

bool SubGhzService::send(const SubGhzFileCommand& cmd) {
    if (!isConfigured_) return false;

    float mhz = cmd.frequency_hz ? (cmd.frequency_hz / 1e6f) : mhz_;
    tune(mhz);  // update antenna (selectRfPathFor) and RX

    if (!applyPresetByName(cmd.preset, mhz)) {
        // fallback async OOK TX
        if (!applyRawSendProfile(mhz)) return false;
    }

    // IMPORTANT: in async TX mode, GDO0 = data input -> we bit-bang on gdo0_
    switch (cmd.protocol) {
        case SubGhzProtocolEnum::RAW:
            return sendRawTimings(cmd.raw_timings);

        case SubGhzProtocolEnum::BinRAW: {
            const int te = cmd.te_us ? cmd.te_us : 100;
            const int total_bits = int(cmd.bitstream_bytes.size()) * 8;
            return sendBinRaw_(cmd.bitstream_bytes, te, total_bits,
                            /*msb_first*/false, /*invert*/false);
        }

        case SubGhzProtocolEnum::RcSwitch: {
            int proto = 11;
            char* end=nullptr;
            long p = std::strtol(cmd.preset.c_str(), &end, 10);
            if (end != cmd.preset.c_str()) proto = (int)p;
            int te = cmd.te_us ? cmd.te_us : ((proto==2)?650:350);
            return sendRcSwitch_(cmd.key, cmd.bits ? cmd.bits : 24, te, proto, /*repeat*/10);
        }

        case SubGhzProtocolEnum::Princeton:
            return sendPrinceton_(cmd.key, cmd.bits ? cmd.bits : 24, cmd.te_us ? cmd.te_us : 350);

        default: {
            // Fallback CAME/HOLTEK/NICE/unk like RcSwitch(11)
            // TE 270 µs, 10 rep
            if (!cmd.key) return false;
            const int te   = cmd.te_us ? cmd.te_us : 270;
            const int bits = cmd.bits  ? cmd.bits  : 24;
            return sendRcSwitch_(cmd.key, bits, te, /*proto*/11, /*repeat*/10);
        }
    }
}

// Tembed S3 CC1101 specific

void SubGhzService::initTembed() {
    // POWER
    const int BOARD_PWR_EN   = 15;

    // I2C
    const int BOARD_SDA_PIN  = 18;
    const int BOARD_SCL_PIN  = 8;

    // TFT
    const int BOARD_TFT_CS   = 41;

    // --------- SPI bus ---------
    const int BOARD_SPI_SCK  = 11;
    const int BOARD_SPI_MOSI = 9;
    const int BOARD_SPI_MISO = 10;

    // TF card
    const int BOARD_SD_CS    = 13;
    const int BOARD_SD_SCK   = BOARD_SPI_SCK;
    const int BOARD_SD_MOSI  = BOARD_SPI_MOSI;
    const int BOARD_SD_MISO  = BOARD_SPI_MISO;

    // LoRa / CC1101 front-end
    const int BOARD_LORA_CS  = 12;
    const int BOARD_LORA_SCK = BOARD_SPI_SCK;
    const int BOARD_LORA_MOSI= BOARD_SPI_MOSI;
    const int BOARD_LORA_MISO= BOARD_SPI_MISO;
    const int BOARD_LORA_IO2 = 38;
    const int BOARD_LORA_IO0 = 3;

    // Disable others spi devices
    pinMode(41, OUTPUT);
    digitalWrite(41, HIGH);
    pinMode(BOARD_SD_CS, OUTPUT);
    digitalWrite(BOARD_SD_CS, HIGH);
    pinMode(BOARD_LORA_CS, OUTPUT);
    digitalWrite(BOARD_LORA_CS, HIGH);

    // Set antenna frequency range, 433 (H/H) as default
    pinMode(rfSw1_, OUTPUT);
    pinMode(rfSw0_, OUTPUT);
    digitalWrite(rfSw1_, HIGH);
    digitalWrite(rfSw0_, HIGH);

    // Power the CC1101
    pinMode(BOARD_PWR_EN, OUTPUT);
    digitalWrite(BOARD_PWR_EN, HIGH);
}

void SubGhzService::selectRfPathFor(float mhz) {
    if (rfSw0_ < 0 || rfSw1_ < 0) return;

    // Map CC1101 antenna to bands

    uint8_t sel; // 0=315, 1=868/915, 2=433
    if (mhz >= 300.0f && mhz <= 348.0f) {
        sel = 0; // 315
    } else if ((mhz >= 387.0f && mhz <= 464.0f)) {
        sel = 2; // 433
    } else if (mhz >= 779.0f && mhz <= 928.0f) {
        sel = 1; // 868/915
    } else {
        // fallback
        sel = 2;
    }

    if (sel == rfSel_) return; // no change
    rfSel_ = sel;

    switch (sel) {
        case 0: // 315 MHz: SW1=1, SW0=0
            digitalWrite(rfSw1_, HIGH);
            digitalWrite(rfSw0_, LOW);
            break;
        case 1: // 868/915 MHz: SW1=0, SW0=1
            digitalWrite(rfSw1_, LOW);
            digitalWrite(rfSw0_, HIGH);
            break;
        case 2: // 433 MHz: SW1=1, SW0=1
        default:
            digitalWrite(rfSw1_, HIGH);
            digitalWrite(rfSw0_, HIGH);
            break;
    }
}
