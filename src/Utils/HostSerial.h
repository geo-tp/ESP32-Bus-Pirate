#ifndef HOST_SERIAL_H
#define HOST_SERIAL_H

#include <Arduino.h>

// Default host link baud when Serial uses UART (CP210x / CH340 bridges).
constexpr unsigned long HOST_SERIAL_UART_BAUD = 115200;

// ESP32-S3 PRG / BOOT (used when the host link has no keyboard in the monitor).
constexpr uint8_t HOST_SERIAL_BOOT_BUTTON_PIN = 0;

inline bool hostSerialUsesCdc() {
#if ARDUINO_USB_CDC_ON_BOOT
    return true;
#else
    return false;
#endif
}

inline void hostSerialDisableReboot() {
#if ARDUINO_USB_CDC_ON_BOOT
    Serial.enableReboot(false);
#endif
}

inline void hostSerialBegin(unsigned long baud = HOST_SERIAL_UART_BAUD) {
#if ARDUINO_USB_CDC_ON_BOOT
    (void)baud;
    Serial.begin();
#else
    Serial.begin(baud);
#endif
}

inline void hostSerialWaitReady(unsigned long baud = HOST_SERIAL_UART_BAUD) {
    hostSerialBegin(baud);
#if ARDUINO_USB_CDC_ON_BOOT
    while (!Serial) {
        delay(10);
    }
#endif
}

// UART host links are always "connected" (no USB CDC attach events).
inline bool hostSerialUartAlwaysConnected() {
    return !hostSerialUsesCdc();
}

// Wait for a host keypress; on UART-only headless boards, PRG also counts.
inline void hostSerialWaitForPress() {
#if defined(DEVICE_S3DEVKIT) && !ARDUINO_USB_CDC_ON_BOOT
    pinMode(HOST_SERIAL_BOOT_BUTTON_PIN, INPUT_PULLUP);
    while (!Serial.available()) {
        if (digitalRead(HOST_SERIAL_BOOT_BUTTON_PIN) == LOW) {
            delay(20);
            while (digitalRead(HOST_SERIAL_BOOT_BUTTON_PIN) == LOW) {
                delay(5);
            }
            return;
        }
        delay(5);
    }
#else
    while (!Serial.available()) {}
#endif
}

#endif // HOST_SERIAL_H
