#pragma once

#include <vector>
#include <string>
#include <numeric>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IDeviceView.h"
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
                     IDeviceView& deviceView,
                     SubGhzService& subGhzService,
                     ArgTransformer& argTransformer,
                     UserInputManager& userInputManager)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceView(deviceView),
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

    // Replay captured frames
    void handleReplay(const TerminalCommand& cmd);

    // Jam signals
    void handleJam(const TerminalCommand& cmd);

    // Configure CC1101
    void handleConfig();

    // Available commands
    void handleHelp();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    IDeviceView& deviceView;
    SubGhzService& subGhzService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};
