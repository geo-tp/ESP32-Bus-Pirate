#include "BluetoothService.h"
#include "esp_mac.h"


// Static var for the sniffer
std::vector<std::string> BluetoothService::bluetoothSniffLog;
portMUX_TYPE BluetoothService::bluetoothSniffMux = portMUX_INITIALIZER_UNLOCKED;
BLEScan* BluetoothService::bleScan = nullptr;
std::string BluetoothService::lastAdParsed = "";

// Callback connect/disconnect
class BluetoothServerCallbacks : public BLEServerCallbacks {
private:
    BluetoothService& service;

public:
    BluetoothServerCallbacks(BluetoothService& svc) : service(svc) {}

    void onConnect(BLEServer* pServer) override {
        service.onConnect();
    }

    void onDisconnect(BLEServer* pServer) override {
        service.onDisconnect();
        pServer->startAdvertising();
    }
};

void BluetoothService::startServer(const std::string& deviceName) {
    stopServer();

    delay(200);
    BLEDevice::init(String(deviceName.c_str()));
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BluetoothServerCallbacks(*this));

    hid = new BLEHIDDevice(server);
    mouseInput = hid->inputReport(1);
    keyboardInput = hid->inputReport(2);

    hid->manufacturer()->setValue("M5Stack");
    hid->pnp(0x02, 0x1234, 0x5678, 0x0100);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t*)HID_REPORT_MAP, 117);
    hid->startServices();

    BLEAdvertising* advertising = server->getAdvertising();
    advertising->addServiceUUID(hid->hidService()->getUUID());
    advertising->start();

    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);
    security->setCapability(ESP_IO_CAP_NONE);
    security->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    mode = BluetoothMode::SERVER;
    connected = true;
}

void BluetoothService::stopServer() {
    if (hid) {
        delete hid;
        hid = nullptr;
    }

    mouseInput = nullptr;
    keyboardInput = nullptr;

    if (BLEDevice::getInitialized()) {
        BLEDevice::deinit();
        delay(100);
    }


    connected = false;
    mode = BluetoothMode::NONE;
}

void BluetoothService::releaseBtClassic() {
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
}

void BluetoothService::onConnect() {
    connected = true;
}

void BluetoothService::onDisconnect() {
    connected = false;
}

bool BluetoothService::isConnected() const {
    return connected;
}

void BluetoothService::mouseMove(int16_t x, int16_t y) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !mouseInput) return;
    sendMouseReport(x, y, 0x0);
}

void BluetoothService::sendKeyboardReport(uint8_t modifier, const std::array<uint8_t, 6>& keys) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !keyboardInput) return;
    uint8_t report[8] = {modifier, 0, keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]};
    keyboardInput->setValue(report, sizeof(report));
    keyboardInput->notify();
}

void BluetoothService::sendKeyboardText(const std::string& text) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !keyboardInput) return;

    for (char c : text) {
        if (c < 0 || c > 127) continue;

        AsciiHid entry = asciiHid[(uint8_t)c];
        if (entry.keycode == 0) continue;

        uint8_t modifier = entry.requiresShift ? 0x02 : 0x00;  // Shift

        std::array<uint8_t, 6> keys = {0};
        keys[0] = entry.keycode;

        sendKeyboardReport(modifier, keys);
        delay(10);

        std::array<uint8_t, 6> emptyKeys = {0};
        sendKeyboardReport(0, emptyKeys);
        delay(10);
    }
}

std::string BluetoothService::getMacAddress() {
    uint8_t mac[6];
    if (esp_read_mac(mac, ESP_MAC_BT) != ESP_OK) {
        return "Unavailable";
    }

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return std::string(macStr);
}

void BluetoothService::sendEmptyReports() {
    if (mode != BluetoothMode::SERVER) return;
    if (mouseInput) {
        uint8_t emptyMouseReport[4] = {0, 0, 0, 0};
        mouseInput->setValue(emptyMouseReport, sizeof(emptyMouseReport));
        mouseInput->notify();
    }
    if (keyboardInput) {
        uint8_t emptyKeyboardReport[8] = {0};
        keyboardInput->setValue(emptyKeyboardReport, sizeof(emptyKeyboardReport));
        keyboardInput->notify();
    }
}

