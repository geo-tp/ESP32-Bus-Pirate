#include "Services/SpiService.h"
#include <ESP32SPISlave.h>
#include "driver/spi_slave.h"

void SpiService::configure(uint8_t mosi, uint8_t miso, uint8_t sclk, uint8_t cs, uint32_t frequency) {
    end();
    csPin = cs;
    spiFrequency = frequency;
    SPI.begin(sclk, miso, mosi, cs);
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
}

void SpiService::end() {
    SPI.end();
}

void SpiService::beginTransaction() {
    SPI.beginTransaction(SPISettings(spiFrequency, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
}

void SpiService::endTransaction() {
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

uint8_t SpiService::transfer(uint8_t data) {
    return SPI.transfer(data);
}

std::string SpiService::readFlashID() {
    uint8_t id[3] = {0};

    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (uint8_t& byte : id) {
        byte = SPI.transfer(0x00);
    }
    endTransaction();

    char buf[32];
    snprintf(buf, sizeof(buf), "%02X %02X %02X", id[0], id[1], id[2]);
    return std::string(buf);
}

void SpiService::readFlashIdRaw(uint8_t* buffer) {
    beginTransaction();
    SPI.transfer(0x9F);  // JEDEC ID command
    for (int i = 0; i < 3; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

uint32_t SpiService::calculateFlashCapacity(uint8_t code) {
    // code 0x11 = 2^17 = 128 KB, etc.
    if (code >= 0x11 && code <= 0x20) {
        return 1UL << code;  // 2^code
    }
    return 0; // Non standard
}

void SpiService::readFlashData(uint32_t address, uint8_t* buffer, size_t length) {
    beginTransaction();
    SPI.transfer(0x03);  // Read Data command
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    for (size_t i = 0; i < length; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    endTransaction();
}

void SpiService::eraseFlashSector(uint32_t address, uint32_t freq) {
    enableFlashWrite(freq);  // 0x06

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x20); // Sector erase
    SPI.transfer((address >> 16) & 0xFF);
    SPI.transfer((address >> 8) & 0xFF);
    SPI.transfer(address & 0xFF);

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    waitForFlashWriteComplete(freq);
}

void SpiService::enableFlashWrite(uint32_t freq) {

    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x06); // Write Enable

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

void SpiService::waitForFlashWriteComplete(uint32_t freq) {
    SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);

    SPI.transfer(0x05); // Read Status Register
    while (true) {
        uint8_t status = SPI.transfer(0x00); // Dummy byte to receive status
        if ((status & 0x01) == 0) break;     // Wait until WIP bit is cleared
        delay(1);
    }

    digitalWrite(csPin, HIGH);
    SPI.endTransaction();
}

void SpiService::writeFlashPage(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq) {
    const size_t maxPerPage = 256; // Page size (standard)

    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunkSize = std::min(maxPerPage, data.size() - offset);

        enableFlashWrite(freq);

        SPI.beginTransaction(SPISettings(freq, MSBFIRST, SPI_MODE0));
        digitalWrite(csPin, LOW);

        SPI.transfer(0x02); // Page Program
        SPI.transfer((address >> 16) & 0xFF);
        SPI.transfer((address >> 8) & 0xFF);
        SPI.transfer(address & 0xFF);

        for (size_t i = 0; i < chunkSize; ++i) {
            SPI.transfer(data[offset + i]);
        }

        digitalWrite(csPin, HIGH);
        SPI.endTransaction();

        waitForFlashWriteComplete(freq);

        address += chunkSize;
        offset += chunkSize;
    }
}

void SpiService::writeFlashPatch(uint32_t address, const std::vector<uint8_t>& data, uint32_t freq) {
    const uint32_t sectorSize = 4096;
    uint32_t sectorStart = address & ~(sectorSize - 1);
    uint32_t offsetInSector = address - sectorStart;

    // Read the concerned sector
    std::vector<uint8_t> sectorData(sectorSize, 0xFF);
    readFlashData(sectorStart, sectorData.data(), sectorSize);

    // Modify data
    for (size_t i = 0; i < data.size(); ++i) {
        if ((offsetInSector + i) < sectorSize) {
            sectorData[offsetInSector + i] = data[i];
        }
    }

    // Erase the sector
    eraseFlashSector(sectorStart, freq);

    // Write modified data
    for (uint32_t i = 0; i < sectorSize; i += 256) {
        std::vector<uint8_t> page(sectorData.begin() + i, sectorData.begin() + i + 256);
        writeFlashPage(sectorStart + i, page, freq);
    }
}

std::string SpiService::executeByteCode(const std::vector<ByteCode>& bytecodes) {
    std::string result;
    bool inTransaction = false;

    for (const auto& code : bytecodes) {
        switch (code.getCommand()) {
            case ByteCodeEnum::Start:
                if (!inTransaction) {
                    beginTransaction();
                    inTransaction = true;
                }
                break;

            case ByteCodeEnum::Stop:
                if (inTransaction) {
                    endTransaction();
                    inTransaction = false;
                }
                break;

            case ByteCodeEnum::Write:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    transfer(code.getData());
                }
                break;

            case ByteCodeEnum::Read:
                for (uint32_t i = 0; i < code.getRepeat(); ++i) {
                    uint8_t val = transfer(0x00);  // dummy byte
                    char hex[5];
                    snprintf(hex, sizeof(hex), "%02X ", val);
                    result += hex;
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

    // Close transaction if left open
    if (inTransaction) {
        endTransaction();
    }

    return result;
}

// #### SPI SLAVE ######

static ESP32SPISlave spiSlave;
static constexpr size_t SLAVE_BUFFER_SIZE = 8;
static std::atomic<bool> slave{false};

static uint8_t slave_tx_buf[SLAVE_BUFFER_SIZE] = {0};
static uint8_t slave_rx_buf[SLAVE_BUFFER_SIZE] = {0};

static std::deque<std::vector<uint8_t>> slaveBuffer;
static std::mutex slaveBufferMutex;

void IRAM_ATTR slaveTransactionCallback(spi_slave_transaction_t* trans, void* arg) {
    if (!slave) return;

    // Copy to vector
    size_t length_bytes = (trans->trans_len + 7) / 8; // trans_len in bits
    std::vector<uint8_t> received(slave_rx_buf, slave_rx_buf + length_bytes);

    {
        // Push thread safe
        std::lock_guard<std::mutex> lock(slaveBufferMutex);
        slaveBuffer.push_back(std::move(received));

        // Limit
        if (slaveBuffer.size() > 100) slaveBuffer.pop_front();
    }
}

void SpiService::startSlave(int sclk, int miso, int mosi, int cs) {
    if (slave) return;
    slave = true;

    spiSlave.setDataMode(SPI_MODE0);
    spiSlave.setQueueSize(1);
    spiSlave.setUserPostTransCbAndArg(slaveTransactionCallback, nullptr);
    spiSlave.begin(FSPI, sclk, miso, mosi, cs);
    
    spiSlave.queue(slave_tx_buf, slave_rx_buf, SLAVE_BUFFER_SIZE);
    spiSlave.trigger();    
}

void SpiService::stopSlave(int sclk, int miso, int mosi, int cs) {
    slave = false;
    
    uint32_t t0 = millis();
    while (!spiSlave.hasTransactionsCompletedAndAllResultsHandled() &&
    (millis() - t0) < 100) {
        delay(1);
    }
    
    delay(100);
    slaveBuffer.clear();
    memset(slave_rx_buf, 0, SLAVE_BUFFER_SIZE);
    memset(slave_tx_buf, 0, SLAVE_BUFFER_SIZE);

    spiSlave.end();
    spi_slave_free(SPI2_HOST);   // FSPI
}

bool SpiService::isSlave() const {
    return slave;
}

std::vector<std::vector<uint8_t>> SpiService::getSlaveData() {
    std::vector<std::vector<uint8_t>> out;

    if (spiSlave.hasTransactionsCompletedAndAllResultsReady(1)) {
        size_t receivedBytes = spiSlave.numBytesReceived();
        if (receivedBytes > SLAVE_BUFFER_SIZE) receivedBytes = SLAVE_BUFFER_SIZE;

        if (receivedBytes > 0) {
            out.emplace_back(slave_rx_buf, slave_rx_buf + receivedBytes);
        }

        // relaunch
        memset(slave_rx_buf, 0, SLAVE_BUFFER_SIZE);
        spiSlave.queue(/*tx*/nullptr, /*rx*/slave_rx_buf, SLAVE_BUFFER_SIZE);
        spiSlave.trigger();
    }

    return out;
}

// #### EEPROM ######

bool SpiService::initEeprom(
    uint8_t mosi, 
    uint8_t miso, uint8_t 
    sclk, uint8_t cs, 
    uint16_t pageSize, 
    uint32_t memSize,  
    uint16_t wp,
    bool small) 
{
    if (eepromInitialized) return true;
    SPI.end();

    // Init eeprom
    if (!eeprom.init(sclk, miso, mosi, cs, wp)) return false;

    // Size
    eeprom.setMemorySize((eeprom_size_t)memSize);
    eeprom.setPageSize(
        pageSize == 16  ? EEPROM_PAGE_SIZE_16  :
        pageSize == 32  ? EEPROM_PAGE_SIZE_32  :
        pageSize == 64  ? EEPROM_PAGE_SIZE_64  :
        pageSize == 128 ? EEPROM_PAGE_SIZE_128 :
                          EEPROM_PAGE_SIZE_256
    );

    if (small) {
        eeprom.setSmallEEPROM();
    }

    eepromInitialized = true;
    return true;
}

bool SpiService::probeEeprom() {
    if (!eepromInitialized) return false;
    return eeprom.probe();
}

bool SpiService::writeEeprom(uint32_t address, uint8_t value) {
    if (!eepromInitialized) return false;
    eeprom.write(address, value);
    return true;
}

uint8_t SpiService::readEeprom(uint32_t address) {
    if (!eepromInitialized) return 0xFF;
    return eeprom.read(address);
}

bool SpiService::writeEepromBuffer(uint32_t address, const uint8_t* data, size_t len) {
    if (!eepromInitialized) return false;
    for (size_t i = 0; i < len; ++i) {
        eeprom.write(address + i, data[i]);
    }
    return true;
}

bool SpiService::readEepromBuffer(uint32_t address, uint8_t* buffer, size_t len) {
    if (!eepromInitialized) return false;
    for (size_t i = 0; i < len; ++i) {
        buffer[i] = eeprom.read(address + i);
    }
    return true;
}

bool SpiService::writeEepromInt(uint32_t address, int32_t value) {
    if (!eepromInitialized) return false;
    eeprom.put(address, value);
    return true;
}

int32_t SpiService::readEepromInt(uint32_t address) {
    if (!eepromInitialized) return 0;
    int32_t value = 0;
    eeprom.get(address, value);
    return value;
}

bool SpiService::writeEepromFloat(uint32_t address, float value) {
    if (!eepromInitialized) return false;
    eeprom.put(address, value);
    return true;
}

float SpiService::readEepromFloat(uint32_t address) {
    if (!eepromInitialized) return 0.0f;
    float value = 0.0f;
    eeprom.get(address, value);
    return value;
}

bool SpiService::writeEepromString(uint32_t address, const std::string& str) {
    if (!eepromInitialized) return false;
    String arduinoStr = String(str.c_str()); 
    eeprom.putString(address, arduinoStr);
    return true;
}

bool SpiService::readEepromString(uint32_t address, std::string& str) {
    if (!eepromInitialized) return false;
    String arduinoStr = String(str.c_str());
    eeprom.getString(address, arduinoStr);
    return true;
}

void SpiService::eraseEepromChip() {
    if (eepromInitialized) eeprom.eraseCompleteEEPROM();
}

void SpiService::eraseEepromSector(uint32_t address) {
    if (eepromInitialized) eeprom.eraseSector(address);
}

void SpiService::eraseEepromPage(uint32_t address) {
    if (eepromInitialized) eeprom.erasePage(address);
}

void SpiService::closeEeprom() {
    eepromInitialized = false;
    eeprom = EEPROM_SPI_WE(); 
}
