#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include "Enums/InfraredProtocolEnum.h"
#include "Enums/ModeEnum.h"
#include "Enums/TerminalTypeEnum.h"

class GlobalState {
private:
    // Version
    const std::string version = "1.3";

    //Pin in use
    std::vector<uint8_t> protectedPins;

    // Builtin
    uint8_t ledPin = 21;

    // SPI
    uint8_t spiCSPin = 12;
    uint8_t spiCLKPin = 40;
    uint8_t spiMISOPin = 39;
    uint8_t spiMOSIPin = 14;
    uint32_t spiFrequency = 20000000;

    // WiFi AP Credentials
    std::string apName = "ESP32-Bus-Pirate";
    std::string apPassword = "readytoboard";

    // NVS
    std::string nvsNamespace = "wifi_settings";
    std::string nvsSsidField = "ssid";
    std::string nvsPasswordField = "pass";

    // Terminal Web UI
    std::string terminalIp = "0.0.0.0";

    // Terminal transmission mode
    TerminalTypeEnum terminalMode = TerminalTypeEnum::Serial;

    //  Current selected mode
    ModeEnum currentMode = ModeEnum::HIZ;

    // PC Terminal Serial Configuration
    unsigned long serialTerminalBaudRate = 115200;

    // OneWire Default Pin
    uint8_t oneWirePin = 1; // pin par défaut

    // TwoWire Default Configuration
    uint8_t twoWireClkPin = 1;
    uint8_t twoWireIoPin = 2;
    uint8_t twoWireRstPin = 3;

    // ThreeWire Default Pins
    uint8_t threeWireCsPin = 5;
    uint8_t threeWireSkPin = 18;
    uint8_t threeWireDiPin = 23;
    uint8_t threeWireDoPin = 19;
    bool threeWireOrg8 = false; // 16-bit organization by default
    uint8_t threeWireeEepromModelIndex = 0; // Default eeprom model index

    // UART Default Configuration
    unsigned long uartBaudRate = 9600;
    uint32_t uartConfig = 0x800001c; // SERIAL_8N1 by default
    bool uartInverted = false;
    uint8_t uartRxPin = 1;
    uint8_t uartTxPin = 2;
    uint8_t uartDataBits = 8;
    std::string uartParity = "None";
    bool uartFlowControl = false;
    uint8_t uartStopBits = 1;

    // HDUART Default Configuration
    unsigned long hdUartBaudRate = 9600;
    uint32_t hdUartConfig = 0x800001c; // SERIAL_8N1
    bool hdUartInverted = false;
    uint8_t hdUartPin = 1;
    uint8_t hdUartDataBits = 8;
    std::string hdUartParity = "N";
    bool hdUartFlowControl = false;
    uint8_t hdUartStopBits = 1;

    // I2C Default Configuration
    uint8_t i2cSclPin = 1;
    uint8_t i2cSdaPin = 2;
    uint32_t i2cFrequency = 100000;

    // Infrared Default Configuration
    uint8_t infraredPin = 44; 
    uint8_t infraredRxPin = 1;
    InfraredProtocolEnum infraredProtocol = InfraredProtocolEnum::_NEC;

    // LED Mode
    uint8_t ledDataPin = 1;
    uint8_t ledClockPin = 2;
    uint16_t ledLength = 1;
    std::string ledProtocol = "ws2812";
    uint8_t ledBrightness = 128;

    // I2S Default Configuration
    uint8_t i2sBclkPin = 41;
    uint8_t i2sLrckPin = 43;
    uint8_t i2sDataPin = 42;
    uint32_t i2sSampleRate = 44100;
    uint8_t i2sBitsPerSample = 16;

    // CAN Default Configuration
    uint8_t canCspin = 1;
    uint8_t canSckPin = 0;
    uint8_t canSiPin = 2;
    uint8_t canSoPin = 3;
    uint32_t canKbps = 120;

    // Ethernet Default Configuration
    uint8_t ethernetCsPin = 5;
    uint8_t ethernetRstPin = 4;
    uint8_t ethernetSckPin = 18;
    uint8_t ethernetMisoPin = 19;
    uint8_t ethernetMosiPin = 23;
    uint8_t ethernetIrqPin = 39;
    uint32_t ethernetFrequency = 20000000; // 20 MHz
    std::array<uint8_t,6> ethernetMac = { 0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x42 };

