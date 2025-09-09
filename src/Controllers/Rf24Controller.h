#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Interfaces/IDeviceView.h"
#include "Models/TerminalCommand.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Services/Rf24Service.h"
#include "Services/PinService.h"
#include "Data/Rf24Channels.h"

class Rf24Controller {
public:
    Rf24Controller(ITerminalView& terminalView,
                   IInput& terminalInput,
                   IDeviceView& deviceView,
                   Rf24Service& rf24Service,
                   PinService& pinService,
                   ArgTransformer& argTransformer,
                   UserInputManager& userInputManager)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceView(deviceView),
      rf24Service(rf24Service),
      pinService(pinService),
      argTransformer(argTransformer),
      userInputManager(userInputManager) {}

    // Entry point for rf24 commands
    void handleCommand(const TerminalCommand& cmd);

    // Ensure NRF24 is configured before use
    void ensureConfigured();

private:
    // Command handlers
    void handleConfig();
    void handleSniff(); 
    void handleScan();
    void handleJam();
    void handleSweep();
    void handleSetChannel();
    void handleHelp();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    IDeviceView& deviceView;
    Rf24Service& rf24Service;
    PinService& pinService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    GlobalState& state = GlobalState::getInstance();

    bool configured = false;
};
