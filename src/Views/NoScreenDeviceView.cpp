#include "NoScreenDeviceView.h"

void NoScreenDeviceView::initialize() {}

SPIClass& NoScreenDeviceView::getScreenSpiInstance() {
    return *(SPIClass*)(nullptr);
}

void NoScreenDeviceView::logo() {}

void NoScreenDeviceView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {}

void NoScreenDeviceView::show(PinoutConfig& config) {}

void NoScreenDeviceView::loading() {}

void NoScreenDeviceView::clear() {}

void NoScreenDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {}

void NoScreenDeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {}

void NoScreenDeviceView::setRotation(uint8_t rotation) {}

void NoScreenDeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {}

void NoScreenDeviceView::horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) {}
