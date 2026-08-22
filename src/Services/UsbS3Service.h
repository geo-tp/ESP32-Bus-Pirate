#pragma once

#include <Arduino.h>
#include <string>
#include "Interfaces/IUsbS3Service.h"

#if CONFIG_IDF_TARGET_ESP32S3

#include <USB.h>
#include <USBMSC.h>
#include <SPI.h>
#include <SD.h>
#include <USBHIDMouse.h>
#include <USBHIDKeyboard.h>
#include <USBHIDGamepad.h>
#include <USBHIDSystemControl.h>
#include "usb/usb_host.h"
#include "usb/usb_types_ch9.h"
#include "usb/usb_types_stack.h"
#include "usb/usb_helpers.h"
#include "esp_log.h"
#include <sstream>

class UsbS3Service : public IUsbS3Service {
public:
    UsbS3Service();

    // Status
    bool isKeyboardActive() const;
    bool isStorageActive() const;
    bool isMouseActive() const;
    bool isGamepadActive() const;
    bool isHostActive() const;
    bool isSystemControlActive() const;

    // Keyboard actions
    void keyboardBegin();
    void keyboardSendString(const std::string& text);
    void keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks);

    // Mass Storage mode
    void storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi);

    // Mouse actions
    void mouseBegin();
    void mouseMove(int x, int y);
    void mouseClick(int button);
    void mouseRelease(int button);

    // Gamepad actions
    void gamepadBegin();
    void gamepadPress(const std::string& name);

    // Host
    bool usbHostBegin();
    std::string usbHostTick();
    void usbHostEnd();
    static void hostClientEventCb(const usb_host_client_event_msg_t *event_msg, void *arg);

    // System control
    void systemControlBegin();
    void systemControlEnd();
    void systemSleep();
    void systemWake();
    void systemPowerOff(uint32_t holdMs = 10);

    // Config
    void configure(const char* productStr, const char* manufacturerStr, const char* serialStr, uint16_t vid, uint16_t pid, const char* webUSBString);
    void reset();
    std::string getUsbSerialFromEfuseMac();

private:
    bool initialized;

    // HID
    bool gamepadActive = false;
    bool keyboardActive = false;
    bool mouseActive = false;
    bool systemControlActive = false;
    unsigned long hidInitTime;

    // Mass Storage
    SPIClass sdSPI;
    bool storageActive;
    static int32_t storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    static int32_t storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    static bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject);
    void setupStorageEvent();

    // Host
    bool stopTinyUsbDevice();
    bool hostInstalled = false;
    usb_host_client_handle_t hostClient = nullptr;
    uint8_t devAddr = 0;
    bool attachPending = false;
    bool detachPending = false;
    bool dumpedThisAttach = false;
    inline static const char* TAG_USBHOST = "UsbHost";
};

#else

// Non-functional stub for targets without USB OTG host/device hardware
// (e.g. ESP32-C6). Mirrors ZigbeeService's unsupported-target behavior:
// compiles everywhere, all operations are no-ops.
class UsbS3Service : public IUsbS3Service {
public:
    UsbS3Service() = default;

    // Status
    bool isKeyboardActive() const override { return false; }
    bool isStorageActive() const override { return false; }
    bool isMouseActive() const override { return false; }
    bool isGamepadActive() const override { return false; }
    bool isHostActive() const override { return false; }
    bool isSystemControlActive() const override { return false; }

    // Keyboard actions
    void keyboardBegin() override {}
    void keyboardSendString(const std::string& text) override { (void)text; }
    void keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks) override {
        (void)data; (void)chunkSize; (void)delayBetweenChunks;
    }

    // Mass Storage mode
    void storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi) override {
        (void)cs; (void)clk; (void)miso; (void)mosi;
    }

    // Mouse actions
    void mouseBegin() override {}
    void mouseMove(int x, int y) override { (void)x; (void)y; }
    void mouseClick(int button) override { (void)button; }
    void mouseRelease(int button) override { (void)button; }

    // Gamepad actions
    void gamepadBegin() override {}
    void gamepadPress(const std::string& name) override { (void)name; }

    // Host
    bool usbHostBegin() override { return false; }
    std::string usbHostTick() override { return std::string(); }
    void usbHostEnd() override {}

    // System control
    void systemControlBegin() override {}
    void systemControlEnd() override {}
    void systemSleep() override {}
    void systemWake() override {}
    void systemPowerOff(uint32_t holdMs = 10) override { (void)holdMs; }

    // Config
    void configure(const char* productStr, const char* manufacturerStr, const char* serialStr,
                   uint16_t vid, uint16_t pid, const char* webUSBString) override {
        (void)productStr; (void)manufacturerStr; (void)serialStr; (void)vid; (void)pid; (void)webUSBString;
    }
    void reset() override {}
    std::string getUsbSerialFromEfuseMac() override { return std::string(); }
};

#endif
