#pragma once

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Services/JtagService.h" 
#include "States/GlobalState.h"
#include "Managers/UserInputManager.h"
#include "Shells/HelpShell.h"

class JtagController {
public:
    // Constructor
    JtagController(ITerminalView& terminalView, IInput& terminalInput, JtagService& jtagService, UserInputManager& userInputManager, HelpShell& helpShell);

    // Entry point for dispatch incoming JTAG command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure configuration is done before running commands
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    JtagService& jtagService;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    bool configured = false;
    GlobalState& state = GlobalState::getInstance();

    // Perform devices scan
    void handleScan(const TerminalCommand& cmd);

    // SWD scan
    void handleScanSwd();

    // JTAG scan
    void handleScanJtag();

    // Handle user configuration
    void handleConfig();

    // Show available commands
    void handleHelp();
};
