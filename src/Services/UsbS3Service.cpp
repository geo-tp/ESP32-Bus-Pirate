#include "UsbS3Service.h"
#include <sstream>  

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

bool UsbS3Service::isHostActive() const {
    return hostInstalled;
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

void UsbS3Service::hostClientEventCb(const usb_host_client_event_msg_t *event_msg, void *arg) {
    auto* self = static_cast<UsbS3Service*>(arg);
    if (!self || !event_msg) return;

    switch (event_msg->event) {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            self->devAddr = event_msg->new_dev.address;
            self->attachPending = true;
            self->detachPending = false;
            self->dumpedThisAttach = false;
            break;

        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            // device gone
            self->detachPending = true;
            self->attachPending = false;
            self->dumpedThisAttach = false;
            self->devAddr = 0;
            break;

        default:
            break;
    }
}

bool UsbS3Service::usbHostBegin() {
    if (hostInstalled) return true;

    usb_host_config_t host_cfg = {};
    host_cfg.skip_phy_setup = false;
    host_cfg.root_port_unpowered = false; 
    host_cfg.intr_flags = 0;
    host_cfg.enum_filter_cb = nullptr;
    host_cfg.fifo_settings_custom = {0, 0, 0}; 
    host_cfg.peripheral_map = 0;

    esp_err_t err = usb_host_install(&host_cfg);
    if (err != ESP_OK) {
        Serial.printf("usb_host_install failed: %d", (int)err);
        return false;
    }

    usb_host_client_config_t client_cfg = {};
    client_cfg.is_synchronous = false; 
    client_cfg.max_num_event_msg = 8;
    client_cfg.async.client_event_callback = &UsbS3Service::hostClientEventCb;
    client_cfg.async.callback_arg = this;

    err = usb_host_client_register(&client_cfg, &hostClient);
    if (err != ESP_OK) {
        Serial.printf("usb_host_client_register failed: %d", (int)err);
        usb_host_uninstall();
        return false;
    }

    // Reset state
    devAddr = 0;
    attachPending = false;
    detachPending = false;
    dumpedThisAttach = false;

    hostInstalled = true;
    return true;
}

void UsbS3Service::usbHostEnd() {
    if (!hostInstalled) return;

    if (hostClient) {
        usb_host_client_unblock(hostClient);
    }

    // free devices
    esp_err_t err = usb_host_device_free_all();
    if (err != ESP_OK && err != ESP_ERR_NOT_FINISHED) {
        ESP_LOGW(TAG_USBHOST, "usb_host_device_free_all err=%d", (int)err);
    }

    // wait for all events to be processed
    uint32_t flags = 0;
    for (int i = 0; i < 50; ++i) {
        flags = 0;
        esp_err_t e2 = usb_host_lib_handle_events(pdMS_TO_TICKS(20), &flags);
        (void)e2;
        if (flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) break;
    }

    // unregister client and uninstall host
    if (hostClient) {
        usb_host_client_deregister(hostClient);
        hostClient = nullptr;
    }

    usb_host_uninstall();

    hostInstalled = false;
    devAddr = 0;
    attachPending = false;
    detachPending = false;
    dumpedThisAttach = false;
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
        hostInstalled = false;
    }
}


