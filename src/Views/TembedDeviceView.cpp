#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "TembedDeviceView.h"
#include "Data/WelcomeScreen.h"

TembedDeviceView::TembedDeviceView() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);
}

SPIClass& TembedDeviceView::getScreenSpiInstance() {
  return screenSpi;
}

void* TembedDeviceView::getScreen() {
  return &tft; 
}

void TembedDeviceView::initialize() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);

  screenSpi.begin(PIN_LCD_SCLK, PIN_LCD_MISO, PIN_LCD_MOSI, PIN_LCD_CS);

  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true);

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);

  setBrightness(brightnessPct);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

void TembedDeviceView::logo() {
  clear();

  // Logo
  tft.setSwapBytes(true);
  tft.pushImage(40, 30, WELCOME_IMAGE_WIDTH, WELCOME_IMAGE_HEIGHT, WelcomeScreen);
  tft.setSwapBytes(false);

  // Sub
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  GlobalState& state = GlobalState::getInstance();
  auto version = "ESP32 Bus Pirate - " + state.getVersion();
  drawCenterText(version.c_str(), 130, 2);
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
  tft.fillRect(0, 35, tft.width(), tft.height() - 35, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("Pin ");
  tft.print(pin);

  const int traceY = 50;
  const int traceH = 80;
  const int centerY = traceY + traceH / 2;

  int x = 10;
  for (size_t i = 1; i < buffer.size(); ++i) {
    uint8_t prev = buffer[i - 1];
    uint8_t curr = buffer[i];

    int y1 = prev ? (centerY - 15) : (centerY + 15);
    int y2 = curr ? (centerY - 15) : (centerY + 15);

    if (curr != prev) {
      tft.drawLine(x, y1, x + step, y1, prev ? TFT_GREEN : TFT_WHITE);
      tft.drawLine(x + step, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE);
    } else {
      tft.drawLine(x, y1, x + step, y2, curr ? TFT_GREEN : TFT_WHITE);
    }

    x += step;
    if (x > tft.width() - step) break;
  }
}

void TembedDeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
  tft.fillRect(0, 35, tft.width(), tft.height() - 35, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("Pin ");
  tft.print(pin);

  const int topY = 35;
  const int h = 135;

  int x = 10;
  for (size_t i = 1; i < buffer.size(); ++i) {
    int prev = topY + (h - 1) - (buffer[i - 1] >> 1);
    int curr = topY + (h - 1) - (buffer[i] >> 1);
    tft.drawLine(x, prev, x + step, curr, TFT_GREEN);
    x += step;
    if (x > tft.width() - step) break;
  }
}

void TembedDeviceView::setRotation(uint8_t rotation) {
  tft.setRotation(rotation);
}

void TembedDeviceView::setBrightness(uint8_t brightness) {
  if (brightness > 100) brightness = 100;
  brightnessPct = brightness;

  uint8_t pwm = (uint8_t)((brightnessPct * 255) / 100);
  tft.setBrightness(pwm);
}

uint8_t TembedDeviceView::getBrightness() {
  return brightnessPct;
}

void TembedDeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {
  (void)submenu;
  (void)searchBar;

  // Zone topbar
  tft.fillRect(0, 0, tft.width(), 30, TFT_BLACK);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(title.c_str(), tft.width() / 2, 20);
  tft.setTextDatum(TL_DATUM);
}

void TembedDeviceView::horizontalSelection(
  const std::vector<std::string>& options,
  uint16_t selectedIndex,
  const std::string& description1,
  const std::string& description2
) {
  const int originY = 30;
  const int h = tft.height() - originY;

  // avoid topbar overlap
  tft.fillRect(0, originY, tft.width(), h, TFT_BLACK);

  // Description 1
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(description1.c_str(), tft.width() / 2, originY + 26);

  // Description 2 hardcoded
  tft.setTextColor(DARK_GREY_RECT, TFT_BLACK);
  tft.drawString("Long press button to shut down", tft.width() / 2, tft.height() - 20);

  // Box option
  const std::string& option = options[selectedIndex];
  int boxX = 60;
  int boxW = tft.width() - 120;
  int boxY = originY + 45;
  int boxH = 50;
  int corner = 8;

  tft.fillRoundRect(boxX, boxY, boxW, boxH, corner, DARK_GREY_RECT);
  tft.drawRoundRect(boxX, boxY, boxW, boxH, corner, TFT_GREEN);

  // Option name
  tft.setTextColor(TFT_WHITE, DARK_GREY_RECT);
  tft.setTextFont(2);
  tft.setTextSize(2);

  int textW = tft.textWidth(option.c_str());
  int textX = (tft.width() - textW) / 2;
  int textH = tft.fontHeight();
  int textY = boxY + (boxH - textH) / 2;

  tft.setTextDatum(TL_DATUM);
  tft.drawString(option.c_str(), textX, textY);

  // Arrows
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.setCursor(35, boxY + 19);
  tft.print("<");
  tft.setCursor(tft.width() - 40, boxY + 19);
  tft.print(">");
}

void TembedDeviceView::drawCenterText(const std::string& text, int y, int fontSize) {
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(fontSize);
  tft.drawString(text.c_str(), tft.width() / 2, y);
  tft.setTextDatum(TL_DATUM);
}

void TembedDeviceView::welcomeSerial(const std::string& baudStr) {
  tft.fillScreen(TFT_BLACK);

  // Titre
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(88, 35);
  tft.println("Open Serial (USB COM)");

  // Rect baudrate
  tft.fillRoundRect(70, 60, 180, 40, 8, DARK_GREY_RECT);
  tft.drawRoundRect(70, 60, 180, 40, 8, TFT_GREEN);

  // Texte baud
  std::string baud = "Baudrate: " + baudStr;
  int textW = tft.textWidth(baud.c_str());
  tft.setTextColor(TFT_WHITE, DARK_GREY_RECT);
  tft.setCursor((tft.width() - textW) / 2, 73);
  tft.print(baud.c_str());

  // Sub
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(80, 107);
  tft.println("Then press a key to start");
}

void TembedDeviceView::welcomeWeb(const std::string& ipStr) {
  tft.fillScreen(TFT_BLACK);

  // Titre
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(82, 34);
  tft.println("Open browser to connect");

  // Rectangle IP
  tft.fillRoundRect(60, 60, 200, 40, 8, DARK_GREY_RECT);
  tft.drawRoundRect(60, 60, 200, 40, 8, TFT_GREEN);

  // Texte IP
  std::string ip = "http://" + ipStr;
  int textW = tft.textWidth(ip.c_str());
  tft.setTextColor(TFT_WHITE, DARK_GREY_RECT);
  tft.setCursor((tft.width() - textW) / 2, 73);
  tft.print(ip.c_str());
}

void TembedDeviceView::shutDown() {
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  drawCenterText("Shutting down...", 80, 2);
  delay(1000);

  // backlight + power
  digitalWrite(PIN_LCD_BL, LOW);
  digitalWrite(PIN_POWER_ON, LOW);
}

void TembedDeviceView::show(PinoutConfig& config) {
  tft.fillScreen(TFT_BLACK);

  const auto& mappings = config.getMappings();
  auto mode = config.getMode();

  // Mode name
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  std::string modeStr = "MODE " + mode;
  tft.drawString(modeStr.c_str(), tft.width() / 2, 20);
  tft.setTextDatum(TL_DATUM);

  // No mapping
  if (mappings.empty()) {
    const int frameX = 20;
    const int frameY = 45;
    const int frameW = tft.width() - 40;
    const int frameH = tft.height() - 70;
    const int frameR = 5;

    tft.fillRoundRect(frameX, frameY, frameW, frameH, frameR, TFT_BLACK);
    tft.drawRoundRect(frameX, frameY, frameW, frameH, frameR, TFT_GREEN);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Nothing to display", tft.width() / 2, frameY + frameH / 2);
    tft.setTextDatum(TL_DATUM);
    return;
  }

  // Mapping list
  int boxHeight = 24;
  int startY = 40;

  for (size_t i = 0; i < mappings.size(); ++i) {
    int y = startY + (int)i * (boxHeight + 4);

    tft.fillRoundRect(20, y, tft.width() - 40, boxHeight, 6, DARK_GREY_RECT);
    tft.drawRoundRect(20, y, tft.width() - 40, boxHeight, 6, TFT_GREEN);

    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, DARK_GREY_RECT);

    int w = tft.textWidth(mappings[i].c_str());
    int textX = (tft.width() - w) / 2;

    tft.setCursor(textX, y + 5);
    tft.print(mappings[i].c_str());
  }
}

#endif