#pragma once

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/LedService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Shells/HelpShell.h"

class LedController {
public:
    LedController(ITerminalView& terminalView, IInput& terminalInput,
                  LedService& ledService, ArgTransformer& argTransformer,
                  UserInputManager& userInputManager, HelpShell& helpShell);

    // Dispatch user command for LED mode
    void handleCommand(const TerminalCommand& cmd);

    // Execute LED instruction from bytecode
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure LED mode is properly configured before use
    void ensureConfigured();

private:
    // Try to autodetect LED protocol by scanning different types
    void handleScan();

    // Fill all LEDs with a single color
    void handleFill(const TerminalCommand& cmd);

    // Set a specific LED index to a specific color
    void handleSet(const TerminalCommand& cmd);

    // Turn off all LEDs and clear the strip
    void handleReset(const TerminalCommand& cmd);

    // Run a predefined LED animation
    void handleAnimation(const TerminalCommand& cmd);

    // Configure LED pin, length and protocol
    void handleConfig();

    // Change LED protocol interactively
    void handleSetProtocol();

    // Display help for available LED commands
    void handleHelp();

    // Convert color arguments to CRGB object
    CRGB parseFlexibleColor(const std::vector<std::string>& args);

    ITerminalView& terminalView;
    IInput& terminalInput;
    LedService& ledService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};