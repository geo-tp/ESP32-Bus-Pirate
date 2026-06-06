#include "SumpLogicAnalyzerAdapter.h"
#include "Inputs/InputKeys.h"
#include "Utils/HostSerial.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "soc/gpio_reg.h"
#include "soc/soc.h"
#include <algorithm>

uint32_t SumpLogicAnalyzerAdapter::calibrateCycleCounterHz() {
    uint32_t c0 = ESP.getCycleCount();
    int64_t t0 = esp_timer_get_time();

    delay(20);

    uint32_t c1 = ESP.getCycleCount();
    int64_t t1 = esp_timer_get_time();

    uint32_t deltaCycles = c1 - c0;
    uint64_t deltaUs = static_cast<uint64_t>(t1 - t0);

    if (deltaCycles == 0 || deltaUs == 0) {
        return ESP.getCpuFreqMHz() * 1000000UL;
    }

    uint32_t measuredHz = static_cast<uint32_t>(
        (static_cast<uint64_t>(deltaCycles) * 1000000ULL) / deltaUs
    );

    if (measuredHz == 0) {
        return ESP.getCpuFreqMHz() * 1000000UL;
    }

    return measuredHz;
}

void IRAM_ATTR SumpLogicAnalyzerAdapter::waitUntilCycle(uint32_t targetCycle) {
    while (static_cast<int32_t>(ESP.getCycleCount() - targetCycle) < 0) {
    }
}

void SumpLogicAnalyzerAdapter::run(const SumpLogicAnalyzerConfig& config, IInput& input) {
    hostSerialDisableReboot();
    Serial.setRxBufferSize(1024);
    hostSerialBegin();

    configure(config, input);

    constexpr uint32_t INPUT_POLL_INTERVAL_MS = 25;
    uint32_t lastInputPollMs = 0;

    while (true) {
        uint8_t command = 0;
        if (readCommandByte(command)) {
            handleCommand(command);
        }

        uint32_t now = millis();
        if ((uint32_t)(now - lastInputPollMs) >= INPUT_POLL_INTERVAL_MS) {
            lastInputPollMs = now;

            if (inputRequestedReset()) {
                ESP.restart();
            }
        }

        yield();
    }
}

void SumpLogicAnalyzerAdapter::configure(const SumpLogicAnalyzerConfig& config, IInput& input) {
    activeConfig = config;
    activeInput = &input;

    maxSampleCount = calculateMaxSampleCount();

    lastDebugRequestedDivider = 0;
    lastDebugRequestedSampleRate = config.sampleRate;

    sampleRate = constrain(config.sampleRate, MIN_SAMPLE_RATE, MAX_SAMPLE_RATE);
    sampleCount = constrain(config.sampleCount, 1UL, maxSampleCount);
    channelCount = constrain(config.channelCount, static_cast<uint8_t>(1), MAX_CHANNELS);
    enabledChannelGroups = 0x01;

    for (uint8_t i = 0; i < channelCount; ++i) {
        pinMode(activeConfig.pins[i], INPUT);
    }

    prepareFastGpioMap();
}

SumpCaptureDebugInfo SumpLogicAnalyzerAdapter::getLastCaptureDebugInfo() {
    return {
        lastDebugRequestedDivider,
        lastDebugRequestedSampleRate,
        lastDebugEffectiveSampleRate,
        lastDebugSampleCount,
        lastDebugFastPathUsed,
        lastDebugCaptureStartUs,
        lastDebugCaptureEndUs,
        lastDebugCaptureDurationUs,
        lastDebugCaptureDurationMs,
        lastDebugMeasuredSamplesPerSecond
    };
}

