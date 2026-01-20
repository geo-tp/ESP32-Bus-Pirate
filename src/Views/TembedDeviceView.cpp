#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "TembedDeviceView.h"
#include <Arduino.h>
#include "Data/WelcomeScreen.h"

TembedDeviceView::TembedDeviceView() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    initDisplayRegs();
    tft.setRotation(3);
    tft.setSwapBytes(true);
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

SPIClass& TembedDeviceView::getScreenSpiInstance() {
    return tft.getSPIinstance();
}

void TembedDeviceView::initDisplayRegs() {
    for (uint8_t i = 0; i < sizeof(lcd_st7789v) / sizeof(lcd_cmd_t); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7F); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }
        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
}

void TembedDeviceView::initialize() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void TembedDeviceView::logo() {
    clear();

    // TODO: Color are not correct, with or without setSwapBytes
    tft.setSwapBytes(true);
    tft.pushImage(40, 30, WELCOME_IMAGE_WIDTH, WELCOME_IMAGE_HEIGHT, WelcomeScreen);
    tft.setSwapBytes(false);

    // Sub
    tft.setTextColor(TFT_WHITE, DARK_GREY_RECT);
    GlobalState& state = GlobalState::getInstance();
    auto version = "ESP32 Bus Pirate - " + state.getVersion();
    drawCenterText(version.c_str(), 130, 1.9);

    delay(3000);
}

void TembedDeviceView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {
    if (terminalType == TerminalTypeEnum::WiFiClient) welcomeWeb(terminalInfos);
    else welcomeSerial(terminalInfos);
}

void TembedDeviceView::loading() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextFont(1);

    tft.fillRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, DARK_GREY_RECT);
    tft.drawRoundRect(20, 20, tft.width() - 40, tft.height() - 40, 5, TFT_GREEN);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("Loading...", 102, 52);
}

void TembedDeviceView::clear() {
    tft.fillScreen(TFT_BLACK);
}

void TembedDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    const int canvasWidth = 320;
    const int canvasHeight = 80;
    const int logicCenterY = canvasHeight / 2;
    //const int step = 1;

    canvas.setColorDepth(8);
    canvas.createSprite(canvasWidth, canvasHeight);
    canvas.fillSprite(TFT_BLACK);

    // Pin num
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextSize(1);
    canvas.setCursor(10, 0);
    canvas.print("Pin ");
    canvas.print(pin);

    // Trace
    int x = 10;
    for (size_t i = 1; i < buffer.size(); ++i) {
        uint8_t prev = buffer[i - 1];
        uint8_t curr = buffer[i];
        int y1 = prev ? logicCenterY - 15 : logicCenterY + 15;
        int y2 = curr ? logicCenterY - 15 : logicCenterY + 15;

        if (curr != prev){
            canvas.drawLine(x, y1, x + step, y1, prev ? TFT_GREEN : TFT_WHITE );
            canvas.drawLine(x + step, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE );
        } else {
            canvas.drawLine(x, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE );
        }
        x += step;
        if (x > canvasWidth - step) break;
    }

    canvas.pushSprite(0, 50);
    canvas.deleteSprite();
}

void TembedDeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    const int canvasWidth = 320;
    const int canvasHeight = 135;

    canvas.setColorDepth(8);
    canvas.createSprite(canvasWidth, canvasHeight);
    canvas.fillSprite(TFT_BLACK);

    // Pin num
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextSize(1);
    canvas.setCursor(10, 0);
    canvas.print("Pin ");
    canvas.print(pin);

    // Trace
    int x = 10;
    for (size_t i = 1; i < buffer.size(); ++i) {
        
        uint8_t prev = canvasHeight - 1 - (buffer[i - 1] >> 1);
        uint8_t curr = canvasHeight - 1 - (buffer[i] >> 1);
        canvas.drawLine(x, prev, x + step, curr, TFT_GREEN);      
        x += step;
        if (x > canvasWidth - step) break;
    }

    canvas.pushSprite(0, 35);
    canvas.deleteSprite();
}

void TembedDeviceView::setRotation(uint8_t rotation) {
    tft.setRotation(rotation);
}

void TembedDeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    drawCenterText(title.c_str(), 20, 2);

    if (submenu) {
        // TODO
    }
    if (searchBar) {
        // TODO
    }
}

void TembedDeviceView::horizontalSelection(
    const std::vector<std::string>& options,
    uint16_t selectedIndex,
    const std::string& description1,
    const std::string& description2)
{
    canvas.setColorDepth(16);
    canvas.createSprite(tft.width(), tft.height() - 30); // without topBar
    canvas.fillSprite(TFT_BLACK);

    // Description 1
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(MC_DATUM);
    canvas.setTextFont(2);
    canvas.setTextSize(1);
    canvas.drawString(description1.c_str(), canvas.width() / 2, 26);

    // Description 2
    canvas.setTextColor(DARK_GREY_RECT, TFT_BLACK);
    canvas.drawString("Long press button to shut down", canvas.width() / 2, canvas.height() - 15);

    // Option placing
    const std::string& option = options[selectedIndex];
    int boxX = 60;
    int boxW = canvas.width() - 120;
    int boxY = 45;
    int boxH = 50;
    int corner = 8;

    // Option Rect
    canvas.fillRoundRect(boxX, boxY, boxW, boxH, corner, DARK_GREY_RECT);
    canvas.drawRoundRect(boxX, boxY, boxW, boxH, corner, TFT_GREEN);

    // Option name
    canvas.setTextColor(TFT_WHITE, DARK_GREY_RECT);
    canvas.setTextFont(2);
    canvas.setTextSize(2);
    int textW = canvas.textWidth(option.c_str(), 2);
    int textX = (canvas.width() - textW) / 2;
    canvas.setCursor(textX, boxY + 10);
    canvas.print(option.c_str());

    // Arrows
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextFont(2);
    canvas.setCursor(35, boxY + 19);
    canvas.print("<");
    canvas.setCursor(canvas.width() - 40, boxY + 19);
    canvas.print(">");

    // Display
    canvas.pushSprite(0, 30); // topbar height

    // Release
    canvas.deleteSprite();
}

