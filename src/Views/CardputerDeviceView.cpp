#if defined(DEVICE_CARDPUTER)

#include "CardputerDeviceView.h"

// Reuse the M5DeviceView implementation
void CardputerDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    M5DeviceView::drawLogicTrace(pin, buffer, step);
}

void CardputerDeviceView::drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) {
    M5DeviceView::drawAnalogicTrace(pin, buffer, step);
}
#endif // DEVICE_CARDPUTER
