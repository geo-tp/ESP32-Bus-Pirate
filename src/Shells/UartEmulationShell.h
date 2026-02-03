#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Services/UartService.h"

class UartEmulationShell {
public:
    UartEmulationShell(
        ITerminalView& view,
        IInput& input,
        UartService& uartService,
        ArgTransformer& argTransformer,
        UserInputManager& userInputManager
    );

    void run();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    UartService& uartService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;

    // Emulation types
    void startEmulation();
    void emulateGps();
    void emulateBoot();
    void emulateShell(bool protectedShell);
    void emulateAt();

    // Helpers
    bool shouldStopByEnter();
    uint8_t nmeaChecksum(const char* payloadNoDollarNoStar);
    std::string nmeaWrap(const char* payloadNoDollarNoStar);

    // Device list
    inline static const char* kDevices[] = {
        " 🧭 GPS (NMEA)",
        " 🔌 Boot sequence",
        " 💬 User shell",
        " 🔒 Protected shell",
        " 📶 AT modem",
        " 🚪 Exit"
    };

    inline static const int kDeviceCount =
        sizeof(kDevices) / sizeof(kDevices[0]);
};
