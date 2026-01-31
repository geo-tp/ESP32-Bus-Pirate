#pragma once

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/ByteCode.h"
#include "Models/TerminalCommand.h"
#include "Services/HdUartService.h"
#include "Services/UartService.h"
#include "Transformers/ArgTransformer.h"
#include "States/GlobalState.h"
#include "Managers/UserInputManager.h"
#include "Shells/HelpShell.h"

class HdUartController {
public:
    HdUartController(ITerminalView& terminalView, IInput& terminalInput, IInput& deviceInput,
                     HdUartService& hdUartService, UartService& uartService, ArgTransformer& argTransformer, 
                     UserInputManager& userInputManager, HelpShell& helpShell);
    
    // Entry point for HDUART command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for HDUART instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // HDUART config check
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    IInput& deviceInput;
    HdUartService& hdUartService;
    UartService& uartService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    
    bool configured = false;
    
    // HDUART Bridge mode read/write on one line
    void handleBridge();

    // Configure HDUART
    void handleConfig();

    // Show HDUART Available commands
    void handleHelp();
};
