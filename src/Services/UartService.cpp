#include "UartService.h"

void UartService::configure(unsigned long baud, uint32_t config, uint8_t rx, uint8_t tx, bool inverted) {
    Serial1.end(); // stop before reconfigure
    Serial1.begin(baud, config, rx, tx, inverted);
}

void UartService::end() {
    Serial1.end();
}

std::string UartService::readLine() {
    std::string input;
    bool lastWasCR = false;

    while (true) {
        if (!Serial1.available()) continue;
        
        char c = Serial1.read();

        if (c == '\r') {
            lastWasCR = true;
            Serial1.println();
            break;
        }

        if (c == '\n') {
            if (!lastWasCR) {
                Serial1.println();
                break;
            }
            continue;
        }

        if (c == '\b' || c == 127) {
            if (!input.empty()) {
                input.pop_back();
                Serial1.print("\b \b");
            }
        } else {
            input += c;
            Serial1.print(c);
            lastWasCR = false;
        }
    }

    return input;
}

void UartService::print(const std::string& msg) {
    Serial1.print(msg.c_str());
}

void UartService::println(const std::string& msg) {
    Serial1.println(msg.c_str());
}

bool UartService::available() const {
    return Serial1.available();
}

char UartService::read() {
    return Serial1.read();
}

void UartService::write(char c) {
    Serial1.write(c);
}

void UartService::write(const std::string& str) {
    Serial1.write(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

std::string UartService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    uint32_t timeout = 2000; // 2 secondes
    uint32_t start;
    uint32_t received = 0;

    for (const ByteCode& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Write:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    Serial1.write(code.getData());
                }
                break;

            case ByteCodeEnum::Read:
                start = millis();
                while (received < code.getRepeat() && (millis() - start < timeout)) {
                    if (Serial1.available()) {
                        char c = Serial1.read();
                        result += c;
                        ++received;
                    } else {
                        delay(10);
                    }
                }
                break;

            case ByteCodeEnum::DelayMs:
                delay(code.getRepeat());
                break;

            case ByteCodeEnum::DelayUs:
                delayMicroseconds(code.getRepeat());
                break;

            default:
                break;
        }
    }

    return result;
}

void UartService::switchBaudrate(unsigned long newBaud) {
    Serial1.updateBaudRate(newBaud);
}

void UartService::flush() {
    Serial.flush();
}

void UartService::clearUartBuffer() {
    const size_t maxBytes = 512; // uart buffer size ?
    size_t count = 0;
    while (count < maxBytes && available()) {
        read();
        count++;
    }
}


uint32_t UartService::buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits) {
    uint32_t config = SERIAL_8N1;
    if (dataBits == 5) config = (stopBits == 2) ? SERIAL_5N2 : SERIAL_5N1;
    else if (dataBits == 6) config = (stopBits == 2) ? SERIAL_6N2 : SERIAL_6N1;
    else if (dataBits == 7) config = (stopBits == 2) ? SERIAL_7N2 : SERIAL_7N1;
    else if (dataBits == 8) config = (stopBits == 2) ? SERIAL_8N2 : SERIAL_8N1;

    if (parity == 'E') config |= 0x02;
    else if (parity == 'O') config |= 0x01;

    return config;
}

/*
XMODEM
*/
File* UartService::currentFile = nullptr;

void UartService::setXmodemBlockSize(int32_t size) {
    xmodemBlockSize = size;
}

void UartService::setXmodemIdSize(int8_t size) {
    xmodemIdSize = size;
}

int32_t UartService::getXmodemBlockSize() const {
    return xmodemBlockSize;
}

int8_t UartService::getXmodemIdSize() const {
    return xmodemIdSize;
}

void UartService::setXmodemCrc(bool enabled) {
    xmodemProtocol = enabled ? XModem::ProtocolType::CRC_XMODEM : XModem::ProtocolType::XMODEM;
}

void UartService::setXmodemReceiveHandler(bool (*handler)(void*, size_t, byte*, size_t)) {
    xmodem.setRecieveBlockHandler(handler);
}

void UartService::setXmodemSendHandler(void (*handler)(void*, size_t, byte*, size_t)) {
    xmodem.setBlockLookupHandler(handler);
}

void UartService::initXmodem() {
    xmodem.begin(Serial1, xmodemProtocol);
    xmodem.setDataSize(xmodemBlockSize);
    xmodem.setIdSize(xmodemIdSize);
}

void UartService::blockLookupHandler(void* blk_id, size_t idSize, byte* data, size_t dataSize) {
    if (!currentFile) {
        return;
    }

    uint32_t blockId = 0;
    for (size_t i = 0; i < idSize; ++i) {
        blockId = (blockId << 8) | ((uint8_t*)blk_id)[i];
    }

    size_t offset = blockId * dataSize;
    if (!currentFile->seek(offset)) {
        return;
    }

    // Read
    size_t readBytes = currentFile->read(data, dataSize);
    if (readBytes < dataSize) {
        memset(data + readBytes, 0x1A, dataSize - readBytes);
    }

    // Progression
    Serial.printf("Sending bloc: %u\n\r", (unsigned int)blockId);
}

