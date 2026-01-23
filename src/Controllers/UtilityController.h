#pragma once

#include <vector>
#include <Arduino.h>
#include <string>
#include <sstream>
#include <algorithm>
#include "Models/TerminalCommand.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IDeviceView.h"
#include "States/GlobalState.h"
#include "Enums/ModeEnum.h"
#include "Services/PinService.h"
#include "Managers/UserInputManager.h"
#include "Transformers/ArgTransformer.h"
#include "Shells/SysInfoShell.h"
#include "Shells/GuideShell.h"

class UtilityController {
public:
    // Constructor
    UtilityController(
        ITerminalView& terminalView, 
        IDeviceView& deviceView, 
        IInput& terminalInput, 
        PinService& pinService, 
        UserInputManager& userInputManager, 
        ArgTransformer& argTransformer,
        SysInfoShell& sysInfoShell,
        GuideShell& guideShell
    );

    // Entry point for global utility commands
    void handleCommand(const TerminalCommand& cmd);

    // Process mode change command and return new mode
    ModeEnum handleModeChangeCommand(const TerminalCommand& cmd);

    // Logic Analyzer on device screen
    void handleLogicAnalyzer(const TerminalCommand& cmd);

    // Analogic plotter on device screen
    void handleAnalogic(const TerminalCommand& cmd);

    // Check if a command is a global utility command
    bool isGlobalCommand(const TerminalCommand& cmd);

private:
    // Display help for utility commands
    void handleHelp();

    // Ask user to select a mode
    ModeEnum handleModeSelect();

    // Enable internal pull-up resistors
    void handleEnablePullups();

    // Disable internal pull-up resistors
    void handleDisablePullups();

    // System information
    void handleSystem();

    // Firmware guide
    void handleGuide();

    ITerminalView& terminalView;
    IDeviceView& deviceView;
    IInput& terminalInput;
    PinService& pinService;
    UserInputManager& userInputManager;
    ArgTransformer& argTransformer;
    SysInfoShell& sysInfoShell;
    GuideShell& guideShell;
    GlobalState& state = GlobalState::getInstance();
};
