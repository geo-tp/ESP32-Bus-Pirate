#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "Interfaces/IInput.h"
#include "Interfaces/ITerminalView.h"
#include "Models/TerminalCommand.h"
#include "Services/RfidService.h"
#include "Managers/UserInputManager.h"
#include "Transformers/ArgTransformer.h"
#include "States/GlobalState.h"

class RfidController {
public:
    RfidController(
        ITerminalView& view,
        IInput& input,
        RfidService& rfidService,
        UserInputManager& uim,
        ArgTransformer& transformer
    );

    // Entry point for commands
    void handleCommand(const TerminalCommand& cmd);

    // Ensure the PN532 is configured
    void ensureConfigured();

private:
    // Command handlers
    void handleRead(const TerminalCommand& cmd);
    void handleWrite(const TerminalCommand& cmd);
    void handleWriteUid(); 
    void handleWriteBlock();
    void handleErase(const TerminalCommand& cmd);
    void handleClone(const TerminalCommand& cmd);
    void handleConfig();
    void handleHelp();

private:
bool configured = false;

    ITerminalView& terminalView;
    IInput& terminalInput;
    RfidService& rfidService;
    UserInputManager& userInputManager;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
};
