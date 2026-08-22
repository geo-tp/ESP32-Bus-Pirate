#pragma once

#include <vector>
#include <Arduino.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "hal/uart_types.h"
#include "soc/uart_periph.h"
#include "Models/ByteCode.h"
#include "Interfaces/IHdUartService.h"

// ESP32-C6 exposes only two HP UARTs (SOC_UART_HP_NUM == 2, its third
// SOC_UART_NUM entry is the LP_UART), so fall back to UART1 there.
// Targets with three HP UARTs (e.g. ESP32-S3) keep using UART2.
#if SOC_UART_HP_NUM > 2
#define HD_UART_PORT UART_NUM_2
#else
#define HD_UART_PORT UART_NUM_1
#endif
#define UART_RX_BUFFER_SIZE 256

class HdUartService : public IHdUartService {
public:
    void configure(unsigned long baud, uint8_t dataBits, char parity, uint8_t stopBits, uint8_t ioPin, bool inverted) override;
    void write(uint8_t data) override;
    void write(const std::string& str) override;
    bool available() const override;
    char read() override;
    std::string readLine() override;
    std::string executeByteCode(const std::vector<ByteCode>& bytecodes) override;
    void flush() override;
    uart_config_t buildUartConfig(unsigned long baud, uint8_t bits, char parity, uint8_t stop);
    void end() override;

private:
    uint8_t ioPin;
    unsigned long baudRate;
    uint32_t serialConfig;
    bool isInverted;

};