void BluetoothService::pairWithAddress(const std::string& addrStr) {
    BLEAddress addr(String(addrStr.c_str()));
    BLEDevice::whiteListAdd(addr);  // Optionnel ?
}

void BluetoothService::sendMouseReport(int16_t x, int16_t y, uint8_t buttons) {
    if (mode != BluetoothMode::SERVER) return;
    if (!connected || !mouseInput) return;
    uint8_t report[4] = {buttons, (uint8_t)x, (uint8_t)y, 0};
    mouseInput->setValue(report, sizeof(report));
    mouseInput->notify();
}

void BluetoothService::clickMouse() {
    sendMouseReport(0, 0, 0x01);  // Left click
    delay(50);
    sendMouseReport(0, 0, 0x00);  // Release
}

void BluetoothService::switchToMode(BluetoothMode newMode) {
    if (mode == newMode) return;

    if (mode == BluetoothMode::SERVER || mode == BluetoothMode::CLIENT) {
        stopServer();
    }

    // Init new mode
    if (newMode == BluetoothMode::CLIENT || newMode == BluetoothMode::SERVER) {
        BLEDevice::init(""); 
    }

    mode = newMode;
}

std::vector<std::string> BluetoothService::scanDevices(int seconds) {
    switchToMode(BluetoothMode::CLIENT);
    stopPassiveBluetoothSniffing();

    auto scan = BLEDevice::getScan();
    scan->setActiveScan(true);
    auto results = scan->start(seconds);

    std::vector<std::string> formattedDevices;

    for (int i = 0; i < results->getCount(); ++i) {
        BLEAdvertisedDevice device = results->getDevice(i);

        String name = device.getName().isEmpty() ? "(unknown)" : device.getName();
        String addr = device.getAddress().toString();
        int rssi = device.getRSSI();
        std::string type = isLikelyConnectable(device) ? "Connectable" : "Non-Connectable";
        std::string entry = std::string(addr.c_str()) + " | " + std::string(name.c_str()) + " | RSSI: " + std::to_string(rssi) + " | Type: " + type;

        formattedDevices.push_back(entry);
    }

    scan->clearResults();
    return formattedDevices;
}

std::vector<std::string> BluetoothService::connectTo(const std::string& addr) {
    std::vector<std::string> serviceUUIDs;

    if (mode != BluetoothMode::CLIENT) {
        BLEDevice::init("BLE-Client");
        mode = BluetoothMode::CLIENT;
    }

    BLEAddress address(String(addr.c_str()));
    BLEClient* client = BLEDevice::createClient();

    if (!client->connect(address)) {
        return serviceUUIDs;
    }

    auto* services = client->getServices();
    if (services) {
        for (const auto& pair : *services) {
            serviceUUIDs.push_back(pair.first);
        }
    }

    client->disconnect();
    return serviceUUIDs;
}

void BluetoothService::init(const std::string& deviceName) {
    if (mode == BluetoothMode::CLIENT && BLEDevice::getInitialized()) {
        // Already init
        return;
    }

    BLEDevice::init(String(deviceName.c_str()));
    mode = BluetoothMode::CLIENT;
}

BluetoothMode BluetoothService::getMode() {
    return mode;
}

bool BluetoothService::spoofMacAddress(const std::string& macStr) {
    if (BLEDevice::getInitialized()) {
        return false;
    }

    esp_bd_addr_t addr;
    int values[6];

    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
               &values[0], &values[1], &values[2],
               &values[3], &values[4], &values[5]) != 6) {
        return false; // invalide
    }

    for (int i = 0; i < 6; i++) {
        if (values[i] < 0 || values[i] > 0xFF) {
            return false; // out of range
        }
        addr[i] = static_cast<uint8_t>(values[i] & 0xFF); // secure
    }

    // for some reason the last byte is always inc by 1 when set
    if (addr[5] != 0x00) {
        addr[5] -= 1; 
    }

    esp_err_t err = esp_base_mac_addr_set(addr);
    return err == ESP_OK;
}