void SumpLogicAnalyzerAdapter::handleCommand(uint8_t command) {
    if (command == SUMP_RESET) {
        resetSumpState();
        return;
    }

    if (command == SUMP_RUN) {
        handleRun();
        return;
    }

    if (command == SUMP_ID) {
        Serial.write(reinterpret_cast<const uint8_t*>("1ALS"), 4);
        return;
    }

    if (command == SUMP_METADATA) {
        writeMetadata();
        return;
    }

    if (command == SUMP_XON || command == SUMP_XOFF) {
        return;
    }

    if (command == SUMP_SET_DIVIDER) {
        uint32_t divider = readLe32();

        uint32_t requestedSampleRate = SUMP_CLOCK_HZ / (divider + 1);

        lastDebugRequestedDivider = divider;
        lastDebugRequestedSampleRate = requestedSampleRate;

        sampleRate = constrain(requestedSampleRate, MIN_SAMPLE_RATE, MAX_SAMPLE_RATE);
        return;
    }

    if (command == SUMP_SET_READ_DELAY) {
        uint16_t readCount = readLe16();
        readLe16();

        uint32_t requestedSampleCount = (static_cast<uint32_t>(readCount) + 1UL) * 4UL;
        sampleCount = constrain(requestedSampleCount, 1UL, maxSampleCount);
        return;
    }

    if (command == SUMP_SET_FLAGS) {
        handleFlags(readLe32());
        return;
    }

    if (command >= 0xC0 && command <= 0xCF) {
        discardBytes(4);
    }
}

