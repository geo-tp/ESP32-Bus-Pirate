#include "I2sService.h"
#include <math.h>

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
  #include <M5Unified.h>
#endif

static i2s_data_bit_width_t toWidth(uint8_t bits) {
    switch (bits) {
        case 8:  return I2S_DATA_BIT_WIDTH_8BIT;
        case 24: return I2S_DATA_BIT_WIDTH_24BIT;
        case 32: return I2S_DATA_BIT_WIDTH_32BIT;
        default: return I2S_DATA_BIT_WIDTH_16BIT;
    }
}

void I2sService::end() {
    if (initialized) {
        i2s.end();
        initialized = false;
    }
}

bool I2sService::isInitialized() const {
    return initialized;
}

inline void I2sService::writeStereo16(int16_t s) {
    // Left (low, high)
    i2s.write((uint8_t)(s & 0xFF));
    i2s.write((uint8_t)((s >> 8) & 0xFF));
    // Right (low, high)
    i2s.write((uint8_t)(s & 0xFF));
    i2s.write((uint8_t)((s >> 8) & 0xFF));
}

void I2sService::configureOutput(uint8_t bclk, uint8_t lrck, uint8_t dout, uint32_t sampleRate, uint8_t bits) {
    end();

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
    M5.Speaker.begin();
#endif

    i2s.setPins(bclk, lrck, dout);

    const i2s_mode_t mode = I2S_MODE_STD;
    const i2s_slot_mode_t slot = I2S_SLOT_MODE_STEREO;
    const i2s_data_bit_width_t w = toWidth(bits);

    if (!i2s.begin(mode, sampleRate, w, slot)) {
        initialized = false;
        return;
    }

    prevBclk = bclk;
    prevLrck = lrck;
    prevDout = dout;
    bitsPerSample = bits;
    sampleRateHz = sampleRate;
    isTx = true;
    initialized = true;
}

void I2sService::configureInput(uint8_t bclk, uint8_t lrck, uint8_t din,
                                uint32_t sampleRate, uint8_t bits) {
    end();

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
    M5.Speaker.end();
    M5.Mic.begin();
#endif

    i2s.setPins(bclk, lrck, -1, din);

    const i2s_mode_t mode = I2S_MODE_STD;
    const i2s_slot_mode_t slotMode = I2S_SLOT_MODE_MONO;
    const i2s_data_bit_width_t w = toWidth(bits);

    if (!i2s.begin(mode, sampleRate, w, slotMode, I2S_STD_SLOT_LEFT)) {
        initialized = false;
        return;
    }

    prevBclk = bclk;
    prevLrck = lrck;
    prevDin  = din;
    bitsPerSample = bits;
    sampleRateHz = sampleRate;
    isTx = false;
    initialized = true;
}

void I2sService::playTone(uint32_t /*sampleRate*/, uint16_t freq, uint16_t durationMs) {
    if (!initialized || !isTx) return;

    const uint32_t sr = sampleRateHz;  
    if (sr == 0 || freq == 0 || durationMs == 0) return;

    static constexpr size_t FRAMES = 256; 
    int16_t buf[FRAMES * 2];
    const int16_t amp = 32767;

    const uint32_t totalFrames = (sr * (uint32_t)durationMs) / 1000UL;
    uint32_t done = 0;

    while (done < totalFrames) {
        size_t n = FRAMES;
        if (done + n > totalFrames) n = (size_t)(totalFrames - done);

        for (size_t i = 0; i < n; i++) {
            float t = (float)(done + (uint32_t)i) / (float)sr;
            int16_t s = (int16_t)(sinf(2.0f * (float)M_PI * (float)freq * t) * (float)amp);

            buf[2 * i]     = s; // L
            buf[2 * i + 1] = s; // R
        }

        const size_t bytes = n * 2 * sizeof(int16_t);
        i2s.write((uint8_t*)buf, bytes);

        done += (uint32_t)n;
    }
}

void I2sService::playToneInterruptible(uint32_t /*sampleRate*/, uint16_t freq, uint32_t durationMs, std::function<bool()> shouldStop) {
    if (!initialized || !isTx) return;

    const uint32_t sr = sampleRateHz; 
    if (sr == 0 || freq == 0 || durationMs == 0) return;

    static constexpr size_t FRAMES = 256;
    int16_t buf[FRAMES * 2];
    const int16_t amp = 32767;

    const uint32_t totalFrames = (sr * durationMs) / 1000UL;
    uint32_t done = 0;

    while (done < totalFrames) {
        if (shouldStop && shouldStop()) break;

        size_t n = FRAMES;
        if (done + n > totalFrames) n = (size_t)(totalFrames - done);

        for (size_t i = 0; i < n; i++) {
            float t = (float)(done + (uint32_t)i) / (float)sr;
            int16_t s = (int16_t)(sinf(2.0f * (float)M_PI * (float)freq * t) * (float)amp);

            buf[2 * i]     = s;
            buf[2 * i + 1] = s;
        }

        const size_t bytes = n * 2 * sizeof(int16_t);
        i2s.write((uint8_t*)buf, bytes);

        done += (uint32_t)n;
    }
}

void I2sService::playPcm(const int16_t* data, size_t numBytes) {
    if (!initialized || !isTx || data == nullptr || numBytes == 0) return;

    const size_t sampleCount = numBytes / sizeof(int16_t);

    for (size_t i = 0; i < sampleCount; i++) {
        int16_t s = data[i];
        writeStereo16(s);
    }
}

size_t I2sService::recordSamples(int16_t* outBuffer, size_t sampleCount) {
    if (!initialized || outBuffer == nullptr || sampleCount == 0) return 0;

    const size_t bytesToRead = sampleCount * sizeof(int16_t);
    uint8_t* buffer = reinterpret_cast<uint8_t*>(outBuffer);

    size_t totalRead = 0;

    while (totalRead < bytesToRead) {
        int got = i2s.readBytes(reinterpret_cast<char*>(buffer + totalRead),
                                bytesToRead - totalRead);

        totalRead += (size_t)got;
    }

    return totalRead / sizeof(int16_t);
}