void BluetoothService::clearBondedDevices() {
    // === NEW ===

    ble_store_config_init();
    ble_store_clear();
    
    // === OLD ===

    // int dev_num = esp_ble_get_bond_device_num();
    // if (dev_num == 0) return;

    // esp_ble_bond_dev_t* bonded = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    // if (!bonded) return;

    // if (esp_ble_get_bond_device_list(&dev_num, bonded) == ESP_OK) {
    //     for (int i = 0; i < dev_num; ++i) {
    //         esp_ble_remove_bond_device(bonded[i].bd_addr);
    //     }
    // }

    // free(bonded);
}

void BluetoothService::PassiveAdvertisedDeviceCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
    // Basic infos
    String name = advertisedDevice.getName().isEmpty() ? "(unknown)" : advertisedDevice.getName();
    String addr = advertisedDevice.getAddress().toString();
    int rssi = advertisedDevice.getRSSI();

    // Check type
    std::string type;
    if (isLikelyConnectable(advertisedDevice)) {
        type = "Connectable    ";
    } else {
        type = "Non-Connectable";
    }

    // Build the log entry
    std::string logEntry = "[BLE] " + std::string(addr.c_str()) + " | " + std::string(name.c_str()) + " | RSSI: " + std::to_string(rssi) + " | Type: " + type;

    // Parse AD infos
    std::string adParsed = parseAdTypes(advertisedDevice.getPayload(), advertisedDevice.getPayloadLength());

    // No AD duplicate
    if (adParsed == lastAdParsed) return;
    lastAdParsed = adParsed;

    // Ad AD infos to log
    if (!adParsed.empty()) {
        logEntry += " | " + adParsed;
    }

    // Add log to shared vector, erase the first element if max size is reach
    portENTER_CRITICAL(&BluetoothService::bluetoothSniffMux);
    if (bluetoothSniffLog.size() > 200) bluetoothSniffLog.erase(bluetoothSniffLog.begin());
    BluetoothService::bluetoothSniffLog.push_back(logEntry);
    portEXIT_CRITICAL(&BluetoothService::bluetoothSniffMux);
}

void BluetoothService::startPassiveBluetoothSniffing() {
    if (!BLEDevice::getInitialized()) {
        BLEDevice::init("Sniffer");
    }

    bleScan = BLEDevice::getScan();
    bleScan->setAdvertisedDeviceCallbacks(new PassiveAdvertisedDeviceCallbacks(), true);
    bleScan->setActiveScan(false);
    bleScan->start(0, nullptr);
}

void BluetoothService::stopPassiveBluetoothSniffing() {
    if (bleScan) {
        bleScan->stop();
        bleScan->setAdvertisedDeviceCallbacks(nullptr);
        bleScan->clearResults();
        bleScan = nullptr;
    }
    bluetoothSniffLog.clear();
}

std::vector<std::string> BluetoothService::getBluetoothSniffLog() {
    std::vector<std::string> copy;
    portENTER_CRITICAL(&bluetoothSniffMux);
    copy.swap(bluetoothSniffLog);
    portEXIT_CRITICAL(&bluetoothSniffMux);
    return copy;
}

bool BluetoothService::isLikelyConnectable(BLEAdvertisedDevice& device) {
    uint8_t* payload = device.getPayload();
    size_t len = device.getPayloadLength();

    for (size_t i = 0; i + 1 < len;) {
        uint8_t field_len = payload[i];
        if (field_len == 0 || i + field_len + 1 > len) break;

        uint8_t ad_type = payload[i + 1];

        if (ad_type == 0x01 && field_len >= 2) { // Flags
            uint8_t flags = payload[i + 2];
            // Bit 0x02 General Discoverable Mode
            // Bit 0x04 BR/EDR Not Supported (Only BLE)
            return (flags & 0x02) != 0;
        }

        i += field_len + 1;
    }

    return false;
}

