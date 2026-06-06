#include "AvrDudeBusPirateAdapter.h"
#include "Inputs/InputKeys.h"
#include "Utils/HostSerial.h"
#include <cstring>
#include <vector>

namespace {
constexpr uint8_t ACK = 0x01;
constexpr uint8_t NAK = 0x00;
constexpr uint32_t DEFAULT_SPI_FREQUENCY = 1000000;
constexpr uint32_t MAX_TRANSFER = 4096;
constexpr uint8_t BIG_FLASH_SIGNATURES[] = {0x97, 0x98, 0xA7, 0xA8, 0xC0};
}

void AvrDudeBusPirateAdapter::run(const AvrDudeBusPirateConfig& adapterConfig, IInput& input) {
    config = adapterConfig;
    if (config.frequency == 0) {
        config.frequency = DEFAULT_SPI_FREQUENCY;
    }

    hostSerialDisableReboot();
    Serial.setRxBufferSize(MAX_TRANSFER + 64);
    hostSerialBegin();

    state = ProtocolState::WaitBbio;
    zeroCount = 0;
    spiMode = SPI_MODE0;
    reqExtendedHighByteWrite = false;
    initializeSpi();

    while (true) {
        if (inputRequestedReset(input)) {
            ESP.restart();
        }

        if (Serial.available() > 0) {
            handleByte(static_cast<uint8_t>(Serial.read()), input);
        }
    }
}

void AvrDudeBusPirateAdapter::initializeSpi() {
    pinMode(config.csPin, OUTPUT);
    digitalWrite(config.csPin, HIGH);
    spi.begin(config.sckPin, config.misoPin, config.mosiPin, config.csPin);
}

void AvrDudeBusPirateAdapter::setChipSelect(bool asserted) {
    digitalWrite(config.csPin, asserted ? LOW : HIGH);
}

void AvrDudeBusPirateAdapter::handleByte(uint8_t command, IInput& input) {
    switch (state) {
        case ProtocolState::WaitBbio:
            if (command == 0x00) {
                writeString("BBIO1");
                state = ProtocolState::RawBitbang;
                zeroCount = 1;
            }
            break;

        case ProtocolState::RawBitbang:
            handleRawBitbangCommand(command);
            break;

        case ProtocolState::RawSpi:
            handleRawSpiCommand(command, input);
            break;
    }
}

void AvrDudeBusPirateAdapter::handleRawBitbangCommand(uint8_t command) {
    if (command == 0x00) {
        if (zeroCount == 0) {
            writeString("BBIO1");
        } else if (zeroCount > 15) {
            zeroCount = 0;
        }
        zeroCount++;
        return;
    }

    zeroCount = 0;

    if (command == 0x01) {
        state = ProtocolState::RawSpi;
        writeString("SPI1");
        return;
    }

    if ((command & 0xF0) == 0x40) {
        configurePeripherals(command);
        return;
    }

    if (command == 0x0F) {
        writeString("Bus Pirate v2.5\r\nCommunity Firmware v7.1\r\nHiZ>");
        state = ProtocolState::WaitBbio;
        return;
    }

    writeByte(NAK);
}

void AvrDudeBusPirateAdapter::handleRawSpiCommand(uint8_t command, IInput& input) {
    if (command == 0x00) {
        state = ProtocolState::RawBitbang;
        zeroCount = 1;
        writeString("BBIO1");
        return;
    }

    if (command == 0x01) {
        writeString("SPI1");
        return;
    }

    if (command == 0x02) {
        setChipSelect(true);
        writeByte(ACK);
        return;
    }

    if (command == 0x03) {
        setChipSelect(false);
        writeByte(ACK);
        return;
    }

    if (command == 0x04 || command == 0x05) {
        handleWriteThenRead(command == 0x04, input);
        return;
    }

    if (command == 0x06) {
        handleAvrExtendedCommand(input);
        return;
    }

    if ((command & 0xF0) == 0x10) {
        handleBulkTransfer(command, input);
        return;
    }

    if ((command & 0xF8) == 0x60) {
        configureSpeed(command);
        return;
    }

    if ((command & 0xF0) == 0x40) {
        configurePeripherals(command);
        return;
    }

    if ((command & 0xF0) == 0x80) {
        configureSpi(command);
        return;
    }

    writeByte(NAK);
}

void AvrDudeBusPirateAdapter::handleBulkTransfer(uint8_t command, IInput& input) {
    uint8_t length = (command & 0x0F) + 1;
    uint8_t tx[16] = {};
    uint8_t rx[16] = {};

    writeByte(ACK);
    for (uint8_t i = 0; i < length; ++i) {
        tx[i] = readByte(input);
        rx[i] = transfer(tx[i]);
        writeByte(rx[i]);
    }

    updateExtendedHighByteRequirement(tx, rx, length);
}

