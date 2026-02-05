#pragma once
#include <string>
#include <vector>
#include "Arduino.h"
#include <XModem.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "hal/uart_types.h"
#include "soc/uart_periph.h"
#include "Models/ByteCode.h"
#include <SD.h>
#include <map>

#define UART_PORT UART_NUM_1

class UartService {
public:
    struct PinActivity {
        uint8_t pin;
        uint32_t edges;
        float edgesPerSec;
        uint32_t approxBaud;
    };

    void configure(unsigned long baud, uint32_t config, uint8_t rx, uint8_t tx, bool inverted);
    void print(const std::string& msg);
    void println(const std::string& msg);
    char read();
    std::string readLine();
    bool available() const;
    void write(char c);
    void write(const std::string& str);
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes);
    void switchBaudrate(unsigned long newBaud);
    void flush();
    void clearUartBuffer();
    void end();
    uint32_t buildUartConfig(uint8_t dataBits, char parity, uint8_t stopBits);
    void initXmodem();
    bool xmodemReceiveToFile(File& file);
    bool xmodemSendFile(File& file);
    static void blockLookupHandler(void* blk_id, size_t idSize, byte* data, size_t dataSize);
    static bool receiveBlockHandler(void* blk_id, size_t idSize, byte* data, size_t dataSize);
    void setXmodemReceiveHandler(bool (*handler)(void*, size_t, byte*, size_t));
    void setXmodemSendHandler(void (*handler)(void*, size_t, byte*, size_t));
    void setXmodemBlockSize(int32_t size);
    void setXmodemIdSize(int8_t size);
    void setXmodemCrc(bool enabled);
    int32_t getXmodemBlockSize() const;
    int8_t getXmodemIdSize() const;
    PinActivity measureUartActivity(uint8_t pin, uint32_t windowMs = 100, bool pullup = true);
    std::vector<PinActivity> scanUartActivity(const std::vector<uint8_t>& pins,
                                              uint32_t windowMs = 100,
                                              uint32_t minEdges = 10,
                                              bool pullup = true);
    uint32_t detectBaudByEdge(uint8_t pin, uint32_t totalMs = 5000, uint32_t windowMs = 300, uint32_t minEdges = 30, bool pullup = true);

private:
    XModem xmodem;
    static File* currentFile;
    int32_t xmodemBlockSize = 128;
    int8_t xmodemIdSize = 1;
    XModem::ProtocolType xmodemProtocol = XModem::ProtocolType::CRC_XMODEM;

    static void IRAM_ATTR onGpioEdge(void* arg);
    inline static volatile uint32_t edgeCounts[50] = {0};
    inline static volatile uint32_t lastEdgeTimeUs = 0;
    inline static volatile uint32_t edgeIntervals[64] = {0};
    inline static volatile uint8_t intervalCount = 0;

    inline static constexpr uint32_t kBaudRates[] = {
        // Legacy 
        110, 300, 600, 1200, 1800, 2000, 2400, 3600, 4800, 7200,
        9600, 10400, 14400, 16000, 19200,

        // Mid-range
        28800, 31250, 32000, 33600, 38400,
        56000, 57600, 64000, 76800,

        // High 
        100000, 115200, 125000, 128000, 153600,
        200000, 230400, 250000, 256000, 307200,

        // Very high
        460800, 500000, 576000, 614400, 750000,
        921600, 1000000, 1152000, 1228800, 1500000,

        // Extreme
        2000000, 2500000, 3000000, 3500000, 4000000
    };

    static constexpr size_t kBaudRatesCount = sizeof(kBaudRates) / sizeof(kBaudRates[0]);
};
