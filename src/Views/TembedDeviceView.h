#pragma once

#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "Interfaces/IDeviceView.h"
#include <TFT_eSPI.h>
#include "States/GlobalState.h"


#ifdef DEVICE_TEMBEDS3CC1101
    #define PIN_LCD_BL   21
#else
    #define PIN_LCD_BL   15
#endif

#define PIN_POWER_ON 46

#define DARK_GREY_RECT 0x4208

typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

// Commands to start the Tembed screen
static const lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};

class TembedDeviceView : public IDeviceView {
public:
    TembedDeviceView();

    void initialize() override;
    SPIClass& getScreenSpiInstance() override;
    void logo() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void show(PinoutConfig& config) override;
    void loading() override;
    void clear() override;
    void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void setRotation(uint8_t rotation) override;
    void topBar(const std::string& title, bool submenu, bool searchBar) override;
    void horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) override;

    void shutDown();

private:
    TFT_eSPI tft;
    TFT_eSprite canvas = TFT_eSprite(&tft);

    void drawCenterText(const std::string& text, int y, int fontSize);
    void initDisplayRegs();
    void welcomeWeb(const std::string& ip);
    void welcomeSerial(const std::string& baud);
};

#endif