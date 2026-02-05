#pragma once
#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <ESP_I2S.h>

class I2sService {
public:
    void configureOutput(uint8_t bclk, uint8_t lrck, uint8_t dout, uint32_t sampleRate, uint8_t bits);
    void configureInput(uint8_t bclk, uint8_t lrck, uint8_t din,  uint32_t sampleRate, uint8_t bits);

    void playTone(uint32_t sampleRate, uint16_t freq, uint16_t durationMs);
    void playToneInterruptible(uint32_t sampleRate, uint16_t freq, uint32_t durationMs, std::function<bool()> shouldStop);

    void playPcm(const int16_t* data, size_t numBytes);
    size_t recordSamples(int16_t* outBuffer, size_t sampleCount);

    void end();
    bool isInitialized() const;

private:
    I2SClass i2s;
    bool initialized = false;
    bool isTx = false;

    // track config
    uint8_t prevBclk = 0, prevLrck = 0, prevDout = 0, prevDin = 0;
    uint8_t bitsPerSample = 16;
    uint32_t sampleRateHz = 8000;

    // helpers 
    inline void writeStereo16(int16_t s);
};
