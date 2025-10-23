#include "Views/NoScreenDeviceView.h"

void NoScreenDeviceView::initialize() {}

SPIClass& NoScreenDeviceView::getScreenSpiInstance() {
    return *(SPIClass*)(nullptr);
}

void NoScreenDeviceView::logo() {}

void NoScreenDeviceView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {}

void NoScreenDeviceView::show(PinoutConfig& config) {}

void NoScreenDeviceView::loading() {}

void NoScreenDeviceView::clear() {}

void NoScreenDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer) {
    // fdufnews 2025/10/22 added a poor man's drawLogicTrace() for Serial
    // draw logic trace on 3 consecutive lines in order to be compatible with small 80x25 terminal window
    Serial.println("Logic trace");
    uint8_t pos = 0;
    for(size_t i = 0; i < buffer.size(); i++, pos++){
        if(pos > 79){
            pos = 0;
            Serial.println();
        }
        Serial.write(buffer[i]?'-':'_');
    }
    uint8_t up3[] = {'\r', 0x1b, '[', '3', 'A'}; // move cursor 3 lines up
    Serial.write(up3, 5);  // to put cursor at the correct place for the next draw
}

void NoScreenDeviceView::setRotation(uint8_t rotation) {}

void NoScreenDeviceView::topBar(const std::string& title, bool submenu, bool searchBar) {}

void NoScreenDeviceView::horizontalSelection(
        const std::vector<std::string>& options,
        uint16_t selectedIndex,
        const std::string& description1,
        const std::string& description2) {}
