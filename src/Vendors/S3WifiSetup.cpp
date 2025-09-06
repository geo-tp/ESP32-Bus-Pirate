#if defined(DEVICE_M5STAMPS3) || defined(DEVICE_S3DEVKIT)

#include "S3WifiSetup.h"
#include <Preferences.h>
#include <WiFi.h>
#include <Arduino.h>
#include <FastLED.h>

#define NVS_SSID_KEY "ssid"
#define NVS_PASS_KEY "pass"

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

Preferences& getPreferences() {
    static Preferences preferences;
    return preferences;
}

bool loadWifiCredentials(String& ssid, String& password) {
    Preferences& preferences = getPreferences();
    preferences.begin("wifi_settings", true);  // readonly = true
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
    return !ssid.isEmpty() && !password.isEmpty();
}

bool setupS3Wifi() {
    // LED RGB init
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    leds[0] = CRGB::White;
    FastLED.show();

    String ssid, password;
    if (!loadWifiCredentials(ssid, password)) {
        leds[0] = CRGB::Blue;
        FastLED.show();
        delay(2000);
        FastLED.clear(true);
        return false;
    }

    WiFi.begin(ssid.c_str(), password.c_str());

    for (int i = 0; i < 15; ++i) {
        if (WiFi.status() == WL_CONNECTED) {
            leds[0] = CRGB::Green;
            FastLED.show();
            delay(1000);
            FastLED.clear(true);
            return true;
        }
        delay(1000);
    }

    leds[0] = CRGB::Red;
    FastLED.show();
    delay(1000);
    FastLED.clear(true);
    
    return false;
}

#endif