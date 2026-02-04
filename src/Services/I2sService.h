#pragma once

#include <stdint.h>
#include <Arduino.h>
#include <functional>
#include <stddef.h>
#include <string>
#include "driver/i2s_std.h"

class I2sService {
public:
    void configureOutput(uint8_t bclk, uint8_t lrck, uint8_t dout, uint32_t sampleRate = 44100, uint8_t bits = 16);
    void configureInput(uint8_t bclk, uint8_t lrck, uint8_t din, uint32_t sampleRate = 44100, uint8_t bits = 16);

    void playTone(uint32_t sampleRate, uint16_t freq, uint16_t durationMs);
    void playToneInterruptible(uint32_t sampleRate, uint16_t freq, uint32_t durationMs, std::function<bool()> shouldStop);
    void playPcm(const int16_t* data, size_t numBytes);
    size_t recordSamples(int16_t* outBuffer, size_t sampleCount);
    void end();
    bool isInitialized() const;    
private:
    bool initialized = false;

    i2s_chan_handle_t tx_chan = nullptr;
    i2s_chan_handle_t rx_chan = nullptr;
    bool isTx = false;

    uint8_t prevBclk = GPIO_NUM_NC;
    uint8_t prevLrck = GPIO_NUM_NC;
    uint8_t prevDout = GPIO_NUM_NC;
    uint8_t prevDin  = GPIO_NUM_NC;

};
