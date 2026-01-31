#pragma once

// Placeholder implementation for 3WIRE controller
// Functionality to be added in future versions

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "States/GlobalState.h"
#include "Services/ThreeWireService.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Managers/UserInputManager.h"
#include "Transformers/ArgTransformer.h"
#include "Shells/ThreeWireEepromShell.h"
#include "Shells/HelpShell.h"

class ThreeWireController {
public:
    ThreeWireController(
        ITerminalView& terminalView,
        IInput& terminalInput,
        UserInputManager& userInputManager,
        ThreeWireService& threeWireService,
        ArgTransformer& argTransformer,
        ThreeWireEepromShell& threeWireEepromShell,
        HelpShell& helpShell
    );

    // Entry point for command handling
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for instruction handling
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure configured before any action
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    ThreeWireService threeWireService;
    UserInputManager& userInputManager;
    ArgTransformer& argTransformer;
    ThreeWireEepromShell& threeWireEepromShell;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    bool org8 = true;

    // Command handlers for eeprom operations
    void handleEeprom(const TerminalCommand& cmd);

    // Configuration handler for settings
    void handleConfig();

    // Available commands
    void handleHelp();
    
};