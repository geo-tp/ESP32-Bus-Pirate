#pragma once

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Managers/UserInputManager.h"
#include "Enums/ModeEnum.h"

class HelpShell {
public:
    HelpShell(ITerminalView& tv, IInput& in, UserInputManager& uim);

    void run(ModeEnum currentMode=ModeEnum::HIZ, bool select = true);

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    UserInputManager& userInputManager;

private:
    // menu
    void cmdGeneral();
    void cmdOneWire();
    void cmdUart();
    void cmdHdUart();
    void cmdI2c();
    void cmdSpi();
    void cmdTwoWire();
    void cmdThreeWire();
    void cmdDio();
    void cmdLed();
    void cmdInfrared();
    void cmdUsb();
    void cmdBluetooth();
    void cmdWifi();
    void cmdJtag();
    void cmdI2s();
    void cmdCan();
    void cmdEthernet();
    void cmdSubGhz();
    void cmdRfid();
    void cmdRf24();
    void cmdAll();

    // helpers
    void printHeader(const char* title);
    void printLines(const char* const* lines, int count);
};
