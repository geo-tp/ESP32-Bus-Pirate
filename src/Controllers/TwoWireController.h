#pragma once

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/TwoWireService.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Shells/SmartCardShell.h"
#include "Shells/HelpShell.h"

class TwoWireController {
public:
    // Constructor
    TwoWireController(
        ITerminalView& terminalView, 
        IInput& terminalInput, 
        UserInputManager& userInputManager, 
        TwoWireService& twoWireService,
        SmartCardShell& smartCardShell,
        HelpShell& helpShell
    );

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for handle compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure 2WIRE is configured before use
    void ensureConfigured();

private:
    // Sniff 2wire bus
    void handleSniff();

    // Smartcard commands
    void handleSmartCard(const TerminalCommand& cmd);

    // User pin configuration
    void handleConfig();

    // Show available commands
    void handleHelp();

    ITerminalView& terminalView;
    IInput& terminalInput;
    UserInputManager& userInputManager;
    TwoWireService& twoWireService;
    SmartCardShell& smartCardShell;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
};