// TODO: Rework this func to something more manageable
// Extract the parsing from this function to another obj
std::string UsbS3Service::usbHostTick() {
    if (!hostInstalled || !hostClient) {
        return "[USB][ERR] Host not started";
    }

    // Run host library events 
    uint32_t lib_flags = 0;
    esp_err_t err = usb_host_lib_handle_events(0, &lib_flags);
    if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
        return "[USB][ERR] usb_host_lib_handle_events err=" + std::to_string((int)err);
    }

    // Run client events
    err = usb_host_client_handle_events(hostClient, 0);
    if (err != ESP_OK && err != ESP_ERR_TIMEOUT) {
        return "[USB][ERR] usb_host_client_handle_events err=" + std::to_string((int)err);
    }

    //  Messages one-shot
    if (detachPending) {
        detachPending = false;
        return "[USB] Device gone.";
    }

    if (attachPending) {
        attachPending = false;
        if (devAddr) {
            return "[USB] Device attached (addr=" + std::to_string(devAddr) + ").";
        }
    }

    // Dump once per attach
    if (devAddr != 0 && !dumpedThisAttach) {
        usb_device_handle_t devHandle = nullptr;

        err = usb_host_device_open(hostClient, devAddr, &devHandle);
        if (err != ESP_OK) {
            return "[USB][ERR] usb_host_device_open addr=" + std::to_string(devAddr) + " err=" + std::to_string((int)err);
        }

        // Device info (often gives extra useful stuff like speed / EP0 size)
        usb_device_info_t dinfo = {};
        esp_err_t infoErr = usb_host_device_info(devHandle, &dinfo);

        const usb_device_desc_t* dev_desc = nullptr;
        err = usb_host_get_device_descriptor(devHandle, &dev_desc);
        if (err != ESP_OK || !dev_desc) {
            usb_host_device_close(hostClient, devHandle);
            return "[USB][ERR] usb_host_get_device_descriptor err=" + std::to_string((int)err);
        }

        const usb_config_desc_t* cfg_desc = nullptr;
        err = usb_host_get_active_config_descriptor(devHandle, &cfg_desc);
        if (err != ESP_OK || !cfg_desc) {
            usb_host_device_close(hostClient, devHandle);
            return "[USB][ERR] usb_host_get_active_config_descriptor err=" + std::to_string((int)err);
        }

        auto hex8 = [](uint8_t v) {
            const char* h = "0123456789abcdef";
            std::string s = "0x00";
            s[2] = h[(v >> 4) & 0x0F];
            s[3] = h[v & 0x0F];
            return s;
        };
        auto hex16 = [](uint16_t v) {
            const char* h = "0123456789abcdef";
            std::string s = "0x0000";
            s[2] = h[(v >> 12) & 0x0F];
            s[3] = h[(v >> 8) & 0x0F];
            s[4] = h[(v >> 4) & 0x0F];
            s[5] = h[v & 0x0F];
            return s;
        };
        auto epTypeStr = [](uint8_t bmAttributes) {
            switch (bmAttributes & 0x03) {
                case 0x00: return "CTRL";
                case 0x01: return "ISOC";
                case 0x02: return "BULK";
                case 0x03: return "INTR";
                default:   return "?";
            }
        };

        std::ostringstream ss;
        ss << "\n===== USB Device (addr " << (int)devAddr << ") =====\n";

        // Device descriptor
        ss << "VID:PID " << hex16(dev_desc->idVendor) << ":" << hex16(dev_desc->idProduct) << "\n";
        ss << "bcdUSB " << ((dev_desc->bcdUSB >> 8) & 0xFF) << "." << (dev_desc->bcdUSB & 0xFF) << "\n";
        ss << "bMaxPacketSize0 " << (int)dev_desc->bMaxPacketSize0 << "\n";
        ss << "Device Class/Sub/Proto "
        << (int)dev_desc->bDeviceClass << "/"
        << (int)dev_desc->bDeviceSubClass << "/"
        << (int)dev_desc->bDeviceProtocol << "\n";
        ss << "bcdDevice " << hex16(dev_desc->bcdDevice) << "\n";
        ss << "iManufacturer=" << (int)dev_desc->iManufacturer
        << " iProduct=" << (int)dev_desc->iProduct
        << " iSerial=" << (int)dev_desc->iSerialNumber << "\n";
        ss << "NumConfigurations " << (int)dev_desc->bNumConfigurations << "\n";

        // Device info (if available)
        ss << "\n[DeviceInfo]\n";
        if (infoErr == ESP_OK) {
            ss << "address=" << (int)dinfo.dev_addr << "\n";
            ss << "speed=" << (int)dinfo.speed << " (enum)\n";
            ss << "mps0=" << (int)dinfo.bMaxPacketSize0 << "\n";
            ss << "config_value=" << (int)dinfo.bConfigurationValue << "\n";
        } else {
            ss << "usb_host_device_info() err=" << (int)infoErr << "\n";
        }

        // Config header
        ss << "\n[Configuration]\n";
        ss << "bConfigurationValue=" << (int)cfg_desc->bConfigurationValue
        << " bNumInterfaces=" << (int)cfg_desc->bNumInterfaces
        << " wTotalLength=" << (int)cfg_desc->wTotalLength << "\n";
        ss << "bmAttributes=" << hex8(cfg_desc->bmAttributes)
        << " bMaxPower=" << (int)cfg_desc->bMaxPower << " (2mA units)\n";

        // Parse descriptors inside config
        ss << "\n[Descriptors]\n";

        const uint8_t* p = (const uint8_t*)cfg_desc;
        const uint8_t* end = p + cfg_desc->wTotalLength;

        bool hasHid = false;
        bool hasInIntr = false;
        bool hasOutIntr = false;
        uint8_t currentIfClass = 0xFF;
        uint8_t ifSub;
        uint8_t ifProto;
        int ifCount = 0;
        int epCount = 0;

        while (p + 2 <= end) {
            uint8_t len = p[0];
            uint8_t type = p[1];
            if (len < 2) break;
            if (p + len > end) break;

            if (type == USB_B_DESCRIPTOR_TYPE_INTERFACE) {
                const usb_intf_desc_t* ifd = (const usb_intf_desc_t*)p;
                ifCount++;
                ifSub   = ifd->bInterfaceSubClass;
                ifProto = ifd->bInterfaceProtocol;
                currentIfClass = ifd->bInterfaceClass;
                if (ifd->bInterfaceClass == 3) hasHid = true;

                ss << "IF " << (int)ifd->bInterfaceNumber
                << " alt " << (int)ifd->bAlternateSetting
                << " class/sub/proto "
                << (int)ifd->bInterfaceClass << "/"
                << (int)ifd->bInterfaceSubClass << "/"
                << (int)ifd->bInterfaceProtocol
                << " eps=" << (int)ifd->bNumEndpoints
                << " iInterface=" << (int)ifd->iInterface
                << "\n";
            }
            else if (type == USB_B_DESCRIPTOR_TYPE_ENDPOINT) {
                const usb_ep_desc_t* ep = (const usb_ep_desc_t*)p;
                epCount++;

                bool in = (ep->bEndpointAddress & 0x80) != 0;
                uint8_t epNum = ep->bEndpointAddress & 0x0F;
                bool intr = ((ep->bmAttributes & 0x03) == 0x03);

                if (currentIfClass == 3 && intr && in) hasInIntr = true;
                if (currentIfClass == 3 && intr && !in) hasOutIntr = true;

                ss << "  EP " << (in ? "IN " : "OUT") << (int)epNum
                << " addr=" << hex8(ep->bEndpointAddress)
                << " type=" << epTypeStr(ep->bmAttributes)
                << " bmAttr=" << hex8(ep->bmAttributes)
                << " maxPacket=" << (int)ep->wMaxPacketSize
                << " interval=" << (int)ep->bInterval
                << "\n";
            }
            else if (type == 0x0B /* IAD */) {
                // Interface Association Descriptor (composite devices)
                ss << "IAD len=" << (int)len;
                if (len >= 8) {
                    uint8_t firstIf = p[2];
                    uint8_t ifCount = p[3];
                    uint8_t cls = p[4], sub = p[5], proto = p[6];
                    uint8_t iFunction = p[7];
                    ss << " firstIf=" << (int)firstIf
                    << " count=" << (int)ifCount
                    << " class/sub/proto " << (int)cls << "/" << (int)sub << "/" << (int)proto
                    << " iFunction=" << (int)iFunction;
                }
                ss << "\n";
            }
            else if (type == 0x21 /* HID */) {
                // HID descriptor: bcdHID,country,numDesc, (type,len)...
                ss << "  HID";
                if (len >= 9) {
                    uint16_t bcdHID = p[2] | (p[3] << 8);
                    uint8_t country = p[4];
                    uint8_t numDesc = p[5];
                    uint8_t descType = p[6];
                    uint16_t descLen = p[7] | (p[8] << 8);
                    ss << " bcdHID=" << hex16(bcdHID)
                    << " country=" << (int)country
                    << " numDesc=" << (int)numDesc
                    << " firstDescType=" << hex8(descType)
                    << " firstDescLen=" << (int)descLen;
                } else {
                    ss << " len=" << (int)len;
                }
                ss << "\n";
            }
            else if (type == 0x24 /* CS_INTERFACE */) {
                ss << "  CS_INTERFACE len=" << (int)len << "\n";
            }
            else if (type == 0x25 /* CS_ENDPOINT */) {
                ss << "  CS_ENDPOINT len=" << (int)len << "\n";
            }
            else {
                // For completeness, show unknown types briefly
                ss << "DESC type=" << hex8(type) << " len=" << (int)len << "\n";
            }

            p += len;
        }

        ss << "\n[Quick analysis]\n";
        ss << "- Interfaces_seen = " << ifCount << " Endpoints_seen = " << epCount << "\n";
        if (currentIfClass == 8) {
            ss << "- USB Mass Storage (usb drive likely)\n";
        }
        else if (currentIfClass == 1) {
            ss << "- USB Audio interface (headset/mic/speaker likely)\n";
        }
        else if (currentIfClass == 2 || currentIfClass == 10) {
            ss << "- USB CDC interface (serial device)\n";
        }
        else if (currentIfClass == 14) {
            ss << "- USB Video interface (webcam/capture device)\n";
        }
        
        if (hasHid && hasInIntr) {
            ss << "- HID interface, likely input device (gamepad/keyboard/mouse)\n";
            ss << "- OUT interrupt endpoint: " << (hasOutIntr ? "yes (rumble/LEDs possible)" : "no (maybe via control transfers)") << "\n";
        } else if (hasHid) {
            ss << "- HID interface but no IN interrupt endpoint found (unusual)\n";
        }

        ss << "===================================\n";

        usb_host_device_close(hostClient, devHandle);

        dumpedThisAttach = true;
        return ss.str();
    }

    return "";
}

