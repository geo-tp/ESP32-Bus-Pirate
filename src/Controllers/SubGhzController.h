#pragma once

#include <vector>
#include <string>
#include <numeric>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Services/SubGhzService.h"
#include <sstream>
#include <cctype>

class SubGhzController {
public:
    SubGhzController(ITerminalView& terminalView,
                     IInput& terminalInput,
                     SubGhzService& subGhzService,
                     ArgTransformer& argTransformer,
                     UserInputManager& userInputManager)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      subGhzService(subGhzService),
      argTransformer(argTransformer),
      userInputManager(userInputManager) {}

    // Entry point for subghz commands
    void handleCommand(const TerminalCommand& cmd);

    // Ensure subghz mode is properly configured before use
    void ensureConfigured();

private:
    // Sniff for signals
    void handleSniff(const TerminalCommand& cmd);

    // Scan for frequencies
    void handleScan(const TerminalCommand& cmd);

    // Set current frequency
    void handleSetFrequency();

    // Configure CC1101
    void handleConfig();

    // Available commands
    void handleHelp();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    SubGhzService& subGhzService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};
