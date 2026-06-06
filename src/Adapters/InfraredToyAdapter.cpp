#include "InfraredToyAdapter.h"

#include "Inputs/InputKeys.h"
#include "Utils/HostSerial.h"

void InfraredToyAdapter::run(const InfraredToyConfig& config, IInput& input) {
    InfraredToyAdapter::config = config;
    InfraredToyAdapter::input = &input;

    begin();

    while (true) {
        pollUsb();

        if (state == ProtocolState::SAMPLE_RX_MODE) {
            flushRxDurationsToUsb();
            captureRxFrame();
            flushRxDurationsToUsb();
        }

        uint32_t now = millis();
        if ((uint32_t)(now - lastInputPollMs) >= INPUT_POLL_INTERVAL_MS) {
            lastInputPollMs = now;

            if (InfraredToyAdapter::input != nullptr &&
                InfraredToyAdapter::input->readChar() != KEY_NONE) {
                stopRxCapture();
                ESP.restart();
            }
        }

        yield();
    }
}

void InfraredToyAdapter::begin() {
    state = ProtocolState::COMMAND_MODE;
    txReturnState = ProtocolState::COMMAND_MODE;

    resetSequenceIndex = 0;
    txStartSequenceIndex = 0;
    lircTxPrefixIndex = 0;

    ioCommandBytesRemaining = 0;
    txExpectedBytes = 0;
    txByteCount = 0;
    txHighByte = 0;

    irHardwareConfigured = false;
    rxCaptureActive = false;
    rxFrameActive = false;
    txPendingHighByte = false;
    txControlByteConsumed = false;
    txLircDoubleBuffered = false;

    rxHead = 0;
    rxTail = 0;

    rxIdleLevel = 1;
    rxLastLevel = 1;
    rxLastEdgeUs = 0;

    rxOverflowCount = 0;

    lastInputPollMs = millis();

    hostSerialDisableReboot();
    Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE);
    Serial.setTimeout(0);
    hostSerialBegin();

    if (!allocateLazyBuffers()) {
        static constexpr char msg[] = "IR Toy adapter: buffer allocation failed\r\n";
        writeExact(reinterpret_cast<const uint8_t*>(msg), sizeof(msg) - 1, 100);

        while (true) {
            delay(1000);
        }
    }

    txDurationsUs.clear();
    txDurationsUs.reserve(MAX_TX_BYTES / 2);
}

bool InfraredToyAdapter::allocateLazyBuffers() {
    if (rxRing == nullptr) {
        rxRing = new uint16_t[RX_RING_CAPACITY];
        if (rxRing == nullptr) {
            return false;
        }
    }

    return true;
}

void InfraredToyAdapter::pollUsb() {
    uint8_t buffer[USB_READ_CHUNK_SIZE];

    while (Serial.available() > 0) {
        size_t read = 0;

        while (read < USB_READ_CHUNK_SIZE && Serial.available() > 0) {
            int c = Serial.read();
            if (c < 0) {
                break;
            }

            buffer[read++] = static_cast<uint8_t>(c);
        }

        if (read > 0) {
            handleHostBytes(buffer, read);
        }

        yield();
    }
}

void InfraredToyAdapter::ensureIrHardwareConfigured() {
    if (irHardwareConfigured) {
        return;
    }

    gpio_set_direction(static_cast<gpio_num_t>(config.rxPin), GPIO_MODE_INPUT);
    gpio_pullup_en(static_cast<gpio_num_t>(config.rxPin));

    gpio_set_direction(static_cast<gpio_num_t>(config.txPin), GPIO_MODE_OUTPUT);
    fastWritePinLow(config.txPin);

    rmtInit(config.txPin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_2, RMT_RESOLUTION_HZ);
    rmtSetCarrier(config.txPin, true, IRTOY_TX_CARRIER_ACTIVE_LEVEL, 38000, 0.33f);
    rmtSetEOT(config.txPin, LOW);

    irHardwareConfigured = true;
}

void InfraredToyAdapter::startRxCapture() {
    ensureIrHardwareConfigured();

    rxIdleLevel = fastReadPin(config.rxPin);
    rxLastLevel = rxIdleLevel;
    rxLastEdgeUs = micros();

    rxFrameActive = false;
    rxCaptureActive = true;
}

void InfraredToyAdapter::stopRxCapture() {
    if (!irHardwareConfigured) {
        return;
    }

    rmtDeinit(config.txPin);

    irHardwareConfigured = false;
    rxCaptureActive = false;
    rxFrameActive = false;
}

