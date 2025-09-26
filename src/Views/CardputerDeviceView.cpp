#if defined(DEVICE_CARDPUTER)

#include "CardputerDeviceView.h"

// Reuse the M5DeviceView implementation
void CardputerDeviceView::drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer) {
    M5DeviceView::drawLogicTrace(pin, buffer);
}

#endif // DEVICE_CARDPUTER
