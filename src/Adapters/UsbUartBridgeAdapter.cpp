#include "UsbUartBridgeAdapter.h"
#include "driver/gpio.h"
#include <USBCDC.h>
#include <algorithm>

namespace {
constexpr size_t USB_RX_BUFFER_SIZE  = 16 * 1024;
constexpr size_t UART_RX_BUFFER_SIZE = 16 * 1024;
constexpr size_t UART_TX_BUFFER_SIZE = 16 * 1024;
constexpr size_t BRIDGE_CHUNK_SIZE   = 512;
constexpr unsigned long DEFAULT_BAUD = 115200;
}

uint32_t UsbUartBridgeAdapter::cdcLineCodingToUartConfig(uint8_t dataBits, uint8_t parity, uint8_t stopBits) {
    bool twoStopBits = stopBits == 2;

    if (parity == 1) {
        if (dataBits == 5) return twoStopBits ? SERIAL_5O2 : SERIAL_5O1;
        if (dataBits == 6) return twoStopBits ? SERIAL_6O2 : SERIAL_6O1;
        if (dataBits == 7) return twoStopBits ? SERIAL_7O2 : SERIAL_7O1;
        return twoStopBits ? SERIAL_8O2 : SERIAL_8O1;
    }

    if (parity == 2) {
        if (dataBits == 5) return twoStopBits ? SERIAL_5E2 : SERIAL_5E1;
        if (dataBits == 6) return twoStopBits ? SERIAL_6E2 : SERIAL_6E1;
        if (dataBits == 7) return twoStopBits ? SERIAL_7E2 : SERIAL_7E1;
        return twoStopBits ? SERIAL_8E2 : SERIAL_8E1;
    }

    if (dataBits == 5) return twoStopBits ? SERIAL_5N2 : SERIAL_5N1;
    if (dataBits == 6) return twoStopBits ? SERIAL_6N2 : SERIAL_6N1;
    if (dataBits == 7) return twoStopBits ? SERIAL_7N2 : SERIAL_7N1;

    return twoStopBits ? SERIAL_8N2 : SERIAL_8N1;
}

void UsbUartBridgeAdapter::configureUart(unsigned long baudRate, uint32_t uartConfig) {
    if (baudRate == 0) {
        return;
    }

    if (baudRate == currentBaudRate && uartConfig == currentUartConfig) {
        return;
    }

    currentBaudRate = baudRate;
    currentUartConfig = uartConfig;

    Serial1.end();

    gpio_reset_pin(static_cast<gpio_num_t>(bridgeConfig.uartRxPin));
    gpio_reset_pin(static_cast<gpio_num_t>(bridgeConfig.uartTxPin));

    Serial1.setRxBufferSize(UART_RX_BUFFER_SIZE);
    Serial1.setTxBufferSize(UART_TX_BUFFER_SIZE);

    Serial1.setTimeout(0);

    Serial1.begin(
        baudRate,
        uartConfig,
        bridgeConfig.uartRxPin,
        bridgeConfig.uartTxPin,
        bridgeConfig.uartInverted
    );
}

#if !ARDUINO_USB_MODE
void UsbUartBridgeAdapter::onLineCoding(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData) {
    (void)arg;
    (void)eventBase;

    if (eventId != ARDUINO_USB_CDC_LINE_CODING_EVENT || eventData == nullptr) {
        return;
    }

    auto* data = static_cast<arduino_usb_cdc_event_data_t*>(eventData);

    uint32_t uartConfig = cdcLineCodingToUartConfig(
        data->line_coding.data_bits,
        data->line_coding.parity,
        data->line_coding.stop_bits
    );

    configureUart(data->line_coding.bit_rate, uartConfig);
}
#endif

void UsbUartBridgeAdapter::pumpUartToUsb() {
    uint8_t buffer[BRIDGE_CHUNK_SIZE];

    int available = Serial1.available();
    if (available <= 0) {
        return;
    }

    int writable = hostSerial->availableForWrite();
    if (writable <= 0) {
        return;
    }

    int count = std::min<int>(available, writable);
    count = std::min<int>(count, BRIDGE_CHUNK_SIZE);

    size_t got = Serial1.readBytes(buffer, count);
    if (got == 0) {
        return;
    }

    hostSerial->write(buffer, got);
}

void UsbUartBridgeAdapter::pumpUsbToUart() {
    uint8_t buffer[BRIDGE_CHUNK_SIZE];

    int available = hostSerial->available();
    if (available <= 0) {
        return;
    }

    int writable = Serial1.availableForWrite();
    if (writable <= 0) {
        return;
    }

    int count = std::min<int>(available, writable);
    count = std::min<int>(count, BRIDGE_CHUNK_SIZE);

    size_t got = hostSerial->readBytes(buffer, count);
    if (got == 0) {
        return;
    }

    Serial1.write(buffer, got);
}

void UsbUartBridgeAdapter::pumpBridgeOnce() {
    pumpUartToUsb();
    pumpUsbToUart();

    // immediate pump after writing to UART to minimize latency
    pumpUartToUsb();
}

void UsbUartBridgeAdapter::run(const UsbUartBridgeConfig& config, IInput& input, IHostSerial& hostSerialRef) {
    hostSerial = &hostSerialRef;
    bridgeConfig = config;
    currentBaudRate = 0;
    currentUartConfig = SERIAL_8N1;

    unsigned long lastInputCheckMs = 0;
    unsigned long lastLineCodingCheckMs = 0;
    uint32_t loopCounter = 0;

    hostSerial->disableReboot();
    hostSerial->setRxBufferSize(USB_RX_BUFFER_SIZE);
    hostSerial->setTimeout(0);
#if !ARDUINO_USB_MODE && ARDUINO_USB_CDC_ON_BOOT
    Serial.onEvent(ARDUINO_USB_CDC_LINE_CODING_EVENT, onLineCoding);
#endif
    hostSerial->begin(DEFAULT_BAUD);

    configureUart(DEFAULT_BAUD, SERIAL_8N1);

    while (true) {
        pumpBridgeOnce();

        unsigned long now = millis();

        if ((uint32_t)(now - lastLineCodingCheckMs) >= 100) {
            lastLineCodingCheckMs = now;

            uint32_t cdcBaud = hostSerial->baudRate();
            if (cdcBaud > 0 && cdcBaud != currentBaudRate) {
                configureUart(cdcBaud, currentUartConfig);
            }
        }

        if ((uint32_t)(now - lastInputCheckMs) >= 20) {
            lastInputCheckMs = now;

            if (input.readChar() != KEY_NONE) {
                Serial1.end();
                ESP.restart();
            }
        }

        loopCounter++;
        if ((loopCounter & 0xFF) == 0) {
            yield();
        }
    }
}