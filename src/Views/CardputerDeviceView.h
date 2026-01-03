#ifdef DEVICE_CARDPUTER

#pragma once

#include "M5DeviceView.h"

class CardputerDeviceView : public M5DeviceView {
public:
    // All disabled except drawLogicTrace
    void initialize() override {}
    SPIClass& getScreenSpiInstance() override { return *(SPIClass*)(nullptr); }
    void welcome(TerminalTypeEnum& /*terminalType*/, std::string& /*terminalInfos*/) override {}
    void logo() override {}
    void show(PinoutConfig& /*config*/) override {}
    void topBar(const std::string& /*title*/, bool /*submenu*/, bool /*searchBar*/) override {}
    void horizontalSelection(const std::vector<std::string>& /*options*/,
                             uint16_t /*selectedIndex*/,
                             const std::string& /*description1*/,
                             const std::string& /*description2*/) override {}
    void loading() override {}

    // Only this one is implemented
    void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
    void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
};

#endif // DEVICE_CARDPUTER
