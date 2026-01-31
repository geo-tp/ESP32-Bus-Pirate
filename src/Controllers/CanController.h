#pragma once

#include "Arduino.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Services/CanService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Shells/HelpShell.h"

class CanController {
public:
    CanController(ITerminalView& terminalView, IInput& terminalInput, UserInputManager& userInputManager,
                  CanService& canService, ArgTransformer& argTransformer, HelpShell& helpShell);
    
    // Entry point to handle CAN commands
    void handleCommand(const TerminalCommand& cmd);

    // Ensure the CAN controller is configured
    void ensureConfigured();
    
private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    CanService& canService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    
    // Sniffing all CAN frames
    void handleSniff();

    // Status of the CAN controller
    void handleStatus();

    // Sending a CAN frame with specific ID
    void handleSend(const TerminalCommand& cmd);

    // Receiving a CAN frame with specific ID
    void handleReceive(const TerminalCommand& cmd);

    // Configuring the CAN controller
    void handleConfig();

    // Help message for CAN commands
    void handleHelp();
};
