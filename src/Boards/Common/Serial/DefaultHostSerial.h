#pragma once

#include <Interfaces/IHostSerial.h>

class DefaultHostSerial : public IHostSerial {
public:
    void begin(unsigned long baud) override {
        Serial.begin(baud);
    }

    void waitReady() override {
        while (!Serial) {
            delay(10);
        }
    }

    int available() override {
        return Serial.available();
    }

    int availableForWrite() override {
        return Serial.availableForWrite();
    }

    int peek() override {
        return Serial.peek();
    }

    int read() override {
        return Serial.read();
    }

    size_t readBytes(uint8_t* buffer, size_t length) override {
        return Serial.readBytes(buffer, length);
    }

    size_t write(uint8_t value) override {
        return Serial.write(value);
    }

    size_t write(const uint8_t* buffer, size_t length) override {
        return Serial.write(buffer, length);
    }

    size_t print(const char* text) override {
        return Serial.print(text);
    }

    size_t println(const char* text) override {
        return Serial.println(text);
    }

    size_t println() override {
        return Serial.println();
    }

    void flush() override {
        Serial.flush();
    }

    bool setRxBufferSize(size_t size) override {
        return Serial.setRxBufferSize(size);
    }

    void setTimeout(unsigned long timeoutMs) override {
        Serial.setTimeout(timeoutMs);
    }

    uint32_t baudRate() override {
        return Serial.baudRate();
    }

    void disableReboot() override {
#if !ARDUINO_USB_MODE && ARDUINO_USB_CDC_ON_BOOT
        Serial.enableReboot(false);
#endif
    }

    void waitForPress() override {
        while (!Serial.available()) {}
    }
};
