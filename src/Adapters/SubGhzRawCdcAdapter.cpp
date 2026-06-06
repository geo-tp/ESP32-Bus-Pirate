#include "SubGhzRawCdcAdapter.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include "Inputs/InputKeys.h"
#include "Utils/HostSerial.h"

void SubGhzRawCdcAdapter::run(const SubGhzRawCdcConfig& adapterConfig, IInput& deviceInput) {
    input = &deviceInput;
    runtimeConfig = new SubGhzRawCdcConfig(adapterConfig);

    begin();

    while (true) {
        pollUsb();

        if (rxReporting) {
            pollRawRx();
        }

        uint32_t now = millis();
        if ((uint32_t)(now - lastInputPollMs) >= INPUT_POLL_INTERVAL_MS) {
            lastInputPollMs = now;

            if (input != nullptr && input->readChar() != KEY_NONE) {
                stopRxReporting();
                if (subGhzService != nullptr) {
                    subGhzService->deinitRfModule();
                }
                Serial.flush();
                ESP.restart();
            }
        }

        yield();
    }
}

void SubGhzRawCdcAdapter::begin() {
    if (runtimeConfig->baudrate == 0) {
        runtimeConfig->baudrate = DEFAULT_BAUDRATE;
    }

    commandLength = 0;
    commandOverflow = false;
    hardwareReady = false;
    rxReporting = false;
    lastInputPollMs = millis();

    hostSerialDisableReboot();
    Serial.setRxBufferSize(4096);
    Serial.setTimeout(0);
    hostSerialBegin(runtimeConfig->baudrate);

    if (!allocateLazyObjects()) {
        Serial.println("ERR:MEM");
        while (true) {
            delay(1000);
        }
    }

    hardwareReady = subGhzService->configure(
        *spi,
        runtimeConfig->sckPin,
        runtimeConfig->misoPin,
        runtimeConfig->mosiPin,
        runtimeConfig->csPin,
        runtimeConfig->gdo0Pin,
        runtimeConfig->frequencyMhz,
        runtimeConfig->paDbm
    );

    if (hardwareReady) {
        subGhzService->applySniffProfile(runtimeConfig->frequencyMhz);
        Serial.println("READY ESP32-BitPirate SubGHz Raw CDC");
    } else {
        Serial.println("ERR:CC1101");
    }
}

bool SubGhzRawCdcAdapter::allocateLazyObjects() {
    if (spi == nullptr) {
        spi = new SPIClass(HSPI);
    }

    if (subGhzService == nullptr) {
        subGhzService = new SubGhzService();
    }

    if (commandBuffer == nullptr) {
        commandBuffer = new char[MAX_COMMAND_LENGTH + 1];
    }

    commandBuffer[0] = '\0';
    return true;
}

void SubGhzRawCdcAdapter::pollUsb() {
    while (Serial.available() > 0) {
        int c = Serial.read();
        if (c < 0) {
            break;
        }

        if (c == '\r' || c == '\n') {
            if (commandOverflow) {
                commandOverflow = false;
                commandLength = 0;
                commandBuffer[0] = '\0';
                printError("LONG");
                continue;
            }

            std::string line = trim(std::string(commandBuffer, commandLength));
            commandLength = 0;
            commandBuffer[0] = '\0';
            if (!line.empty()) {
                handleLine(line);
            }
            continue;
        }

        if (commandLength < MAX_COMMAND_LENGTH && std::isprint(static_cast<unsigned char>(c))) {
            commandBuffer[commandLength++] = static_cast<char>(c);
            commandBuffer[commandLength] = '\0';
            if (std::strcmp(commandBuffer, "V") == 0 || std::strcmp(commandBuffer, "v") == 0 ||
                std::strcmp(commandBuffer, "?") == 0 ||
                std::strcmp(commandBuffer, "R") == 0 || std::strcmp(commandBuffer, "r") == 0 ||
                std::strcmp(commandBuffer, "X00") == 0 || std::strcmp(commandBuffer, "x00") == 0 ||
                std::strcmp(commandBuffer, "X21") == 0 || std::strcmp(commandBuffer, "x21") == 0) {
                handleLine(std::string(commandBuffer, commandLength));
                commandLength = 0;
                commandBuffer[0] = '\0';
            }
        } else if (std::isprint(static_cast<unsigned char>(c))) {
            commandOverflow = true;
        }
    }
}

