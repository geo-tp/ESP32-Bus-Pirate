/**
 * @file CardWifiSetup.h
 * @author Aurélio Avanzi Dutch version Roland Breedveld adapted by Milton Matuda for Mic WebServer
 * @brief https://github.com/cyberwisk/M5Card_Wifi_KeyBoard_Setup/tree/main/M5Card_Wifi_KeyBoard_Setup
 * @version Apha 0.4BR
 * @date 2024-11-14
 *
 * @Hardwares: M5Cardputer - https://docs.m5stack.com/en/core/Cardputer
 * @Dependent Librarys:
 * M5Cardputer: https://github.com/m5stack/M5Cardputer
 * WiFi: https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi
 **/

#ifdef DEVICE_CARDPUTER


#include "CardWifiSetup.h"

String inputText(const String& prompt, int x, int y) {
    String data = "> ";

    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setTextScroll(true);
    M5Cardputer.Display.drawString(prompt, x, y);

    while (1) {
        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
                for (auto i : status.word) {
                    data += i;
                }
                if (status.del && data.length() > 2) {
                    data.remove(data.length() - 1);
                }
                if (status.enter) {
                    data.remove(0, 2);
                    return data;
                }
                M5Cardputer.Display.fillRect(0, y - 4, M5Cardputer.Display.width(), 25, BLACK);
                M5Cardputer.Display.drawString(data, 4, y);
            }
        }
        delay(20);
    }
}

int scanWifiNetworks() {
    int networksCount = 0;

    while (networksCount == 0) {
        M5Cardputer.Display.fillScreen(TFT_BLACK);
        M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
        M5Cardputer.Display.setTextSize(1.5);
        M5.Lcd.fillRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x0841);
        M5.Lcd.drawRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x05A3);
        M5Cardputer.Display.drawString("Scanning Networks", 45, 60);
        networksCount = WiFi.scanNetworks();

        if (networksCount == 0) {
            M5Cardputer.Display.drawString("No networks found", 45, 85);
            delay(2000);
        }
    }
    
    return networksCount;
}

String selectWifiNetwork(int numNetworks) {
    int selectedNetwork = 0;

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(0x05A3);
    M5Cardputer.Display.setTextSize(1.6);
    M5Cardputer.Display.drawString("Select Network", 1, 1);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);

    while (1) {
        for (int i = 0; i < 7 && i < numNetworks; ++i) {
            String ssid = WiFi.SSID(i);
            if (i == selectedNetwork) {
                M5Cardputer.Display.drawString("-> " + ssid, 1, 18 + i * 18);
            } else {
                M5Cardputer.Display.drawString("   " + ssid, 1, 18 + i * 18);
            }
        }

        M5Cardputer.update();
        if (M5Cardputer.Keyboard.isChange()) {
            if (M5Cardputer.Keyboard.isPressed()) {
                M5Cardputer.Display.fillRect(0, 11, 20, 75, BLACK);
                Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

                if (M5Cardputer.Keyboard.isKeyPressed(';') && selectedNetwork > 0) {
                    selectedNetwork--;
                }
                if (M5Cardputer.Keyboard.isKeyPressed('.') && selectedNetwork < min(4, numNetworks - 1)) {
                    selectedNetwork++;
                }
                if (status.enter) {
                    return WiFi.SSID(selectedNetwork);
                }
            }
        }
        delay(20);
    }
}

void setWifiCredentials(String ssid, String password) {
    Preferences& preferences = getPreferences();
    preferences.begin(NVS_DOMAIN, false);
    preferences.putString(NVS_SSID_KEY, ssid);
    preferences.putString(NVS_PASS_KEY, password);
    preferences.end();
}

void getWifiCredentials(String &ssid, String &password) {
    Preferences& preferences = getPreferences();
    preferences.begin("wifi_settings", false);
    ssid = preferences.getString(NVS_SSID_KEY, "");
    password = preferences.getString(NVS_PASS_KEY, "");
    preferences.end();
}

bool connectToWifi(String wifiSSID, String wifiPassword) {
    int tm = 0;

    WiFi.disconnect();
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5.Lcd.fillRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x0841);
    M5.Lcd.drawRoundRect(20, 20, M5.Lcd.width() - 40, M5.Lcd.height() - 40, 5, 0x05A3);
    M5Cardputer.Display.drawString("Connecting", 70, 55);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.setCursor(35, 75);
    M5Cardputer.Display.setTextSize(0.5); // for the loading dots

    while (tm++ < 110 && WiFi.status() != WL_CONNECTED) {
        delay(160);
        M5Cardputer.Display.print(".");

        if (tm == 55) {
            M5Cardputer.Display.setCursor(35, 80);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.disconnect();
    M5Cardputer.Display.setTextColor(TFT_LIGHTGREY);
    M5Cardputer.Display.setTextSize(1.4);
    M5Cardputer.Display.drawString("Failed to connect to WiFi", 18, 90);
    delay(3000);
    return false;
}

String askWifiPassword(String ssid) {
    M5Cardputer.Display.fillScreen(TFT_BLACK);
    M5Cardputer.Display.setTextColor(0x05A3);
    M5Cardputer.Display.drawString("SSID: " + ssid, 2, 2);
    M5Cardputer.Display.setTextColor(TFT_LIGHTGRAY);
    M5Cardputer.Display.drawString("Enter the password", 37, 90);
    return inputText("> ", 4, M5Cardputer.Display.height() - 24);
}

void setupCardputerWifi() {
    bool connected = false;
    int numNetworks = 0;
    String savedSSID;
    String savedPassword;
    String selectedSSID;
    String wifiPassword;

    while (!connected) {
        // Scan Networks
        numNetworks = scanWifiNetworks();

        // Select Network
        selectedSSID = selectWifiNetwork(numNetworks);

        // Get saved credentials
        getWifiCredentials(savedSSID, savedPassword);

        // Saved SSID is empty or selected SSID doesn't match saved SSID
        if (savedSSID.isEmpty() || savedSSID != selectedSSID) {
            wifiPassword = askWifiPassword(selectedSSID);
        } else {
            wifiPassword = savedPassword;
        }
        
        connected = connectToWifi(selectedSSID, wifiPassword);

        if (connected) { 
            setWifiCredentials(selectedSSID, wifiPassword);
        } else {
            setWifiCredentials("", ""); // erase creds
        }
    }
}

#endif