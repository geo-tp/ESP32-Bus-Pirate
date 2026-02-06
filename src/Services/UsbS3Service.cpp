#include "UsbS3Service.h"

UsbS3Service::UsbS3Service()
  : keyboardActive(false), storageActive(false), initialized(false) {}

void UsbS3Service::keyboardBegin() {
    if (keyboardActive) return;

    USB.begin();
    keyboard.begin();
    hidInitTime = millis();
    keyboardActive = true;
}

void UsbS3Service::storageBegin(uint8_t cs, uint8_t clk, uint8_t miso, uint8_t mosi) {
    if (initialized) return;

    sdSPI.begin(clk, miso, mosi, cs);
    if (!SD.begin(cs, sdSPI)) {
        storageActive = false;
        return;
    }

    // Setup MSC
    uint32_t secSize = SD.sectorSize();
    uint32_t numSectors = SD.numSectors();

    msc.vendorID("ESP32");
    msc.productID("USB_MSC");
    msc.productRevision("1.0");
    msc.onRead(storageReadCallback);
    msc.onWrite(storageWriteCallback);
    msc.onStartStop(usbStartStopCallback);
    msc.mediaPresent(true);
    msc.begin(numSectors, secSize);

    // Setup USB events
    setupStorageEvent();

    USB.begin();
    storageActive = true;
    initialized = true;
}

void UsbS3Service::keyboardSendString(const std::string& text) {
    if (!keyboardActive) return;

    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    keyboard.releaseAll();
    for (char c : text) {
        keyboard.write(c);
    }
}

void UsbS3Service::keyboardSendChunkedString(const std::string& data, size_t chunkSize, unsigned long delayBetweenChunks) {
    if (!keyboardActive) return;

    size_t totalLength = data.length();
    size_t sentLength = 0;

    while (sentLength < totalLength) {
        size_t currentChunkSize = std::min(chunkSize, totalLength - sentLength);
        std::string chunk = data.substr(sentLength, currentChunkSize);
        keyboardSendString(chunk);
        sentLength += currentChunkSize;
        delay(delayBetweenChunks);
    }
}

void UsbS3Service::mouseBegin() {
    if (mouseActive) return;

    USB.begin();
    mouse.begin();
    mouseActive = true;
    hidInitTime = millis();
}

void UsbS3Service::mouseMove(int x, int y) {
    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    mouse.move(x, y);
}

void UsbS3Service::mouseClick(int button) {
    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    mouse.press(button);
    delay(50);
    mouse.release(button);
}

void UsbS3Service::mouseRelease(int button) {
    if (!mouseActive) return;
    mouse.release(button);
}

void UsbS3Service::gamepadBegin() {
    if (gamepadActive) return;

    gamepad.begin();
    USB.begin();
    gamepadActive = true;
    hidInitTime = millis();
}

void UsbS3Service::gamepadPress(const std::string& name) {
    if (!gamepadActive) return;

    // Wait HID init
    while (millis() - hidInitTime < 1500) {
        delay(10);
    }

    // Reset before any new input
    gamepad.hat(HAT_CENTER);
    for (int i = 0; i < 32; ++i) {
        gamepad.releaseButton(i);
    }

    if (name == "up") {
        gamepad.hat(HAT_UP);
    } else if (name == "down") {
        gamepad.hat(HAT_DOWN);
    } else if (name == "left") {
        gamepad.hat(HAT_LEFT);
    } else if (name == "right") {
        gamepad.hat(HAT_RIGHT);
    } else if (name == "a") {
        gamepad.pressButton(BUTTON_A);
    } else if (name == "b") {
        gamepad.pressButton(BUTTON_B);
    } else {
        return; // unknow
    }

    delay(500);  // simulate short press

    // Reset state
    gamepad.hat(HAT_CENTER);
    gamepad.releaseButton(BUTTON_A);
    gamepad.releaseButton(BUTTON_B);
}


bool UsbS3Service::isKeyboardActive() const {
    return keyboardActive;
}

bool UsbS3Service::isStorageActive() const {
    return storageActive;
}

bool UsbS3Service::isMouseActive() const {
    return mouseActive;
}

bool UsbS3Service::isGamepadActive() const {
    return gamepadActive;
}

int32_t UsbS3Service::storageReadCallback(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;

    uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
    for (uint32_t i = 0; i < bufsize / secSize; ++i) {
        if (!SD.readRAW(buf + i * secSize, lba + i)) {
            return -1;
        }
    }
    return bufsize;
}

int32_t UsbS3Service::storageWriteCallback(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {
    const uint32_t secSize = SD.sectorSize();
    if (secSize == 0) return -1;

    uint64_t freeSpace = SD.totalBytes() - SD.usedBytes();
    if (bufsize > freeSpace) {
        return -1;
    }

    for (uint32_t i = 0; i < bufsize / secSize; ++i) {
        uint8_t blk[secSize];
        memcpy(blk, buffer + i * secSize, secSize);
        if (!SD.writeRAW(blk, lba + i)) {
            return -1;
        }
    }
    return bufsize;
}

bool UsbS3Service::usbStartStopCallback(uint8_t power_condition, bool start, bool load_eject) {
    return true;
}

void UsbS3Service::setupStorageEvent() {
    USB.onEvent([](void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
        if (event_base == ARDUINO_USB_EVENTS) {
            auto* data = reinterpret_cast<arduino_usb_event_data_t*>(event_data);
            switch (event_id) {
                case ARDUINO_USB_STARTED_EVENT:
                    // Started
                    break;
                case ARDUINO_USB_STOPPED_EVENT:
                    // Stopped
                    break;
                case ARDUINO_USB_SUSPEND_EVENT:
                    // Display a message or icon
                    break;
                case ARDUINO_USB_RESUME_EVENT:
                    // Display a message or icon
                    break;
                default:
                    break;
            }
        }
    });
}

void UsbS3Service::reset() {
    if (initialized) {
        keyboard.releaseAll(); // si clavier actif
        msc.end();             // si stockage actif
        keyboard.end();
        gamepad.end();
        mouse.end();
        initialized = false;
        keyboardActive = false;
        storageActive = false;
        mouseActive = false;
        gamepadActive = false;
    }
}