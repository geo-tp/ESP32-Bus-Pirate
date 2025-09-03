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

    #ifdef DEVICE_TEMBEDS3CC1101

    initTembed();

    #endif

    // Initialize CC1101
    ELECHOUSE_cc1101.setSPIinstance(&spi);
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

bool SubGhzService::sendRawFrame(int pin, const std::vector<rmt_item32_t>& items, uint32_t tick_per_us) {
    if (!isConfigured_ || items.empty()) return false;

    // Output
    gpio_config_t io{};
    io.pin_bit_mask = (1ULL << pin);
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&io) != ESP_OK) return false;

    // tick to us
    auto ticks_to_us = [tick_per_us](uint32_t ticks) -> uint32_t {
        if (!tick_per_us) return 0;
        return (ticks + (tick_per_us/2)) / tick_per_us;
    };

    // Bit bang seq
    for (const auto& it : items) {
        gpio_set_level((gpio_num_t)pin, it.level0 ? 1 : 0);
        uint32_t us0 = ticks_to_us(it.duration0);
        if (us0) esp_rom_delay_us(us0);

        gpio_set_level((gpio_num_t)pin, it.level1 ? 1 : 0);
        uint32_t us1 = ticks_to_us(it.duration1);
        if (us1) esp_rom_delay_us(us1);
    }

    // Reset
    gpio_set_level((gpio_num_t)pin, 0);
    return true;
}

bool SubGhzService::sendRandomBurst(int pin)
{
    if (!isConfigured_) return false;

    // --- Paramètres interne simples (modifiable plus tard si besoin) ---
    constexpr int ITEMS_PER_BURST = 256;   // nb d'items (couples phase0/phase1)
    constexpr int MEAN_US         = 200;   // largeur moyenne en µs par phase
    constexpr int JITTER_PCT      = 30;    // ±% autour de la moyenne
    // -------------------------------------------------------------------

    // Config pin en sortie (une fois par appel, simple et robuste)
    gpio_config_t io{};
    io.pin_bit_mask = (1ULL << pin);
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_DISABLE;
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.intr_type = GPIO_INTR_DISABLE;
    if (gpio_config(&io) != ESP_OK) return false;

    // Calcul bornes jitter
    int j     = (MEAN_US * JITTER_PCT) / 100;
    int minUs = MEAN_US - j; if (minUs < 1) minUs = 1;
    int maxUs = MEAN_US + j; if (maxUs < minUs) maxUs = minUs;

    auto rnd_between = [&](int lo, int hi) -> uint32_t {
        uint32_t span = static_cast<uint32_t>(hi - lo + 1);
        return static_cast<uint32_t>(lo) + (esp_random() % span);
    };

    // Niveau initial aléatoire
    int level = (esp_random() & 1) ? 1 : 0;

    // Bit-bang : on alterne H/L avec durées aléatoires (phase0/phase1)
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

    // Idle bas
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
    ELECHOUSE_cc1101.SetTx(mhz_);
    return true;
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
