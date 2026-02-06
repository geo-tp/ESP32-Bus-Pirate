#pragma once

#include <Arduino.h>
#include <USB.h>
#include <USBMSC.h>
#include <SPI.h>
#include <SD.h>
#include <string>
#include <USBHIDMouse.h>
#include <USBHIDKeyboard.h>
#include <USBHIDGamepad.h>
#include "Interfaces/IUsbService.h"

// ###############################################################################
// ⚠️  USB CDC (Serial over USB) can't be used at the same time as other USB modes
//     (like Mass Storage, HID, MIDI, etc).
//     ➤ If you enable any other USB mode, USB CDC will automatically stop.
// ###############################################################################

class UsbS3Service: public IUsbService {
public:
    UsbS3Service();

    // Begin USB HID keyboard mode
    void keyboardBegin();
    void keyboardSendString(const std::string& text);
    void keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks);

    // Begin USB Mass Storage mode
    void storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi);

    // Mouse actions
    void mouseBegin();
    void mouseMove(int x, int y);
    void mouseClick(int button);
    void mouseRelease(int button);

    // Gamepad Actions
    void gamepadBegin();
    void gamepadPress(const std::string& name);
    
    // Status
    bool isKeyboardActive() const;
    bool isStorageActive() const;
    bool isMouseActive() const;
    bool isGamepadActive() const;

    void reset();

private:
    // HID
    USBHIDKeyboard keyboard;
    USBHIDMouse mouse;
    USBHIDGamepad gamepad;
    bool gamepadActive = false;
    bool keyboardActive = false;
    bool mouseActive = false;
    unsigned long hidInitTime;
    

    // Mass Storage
    USBMSC msc;
    SPIClass sdSPI;
    bool storageActive;

    bool initialized;

    // USB MSC callbacks
    static int32_t storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    static int32_t storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    static bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject);

    void setupStorageEvent();
};