    // SubGHz Default Configuration
    uint8_t subGhzSckPin = 12;
    uint8_t subGhzMisoPin = 13;
    uint8_t subGhzMosiPin = 14;
    uint8_t subGhzCsPin = 15;
    uint8_t subGhzGdoPin = 27;
    float subGhzFrequency = 433.92; // mhz
    uint8_t subGhzPower = 10; // dBm
    uint8_t subGhzModulation = 2; // 0=FSK, 1=OOK, 2=ASK
    uint32_t subGhzBandwidth = 125000; // 125 kHz

    // NRF24 Default Configuration
    uint8_t rf24CsnPin = 1;
    uint8_t rf24CePin = 3;
    uint8_t rf24SckPin = 5;
    uint8_t rf24MisoPin = 7;
    uint8_t rf24MosiPin = 9;

    // RFID (I2C PN532)
    uint8_t rfidSdaPin = 1;
    uint8_t rfidSclPin = 2;

    // JTAG Default Pin
    std::vector<uint8_t> jtagScanPins = { 1, 3, 5, 7, 9 };

    // SD Card Default Configuration
    uint8_t sdCardCsPin = 12;
    uint8_t sdCardClkPin = 40;
    uint8_t sdCardMisoPin = 39;
    uint8_t sdCardMosiPin = 14;
    uint32_t sdCardFrequency = 20000000; // 20 MHz
    bool hasInternalSdCard = false;

    // SD Card File Limits
    size_t fileCountLimit = 512;
    size_t fileCacheLimit = 64;

public:
    GlobalState(const GlobalState&) = delete;
    GlobalState& operator=(const GlobalState&) = delete;

    static GlobalState& getInstance() {
        static GlobalState instance;
        return instance;
    }

    // Version
    const std::string& getVersion() const { return version; }

    // Builtin
    uint8_t getLedPin() const { return ledPin; }

    // SPI
    uint8_t getSpiCSPin() const { return spiCSPin; }
    uint8_t getSpiCLKPin() const { return spiCLKPin; }
    uint8_t getSpiMISOPin() const { return spiMISOPin; }
    uint8_t getSpiMOSIPin() const { return spiMOSIPin; }
    uint32_t getSpiFrequency() const { return spiFrequency; }
    
    void setSpiCSPin(uint8_t pin) { spiCSPin = pin; }
    void setSpiCLKPin(uint8_t pin) { spiCLKPin = pin; }
    void setSpiMISOPin(uint8_t pin) { spiMISOPin = pin; }
    void setSpiMOSIPin(uint8_t pin) { spiMOSIPin = pin; }
    void setSpiFrequency(uint32_t freq) { spiFrequency = freq; }

    // AP WiFi
    const std::string& getApName() const { return apName; }
    const std::string& getApPassword() const { return apPassword; }

    void setApName(const std::string& name) { apName = name; }
    void setApPassword(const std::string& pass) { apPassword = pass; }

    // Terminal IP
    const std::string& getTerminalIp() const { return terminalIp; }
    void setTerminalIp(const std::string& ip) { terminalIp = ip; }

    // Terminal transmission mode
    TerminalTypeEnum getTerminalMode() const { return terminalMode; }
    void setTerminalMode(TerminalTypeEnum mode) { terminalMode = mode; }

    // Current Mode
    ModeEnum getCurrentMode() const { return currentMode; }
    void setCurrentMode(ModeEnum mode) { currentMode = mode; }

    // Serial Terminal Baud
    unsigned long getSerialTerminalBaudRate() const { return serialTerminalBaudRate; }
    void setSerialTerminalBaudRate(unsigned long rate) { serialTerminalBaudRate = rate; }

    // OneWire
    uint8_t getOneWirePin() const { return oneWirePin; }
    void setOneWirePin(uint8_t pin) { oneWirePin = pin; }

    // TwoWire
    uint8_t getTwoWireClkPin() const { return twoWireClkPin; }
    uint8_t getTwoWireIoPin() const { return twoWireIoPin; }
    uint8_t getTwoWireRstPin() const { return twoWireRstPin; }