std::string BluetoothService::parseAdTypes(const uint8_t* payload, size_t len) {
    std::string result;
    size_t i = 0;

    while (i + 1 < len) {
        uint8_t field_len = payload[i];
        if (field_len == 0 || i + field_len + 1 > len) break;

        uint8_t ad_type = payload[i + 1];
        const uint8_t* data = payload + i + 2;

        std::string entry;

        switch (ad_type) {
            case 0x01: { // Flags
                entry = "AD 0x01: Flags: ";
                uint8_t flags = data[0];
                if (flags & 0x01) entry += "LE Limited, ";
                if (flags & 0x02) entry += "LE General, ";
                if (flags & 0x04) entry += "BR/EDR Not Supported, ";
                if (flags & 0x08) entry += "LE+BR/EDR (Controller), ";
                if (flags & 0x10) entry += "LE+BR/EDR (Host), ";
                if (!entry.empty() && entry.back() == ' ') entry.erase(entry.size() - 2); // remove ", "
                break;
            }
            case 0x02:
            case 0x03: { // 16bit UUIDs
                entry = "AD 0x03: UUID16: ";
                for (int j = 0; j + 1 < field_len - 1; j += 2) {
                    uint16_t uuid = data[j] | (data[j + 1] << 8);
                    char uuidStr[8];
                    snprintf(uuidStr, sizeof(uuidStr), "0x%04X ", uuid);
                    entry += uuidStr;
                }
                break;
            }
            case 0x06:
            case 0x07: { // 128bit UUIDs
                entry = "AD 0x07: UUID128: ";
                for (int j = 0; j + 15 < field_len - 1; j += 16) {
                    char uuidStr[40];
                    snprintf(uuidStr, sizeof(uuidStr),
                        "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X ",
                        data[j+15], data[j+14], data[j+13], data[j+12],
                        data[j+11], data[j+10],
                        data[j+9], data[j+8],
                        data[j+7], data[j+6],
                        data[j+5], data[j+4], data[j+3], data[j+2], data[j+1], data[j+0]);
                    entry += uuidStr;
                }
                break;
            }
            case 0x08: // Shortened Local Name
            case 0x09: { // Complete Local Name
                entry = "AD 0x09: Name: ";
                entry.append(reinterpret_cast<const char*>(data), field_len - 1);
                break;
            }
            case 0x0A: { // Tx Power Level
                entry = "AD 0x0A: Tx Power: ";
                char val[6];
                snprintf(val, sizeof(val), "%d dBm", (int8_t)data[0]);
                entry += val;
                break;
            }
            case 0x16: {
                entry = "AD 0x16: ServiceData16: ";
                // UUID16 + raw data
                if (field_len >= 3) {
                    uint16_t uuid = data[0] | (data[1] << 8);
                    char uuidStr[16];
                    snprintf(uuidStr, sizeof(uuidStr), "UUID 0x%04X, Data: ", uuid);
                    entry += uuidStr;
                    for (int j = 2; j < field_len - 1; ++j) {
                        char byteStr[4];
                        snprintf(byteStr, sizeof(byteStr), "%02X ", data[j]);
                        entry += byteStr;
                    }
                }
                break;
            }
            case 0xFF: { // Manufacturer Specific
                entry = "AD 0xFF: Raw ";
                for (int j = 0; j < field_len - 1; ++j) {
                    char byteStr[4];
                    snprintf(byteStr, sizeof(byteStr), "%02X ", data[j]);
                    entry += byteStr;
                }
                break;
            }
            default: {
                char typeLabel[24];
                snprintf(typeLabel, sizeof(typeLabel), "AD 0x%02X: Raw ", ad_type);
                entry = typeLabel;
                for (int j = 0; j < field_len - 1; ++j) {
                    char byteStr[4];
                    snprintf(byteStr, sizeof(byteStr), "%02X ", data[j]);
                    entry += byteStr;
                }
                break;
            }
        }

        // Trim space if needed
        if (!entry.empty() && entry.back() == ' ') entry.pop_back();

        // Add entry with separator
        result += entry + " | ";

        i += field_len + 1;
    }

    // Remove trailing " | " if present
    if (result.size() >= 3 && result.substr(result.size() - 3) == " | ")
        result.erase(result.size() - 3);

    return result;
}

const uint8_t BluetoothService::HID_REPORT_MAP[] = {
    // Mouse report
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x85, 0x01,        //     Report ID (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var,Abs)
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Cnst,Var,Abs)
    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data,Var,Rel)
    0xC0,              //   End Collection
    0xC0,              // End Collection

    // Keyboard report
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0xE0,        //   Usage Minimum (224)
    0x29, 0xE7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Cnst,Var,Abs)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (1)
    0x29, 0x05,        //   Usage Maximum (5)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Cnst,Var,Abs)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Key Codes)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data,Array)
    0xC0               // End Collection
};