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
#include "usb/usb_host.h"
#include "usb/usb_types_ch9.h"
#include "usb/usb_types_stack.h"
#include "usb/usb_helpers.h"
#include "esp_log.h"
#include <sstream>


// ###############################################################################
// ⚠️  USB CDC (Serial over USB) can't be used at the same time as other USB modes
//     (like Mass Storage, HID, MIDI, etc).
//     ➤ If you enable any other USB mode, USB CDC will automatically stop.
// ###############################################################################

class UsbS3Service {
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
    bool isHostActive() const;

    // Host 
    bool usbHostBegin();
    std::string usbHostTick();
    void usbHostEnd();
    static void hostClientEventCb(const usb_host_client_event_msg_t *event_msg, void *arg);

    void reset();

private:
    bool initialized;

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
    static int32_t storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
    static int32_t storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
    static bool usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject);
    void setupStorageEvent();

    // Host
    bool hostInstalled = false;
    usb_host_client_handle_t hostClient = nullptr;
    uint8_t devAddr = 0;
    bool attachPending = false;
    bool detachPending = false;
    bool dumpedThisAttach = false;
    inline static const char* TAG_USBHOST = "UsbHost";

};