    void setTwoWireClkPin(uint8_t pin) { twoWireClkPin = pin; }
    void setTwoWireIoPin(uint8_t pin) { twoWireIoPin = pin; }
    void setTwoWireRstPin(uint8_t pin) { twoWireRstPin = pin; }

    // ThreeWire
    uint8_t getThreeWireCsPin() const { return threeWireCsPin; }
    uint8_t getThreeWireSkPin() const { return threeWireSkPin; }
    uint8_t getThreeWireDiPin() const { return threeWireDiPin; }
    uint8_t getThreeWireDoPin() const { return threeWireDoPin; }
    bool isThreeWireOrg8() const { return threeWireOrg8; }
    uint8_t getThreeWireEepromModelIndex() const { return threeWireeEepromModelIndex; }

    void setThreeWireCsPin(uint8_t pin) { threeWireCsPin = pin; }
    void setThreeWireSkPin(uint8_t pin) { threeWireSkPin = pin ; }
    void setThreeWireDiPin(uint8_t pin) { threeWireDiPin = pin; }
    void setThreeWireDoPin(uint8_t pin) { threeWireDoPin = pin; }
    void setThreeWireOrg8(bool org8) { threeWireOrg8 = org8; }
    void setThreeWireEepromModelIndex(uint8_t index) { threeWireeEepromModelIndex = index; }

    // UART
    unsigned long getUartBaudRate() const { return uartBaudRate; }
    uint32_t getUartConfig() const { return uartConfig; }
    uint8_t getUartRxPin() const { return uartRxPin; }
    uint8_t getUartTxPin() const { return uartTxPin; }
    uint8_t getUartDataBits() const { return uartDataBits; }
    const std::string& getUartParity() const { return uartParity; }
    uint8_t getUartStopBits() const { return uartStopBits; }
    bool isUartInverted() const { return uartInverted; }
    bool isUartFlowControl() const { return uartFlowControl; }

    void setUartBaudRate(unsigned long b) { uartBaudRate = b; }
    void setUartConfig(uint32_t c) { uartConfig = c; }
    void setUartInverted(bool inv) { uartInverted = inv; }
    void setUartRxPin(uint8_t pin) { uartRxPin = pin; }
    void setUartTxPin(uint8_t pin) { uartTxPin = pin; }
    void setUartDataBits(uint8_t bits) { uartDataBits = bits; }
    void setUartParity(const std::string& parityVal) { uartParity = parityVal; }
    void setUartFlowControl(bool enabled) { uartFlowControl = enabled; }
    void setUartStopBits(uint8_t bits) { uartStopBits = bits; }

    // HDUART
    unsigned long getHdUartBaudRate() const { return hdUartBaudRate; }
    uint32_t getHdUartConfig() const { return hdUartConfig; }
    uint8_t getHdUartPin() const { return hdUartPin; }
    uint8_t getHdUartDataBits() const { return hdUartDataBits; }
    const std::string& getHdUartParity() const { return hdUartParity; }
    uint8_t getHdUartStopBits() const { return hdUartStopBits; }
    bool isHdUartInverted() const { return hdUartInverted; }
    bool isHdUartFlowControl() const { return hdUartFlowControl; }

    void setHdUartBaudRate(unsigned long b) { hdUartBaudRate = b; }
    void setHdUartConfig(uint32_t c) { hdUartConfig = c; }
    void setHdUartInverted(bool inv) { hdUartInverted = inv; }
    void setHdUartPin(uint8_t pin) { hdUartPin = pin; }
    void setHdUartDataBits(uint8_t bits) { hdUartDataBits = bits; }
    void setHdUartParity(const std::string& parityVal) { hdUartParity = parityVal; }
    void setHdUartFlowControl(bool enabled) { hdUartFlowControl = enabled; }
    void setHdUartStopBits(uint8_t bits) { hdUartStopBits = bits; }

    // I2C
    uint8_t getI2cSdaPin() const { return i2cSdaPin; }
    uint8_t getI2cSclPin() const { return i2cSclPin; }
    uint32_t getI2cFrequency() const { return i2cFrequency; }

