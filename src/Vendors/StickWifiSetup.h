#pragma one

#if defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)

#define NVS_SSID_KEY "ssid"
#define NVS_PASS_KEY "pass"
#define NVS_DOMAIN "wifi_settings"

#include <Arduino.h>
#include <Preferences.h>
#include <M5Unified.h>
#include <WiFi.h>

bool setupStickWifi();


#endif