bool UartService::receiveBlockHandler(void* blk_id, size_t idSize, byte* data, size_t dataSize) {
    if (!currentFile) {
        return false;
    }

    // Progression
    uint32_t blockId = 0;
    for (size_t i = 0; i < idSize; ++i) {
        blockId = (blockId << 8) | ((uint8_t*)blk_id)[i];
    }
    Serial.printf("Receiving bloc: %u\r\n", (unsigned int)blockId);

    // Write
    return currentFile->write(data, dataSize) == dataSize;
}

bool UartService::xmodemSendFile(File& file) {
    if (!file || file.isDirectory()) return false;

    // Xmodem Init
    initXmodem();
    currentFile = &file;
    xmodem.setBlockLookupHandler(blockLookupHandler);

    // Calculate
    size_t fileSize = file.size();
    size_t blockSize = xmodemBlockSize;
    size_t totalBlocks = (fileSize + blockSize - 1) / blockSize;
    const size_t idSize = xmodemIdSize;

    // Construct Ids
    byte* all_ids = (byte*)malloc(totalBlocks * idSize);
    for (size_t i = 0; i < totalBlocks; ++i) {
        unsigned long long blk_id = i + 1;
        for (size_t j = 0; j < idSize; ++j) {
            all_ids[i * idSize + j] = (blk_id >> (8 * (idSize - j - 1))) & 0xFF;
        }
    }

    // Data, lookup_handler will deal with empty data
    byte** dummy_data = (byte**)malloc(sizeof(byte*) * totalBlocks);
    size_t* dummy_lens = (size_t*)malloc(sizeof(size_t) * totalBlocks);
    for (size_t i = 0; i < totalBlocks; ++i) {
        dummy_data[i] = nullptr;
        dummy_lens[i] = blockSize;
    }

    // Configure container for send_bulk_data
    struct XModem::bulk_data container = {
        .data_arr = dummy_data,
        .len_arr = dummy_lens,
        .id_arr = all_ids,
        .count = totalBlocks
    };

    // Send
    bool result = xmodem.send_bulk_data(container);

    // Release
    free(all_ids);
    free(dummy_data);
    free(dummy_lens);
    currentFile = nullptr;

    return result;
}


bool UartService::xmodemReceiveToFile(File& file) {
    if (!file || file.isDirectory()) return false;

    // Init
    initXmodem();
    currentFile = &file;

    // Receive
    xmodem.setRecieveBlockHandler(receiveBlockHandler);
    bool ok = xmodem.receive();

    currentFile = nullptr;
    return ok;
}

void IRAM_ATTR UartService::onGpioEdge(void* arg) {
    uint32_t pin = (uint32_t)arg;

    uint32_t now = micros();
    if (lastEdgeTimeUs != 0 && intervalCount < 64) {
        edgeIntervals[intervalCount++] = now - lastEdgeTimeUs;
    }
    lastEdgeTimeUs = now;

    if (pin < 50) {
        edgeCounts[pin]++;
    }
}

UartService::PinActivity UartService::measureUartActivity(uint8_t pin, uint32_t windowMs, bool pullup) {
    edgeCounts[pin] = 0;
    lastEdgeTimeUs = 0;
    intervalCount = 0;

    // Configure GPIO
    gpio_config_t io = {};
    io.intr_type = GPIO_INTR_ANYEDGE;
    io.mode = GPIO_MODE_INPUT;
    io.pin_bit_mask = (1ULL << pin);
    io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io.pull_up_en = pullup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
    gpio_config(&io);

    // Attach ISR and measure
    gpio_isr_handler_add((gpio_num_t)pin, &UartService::onGpioEdge, (void*)pin);
    uint32_t start = millis();
    while ((millis() - start) < windowMs) {
        delay(1);
    }
    gpio_isr_handler_remove((gpio_num_t)pin);

    // Calculate edges per second
    uint32_t edges = edgeCounts[pin];
    float eps = (windowMs > 0) ? (edges * 1000.0f / windowMs) : 0.0f;

    // Baud estimating
    uint32_t approxBaud = 0;
    if (intervalCount > 8) {
        std::vector<uint32_t> tmp(edgeIntervals, edgeIntervals + intervalCount);
        std::sort(tmp.begin(), tmp.end());
        uint32_t med = tmp[tmp.size() / 2];   // microseconds

        if (med > 0) {
            approxBaud = 1000000UL / med;

            if (approxBaud < 1200 || approxBaud > 1000000) {
                approxBaud = 0;
            } else {
                approxBaud = (approxBaud / 100) * 100;
            }
        }
    }

    return { pin, edges, eps, approxBaud };
}

std::vector<UartService::PinActivity> UartService::scanUartActivity(const std::vector<uint8_t>& pins,
                                                                    uint32_t windowMs,
                                                                    uint32_t minEdges,
                                                                    bool pullup) {
    std::vector<PinActivity> out;
    out.reserve(pins.size());

    for (auto pin : pins) {
        auto r = measureUartActivity(pin, windowMs, pullup);
        if (r.edges >= minEdges) {
            out.push_back(r);
        }
    }

    std::sort(out.begin(), out.end(), [](const PinActivity& a, const PinActivity& b) {
        return a.edges > b.edges;
    });

    return out;
}