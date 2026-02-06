#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "BLEHIDDevice.h"
#include "HIDTypes.h"
#include "Data/AsciiHid.h"

enum class BluetoothMode {
    NONE,
    SERVER,
    CLIENT
};

struct ScannedDevice {
    std::string name;
    std::string address;
    int rssi;
    std::string type;
    std::string adSummary;
};

class BluetoothService {
private:
    BLEHIDDevice* hid = nullptr;
    BLECharacteristic* mouseInput = nullptr;
    BLECharacteristic* keyboardInput = nullptr;
    bool connected = false;
    static const uint8_t HID_REPORT_MAP[];
    BluetoothMode mode = BluetoothMode::NONE;
    static BLEScan* bleScan;
    static std::string lastAdParsed;
    inline static uint32_t receivedFramesCount = 0;

public:
    class PassiveAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    public:
        void onResult(BLEAdvertisedDevice advertisedDevice) override;
    };

    // begin / end server BT
    void startServer(const std::string& deviceName = "Bus-Pirate-Blueooth");
    void stopServer();
    void releaseBtClassic();

    // Init client
    void init(const std::string& deviceName = "Bus-Pirate-Bluetooth");
    
    // Pair as client
    void pairWithAddress(const std::string& addrStr);      // pair <addr>

    // Connexion
    void onConnect();
    void onDisconnect();
    bool isConnected() const;

    // HID – Kb
    void sendKeyboardText(const std::string& text);
    void sendKeyboardReport(uint8_t modifier, const std::array<uint8_t, 6>& keys);

    // HID – Mouse
    void mouseMove(int16_t x, int16_t y);
    void clickMouse();  // Simule un clic 
    void sendMouseReport(int16_t x, int16_t y, uint8_t buttons);

    // Utils
    void sendEmptyReports();
    bool spoofMacAddress(const std::string& macStr);
    std::string getMacAddress();
    BluetoothMode getMode();
    void switchToMode(BluetoothMode newMode);
    
    // Scan
    std::vector<std::string> scanDevices(int seconds = 10);
    std::vector<std::string> connectTo(const std::string& addr);
    
    // Bluetooth sniffing
    class PassiveBLEAdvertisedDeviceCallbacks;
    static void startPassiveBluetoothSniffing();
    static void stopPassiveBluetoothSniffing();
    static std::vector<std::string> getBluetoothSniffLog();
    static bool isLikelyConnectable(BLEAdvertisedDevice& device);
    static std::string parseAdTypes(const uint8_t* payload, size_t len);
    static std::vector<std::string> bluetoothSniffLog;
    static portMUX_TYPE bluetoothSniffMux;
};

