#pragma once

#if defined(DEVICE_CARDPUTER) || defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)

#include <algorithm>
#include <M5Unified.h>
#include "Interfaces/IDeviceView.h"
#include "Enums/ModeEnum.h"
#include "States/GlobalState.h"
#include "Models/PinoutConfig.h"
#include "Enums/ModeEnum.h"
#include "Enums/TerminalTypeEnum.h"

#define BACKGROUND_COLOR TFT_BLACK
#define PRIMARY_COLOR 0x05A3
#define RECT_COLOR_DARK 0x0841
#define RECT_COLOR_LIGHT 0xd69a
#define TEXT_COLOR 0xE71C
#define TEXT_COLOR_ALT TFT_DARKGRAY

#define DEFAULT_MARGIN 5
#define DEFAULT_ROUND_RECT 5

#define TOP_BAR_SIZE 30

class M5DeviceView : public IDeviceView {
public:
    void initialize() override;
    SPIClass& getScreenSpiInstance() override;
    void logo() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void show(PinoutConfig& config) override;
    void loading() override;
    void clear() override;
    void setRotation(uint8_t rotation);
    void topBar(const std::string& title, bool submenu, bool searchBar) override;
    void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) override;

private:
    void welcomeSerial(const std::string& baudStr);
    void welcomeWeb(const std::string& ipStr);
    void showDetailedConfig(const PinoutConfig& config, int selectedIndex);
    void drawRect(bool selected, uint8_t margin, uint16_t startY, uint16_t sizeX, uint16_t sizeY);
    void showModeName(std::string& mode, int y);
    void noMapping();
    
};

#endif