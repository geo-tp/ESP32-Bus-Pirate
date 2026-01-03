// IDeviceView.h
#pragma once

#include <string>
#include "Enums/ModeEnum.h"
#include "Enums/TerminalTypeEnum.h"
#include "Models/PinoutConfig.h"
#include <SPI.h>

class IDeviceView {
public:
    virtual ~IDeviceView() = default;

    // Initiliaze the view
    virtual void initialize() = 0;

    // Get the SPI instance
    virtual SPIClass& getScreenSpiInstance() = 0;

    // Show logo
    virtual void logo() = 0;

    // Show Welcome screen
    virtual void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) = 0;

    // Show the mode and pinout
    virtual void show(PinoutConfig& config) = 0;

    // Show loading
    virtual void loading() = 0;

    // Clear the view
    virtual void clear() = 0;

    // Logic analyzer
    virtual void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) = 0;

    // Analogic plotter
    virtual void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) = 0;

    // Set screen rotation
    virtual void setRotation(uint8_t rotation) = 0;

    // Display a main title
    virtual void topBar(const std::string& title, bool submenu, bool searchBar) = 0;

    // Select an item horizontaly
    virtual void horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) = 0;
};