void SumpLogicAnalyzerAdapter::handleRun() {
    capture();

    if (captureAborted || samples == nullptr || sampleCount == 0) {
        return;
    }

    // The libsigrok OLS driver expects classic OLS memory dump order:
    // newest sample first. It reverses the stream back to chronological order
    // on the host side. Sending samples[0..N-1] here makes PulseView display
    // the capture reversed in time.
    if (enabledChannelGroups == 0x01) {
        for (uint32_t i = sampleCount; i > 0; --i) {
            if (((sampleCount - i) & (UPLOAD_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
                return;
            }

            Serial.write(samples[i - 1]);
        }
        return;
    }

    for (uint32_t i = sampleCount; i > 0; --i) {
        if (((sampleCount - i) & (UPLOAD_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
            return;
        }

        uint8_t sample = samples[i - 1];

        if (enabledChannelGroups & 0x01) {
            Serial.write(sample);
        }

        for (uint8_t group = 1; group < 4; ++group) {
            if (enabledChannelGroups & (1U << group)) {
                Serial.write(static_cast<uint8_t>(0));
            }
        }
    }
}

void SumpLogicAnalyzerAdapter::handleFlags(uint32_t flags) {
    (void)flags;

    // MVP captures only D0..D7.
    enabledChannelGroups = 0x01;
}

void SumpLogicAnalyzerAdapter::resetSumpState() {
    captureAborted = true;
    enabledChannelGroups = 0x01;

    while (Serial.available() > 0) {
        Serial.read();
    }
}

bool SumpLogicAnalyzerAdapter::consumePendingReset() {
    while (Serial.available() > 0) {
        int command = Serial.peek();

        if (command == SUMP_RESET) {
            resetSumpState();
            return true;
        }

        if (command == SUMP_XON || command == SUMP_XOFF) {
            Serial.read();
            continue;
        }

        return false;
    }

    return false;
}

uint32_t SumpLogicAnalyzerAdapter::calculateMaxSampleCount() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t maxAllocHeap = ESP.getMaxAllocHeap();
    uint32_t usableHeap = std::min(freeHeap, maxAllocHeap);

    if (usableHeap <= MIN_CAPTURE_HEAP_RESERVE + DEFAULT_SAMPLE_COUNT) {
        return DEFAULT_SAMPLE_COUNT;
    }

    uint32_t availableForSamples = usableHeap - MIN_CAPTURE_HEAP_RESERVE;
    availableForSamples = std::max<uint32_t>(availableForSamples, DEFAULT_SAMPLE_COUNT);

    return std::min<uint32_t>(availableForSamples, MAX_PROTOCOL_SAMPLE_COUNT);
}

void SumpLogicAnalyzerAdapter::prepareFastGpioMap() {
    hasHighPins = false;

    uint32_t lowMasks[MAX_CHANNELS] = {};
    uint32_t highMasks[MAX_CHANNELS] = {};

    for (uint8_t i = 0; i < channelCount; ++i) {
        uint8_t pin = activeConfig.pins[i];

        if (pin < 32) {
            lowMasks[i] = 1UL << pin;
        } else {
            hasHighPins = true;
            highMasks[i] = 1UL << (pin - 32);
        }
    }

    lowMask0 = lowMasks[0];
    lowMask1 = lowMasks[1];
    lowMask2 = lowMasks[2];
    lowMask3 = lowMasks[3];
    lowMask4 = lowMasks[4];
    lowMask5 = lowMasks[5];
    lowMask6 = lowMasks[6];
    lowMask7 = lowMasks[7];

    highMask0 = highMasks[0];
    highMask1 = highMasks[1];
    highMask2 = highMasks[2];
    highMask3 = highMasks[3];
    highMask4 = highMasks[4];
    highMask5 = highMasks[5];
    highMask6 = highMasks[6];
    highMask7 = highMasks[7];
}

void SumpLogicAnalyzerAdapter::writeMetadata() {
    writeStringMetadata(0x01, "ESP32-BP SUMP");
    writeStringMetadata(0x02, "Polling MVP");
    writeUint32Metadata(0x20, MAX_CHANNELS);
    writeUint32Metadata(0x21, maxSampleCount);
    writeUint32Metadata(0x23, MAX_SAMPLE_RATE);
    writeUint32Metadata(0x24, 2);
    Serial.write(static_cast<uint8_t>(0x00));
}

void SumpLogicAnalyzerAdapter::writeStringMetadata(uint8_t key, const char* value) {
    Serial.write(key);
    Serial.print(value);
    Serial.write(static_cast<uint8_t>(0x00));
}

void SumpLogicAnalyzerAdapter::writeUint32Metadata(uint8_t key, uint32_t value) {
    Serial.write(key);
    writeBe32(value);
}

void SumpLogicAnalyzerAdapter::writeUint8Metadata(uint8_t key, uint8_t value) {
    Serial.write(key);
    Serial.write(value);
}

bool SumpLogicAnalyzerAdapter::readCommandByte(uint8_t& value) {
    if (Serial.available() <= 0) {
        return false;
    }

    int readValue = Serial.read();
    if (readValue < 0) {
        return false;
    }

    value = static_cast<uint8_t>(readValue);
    return true;
}

uint32_t SumpLogicAnalyzerAdapter::readLe32() {
    for (uint8_t i = 0; i < 4; ++i) {
        while (Serial.available() <= 0) {
            if (inputRequestedReset()) {
                ESP.restart();
            }
            delay(1);
        }

        commandBuffer[i] = static_cast<uint8_t>(Serial.read());
    }

    return static_cast<uint32_t>(commandBuffer[0])
        | (static_cast<uint32_t>(commandBuffer[1]) << 8)
        | (static_cast<uint32_t>(commandBuffer[2]) << 16)
        | (static_cast<uint32_t>(commandBuffer[3]) << 24);
}

uint16_t SumpLogicAnalyzerAdapter::readLe16() {
    for (uint8_t i = 0; i < 2; ++i) {
        while (Serial.available() <= 0) {
            if (inputRequestedReset()) {
                ESP.restart();
            }
            delay(1);
        }

        commandBuffer[i] = static_cast<uint8_t>(Serial.read());
    }

    return static_cast<uint16_t>(commandBuffer[0])
        | (static_cast<uint16_t>(commandBuffer[1]) << 8);
}

void SumpLogicAnalyzerAdapter::discardBytes(uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) {
        while (Serial.available() <= 0) {
            if (inputRequestedReset()) {
                ESP.restart();
            }
            delay(1);
        }

        Serial.read();
    }
}

void SumpLogicAnalyzerAdapter::writeBe32(uint32_t value) {
    Serial.write(static_cast<uint8_t>((value >> 24) & 0xFF));
    Serial.write(static_cast<uint8_t>((value >> 16) & 0xFF));
    Serial.write(static_cast<uint8_t>((value >> 8) & 0xFF));
    Serial.write(static_cast<uint8_t>(value & 0xFF));
}

bool SumpLogicAnalyzerAdapter::allocateSampleBuffer(uint32_t requestedSampleCount) {
    if (samples != nullptr && sampleCapacity >= requestedSampleCount) {
        return true;
    }

    if (samples != nullptr) {
        heap_caps_free(samples);
        samples = nullptr;
        sampleCapacity = 0;
    }

    samples = static_cast<uint8_t*>(
        heap_caps_malloc(requestedSampleCount, MALLOC_CAP_8BIT)
    );

    if (samples == nullptr) {
        return false;
    }

    sampleCapacity = requestedSampleCount;
    return true;
}

void SumpLogicAnalyzerAdapter::capture() {
    captureAborted = false;

    sampleRate = constrain(sampleRate, MIN_SAMPLE_RATE, MAX_SAMPLE_RATE);
    sampleCount = constrain(sampleCount, 1UL, maxSampleCount);

    if (!allocateSampleBuffer(sampleCount)) {
        captureAborted = true;
        sampleCount = 0;
        lastDebugEffectiveSampleRate = sampleRate;
        lastDebugSampleCount = 0;
        lastDebugFastPathUsed = false;
        lastDebugCaptureStartUs = 0;
        lastDebugCaptureEndUs = 0;
        lastDebugCaptureDurationUs = 0;
        lastDebugCaptureDurationMs = 0;
        lastDebugMeasuredSamplesPerSecond = 0;
        return;
    }

    lastDebugEffectiveSampleRate = sampleRate;
    lastDebugSampleCount = sampleCount;
    lastDebugFastPathUsed = false;

    captureTimed();
}

void SumpLogicAnalyzerAdapter::captureTimed() {
    uint32_t cycleCounterHz = calibrateCycleCounterHz();

    if (sampleRate == 0) {
        sampleRate = MIN_SAMPLE_RATE;
    }

    uint64_t cyclesPerSampleQ32 =
        (static_cast<uint64_t>(cycleCounterHz) << 32) / sampleRate;

    if (cyclesPerSampleQ32 == 0) {
        cyclesPerSampleQ32 = 1;
    }

    uint32_t startCycle = ESP.getCycleCount();
    uint64_t nextCycleQ32 = 0;

    lastDebugCaptureStartUs = esp_timer_get_time();

    if (!hasHighPins && channelCount == 6) {
        for (uint32_t i = 0; i < sampleCount; ++i) {
            if ((i & (CAPTURE_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
                return;
            }

            uint32_t targetCycle = startCycle + static_cast<uint32_t>(nextCycleQ32 >> 32);
            waitUntilCycle(targetCycle);

            samples[i] = readSample6LowOnlyFast();
            nextCycleQ32 += cyclesPerSampleQ32;
        }
    } else if (!hasHighPins) {
        for (uint32_t i = 0; i < sampleCount; ++i) {
            if ((i & (CAPTURE_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
                return;
            }

            uint32_t targetCycle = startCycle + static_cast<uint32_t>(nextCycleQ32 >> 32);
            waitUntilCycle(targetCycle);

            samples[i] = readSample8LowOnlyFast();
            nextCycleQ32 += cyclesPerSampleQ32;
        }
    } else if (channelCount == 6) {
        for (uint32_t i = 0; i < sampleCount; ++i) {
            if ((i & (CAPTURE_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
                return;
            }

            uint32_t targetCycle = startCycle + static_cast<uint32_t>(nextCycleQ32 >> 32);
            waitUntilCycle(targetCycle);

            samples[i] = readSample6Fast();
            nextCycleQ32 += cyclesPerSampleQ32;
        }
    } else {
        for (uint32_t i = 0; i < sampleCount; ++i) {
            if ((i & (CAPTURE_ABORT_CHECK_INTERVAL - 1)) == 0 && consumePendingReset()) {
                return;
            }

            uint32_t targetCycle = startCycle + static_cast<uint32_t>(nextCycleQ32 >> 32);
            waitUntilCycle(targetCycle);

            samples[i] = readSample8Fast();
            nextCycleQ32 += cyclesPerSampleQ32;
        }
    }

    lastDebugCaptureEndUs = esp_timer_get_time();
    lastDebugCaptureDurationUs = lastDebugCaptureEndUs - lastDebugCaptureStartUs;
    lastDebugCaptureDurationMs = static_cast<uint32_t>(
        lastDebugCaptureDurationUs / 1000ULL
    );

    lastDebugMeasuredSamplesPerSecond = lastDebugCaptureDurationUs == 0 ? 0 : static_cast<uint32_t>(
        (static_cast<uint64_t>(sampleCount) * 1000000ULL) / lastDebugCaptureDurationUs
    );
}

void SumpLogicAnalyzerAdapter::captureAsFastAsPossible() {
    lastDebugEffectiveSampleRate = sampleRate;
    lastDebugSampleCount = sampleCount;
    lastDebugFastPathUsed = true;
    lastDebugCaptureStartUs = esp_timer_get_time();

    uint32_t i = 0;

    if (channelCount == 6 && hasHighPins) {
        for (; i + 7 < sampleCount; i += 8) {
            samples[i + 0] = readSample6Fast();
            samples[i + 1] = readSample6Fast();
            samples[i + 2] = readSample6Fast();
            samples[i + 3] = readSample6Fast();
            samples[i + 4] = readSample6Fast();
            samples[i + 5] = readSample6Fast();
            samples[i + 6] = readSample6Fast();
            samples[i + 7] = readSample6Fast();
        }

        for (; i < sampleCount; ++i) {
            samples[i] = readSample6Fast();
        }

        goto capture_done;
    }

    if (hasHighPins) {
        for (; i + 7 < sampleCount; i += 8) {
            samples[i + 0] = readSample8Fast();
            samples[i + 1] = readSample8Fast();
            samples[i + 2] = readSample8Fast();
            samples[i + 3] = readSample8Fast();
            samples[i + 4] = readSample8Fast();
            samples[i + 5] = readSample8Fast();
            samples[i + 6] = readSample8Fast();
            samples[i + 7] = readSample8Fast();
        }

        for (; i < sampleCount; ++i) {
            samples[i] = readSample8Fast();
        }

        goto capture_done;
    }

    if (channelCount == 6) {
        for (; i + 7 < sampleCount; i += 8) {
            samples[i + 0] = readSample6LowOnlyFast();
            samples[i + 1] = readSample6LowOnlyFast();
            samples[i + 2] = readSample6LowOnlyFast();
            samples[i + 3] = readSample6LowOnlyFast();
            samples[i + 4] = readSample6LowOnlyFast();
            samples[i + 5] = readSample6LowOnlyFast();
            samples[i + 6] = readSample6LowOnlyFast();
            samples[i + 7] = readSample6LowOnlyFast();
        }

        for (; i < sampleCount; ++i) {
            samples[i] = readSample6LowOnlyFast();
        }

        goto capture_done;
    }

    for (; i + 7 < sampleCount; i += 8) {
        samples[i + 0] = readSample8LowOnlyFast();
        samples[i + 1] = readSample8LowOnlyFast();
        samples[i + 2] = readSample8LowOnlyFast();
        samples[i + 3] = readSample8LowOnlyFast();
        samples[i + 4] = readSample8LowOnlyFast();
        samples[i + 5] = readSample8LowOnlyFast();
        samples[i + 6] = readSample8LowOnlyFast();
        samples[i + 7] = readSample8LowOnlyFast();
    }

    for (; i < sampleCount; ++i) {
        samples[i] = readSample8LowOnlyFast();
    }

capture_done:
    lastDebugCaptureEndUs = esp_timer_get_time();
    lastDebugCaptureDurationUs = lastDebugCaptureEndUs - lastDebugCaptureStartUs;
    lastDebugCaptureDurationMs = static_cast<uint32_t>(
        lastDebugCaptureDurationUs / 1000ULL
    );

    lastDebugMeasuredSamplesPerSecond = lastDebugCaptureDurationUs == 0 ? 0 : static_cast<uint32_t>(
        (static_cast<uint64_t>(sampleCount) * 1000000ULL) / lastDebugCaptureDurationUs
    );
}

uint8_t SumpLogicAnalyzerAdapter::readSample6Fast() {
    uint8_t value = 0;
    uint32_t low = REG_READ(GPIO_IN_REG);
    uint32_t high = REG_READ(GPIO_IN1_REG);

    if ((low & lowMask0) || (high & highMask0)) value |= 0x01;
    if ((low & lowMask1) || (high & highMask1)) value |= 0x02;
    if ((low & lowMask2) || (high & highMask2)) value |= 0x04;
    if ((low & lowMask3) || (high & highMask3)) value |= 0x08;
    if ((low & lowMask4) || (high & highMask4)) value |= 0x10;
    if ((low & lowMask5) || (high & highMask5)) value |= 0x20;

    return value;
}

uint8_t SumpLogicAnalyzerAdapter::readSample6LowOnlyFast() {
    uint8_t value = 0;
    uint32_t low = REG_READ(GPIO_IN_REG);

    if (low & lowMask0) value |= 0x01;
    if (low & lowMask1) value |= 0x02;
    if (low & lowMask2) value |= 0x04;
    if (low & lowMask3) value |= 0x08;
    if (low & lowMask4) value |= 0x10;
    if (low & lowMask5) value |= 0x20;

    return value;
}

uint8_t SumpLogicAnalyzerAdapter::readSample8Fast() {
    uint8_t value = 0;
    uint32_t low = REG_READ(GPIO_IN_REG);
    uint32_t high = REG_READ(GPIO_IN1_REG);

    if ((low & lowMask0) || (high & highMask0)) value |= 0x01;
    if ((low & lowMask1) || (high & highMask1)) value |= 0x02;
    if ((low & lowMask2) || (high & highMask2)) value |= 0x04;
    if ((low & lowMask3) || (high & highMask3)) value |= 0x08;
    if ((low & lowMask4) || (high & highMask4)) value |= 0x10;
    if ((low & lowMask5) || (high & highMask5)) value |= 0x20;
    if ((low & lowMask6) || (high & highMask6)) value |= 0x40;
    if ((low & lowMask7) || (high & highMask7)) value |= 0x80;

    return value;
}

uint8_t SumpLogicAnalyzerAdapter::readSample8LowOnlyFast() {
    uint8_t value = 0;
    uint32_t low = REG_READ(GPIO_IN_REG);

    if (low & lowMask0) value |= 0x01;
    if (low & lowMask1) value |= 0x02;
    if (low & lowMask2) value |= 0x04;
    if (low & lowMask3) value |= 0x08;
    if (low & lowMask4) value |= 0x10;
    if (low & lowMask5) value |= 0x20;
    if (low & lowMask6) value |= 0x40;
    if (low & lowMask7) value |= 0x80;

    return value;
}

bool SumpLogicAnalyzerAdapter::inputRequestedReset() {
    return activeInput != nullptr && activeInput->readChar() != KEY_NONE;
}
