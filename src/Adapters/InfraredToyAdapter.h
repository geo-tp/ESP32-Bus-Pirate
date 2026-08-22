#pragma once

#include <Arduino.h>
#include <array>
#include <vector>
#include <algorithm>
#include <new>
#include "driver/gpio.h"
#include "esp32-hal-rmt.h"
#include "soc/gpio_struct.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IHostSerial.h"

// Exclusive one-shot USB CDC adapter compatible with the USB IR Toy protocol.

struct InfraredToyConfig {
    uint8_t txPin;
    uint8_t rxPin;
};

class InfraredToyAdapter {
public:
    static void run(const InfraredToyConfig& config, IInput& input, IHostSerial& hostSerial);

private:
    enum class ProtocolState : uint8_t {
        COMMAND_MODE,
        SAMPLE_RX_MODE,
        TX_RECEIVE_CHUNKS,
        TX_TRANSMITTING,
    };

    static constexpr uint32_t IRTOY_UNIT_US = 21;
    static constexpr uint16_t IRTOY_END_MARKER = 0xFFFF;

    static constexpr uint8_t TX_CHUNK_SIZE = 64;
    static_assert(TX_CHUNK_SIZE > 0, "TX_CHUNK_SIZE must not be zero");

    static constexpr size_t USB_READ_CHUNK_SIZE = 64;
    static constexpr size_t SERIAL_RX_BUFFER_SIZE = 4096;

    static constexpr size_t RX_RING_CAPACITY = 512;
    static constexpr size_t MAX_TX_BYTES = 8192;

    static constexpr uint32_t INPUT_POLL_INTERVAL_MS = 25;

    static constexpr uint32_t RMT_RESOLUTION_HZ = 500000;
    static constexpr uint32_t RMT_TICK_US = 2;
    static constexpr uint32_t RMT_RX_TIMEOUT_US = 40000;
    static constexpr uint16_t RMT_MAX_DURATION_TICKS = 0x7FFF;
    static constexpr uint32_t RMT_MAX_DURATION_US = RMT_MAX_DURATION_TICKS * RMT_TICK_US;
    static constexpr bool IRTOY_TX_CARRIER_ACTIVE_LEVEL = false;

    static void begin();
    static bool allocateLazyBuffers();
    static void pollUsb();

    static void ensureIrHardwareConfigured();
    static void startRxCapture();
    static void stopRxCapture();

    static void handleHostBytes(const uint8_t* data, size_t len);
    static void handleCommandByte(uint8_t b);

    static bool writeExact(const uint8_t* data, size_t len, uint32_t timeoutMs);
    static void sendVersion();

    static void enterSampleMode();
    static void exitSampleMode();
    static void resetProtocolState();

    static void startTransmitUpload();
    static void requestTxChunk();
    static void processTxByte(uint8_t b);
    static void finishTransmit(bool success);
    static void transmitUploadedDurations();
    static void captureRxFrame();

    static bool pushRxDurationUs(uint32_t us);
    static void appendTxDuration(std::vector<rmt_data_t>& symbols, bool pulse, uint32_t us);

    static void flushRxDurationsToUsb();

    static uint16_t convertUsToIrToyTicks(uint32_t us);
    static uint32_t convertIrToyTicksToUs(uint16_t ticks);
    static uint16_t convertUsToRmtTicksClamped(uint32_t us);

    static bool trackResetSequence(uint8_t b);
    static bool trackTxStartSequence(uint8_t b);

    static void writeBe16(uint16_t value);

    static bool rxRingEmpty();
    static void clearRxRing();
    static void pushRxDuration(uint16_t value);
    static uint16_t popRxDuration();

    static inline uint8_t fastReadPin(uint8_t pin) {
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32C5) || defined(CONFIG_IDF_TARGET_ESP32H2)
        return (GPIO.in.val >> pin) & 0x1;
#else
        if (pin < 32) {
            return (GPIO.in >> pin) & 0x1;
        }

        return (GPIO.in1.val >> (pin - 32)) & 0x1;
#endif
    }

    static inline void fastWritePinLow(uint8_t pin) {
#if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32C5) || defined(CONFIG_IDF_TARGET_ESP32H2)
        GPIO.out_w1tc.val = (1UL << pin);
#else
        if (pin < 32) {
            GPIO.out_w1tc = (1UL << pin);
            return;
        }

        GPIO.out1_w1tc.val = (1UL << (pin - 32));
#endif
    }

    static inline InfraredToyConfig config = {0, 0};
    static inline IHostSerial* hostSerial = nullptr;
    static inline IInput* input = nullptr;

    static inline ProtocolState state = ProtocolState::COMMAND_MODE;
    static inline ProtocolState txReturnState = ProtocolState::COMMAND_MODE;

    static inline std::vector<uint32_t> txDurationsUs = {};
    static inline uint16_t* rxRing = nullptr;

    static inline size_t rxHead = 0;
    static inline size_t rxTail = 0;

    static inline size_t txByteCount = 0;
    static inline uint32_t lastInputPollMs = 0;

    static inline uint8_t resetSequenceIndex = 0;
    static inline uint8_t txStartSequenceIndex = 0;
    static inline uint8_t lircTxPrefixIndex = 0;

    static inline uint8_t ioCommandBytesRemaining = 0;
    static inline uint8_t txExpectedBytes = 0;
    static inline uint8_t txHighByte = 0;

    static inline bool irHardwareConfigured = false;
    static inline bool rxCaptureActive = false;
    static inline bool rxFrameActive = false;

    static inline bool txPendingHighByte = false;
    static inline bool txControlByteConsumed = false;
    static inline bool txLircDoubleBuffered = false;

    static inline uint8_t rxIdleLevel = 1;
    static inline uint8_t rxLastLevel = 1;
    static inline uint32_t rxLastEdgeUs = 0;

    static inline uint32_t rxOverflowCount = 0;
};
