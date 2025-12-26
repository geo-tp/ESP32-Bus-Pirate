#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "TembedWifiSetup.h"

#include <WiFi.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <RotaryEncoder.h>
#include <esp_sleep.h>
#include "Inputs/InputKeys.h"

#ifdef DEVICE_TEMBEDS3CC1101
    #define TEMBED_PIN_ENCODE_A          4
    #define TEMBED_PIN_ENCODE_B          5
    #define TEMBED_PIN_SIDE_BTN          6
#else
    #define TEMBED_PIN_ENCODE_A          2
    #define TEMBED_PIN_ENCODE_B          1
    #define TEMBED_PIN_SIDE_BTN          0 // side button is reset
#endif

#define TEMBED_PIN_ENCODE_BTN            0

TFT_eSPI tft;
RotaryEncoder encoder(TEMBED_PIN_ENCODE_A, TEMBED_PIN_ENCODE_B, RotaryEncoder::LatchMode::TWO03);

char lastInput;
int lastPos;
bool lastButton;

void checkShutdownRequest() {
    if (!digitalRead(TEMBED_PIN_ENCODE_BTN) || !digitalRead(TEMBED_PIN_SIDE_BTN)) {
        unsigned long start = millis();
        for (int i = 0; i < 30; ++i) {
            if (digitalRead(TEMBED_PIN_ENCODE_BTN) && digitalRead(TEMBED_PIN_SIDE_BTN)) return;
            delay(100);
        }

        tft.fillScreen(TFT_BLACK);
        tft.setCursor(40, 60);
        tft.setTextColor(TFT_WHITE);
        tft.setTextFont(2);
        int16_t x1, y1;
        uint16_t w, h;
        std::string text = "Shutting down...";
        tft.setTextDatum(MC_DATUM);
        tft.drawString(text.c_str(), tft.width() / 2, 80);
        // Shutdown
        delay(3000);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)TEMBED_PIN_SIDE_BTN, 0);
        esp_deep_sleep_start();
    }
}

void tick() {
    encoder.tick();
    int pos = encoder.getPosition();
    if (pos < lastPos) {
        lastInput = KEY_ARROW_LEFT;
        lastPos = pos;
    } else if (pos > lastPos) {
        lastInput = KEY_ARROW_RIGHT;
        lastPos = pos;
    } else if (!digitalRead(TEMBED_PIN_ENCODE_BTN) && !lastButton) {
        lastInput = KEY_OK;
        lastButton = true;
    } else if (digitalRead(TEMBED_PIN_ENCODE_BTN)) {
        lastButton = false;
    }
    checkShutdownRequest();
}

char readChar() {
    tick();
    char c = lastInput;
    lastInput = KEY_NONE;
    return c;
}

char handler() {
    while (true) {
        char c = readChar();
        if (c != KEY_NONE) return c;
        delay(5);
    }
}

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

void drawWifiBox(uint16_t borderColor, const String& msg1, const String& msg2, uint8_t textSize = 2) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(1);

    tft.fillRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, DARK_GREY);
    tft.drawRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, borderColor);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(textSize);
    tft.drawString(msg1, 37, 45);

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY);
    tft.drawString(msg2, 37, 70);
}

String selectWifiNetwork(TFT_eSPI& tft) {
    int networksCount = 0;

    while (networksCount == 0) {
        drawWifiBox(TFT_GREEN, "Scanning WiFi...", "");
        networksCount = WiFi.scanNetworks();
        if (networksCount == 0) {
            drawWifiBox(TFT_RED, "No networks found", "Retrying...");
            delay(2000);
        }
    }

    int selected = 0;
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.drawString("Select Wi-Fi:", 10, 10);
    while (true) {
        tft.fillRect(0, 40, tft.width(), tft.height() - 40, TFT_BLACK);
        tft.setTextSize(1);
        for (int i = 0; i < min(5, networksCount); ++i) {
            String ssid = WiFi.SSID(i);
            if (i == selected) {
                tft.setTextColor(TFT_GREEN);
                tft.drawString("> " + ssid, 20, 40 + i * 15);
            } else {
                tft.setTextColor(TFT_LIGHTGREY);
                tft.drawString("  " + ssid, 20, 40 + i * 15);
            }
        }

        char key = handler();
        if (key == KEY_ARROW_RIGHT) {
            selected = (selected - 1 + networksCount) % networksCount;
        } else if (key == KEY_ARROW_LEFT) {
            selected = (selected + 1) % networksCount;
        } else if (key == KEY_OK) {
            return WiFi.SSID(selected);
        }
    }
}

String enterText(const String& label, TFT_eSPI& tft) {
     const String charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_ .@!#$%^&*()=+{}[]|\\:;\"'<>,?/~\x08\x0D";
    int index = 0;
    String text = "";

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.drawString(label, 10, 10);
    tft.setTextSize(1);
    tft.drawString("Wheel to select - [OK] to Confirm - [<-] to Erase", 10, tft.height() - 20);

    while (true) {
        char currentChar = charset[index];

        String alias;
        if (currentChar == '\x08') {
            alias = "<-";
        } else if (currentChar == '\x0D') {
            alias = "OK";
        } else {
            alias = String(currentChar);
        }

        String charDisplay = (currentChar == '\x08' || currentChar == '\x0D')
                             ? "[" + alias + "]"
                             : alias;
        String display = text + charDisplay;

        tft.fillRect(0, 50, tft.width(), tft.height() - 100, TFT_BLACK);
        tft.setTextSize(1);
        tft.setTextFont(2);
        tft.setTextColor(TFT_LIGHTGREY);
        tft.drawString(display, 10, 70);
        tft.setTextColor(TFT_WHITE);
        char key = handler();
        if (key == KEY_ARROW_RIGHT) {
            index = (index - 1 + charset.length()) % charset.length();
        } else if (key == KEY_ARROW_LEFT) {
            index = (index + 1) % charset.length();
        } else if (key == KEY_OK) {
            if (currentChar == '\x08') {
                if (!text.isEmpty()) text.remove(text.length() - 1);
            } else if (currentChar == '\x0D') {
                return text;
            } else {
                text += currentChar;
            }
        }
    }
}

void setWifiCredentials(String ssid, String password) {
    Preferences& preferences = getPreferences();
    preferences.begin("wifi_settings", false);
    preferences.putString(NVS_SSID_KEY, ssid);
    preferences.putString(NVS_PASS_KEY, password);
    preferences.end();
}

bool setupTembedWifi() {
    tft.setRotation(3);
    String ssid, password;

    while (true) {

        
        if (!loadWifiCredentials(ssid, password)) {
            ssid = selectWifiNetwork(tft);
            password = enterText("Enter password", tft);
            setWifiCredentials(ssid, password);
        }
        
        drawWifiBox(TFT_GREEN, "Connecting", ssid);
        WiFi.begin(ssid.c_str(), password.c_str());
        
        tft.setTextColor(TFT_LIGHTGREY);
        tft.setCursor(55, 100);
        for (int i = 0; i < 30; ++i) {
            if (WiFi.status() == WL_CONNECTED) {
                drawWifiBox(TFT_GREEN, "Connected", "");
                delay(2000);
                return true;
            }
            delay(600);
            tft.print(".");
        }
        
        drawWifiBox(TFT_WHITE, "Failed to connect", "Retry or setup Wi-Fi with Serial");
        setWifiCredentials("", ""); // reset
        WiFi.disconnect(true);
        WiFi.mode(WIFI_STA);
        delay(3000);
    }

    return false;
}


#endif