void SubGhzRawCdcAdapter::handleLine(const std::string& line) {
    if (line == "V" || line == "v") {
        printVersion();
        return;
    }

    if (line == "?") {
        printHelp();
        printOk();
        return;
    }

    if (line == "R" || line == "r") {
        if (!hardwareReady) {
            printError("CC1101");
            return;
        }

        Serial.print("RSSI:");
        Serial.println(subGhzService->measurePeakRssi(RSSI_HOLD_MS));
        return;
    }

    char command = line[0];
    std::string value = line.size() > 1 ? trim(line.substr(1)) : "";

    if (command == 'F' || command == 'f') {
        handleFrequencyCommand(value);
        return;
    }

    if (command == 'P' || command == 'p') {
        handlePresetCommand(value);
        return;
    }

    if (command == 'X' || command == 'x') {
        handleRxModeCommand(value);
        return;
    }

    if (command == 'l') {
        if (value == "00" || value == "01" || value == "02") {
            printOk();
        } else {
            printError("LED");
        }
        return;
    }

    if (command == 'B' || command == 'b') {
        if (value == "00") {
            printOk();
        } else {
            printError("BOOT");
        }
        return;
    }

    if (command == 'G' || command == 'g') {
        handleRawSendCommand(value);
        return;
    }

    printError("UNKNOWN");
}

void SubGhzRawCdcAdapter::handleFrequencyCommand(const std::string& value) {
    if (!hardwareReady) {
        printError("CC1101");
        return;
    }

    float mhz = 0.0f;
    if (!parseFrequency(value, mhz)) {
        printError("FREQ");
        return;
    }

    runtimeConfig->frequencyMhz = mhz;
    subGhzService->tune(mhz);
    subGhzService->applySniffProfile(mhz);
    restartRxReporting();
    printOk();
}

void SubGhzRawCdcAdapter::handlePresetCommand(const std::string& value) {
    if (!hardwareReady) {
        printError("CC1101");
        return;
    }

    float mhz = 0.0f;
    if (!parseFrequency(value, mhz)) {
        printError("FREQ");
        return;
    }

    runtimeConfig->frequencyMhz = mhz;
    subGhzService->tune(runtimeConfig->frequencyMhz);
    subGhzService->applySniffProfile(runtimeConfig->frequencyMhz);
    restartRxReporting();
    printOk();
}

void SubGhzRawCdcAdapter::handleRxModeCommand(const std::string& value) {
    if (value.empty()) {
        Serial.print("X:");
        Serial.println(rxReporting ? "21" : "00");
        return;
    }

    if (value == "00") {
        stopRxReporting();
        printOk();
        return;
    }

    if (value == "21") {
        if (!hardwareReady) {
            printError("CC1101");
            return;
        }

        startRxReporting();
        printOk();
        return;
    }

    printError("X");
}

void SubGhzRawCdcAdapter::handleRawSendCommand(const std::string& value) {
    if (!hardwareReady) {
        printError("CC1101");
        return;
    }

    std::vector<int32_t> timings;
    if (!parseTimings(value, timings)) {
        printError("G");
        return;
    }
    normalizeTxPolarity(timings);

    uint64_t totalUs = 0;
    for (int32_t timing : timings) {
        totalUs += static_cast<uint32_t>(timing < 0 ? -timing : timing);
    }

    bool wasReporting = rxReporting;
    stopRxReporting();
    bool txProfileOk = subGhzService->applyRawSendProfile(runtimeConfig->frequencyMhz);
    if (!txProfileOk) {
        if (wasReporting) {
            startRxReporting();
        }
        printError("TXPROFILE");
        return;
    }

    bool sent = subGhzService->sendRawTimings(timings);
    subGhzService->applySniffProfile(runtimeConfig->frequencyMhz);
    if (wasReporting) {
        startRxReporting();
    }

    if (sent) {
        Serial.print("OK:TX:COUNT:");
        Serial.print(timings.size());
        Serial.print(":US:");
        Serial.println(static_cast<uint32_t>(std::min<uint64_t>(totalUs, 0xFFFFFFFFULL)));
    } else {
        printError("TX");
    }
}

void SubGhzRawCdcAdapter::pollRawRx() {
    std::vector<rmt_symbol_word_t> frame = subGhzService->readRawChunk();
    if (!frame.empty() && printRawFrame(frame)) {
        int rssi = subGhzService->measurePeakRssi(1);
        Serial.print("RSSI:");
        Serial.println(rssi);
    }
}

void SubGhzRawCdcAdapter::startRxReporting() {
    if (!hardwareReady) {
        return;
    }

    subGhzService->applySniffProfile(runtimeConfig->frequencyMhz);
    rxReporting = subGhzService->startRawSniffer(runtimeConfig->gdo0Pin);
}

void SubGhzRawCdcAdapter::stopRxReporting() {
    if (!rxReporting) {
        return;
    }

    subGhzService->stopRawSniffer();
    rxReporting = false;
}