void InfraredToyAdapter::handleHostBytes(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        uint8_t b = data[i];

        if (trackResetSequence(b)) {
            resetProtocolState();
            continue;
        }

        if (state != ProtocolState::TX_RECEIVE_CHUNKS && trackTxStartSequence(b)) {
            startTransmitUpload();
            continue;
        }

        if (txControlByteConsumed) {
            continue;
        }

        if (state == ProtocolState::TX_RECEIVE_CHUNKS) {
            processTxByte(b);
        } else {
            handleCommandByte(b);
        }
    }
}

void InfraredToyAdapter::handleCommandByte(uint8_t b) {
    if (b == 'v') {
        sendVersion();
        return;
    }

    if (b == 's' || b == 'S') {
        enterSampleMode();
        return;
    }

    if (b == 0x00 && state == ProtocolState::SAMPLE_RX_MODE) {
        exitSampleMode();
    }
}

bool InfraredToyAdapter::writeExact(const uint8_t* data, size_t len, uint32_t timeoutMs) {
    size_t written = 0;
    uint32_t start = millis();

    while (written < len) {
        int space = Serial.availableForWrite();
        if (space > 0) {
            size_t chunk = std::min<size_t>(static_cast<size_t>(space), len - written);
            written += Serial.write(data + written, chunk);
        }

        if ((uint32_t)(millis() - start) > timeoutMs) {
            return false;
        }

        yield();
    }

    Serial.flush();
    return true;
}

void InfraredToyAdapter::sendVersion() {
    static constexpr uint8_t version[] = {'V', '2', '2', '2'};
    writeExact(version, sizeof(version), 100);
}

void InfraredToyAdapter::enterSampleMode() {
    state = ProtocolState::SAMPLE_RX_MODE;
    clearRxRing();

    static constexpr uint8_t sampleAck[] = {'S', '0', '1'};
    writeExact(sampleAck, sizeof(sampleAck), 100);

    startRxCapture();
}

void InfraredToyAdapter::exitSampleMode() {
    state = ProtocolState::COMMAND_MODE;
    clearRxRing();
    stopRxCapture();
}

void InfraredToyAdapter::resetProtocolState() {
    state = ProtocolState::COMMAND_MODE;
    txReturnState = ProtocolState::COMMAND_MODE;

    resetSequenceIndex = 0;
    txStartSequenceIndex = 0;
    lircTxPrefixIndex = 0;

    ioCommandBytesRemaining = 0;
    txExpectedBytes = 0;
    txByteCount = 0;
    txHighByte = 0;

    txPendingHighByte = false;
    txControlByteConsumed = false;
    txLircDoubleBuffered = false;

    txDurationsUs.clear();
    clearRxRing();

    rxFrameActive = false;
    stopRxCapture();
}

void InfraredToyAdapter::startTransmitUpload() {
    txReturnState = state == ProtocolState::SAMPLE_RX_MODE
        ? ProtocolState::SAMPLE_RX_MODE
        : ProtocolState::COMMAND_MODE;

    stopRxCapture();
    ensureIrHardwareConfigured();

    state = ProtocolState::TX_RECEIVE_CHUNKS;
    txExpectedBytes = 0;
    txByteCount = 0;
    txPendingHighByte = false;
    txDurationsUs.clear();

    requestTxChunk();
}

void InfraredToyAdapter::requestTxChunk() {
    if (state != ProtocolState::TX_RECEIVE_CHUNKS) {
        return;
    }

    size_t remaining = MAX_TX_BYTES > txByteCount ? MAX_TX_BYTES - txByteCount : 0;
    txExpectedBytes = static_cast<uint8_t>(std::min<size_t>(static_cast<size_t>(TX_CHUNK_SIZE), remaining));

    if (txExpectedBytes == 0) {
        finishTransmit(false);
        return;
    }

    writeExact(&txExpectedBytes, 1, 100);
}

void InfraredToyAdapter::processTxByte(uint8_t b) {
    if (txExpectedBytes == 0) {
        requestTxChunk();
        return;
    }

    --txExpectedBytes;
    ++txByteCount;

    if (!txPendingHighByte) {
        txPendingHighByte = true;
        txHighByte = b;
    } else {
        txPendingHighByte = false;

        uint16_t ticks = (static_cast<uint16_t>(txHighByte) << 8) | b;
        if (ticks == IRTOY_END_MARKER) {
            if (txLircDoubleBuffered) {
                uint8_t ready = TX_CHUNK_SIZE;
                writeExact(&ready, 1, 100);
            }

            finishTransmit(true);
            return;
        }

        txDurationsUs.push_back(convertIrToyTicksToUs(ticks));
    }

    if (txExpectedBytes == 0) {
        requestTxChunk();
    }
}