    void setI2cSdaPin(uint8_t pin) { i2cSdaPin = pin; }
    void setI2cSclPin(uint8_t pin) { i2cSclPin = pin; }
    void setI2cFrequency(uint32_t freq) { i2cFrequency = freq; }

    // Infrared
    uint8_t getInfraredTxPin() const { return infraredPin; }
    uint8_t getInfraredRxPin() const { return infraredRxPin; }
    void setInfraredTxPin(uint8_t pin) { infraredPin = pin; }
    void setInfraredRxPin(uint8_t pin) { infraredRxPin = pin; }
    InfraredProtocolEnum getInfraredProtocol() const { return infraredProtocol ; }
    void setInfraredProtocol(InfraredProtocolEnum prot) {  infraredProtocol = prot; }

    // LED Mode
    uint8_t getLedDataPin() const { return ledDataPin; }
    uint8_t getLedClockPin() const { return ledClockPin; }
    uint16_t getLedLength() const { return ledLength; }
    const std::string& getLedProtocol() const { return ledProtocol; }
    uint8_t getLedBrightness() const { return ledBrightness; }

    void setLedDataPin(uint8_t pin) { ledDataPin = pin; }
    void setLedClockPin(uint8_t pin) { ledClockPin = pin; }
    void setLedLength(uint16_t len) { ledLength = len; }
    void setLedProtocol(const std::string& prot) { ledProtocol = prot; }
    void setLedBrightness(uint8_t brightness) { ledBrightness = brightness; }

    // I2S
    uint8_t getI2sBclkPin() const { return i2sBclkPin; }
    uint8_t getI2sLrckPin() const { return i2sLrckPin; }
    uint8_t getI2sDataPin() const { return i2sDataPin; }
    uint32_t getI2sSampleRate() const { return i2sSampleRate; }
    uint8_t getI2sBitsPerSample() const { return i2sBitsPerSample; }

    void setI2sBclkPin(uint8_t pin) { i2sBclkPin = pin; }
    void setI2sLrckPin(uint8_t pin) { i2sLrckPin = pin; }
    void setI2sDataPin(uint8_t pin) { i2sDataPin = pin; }
    void setI2sSampleRate(uint32_t rate) { i2sSampleRate = rate; }
    void setI2sBitsPerSample(uint8_t bits) { i2sBitsPerSample = bits; }

    // JTAG
    const std::vector<uint8_t>& getJtagScanPins() const { return jtagScanPins; }
    void setJtagScanPins(const std::vector<uint8_t>& pins) { jtagScanPins = pins; }

    // CAN
    uint8_t getCanCspin() const { return canCspin; }
    uint8_t getCanSckPin() const { return canSckPin; }
    uint8_t getCanSiPin() const { return canSiPin; }
    uint8_t getCanSoPin() const { return canSoPin; }
    uint32_t getCanKbps() const { return canKbps; }

    void setCanCspin(uint8_t pin) { canCspin = pin; }
    void setCanSckPin(uint8_t pin) { canSckPin = pin; }
    void setCanSiPin(uint8_t pin) { canSiPin = pin; }
    void setCanSoPin(uint8_t pin) { canSoPin = pin; }
    void setCanKbps(uint32_t kbps) { canKbps = kbps; } 

    // Ethernet
    uint8_t getEthernetCsPin() const { return ethernetCsPin; }
    uint8_t getEthernetRstPin() const { return ethernetRstPin; }
    uint8_t getEthernetSckPin() const { return ethernetSckPin; }
    uint8_t getEthernetMisoPin() const { return ethernetMisoPin; }
    uint8_t getEthernetMosiPin() const { return ethernetMosiPin; }
    uint32_t getEthernetFrequency() const { return ethernetFrequency; }
    uint8_t getEthernetIrqPin() const { return ethernetIrqPin; }
    const std::array<uint8_t,6>& getEthernetMac() const { return ethernetMac; }

