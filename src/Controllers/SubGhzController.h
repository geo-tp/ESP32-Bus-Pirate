#pragma once

#include <vector>
#include <string>
#include <numeric>
#include <sstream>
#include <cctype>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IDeviceView.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/ArgTransformer.h"
#include "Transformers/SubGhzTransformer.h"
#include "Managers/UserInputManager.h"
#include "Managers/SubGhzAnalyzeManager.h"
#include "States/GlobalState.h"
#include "Services/SubGhzService.h"
#include "Services/PinService.h"
#include "Services/LittleFsService.h"
#include "Services/I2sService.h"
#include "Data/SubGhzProtocols.h"

class SubGhzController {
public:
    SubGhzController(ITerminalView& terminalView,
                     IInput& terminalInput,
                     IDeviceView& deviceView,
                     SubGhzService& subGhzService,
                     PinService& pinService,
                     I2sService& i2sService,
                     LittleFsService& littleFsService,
                     ArgTransformer& argTransformer,
                     SubGhzTransformer& subGhzTransformer,
                     UserInputManager& userInputManager,
                     SubGhzAnalyzeManager& subGhzAnalyzeManager)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceView(deviceView),
      subGhzService(subGhzService),
      pinService(pinService),
      i2sService(i2sService),
      littleFsService(littleFsService),
      argTransformer(argTransformer),
      subGhzTransformer(subGhzTransformer),
      userInputManager(userInputManager),
      subGhzAnalyzeManager(subGhzAnalyzeManager) {}

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

    // Band Jam
    void handleBandJam();

    // Decode signals
    void handleDecode(const TerminalCommand& cmd);

    // Show signal trace
    void handleTrace();

    // Sweep and analyze signals
    void handleSweep();

    // Bruteforce attack
    void handleBruteforce();

    // Load .sub files
    void handleLoad();

    // Convert RSSI to audio
    void handleListen();

    // Configure CC1101
    void handleConfig();

    // Available commands
    void handleHelp();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    IDeviceView& deviceView;
    SubGhzService& subGhzService;
    PinService& pinService;
    I2sService& i2sService;
    LittleFsService& littleFsService;
    ArgTransformer& argTransformer;
    SubGhzTransformer& subGhzTransformer;
    UserInputManager& userInputManager;
    SubGhzAnalyzeManager& subGhzAnalyzeManager;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};
