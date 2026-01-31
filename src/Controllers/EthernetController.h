#pragma once

#include <Arduino.h>
#include <string>
#include <vector>
#include <array>

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/EthernetService.h"
#include "Services/SshService.h"
#include "Services/NetcatService.h"
#include "Services/NmapService.h"
#include "Services/ICMPService.h"
#include "Transformers/ArgTransformer.h"
#include "Models/TerminalCommand.h"
#include "States/GlobalState.h"
#include "Managers/UserInputManager.h"
#include "Abstracts/ANetworkController.h"

class EthernetController  : public ANetworkController {
public:
    using ANetworkController::ANetworkController;

    // Entry point for Ethernet command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure Ethernet is configured before any action
    void ensureConfigured();

private:
    // Configure the W5500
    void handleConfig();

    // Ethernet status
    void handleStatus();

    // Connect using DHCP
    void handleConnect();

    // Hard reset the W5500 (via RST)
    void handleReset();

    // Help message for Ethernet commands
    void handleHelp();

private:
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
};