void AvrDudeBusPirateAdapter::handleWriteThenRead(bool autoChipSelect, IInput& input) {
    uint16_t writeLength = readBe16(input);
    uint16_t readLength = readBe16(input);

    if (writeLength > MAX_TRANSFER || readLength > MAX_TRANSFER) {
        for (uint16_t i = 0; i < writeLength; ++i) {
            readByte(input);
        }
        writeByte(NAK);
        return;
    }

    std::vector<uint8_t> writeData(writeLength);
    for (uint16_t i = 0; i < writeLength; ++i) {
        writeData[i] = readByte(input);
    }

    if (writeLength == 0 && readLength == 0) {
        writeByte(ACK);
        return;
    }

    if (autoChipSelect) {
        setChipSelect(true);
    }

    for (uint16_t i = 0; i < writeLength; ++i) {
        transfer(writeData[i]);
    }

    writeByte(ACK);
    for (uint16_t i = 0; i < readLength; ++i) {
        writeByte(transfer(0x00));
    }

    if (autoChipSelect) {
        setChipSelect(false);
    }
}

void AvrDudeBusPirateAdapter::handleAvrExtendedCommand(IInput& input) {
    writeByte(ACK);
    uint8_t command = readByte(input);

    switch (command) {
        case 0x00:
            writeByte(ACK);
            break;

        case 0x01:
            writeByte(ACK);
            writeByte(0x00);
            writeByte(0x01);
            break;

        case 0x02:
            handleAvrExtendedPagedRead(input);
            break;

        default:
            writeByte(NAK);
            break;
    }
}

void AvrDudeBusPirateAdapter::handleAvrExtendedPagedRead(IInput& input) {
    uint32_t address = readBe32(input);
    uint32_t length = readBe32(input);

    writeByte(ACK);
    while (length > 0) {
        if (reqExtendedHighByteWrite) {
            transfer(0x4D);
            transfer(0x00);
            transfer((address >> 16) & 0x03);
            transfer(0x00);
        }

        transfer(0x20);
        transfer((address >> 8) & 0xFF);
        transfer(address & 0xFF);
        writeByte(transfer(0x00));
        length--;

        if (length > 0) {
            transfer(0x28);
            transfer((address >> 8) & 0xFF);
            transfer(address & 0xFF);
            writeByte(transfer(0x00));
            length--;
        }

        address++;
    }
}

void AvrDudeBusPirateAdapter::configurePeripherals(uint8_t command) {
    setChipSelect((command & 0x01) == 0);
    writeByte(ACK);
}

void AvrDudeBusPirateAdapter::configureSpeed(uint8_t command) {
    static const uint32_t speeds[] = {
        30000,
        125000,
        250000,
        1000000,
        2000000,
        2600000,
        4000000,
        8000000,
    };

    config.frequency = speeds[command & 0x07];
    writeByte(ACK);
}

void AvrDudeBusPirateAdapter::configureSpi(uint8_t command) {
    bool clockIdleHigh = (command & 0x04) != 0;
    bool clockEdgeActiveToIdle = (command & 0x02) != 0;

    if (!clockIdleHigh && clockEdgeActiveToIdle) {
        spiMode = SPI_MODE0;
    } else if (!clockIdleHigh && !clockEdgeActiveToIdle) {
        spiMode = SPI_MODE1;
    } else if (clockIdleHigh && clockEdgeActiveToIdle) {
        spiMode = SPI_MODE2;
    } else {
        spiMode = SPI_MODE3;
    }

    initializeSpi();
    writeByte(ACK);
}

uint8_t AvrDudeBusPirateAdapter::transfer(uint8_t value) {
    SPISettings settings(config.frequency, MSBFIRST, spiMode);
    spi.beginTransaction(settings);
    uint8_t response = spi.transfer(value);
    spi.endTransaction();
    return response;
}

uint8_t AvrDudeBusPirateAdapter::readByte(IInput& input) {
    while (Serial.available() <= 0) {
        if (inputRequestedReset(input)) {
            ESP.restart();
        }
    }
    return static_cast<uint8_t>(Serial.read());
}

uint16_t AvrDudeBusPirateAdapter::readBe16(IInput& input) {
    uint16_t value = static_cast<uint16_t>(readByte(input)) << 8;
    value |= readByte(input);
    return value;
}

uint32_t AvrDudeBusPirateAdapter::readBe32(IInput& input) {
    uint32_t value = static_cast<uint32_t>(readByte(input)) << 24;
    value |= static_cast<uint32_t>(readByte(input)) << 16;
    value |= static_cast<uint32_t>(readByte(input)) << 8;
    value |= readByte(input);
    return value;
}

void AvrDudeBusPirateAdapter::writeByte(uint8_t value) {
    Serial.write(value);
}

void AvrDudeBusPirateAdapter::writeBytes(const uint8_t* data, size_t length) {
    Serial.write(data, length);
}

void AvrDudeBusPirateAdapter::writeString(const char* value) {
    Serial.write(reinterpret_cast<const uint8_t*>(value), strlen(value));
}

bool AvrDudeBusPirateAdapter::inputRequestedReset(IInput& input) {
    return input.readChar() != KEY_NONE;
}

void AvrDudeBusPirateAdapter::updateExtendedHighByteRequirement(const uint8_t* tx, const uint8_t* rx, uint16_t length) {
    if (length < 4 || tx[0] != 0x30 || tx[2] != 0x01) {
        return;
    }

    reqExtendedHighByteWrite = false;
    for (uint8_t signature : BIG_FLASH_SIGNATURES) {
        if (rx[3] == signature) {
            reqExtendedHighByteWrite = true;
            break;
        }
    }
}
