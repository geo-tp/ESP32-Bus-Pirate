#ifdef DEVICE_C6DEVKIT

#include "Boards/C6DevKit/C6DevKitBoard.h"

void C6DevKitBoard::initialize() {
    deviceView.initialize();
}

IDeviceView& C6DevKitBoard::getDeviceView() {
    return deviceView;
}

IInput& C6DevKitBoard::getDeviceInput() {
    return deviceInput;
}

IHostSerial& C6DevKitBoard::getHostSerial() {
    return hostSerial;
}

#endif
