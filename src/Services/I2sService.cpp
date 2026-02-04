#include "I2sService.h"
#include "math.h"

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
    #include <M5Unified.h>
#endif

static void stop_and_delete(i2s_chan_handle_t& ch) {
    if (!ch) return;
    i2s_channel_disable(ch);
    i2s_del_channel(ch);
    ch = nullptr;
}

void I2sService::configureOutput(uint8_t bclk, uint8_t lrck, uint8_t dout, uint32_t sampleRate, uint8_t bits) {
    // stop previous
    stop_and_delete(tx_chan);
    stop_and_delete(rx_chan);

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
    M5.Mic.end();
    M5.Speaker.begin();
#endif

    // create channel (TX only)
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, nullptr));
    isTx = true;

    // bits mapping
    i2s_data_bit_width_t w = I2S_DATA_BIT_WIDTH_16BIT;
    if (bits == 8)  w = I2S_DATA_BIT_WIDTH_8BIT;
    if (bits == 24) w = I2S_DATA_BIT_WIDTH_24BIT;
    if (bits == 32) w = I2S_DATA_BIT_WIDTH_32BIT;

    // std config
    i2s_std_config_t cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(w, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)bclk,
            .ws   = (gpio_num_t)lrck,
            .dout = (gpio_num_t)dout,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

    prevBclk = bclk;
    prevLrck = lrck;
    prevDout = dout;
    initialized = true;
}

void I2sService::configureInput(uint8_t bclk, uint8_t lrck, uint8_t din, uint32_t sampleRate, uint8_t bits) {
    stop_and_delete(tx_chan);
    stop_and_delete(rx_chan);

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_STICKS3)
    M5.Speaker.end();
    M5.Mic.begin();
#endif

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, nullptr, &rx_chan));
    isTx = false;

    i2s_data_bit_width_t w = I2S_DATA_BIT_WIDTH_16BIT;
    if (bits == 8)  w = I2S_DATA_BIT_WIDTH_8BIT;
    if (bits == 24) w = I2S_DATA_BIT_WIDTH_24BIT;
    if (bits == 32) w = I2S_DATA_BIT_WIDTH_32BIT;

    i2s_std_config_t cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(w, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = (gpio_num_t)bclk,
            .ws   = (gpio_num_t)lrck,
            .dout = I2S_GPIO_UNUSED,
            .din  = (gpio_num_t)din,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));

    prevBclk = bclk;
    prevLrck = lrck;
    prevDin  = din;
    initialized = true;
}


void I2sService::playTone(uint32_t sampleRate, uint16_t freq, uint16_t durationMs) {
    if (!initialized) return;

    int samples = (sampleRate * durationMs) / 1000;
    int16_t buffer[2];

    for (int i = 0; i < samples; ++i) {
        float t = (float)i / sampleRate;
        int16_t sample = sinf(2.0f * PI * freq * t) * 32767;
        buffer[0] = sample;
        buffer[1] = sample;
        size_t written;
        i2s_channel_write(tx_chan, buffer, sizeof(buffer), &written, portMAX_DELAY);
    }
}

void I2sService::playToneInterruptible(uint32_t sampleRate, uint16_t freq, uint32_t durationMs, std::function<bool()> shouldStop) {
    const int16_t amplitude = 32767;
    const int chunkDurationMs = 20;
    const int samplesPerChunk = (sampleRate * chunkDurationMs) / 1000;
    int16_t buffer[2 * samplesPerChunk];

    uint32_t elapsed = 0;
    while (elapsed < durationMs) {
        for (int i = 0; i < samplesPerChunk; ++i) {
            float t = (float)(i + (elapsed * sampleRate / 1000)) / sampleRate;
            int16_t sample = sinf(2.0f * PI * freq * t) * amplitude;
            buffer[2 * i] = sample;
            buffer[2 * i + 1] = sample;
        }

        size_t written;
        i2s_channel_write(tx_chan, buffer, sizeof(buffer), &written, portMAX_DELAY);
        elapsed += chunkDurationMs;

        if (shouldStop()) {
            break;
        }
    }
}

void I2sService::playPcm(const int16_t* data, size_t numBytes) {
    if (!initialized || data == nullptr || numBytes == 0) return;

    size_t bytesWritten = 0;
    i2s_channel_write(tx_chan, data, numBytes, &bytesWritten, portMAX_DELAY);
}

size_t I2sService::recordSamples(int16_t* outBuffer, size_t sampleCount) {
    if (!initialized) return 0;

    size_t totalRead = 0;
    size_t bytesToRead = sampleCount * sizeof(int16_t);

    uint8_t* buffer = reinterpret_cast<uint8_t*>(outBuffer);
    while (totalRead < bytesToRead) {
        size_t readBytes = 0;
        i2s_channel_read(rx_chan, buffer + totalRead, bytesToRead - totalRead, &readBytes, portMAX_DELAY);
        totalRead += readBytes;
    }

    return totalRead / sizeof(int16_t);
}

void I2sService::end() {
    stop_and_delete(tx_chan);
    stop_and_delete(rx_chan);
    initialized = false;
}

bool I2sService::isInitialized() const {
    return initialized;
}