    void setEthernetCsPin(uint8_t pin) { ethernetCsPin = pin; }
    void setEthernetRstPin(uint8_t pin) { ethernetRstPin = pin; }
    void setEthernetSckPin(uint8_t pin) { ethernetSckPin = pin; }
    void setEthernetMisoPin(uint8_t pin) { ethernetMisoPin = pin; }
    void setEthernetMosiPin(uint8_t pin) { ethernetMosiPin = pin; }
    void setEthernetFrequency(uint32_t freq) { ethernetFrequency = freq; }
    void setEthernetIrqPin(uint8_t pin) { ethernetIrqPin = pin; }
    void setEthernetMac(const std::array<uint8_t,6>& mac) { ethernetMac = mac; }

    // SubGHz
    uint8_t getSubGhzSckPin() const { return subGhzSckPin; }
    uint8_t getSubGhzMisoPin() const { return subGhzMisoPin; }
    uint8_t getSubGhzMosiPin() const { return subGhzMosiPin; }
    uint8_t getSubGhzCsPin() const { return subGhzCsPin; }
    uint8_t getSubGhzGdoPin() const { return subGhzGdoPin; }
    float getSubGhzFrequency() const { return subGhzFrequency; }

    void setSubGhzSckPin(uint8_t pin) { subGhzSckPin = pin; }
    void setSubGhzMisoPin(uint8_t pin) { subGhzMisoPin = pin; }
    void setSubGhzMosiPin(uint8_t pin) { subGhzMosiPin = pin; }
    void setSubGhzCsPin(uint8_t pin) { subGhzCsPin = pin; }
    void setSubGhzGdoPin(uint8_t pin) { subGhzGdoPin = pin; }
    void setSubGhzFrequency(float freq) { subGhzFrequency = freq; }

    // NRF24
    uint8_t getRf24CsnPin() const { return rf24CsnPin; }
    uint8_t getRf24CePin() const { return rf24CePin; }
    uint8_t getRf24SckPin() const { return rf24SckPin; }
    uint8_t getRf24MisoPin() const { return rf24MisoPin; }
    uint8_t getRf24MosiPin() const { return rf24MosiPin; }

    void setRf24CsnPin(uint8_t pin) { rf24CsnPin = pin; }
    void setRf24CePin(uint8_t pin) { rf24CePin = pin; }
    void setRf24SckPin(uint8_t pin) { rf24SckPin = pin; }
    void setRf24MisoPin(uint8_t pin) { rf24MisoPin = pin; }
    void setRf24MosiPin(uint8_t pin) { rf24MosiPin = pin; }

    // RFID (I2C PN532)
    uint8_t getRfidSdaPin() const { return rfidSdaPin; }
    uint8_t getRfidSclPin() const { return rfidSclPin; }

    void setRfidSdaPin(uint8_t pin) { rfidSdaPin = pin; }
    void setRfidSclPin(uint8_t pin) { rfidSclPin = pin; }

    // SD Card
    uint8_t getSdCardCsPin() const { return sdCardCsPin; }
    uint8_t getSdCardClkPin() const { return sdCardClkPin; }
    uint8_t getSdCardMisoPin() const { return sdCardMisoPin; }
    uint8_t getSdCardMosiPin() const { return sdCardMosiPin; }
    uint32_t getSdCardFrequency() const { return sdCardFrequency; }
    bool getHasInternalSdCard() const { return hasInternalSdCard; }

    void setSdCardCsPin(uint8_t pin) { sdCardCsPin = pin; }
    void setSdCardClkPin(uint8_t pin) { sdCardClkPin = pin; }
    void setSdCardMisoPin(uint8_t pin) { sdCardMisoPin = pin; }
    void setSdCardMosiPin(uint8_t pin) { sdCardMosiPin = pin; }
    void setSdCardFrequency(uint32_t freq) { sdCardFrequency = freq; }

    // SD File Limits
    size_t getFileCountLimit() const { return fileCountLimit; }
    size_t getFileCacheLimit() const { return fileCacheLimit; }

    void setFileCountLimit(size_t count) { fileCountLimit = count; }
    void setFileCacheLimit(size_t cache) { fileCacheLimit = cache; }

    // NVS
    const std::string& getNvsNamespace() const { return nvsNamespace; }
    void setNvsNamespace(const std::string& ns) { nvsNamespace = ns; }