void InfraredToyAdapter::finishTransmit(bool success) {
    state = ProtocolState::TX_TRANSMITTING;

    if (success && !txDurationsUs.empty()) {
        transmitUploadedDurations();
    }

    uint16_t emittedCount = static_cast<uint16_t>(std::min<size_t>(txByteCount, 0xFFFF));
    uint8_t txCountResponse[3] = {
        't',
        static_cast<uint8_t>((emittedCount >> 8) & 0xFF),
        static_cast<uint8_t>(emittedCount & 0xFF)
    };

    writeExact(txCountResponse, sizeof(txCountResponse), 100);

    if (success) {
        static constexpr uint8_t complete = 'C';
        yield();
        writeExact(&complete, 1, 100);
    }

    txDurationsUs.clear();
    txExpectedBytes = 0;
    txByteCount = 0;
    txHighByte = 0;
    txPendingHighByte = false;
    txLircDoubleBuffered = false;

    state = success ? txReturnState : ProtocolState::COMMAND_MODE;

    if (state == ProtocolState::SAMPLE_RX_MODE) {
        startRxCapture();
    }
}

void InfraredToyAdapter::transmitUploadedDurations() {
    ensureIrHardwareConfigured();

    std::vector<rmt_data_t> symbols;
    symbols.reserve(txDurationsUs.size());

    bool pulse = true;
    for (uint32_t us : txDurationsUs) {
        appendTxDuration(symbols, pulse, us);
        pulse = !pulse;
    }

    if (!symbols.empty()) {
        rmtWrite(config.txPin, symbols.data(), symbols.size(), RMT_WAIT_FOR_EVER);
    }
}

void InfraredToyAdapter::captureRxFrame() {
    if (state != ProtocolState::SAMPLE_RX_MODE) {
        return;
    }

    if (!irHardwareConfigured || !rxCaptureActive) {
        startRxCapture();
        return;
    }

    uint32_t nowUs = micros();
    uint8_t level = fastReadPin(config.rxPin);

    if (level != rxLastLevel) {
        uint32_t durationUs = nowUs - rxLastEdgeUs;
        bool previousWasPulse = rxLastLevel != rxIdleLevel;

        if (!rxFrameActive) {
            if (previousWasPulse) {
                rxFrameActive = true;
                pushRxDurationUs(durationUs);
            }
        } else {
            pushRxDurationUs(durationUs);
        }

        rxLastLevel = level;
        rxLastEdgeUs = nowUs;
        return;
    }

    if (rxFrameActive &&
        level == rxIdleLevel &&
        (uint32_t)(nowUs - rxLastEdgeUs) >= RMT_RX_TIMEOUT_US) {
        pushRxDuration(IRTOY_END_MARKER);
        rxFrameActive = false;
        rxLastEdgeUs = nowUs;
    }
}

bool InfraredToyAdapter::pushRxDurationUs(uint32_t us) {
    static constexpr uint32_t maxTicks = 0xFFFE;
    static constexpr uint32_t maxUs = maxTicks * IRTOY_UNIT_US;

    if (us > maxUs) {
        pushRxDuration(IRTOY_END_MARKER);
        return false;
    }

    pushRxDuration(convertUsToIrToyTicks(us));
    return true;
}

void InfraredToyAdapter::appendTxDuration(std::vector<rmt_data_t>& symbols, bool pulse, uint32_t us) {
    uint8_t level = pulse ? 1 : 0;

    while (us > 0) {
        uint32_t chunkUs = std::min<uint32_t>(us, RMT_MAX_DURATION_US);
        uint16_t chunkTicks = convertUsToRmtTicksClamped(chunkUs);

        us -= chunkUs;

        if (symbols.empty() || symbols.back().duration1 != 0) {
            rmt_data_t symbol{};
            symbol.level0 = level;
            symbol.duration0 = chunkTicks;
            symbol.level1 = level;
            symbol.duration1 = 0;
            symbols.push_back(symbol);
        } else {
            symbols.back().level1 = level;
            symbols.back().duration1 = chunkTicks;
        }
    }
}

void InfraredToyAdapter::flushRxDurationsToUsb() {
    while (!rxRingEmpty() && Serial.availableForWrite() >= 2) {
        uint16_t value = popRxDuration();
        writeBe16(value);
    }
}

