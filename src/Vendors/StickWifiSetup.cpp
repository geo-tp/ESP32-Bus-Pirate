#if defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)

#include "StickWifiSetup.h"

Preferences& getPreferences() {
    static Preferences preferences;
    return preferences;
}

bool loadWifiCredentials(String& ssid, String& password) {
    Preferences& preferences = getPreferences();
    preferences.begin("wifi_settings", true);
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
    return !ssid.isEmpty() && !password.isEmpty();
}

bool setupStickWifi() {
    String ssid, password;
    M5.Lcd.setTextColor(TFT_LIGHTGRAY);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.fillRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x0841);
    
    if (!loadWifiCredentials(ssid, password)) {
        M5.Lcd.drawRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, TFT_RED);

        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.setTextSize(2.2);
        M5.Lcd.drawString("No saved WiFi", 37, 45);

        M5.Lcd.setTextSize(1.5);
        M5.Lcd.setTextColor(TFT_LIGHTGRAY);
        M5.Lcd.drawString("USB Serial to setup", 36, 70);

        delay(5000);
        return false;
    }

    M5.Lcd.drawRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x05A3);
    WiFi.begin(ssid.c_str(), password.c_str());
    M5.Lcd.setTextSize(1.8);
    M5.Lcd.setTextColor(TFT_LIGHTGRAY);
    M5.Lcd.drawString("Connecting", 67, 60);

    M5.Lcd.setTextSize(0.8);
    M5.Lcd.setCursor(55, 79);
    for (int i = 0; i < 30; ++i) {
        if (WiFi.status() == WL_CONNECTED) {
            return true;
        }
        delay(800);
        M5.Lcd.print(".");
    }

    M5.Lcd.fillRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x0841);
    M5.Lcd.drawRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, TFT_RED);

    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.drawString("Failed to connect", 43, 45);
    
    M5.Lcd.setTextColor(TFT_LIGHTGRAY);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.drawString("USB Serial to setup", 36, 70);

    delay(4000);
    return false;
}

#endif