void TembedDeviceView::drawCenterText(const std::string& text, int y, int fontSize) {
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(fontSize);
    tft.drawString(text.c_str(), tft.width() / 2, y);
}

void TembedDeviceView::welcomeSerial(const std::string& baudStr) {
    canvas.setColorDepth(16);
    canvas.setTextFont(2);
    canvas.createSprite(tft.width(), tft.height());
    canvas.fillSprite(TFT_BLACK);

    // Titre
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(TL_DATUM);
    canvas.setCursor(88, 35);
    canvas.println("Open Serial (USB COM)");

    // Rect baudrate
    canvas.fillRoundRect(70, 60, 180, 40, 8, DARK_GREY_RECT);
    canvas.drawRoundRect(70, 60, 180, 40, 8, TFT_GREEN);

    // Texte Baud
    std::string baud = "Baudrate: " + baudStr;
    int16_t textW = canvas.textWidth(baud.c_str(), 2);
    canvas.setCursor((canvas.width() - textW) / 2, 73);
    canvas.setTextColor(TFT_WHITE, DARK_GREY_RECT);
    canvas.print(baud.c_str());

    // Sub
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setCursor(80, 107);
    canvas.println("Then press a key to start");

    canvas.pushSprite(0, 0);
    canvas.deleteSprite();
}


void TembedDeviceView::welcomeWeb(const std::string& ipStr) {
    canvas.setColorDepth(16);
    canvas.createSprite(tft.width(), tft.height());
    canvas.fillSprite(TFT_BLACK);

    // Titre
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextFont(2);
    canvas.setCursor(82, 34);
    canvas.println("Open browser to connect");

    // Rectangle IP
    canvas.fillRoundRect(60, 60, 200, 40, 8, DARK_GREY_RECT);
    canvas.drawRoundRect(60, 60, 200, 40, 8, TFT_GREEN);

    // Texte IP
    std::string ip = "http://" + ipStr;
    int16_t textW = canvas.textWidth(ip.c_str(), 2);
    canvas.setCursor((canvas.width() - textW) / 2, 73);
    canvas.setTextColor(TFT_WHITE, DARK_GREY_RECT);
    canvas.setTextFont(2);
    canvas.print(ip.c_str());

    canvas.pushSprite(0, 0);
    canvas.deleteSprite();
}

void TembedDeviceView::shutDown() {
    tft.setRotation(3); // it is called from input
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 60);
    tft.setTextColor(TFT_WHITE);
    tft.setTextFont(2);
    drawCenterText("Shutting down...", 80, 2);
    delay(1000);
    digitalWrite(PIN_LCD_BL, LOW);
    digitalWrite(PIN_POWER_ON, LOW);
    initialize(); // black screen
}

void TembedDeviceView::show(PinoutConfig& config) {
    clear();

    const auto& mappings = config.getMappings();
    auto mode = config.getMode();

    canvas.setColorDepth(16);
    canvas.createSprite(tft.width(), tft.height());
    canvas.fillSprite(TFT_BLACK);

    // Mode name
    canvas.setTextColor(TFT_GREEN);
    canvas.setTextFont(2.5);
    std::string modeStr = "MODE " + mode;
    int modeW = canvas.textWidth(modeStr.c_str(), 2);
    int modeX = (canvas.width() - modeW) / 2;
    canvas.drawString(modeStr.c_str(), modeX, 13);

    // No mapping
    if (mappings.empty()) {
        canvas.fillRoundRect(20, 45, canvas.width() - 40, canvas.height() - 70, 5, TFT_BLACK);
        canvas.drawRoundRect(20, 45, canvas.width() - 40, canvas.height() - 70, 5, TFT_GREEN);

        canvas.setTextColor(TFT_WHITE);
        canvas.setTextFont(2);
        canvas.drawString("Nothing to display", 100, 85);
        canvas.pushSprite(0, 0);
        canvas.deleteSprite();
        return;
    }

    // Mapping
    int boxHeight = 24;
    int startY = 40;
    int spacing = 4;
    int margin = 10;

    for (size_t i = 0; i < mappings.size(); ++i) {
        const std::string& mapping = mappings[i];
        int y = startY + i * (boxHeight + spacing);

        canvas.fillRoundRect(margin * 2, y, canvas.width() - 4 * margin, boxHeight, 6, DARK_GREY_RECT);
        canvas.drawRoundRect(margin  *2, y, canvas.width() - 4 * margin, boxHeight, 6, TFT_GREEN);

        int textW = canvas.textWidth(mapping.c_str(), 2);
        int textX = (canvas.width() - textW) / 2;
        int textY = y + (boxHeight - 16) / 2 + 1;

        canvas.setTextColor(TFT_WHITE, DARK_GREY_RECT);
        canvas.setCursor(textX, textY);
        canvas.setTextFont(2);
        canvas.print(mapping.c_str());
    }

    canvas.pushSprite(0, 0);
    canvas.deleteSprite();
}

#endif