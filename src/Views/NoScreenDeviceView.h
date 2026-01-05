#pragma once

#include "Interfaces/IDeviceView.h"
#include <iostream>
#include <SPI.h>

class NoScreenDeviceView : public IDeviceView {
public:
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
};