void SubGhzRawCdcAdapter::restartRxReporting() {
    if (!rxReporting) {
        return;
    }

    subGhzService->stopRawSniffer();
    rxReporting = subGhzService->startRawSniffer(runtimeConfig->gdo0Pin);
}

void SubGhzRawCdcAdapter::printHelp() {
    Serial.println("V          version");
    Serial.println("?          help");
    Serial.println("F433.920   tune frequency MHz");
    Serial.println("P433.920   OOK/raw sniff preset");
    Serial.println("X00        disable RX reporting");
    Serial.println("X21        enable RAW RX reporting");
    Serial.println("X          show RX reporting state");
    Serial.println("R          peak RSSI");
    Serial.println("G+350,-1050,+350,-350  send raw timings us");
}

void SubGhzRawCdcAdapter::printVersion() {
    Serial.println("V 1.0 ESP32-BitPirate SubGHz Raw CDC");
}

void SubGhzRawCdcAdapter::printOk() {
    Serial.println("OK");
}

void SubGhzRawCdcAdapter::printError(const char* message) {
    Serial.print("ERR:");
    Serial.println(message);
}

bool SubGhzRawCdcAdapter::printRawFrame(const std::vector<rmt_symbol_word_t>& frame) {
    auto hasDuration = [](uint32_t duration) {
        return duration != 0;
    };

    size_t durationCount = 0;
    for (const auto& symbol : frame) {
        if (hasDuration(symbol.duration0)) {
            ++durationCount;
        }
        if (hasDuration(symbol.duration1)) {
            ++durationCount;
        }
    }

    if (durationCount < 2) {
        return false;
    }

    if ((durationCount & 1U) != 0) {
        --durationCount;
    }

    if (durationCount == 0) {
        return false;
    }

    size_t emitted = 0;
    auto printDuration = [&](bool level, uint32_t duration) {
        if (duration == 0 || emitted >= durationCount) {
            return;
        }

        if (emitted > 0) {
            Serial.print(",");
        }

        uint32_t clamped = std::min<uint32_t>(duration, 20000000UL);
        bool mark = RAW_RX_INVERT_LEVEL ? !level : level;
        if (mark) {
            Serial.print("+");
            Serial.print(clamped);
        } else {
            Serial.print("-");
            Serial.print(clamped);
        }
        ++emitted;
    };

    Serial.print("RAW:");
    for (const auto& symbol : frame) {
        printDuration(symbol.level0 != 0, symbol.duration0);
        printDuration(symbol.level1 != 0, symbol.duration1);
    }
    Serial.println();
    return emitted > 0;
}

bool SubGhzRawCdcAdapter::parseTimings(const std::string& input, std::vector<int32_t>& timings) {
    std::string payload = stripRawPrefix(input);
    const char* p = payload.c_str();

    timings.reserve(64);
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t' || *p == ',') {
            ++p;
        }

        if (*p == '\0') {
            break;
        }

        char* end = nullptr;
        long value = std::strtol(p, &end, 10);
        if (end == p || value == 0 || value < -20000000L || value > 20000000L) {
            return false;
        }

        timings.push_back(static_cast<int32_t>(value));
        if (timings.size() > MAX_TX_TIMINGS) {
            return false;
        }

        p = end;
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
        if (*p != '\0' && *p != ',') {
            return false;
        }
    }

    return timings.size() >= 2 && (timings.size() & 1U) == 0;
}

void SubGhzRawCdcAdapter::normalizeTxPolarity(std::vector<int32_t>& timings) {
    if (!RAW_TX_INVERT_POLARITY) {
        return;
    }

    for (int32_t& timing : timings) {
        timing = -timing;
    }
}

bool SubGhzRawCdcAdapter::parseFrequency(const std::string& input, float& mhz) {
    std::string value = trim(input);
    if (value.empty()) {
        return false;
    }

    char* end = nullptr;
    mhz = std::strtof(value.c_str(), &end);
    if (end == value.c_str() || mhz <= 0.0f || mhz > 1000.0f) {
        return false;
    }

    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end))) {
            return false;
        }
        ++end;
    }

    return true;
}

std::string SubGhzRawCdcAdapter::trim(const std::string& input) {
    auto begin = std::find_if_not(input.begin(), input.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    auto end = std::find_if_not(input.rbegin(), input.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();

    if (begin >= end) {
        return "";
    }

    return std::string(begin, end);
}

std::string SubGhzRawCdcAdapter::stripRawPrefix(const std::string& input) {
    std::string payload = trim(input);
    if (payload.rfind("RAW:", 0) == 0 || payload.rfind("raw:", 0) == 0) {
        return trim(payload.substr(4));
    }

    return payload;
}