    const std::string& getNvsPasswordField() const { return nvsPasswordField; }
    void setNvsPasswordField(const std::string& f) { nvsPasswordField = f; }

    const std::string& getNvsSsidField() const { return nvsSsidField; }
    void setNvsSsidField(const std::string& f) { nvsSsidField = f; }

    // Protected
    const std::vector<uint8_t>& getProtectedPins() const {
        return protectedPins;
    }

    bool isPinProtected(uint8_t pin) const {
        return std::find(protectedPins.begin(), protectedPins.end(), pin) != protectedPins.end();
    }

    bool isPinAnalog(uint8_t pin) const {
        return ((pin > 0) && (pin < 19));
    }

    // Constructor
    GlobalState() {
        #ifdef LED_PIN
            ledPin = LED_PIN;
        #endif
        #ifdef SPI_CS_PIN
            spiCSPin = SPI_CS_PIN;
        #endif
        #ifdef SPI_CLK_PIN
            spiCLKPin = SPI_CLK_PIN;
        #endif
        #ifdef SPI_MISO_PIN
            spiMISOPin = SPI_MISO_PIN;
        #endif
        #ifdef SPI_MOSI_PIN
            spiMOSIPin = SPI_MOSI_PIN;
        #endif
        #ifdef ONEWIRE_PIN
            oneWirePin = ONEWIRE_PIN;
        #endif
        #ifdef UART_BAUD
            uartBaudRate = UART_BAUD;
        #endif
        #ifdef UART_RX_PIN
            uartRxPin = UART_RX_PIN;
        #endif
        #ifdef UART_TX_PIN
            uartTxPin = UART_TX_PIN;
        #endif
        #ifdef HDUART_BAUD
            hdUartBaudRate = HDUART_BAUD;
        #endif
        #ifdef HDUART_PIN
            hdUartPin = HDUART_PIN;
        #endif
        #ifdef I2C_SCL_PIN
            i2cSclPin = I2C_SCL_PIN;
        #endif
        #ifdef I2C_SDA_PIN
            i2cSdaPin = I2C_SDA_PIN;
        #endif
        #ifdef I2C_FREQ
            i2cFrequency = I2C_FREQ;
        #endif
        #ifdef IR_TX_PIN
            infraredPin = IR_TX_PIN;
        #endif
        #ifdef IR_RX_PIN
            infraredRxPin = IR_RX_PIN;
        #endif
        #ifdef I2S_BCLK_PIN
            i2sBclkPin = I2S_BCLK_PIN;
        #endif
        #ifdef I2S_LRCK_PIN
            i2sLrckPin = I2S_LRCK_PIN;
        #endif
        #ifdef I2S_DATA_PIN
            i2sDataPin = I2S_DATA_PIN;
        #endif
        #ifdef I2S_SAMPLE_RATE
            i2sSampleRate = I2S_SAMPLE_RATE;
        #endif
        #ifdef I2S_BITS
            i2sBitsPerSample = I2S_BITS;
        #endif
        #ifdef TWOWIRE_CLK_PIN
            twoWireClkPin = TWOWIRE_CLK_PIN;
        #endif
        #ifdef TWOWIRE_IO_PIN
            twoWireIoPin = TWOWIRE_IO_PIN;
        #endif
        #ifdef TWOWIRE_RST_PIN
            twoWireRstPin = TWOWIRE_RST_PIN;
        #endif
        #ifdef THREEWIRE_CS_PIN
            threeWireCsPin = THREEWIRE_CS_PIN;
        #endif
        #ifdef THREEWIRE_SK_PIN
            threeWireSkPin = THREEWIRE_SK_PIN;
        #endif
        #ifdef THREEWIRE_DI_PIN 
            threeWireDiPin = THREEWIRE_DI_PIN;
        #endif
        #ifdef THREEWIRE_DO_PIN 
            threeWireDoPin = THREEWIRE_DO_PIN;
        #endif
        #ifdef LED_DATA_PIN
            ledDataPin = LED_DATA_PIN;
        #endif
        #ifdef LED_CLOCK_PIN
            ledClockPin = LED_CLOCK_PIN;
        #endif
        #ifdef CAN_CS_PIN
            canCspin = CAN_CS_PIN;
        #endif
        #ifdef CAN_SCK_PIN
            canSckPin = CAN_SCK_PIN;
        #endif
        #ifdef CAN_SI_PIN
            canSiPin = CAN_SI_PIN;
        #endif
        #ifdef CAN_SO_PIN
            canSoPin = CAN_SO_PIN;
        #endif
        #ifdef CAN_KBPS
            canKbps = CAN_KBPS;
        #endif
        #ifdef ETHERNET_CS_PIN
            ethernetCsPin = ETHERNET_CS_PIN;
        #endif
        #ifdef ETHERNET_CLK_PIN
            ethernetSckPin = ETHERNET_CLK_PIN;
        #endif
        #ifdef ETHERNET_MISO_PIN
            ethernetMisoPin = ETHERNET_MISO_PIN;
        #endif
        #ifdef ETHERNET_MOSI_PIN
            ethernetMosiPin = ETHERNET_MOSI_PIN;
        #endif
        #ifdef ETHERNET_IRQ_PIN
            ethernetIrqPin = ETHERNET_IRQ_PIN;
        #endif
        #ifdef SUBGHZ_SCK_PIN
            subGhzSckPin = SUBGHZ_SCK_PIN;
        #endif
        #ifdef SUBGHZ_SO_PIN
            subGhzMisoPin = SUBGHZ_SO_PIN;
        #endif
        #ifdef SUBGHZ_SI_PIN
            subGhzMosiPin = SUBGHZ_SI_PIN;
        #endif
        #ifdef SUBGHZ_CS_PIN
            subGhzCsPin = SUBGHZ_CS_PIN;
        #endif
        #ifdef SUBGHZ_GDO_PIN
            subGhzGdoPin = SUBGHZ_GDO_PIN;
        #endif
        #ifdef I2C_SCL_PIN
            rfidSclPin = I2C_SCL_PIN;
        #endif
        #ifdef I2C_SDA_PIN
            rfidSdaPin = I2C_SDA_PIN;
        #endif
        #ifdef SUBGHZ_CS_PIN
            rf24CsnPin = SUBGHZ_CS_PIN;
        #endif
        #ifdef SUBGHZ_GDO_PIN
            rf24CePin = SUBGHZ_GDO_PIN;
        #endif
        #ifdef SUBGHZ_SCK_PIN
            rf24SckPin = SUBGHZ_SCK_PIN;
        #endif
        #ifdef SUBGHZ_SO_PIN
            rf24MisoPin = SUBGHZ_SO_PIN;
        #endif
        #ifdef SUBGHZ_SI_PIN
            rf24MosiPin = SUBGHZ_SI_PIN;
        #endif
        #ifdef SDCARD_CS_PIN
            hasInternalSdCard = true;
            sdCardCsPin = SDCARD_CS_PIN;
        #else 
            sdCardCsPin = spiCSPin;
        #endif
        #ifdef SDCARD_CLK_PIN
            sdCardClkPin = SDCARD_CLK_PIN;
        #else 
            sdCardClkPin = spiCLKPin;
        #endif
        #ifdef SDCARD_MISO_PIN
            sdCardMisoPin = SDCARD_MISO_PIN;
        #else
            sdCardMisoPin = spiMISOPin;
        #endif
        #ifdef SDCARD_MOSI_PIN
            sdCardMosiPin = SDCARD_MOSI_PIN;
        #else
            sdCardMosiPin = spiMOSIPin;
        #endif

        #ifdef PROTECTED_PINS
        {
            protectedPins.clear();
            std::string pinsStr = PROTECTED_PINS;
            std::stringstream ss(pinsStr);
            std::string item;
            while (std::getline(ss, item, ',')) {
                protectedPins.push_back(std::stoi(item));
            }
        }
        #endif

        #ifdef JTAG_SCAN_PINS
        {
            jtagScanPins.clear();
            std::string pinsStr = JTAG_SCAN_PINS;
            std::stringstream ss(pinsStr);
            std::string item;
            while (std::getline(ss, item, ',')) {
                jtagScanPins.push_back((std::stoi(item)));
            }
        }
        #endif
    }
};
