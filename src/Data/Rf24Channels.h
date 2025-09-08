#pragma once
#include <Arduino.h>

// Group struct
struct Rf24ChannelGroup {
    const char*        name;
    const uint8_t*     channels;
    size_t             count;
};

// Wi-Fi
static const uint8_t WIFI_CHANNELS[] = {
    6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,         // Channel 1
    22, 24, 26, 28,                                             // mid band
    30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, // Channel 6
    46, 48, 50, 52,                                             // mid band
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68      // Channel 11
};
static const size_t WIFI_CHANNELS_COUNT = sizeof(WIFI_CHANNELS) / sizeof(WIFI_CHANNELS[0]);

// BLE
static const uint8_t BLE_CHANNELS[] = {1, 2, 3, 25, 26, 27, 79, 80, 81};
static const size_t BLE_CHANNELS_COUNT = sizeof(BLE_CHANNELS) / sizeof(BLE_CHANNELS[0]);

// Bluetooth 
static const uint8_t BLUETOOTH_CHANNELS[] = {
    2,  4,  6,  8,  10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40,
    42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80
};
static const size_t BLUETOOTH_CHANNELS_COUNT = sizeof(BLUETOOTH_CHANNELS) / sizeof(BLUETOOTH_CHANNELS[0]);

// USB wireless
static const uint8_t USB_CHANNELS[] = {40, 50, 60};
static const size_t USB_CHANNELS_COUNT = sizeof(USB_CHANNELS) / sizeof(USB_CHANNELS[0]);

// Video streaming
static const uint8_t VIDEO_CHANNELS[] = {70, 75, 80};
static const size_t VIDEO_CHANNELS_COUNT = sizeof(VIDEO_CHANNELS) / sizeof(VIDEO_CHANNELS[0]);

// RC toys/drones
static const uint8_t RC_CHANNELS[] = {1, 3, 5, 7};
static const size_t RC_CHANNELS_COUNT = sizeof(RC_CHANNELS) / sizeof(RC_CHANNELS[0]);

// All groups
static const Rf24ChannelGroup RF24_GROUPS[] = {
    { " WiFi", WIFI_CHANNELS, WIFI_CHANNELS_COUNT },
    { " BLE", BLE_CHANNELS, BLE_CHANNELS_COUNT },
    { " Bluetooth", BLUETOOTH_CHANNELS, BLUETOOTH_CHANNELS_COUNT },
    { " USB", USB_CHANNELS, USB_CHANNELS_COUNT },
    { " Video", VIDEO_CHANNELS, VIDEO_CHANNELS_COUNT },
    { " RC Toys", RC_CHANNELS, RC_CHANNELS_COUNT },
};

static const size_t RF24_GROUP_COUNT = sizeof(RF24_GROUPS)/sizeof(RF24_GROUPS[0]);