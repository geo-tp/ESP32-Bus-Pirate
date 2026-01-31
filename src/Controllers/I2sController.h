#pragma once

#include <Arduino.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/I2sService.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Models/TerminalCommand.h"
#include "States/GlobalState.h"
#include "Data/PcmSoundTestComplete.h"
#include "Shells/HelpShell.h"

class I2sController {
public:
    I2sController(ITerminalView& terminalView, IInput& terminalInput,
                  I2sService& i2sService, ArgTransformer& argTransformer,
                  UserInputManager& userInputManager, HelpShell& helpShell);

    // Entry point for I2S cmd
    void handleCommand(const TerminalCommand& cmd);

    // Ensore I2S config before any action
    void ensureConfigured();

private:
    // Configure I2S pins and parameters interactively
    void handleConfig();

    // Play a tone at given frequency and optional duration
    void handlePlay(const TerminalCommand& cmd);

    // Record audio from I2S mic and display signal preview
    void handleRecord(const TerminalCommand& cmd);

    // Run a full I2S test (speaker + mic)
    void handleTest(const TerminalCommand& cmd);

    // Test I2S microphone and analyze input signal
    void handleTestMic();

    // Play test sounds on I2S speaker
    void handleTestSpeaker();

    // Reset I2S to output mode
    void handleReset();

    // Show available I2S commands
    void handleHelp();

    ITerminalView& terminalView;
    IInput& terminalInput;
    I2sService& i2sService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
};