uint16_t InfraredToyAdapter::convertUsToIrToyTicks(uint32_t us) {
    uint32_t ticks = (us + (IRTOY_UNIT_US / 2)) / IRTOY_UNIT_US;

    if (ticks == 0) {
        ticks = 1;
    }

    if (ticks > 0xFFFE) {
        ticks = 0xFFFE;
    }

    return static_cast<uint16_t>(ticks);
}

uint32_t InfraredToyAdapter::convertIrToyTicksToUs(uint16_t ticks) {
    return static_cast<uint32_t>(ticks) * IRTOY_UNIT_US;
}

uint16_t InfraredToyAdapter::convertUsToRmtTicksClamped(uint32_t us) {
    uint32_t ticks = (us + (RMT_TICK_US - 1)) / RMT_TICK_US;

    if (ticks == 0) {
        ticks = 1;
    }

    if (ticks > RMT_MAX_DURATION_TICKS) {
        ticks = RMT_MAX_DURATION_TICKS;
    }

    return static_cast<uint16_t>(ticks);
}

bool InfraredToyAdapter::trackResetSequence(uint8_t b) {
    static constexpr std::array<uint8_t, 7> resetSequence = {
        0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    if (b == resetSequence[resetSequenceIndex]) {
        ++resetSequenceIndex;

        if (resetSequenceIndex == resetSequence.size()) {
            resetSequenceIndex = 0;
            return true;
        }

        return false;
    }

    resetSequenceIndex = (b == resetSequence[0]) ? 1 : 0;
    return false;
}

bool InfraredToyAdapter::trackTxStartSequence(uint8_t b) {
    static constexpr std::array<uint8_t, 3> lircTxPrefixSequence = {
        0x24, 0x25, 0x26
    };

    static constexpr std::array<uint8_t, 4> kernelTxStartSequence = {
        0x26, 0x24, 0x25, 0x03
    };

    txControlByteConsumed = false;

    if (ioCommandBytesRemaining > 0) {
        --ioCommandBytesRemaining;
        txControlByteConsumed = true;
        return false;
    }

    if (b == 0x30 || b == 0x31) {
        ioCommandBytesRemaining = 2;
        txControlByteConsumed = true;
        return false;
    }

    if (lircTxPrefixIndex < lircTxPrefixSequence.size() &&
        b == lircTxPrefixSequence[lircTxPrefixIndex]) {
        ++lircTxPrefixIndex;
        txControlByteConsumed = true;

        if (lircTxPrefixIndex == lircTxPrefixSequence.size()) {
            return false;
        }
    } else if (b != 0x03) {
        lircTxPrefixIndex = (b == lircTxPrefixSequence[0]) ? 1 : 0;
    }

    if (b == 0x03 && lircTxPrefixIndex == lircTxPrefixSequence.size()) {
        lircTxPrefixIndex = 0;
        txStartSequenceIndex = 0;
        txLircDoubleBuffered = true;
        return true;
    }

    if (b == kernelTxStartSequence[txStartSequenceIndex]) {
        ++txStartSequenceIndex;

        if (txStartSequenceIndex == kernelTxStartSequence.size()) {
            txStartSequenceIndex = 0;
            lircTxPrefixIndex = 0;
            txLircDoubleBuffered = false;
            return true;
        }

        return false;
    }

    txStartSequenceIndex = (b == kernelTxStartSequence[0]) ? 1 : 0;
    return false;
}

void InfraredToyAdapter::writeBe16(uint16_t value) {
    uint8_t out[2] = {
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };

    Serial.write(out, sizeof(out));
}

bool InfraredToyAdapter::rxRingEmpty() {
    return rxHead == rxTail;
}

void InfraredToyAdapter::clearRxRing() {
    rxHead = 0;
    rxTail = 0;
}

void InfraredToyAdapter::pushRxDuration(uint16_t value) {
    if (rxRing == nullptr) {
        return;
    }

    size_t next = (rxHead + 1) % RX_RING_CAPACITY;

    if (next == rxTail) {
        rxTail = (rxTail + 1) % RX_RING_CAPACITY;
        ++rxOverflowCount;
    }

    rxRing[rxHead] = value;
    rxHead = next;
}

uint16_t InfraredToyAdapter::popRxDuration() {
    if (rxRing == nullptr) {
        return IRTOY_END_MARKER;
    }

    uint16_t value = rxRing[rxTail];
    rxTail = (rxTail + 1) % RX_RING_CAPACITY;
    return value;
}
