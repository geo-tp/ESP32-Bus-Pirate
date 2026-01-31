#pragma once

#include <vector>
#include "Interfaces/ITerminalView.h"
#include "Services/SpiService.h" 
#include "Services/SdService.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Shells/SdCardShell.h"
#include "Shells/SpiFlashShell.h"
#include "States/GlobalState.h"
#include "Data/FlashDatabase.h"
#include "Shells/SpiEepromShell.h"
#include "Shells/HelpShell.h"

class SpiController {
public:
    // Constructor
    SpiController(
        ITerminalView& terminalView, 
        IInput& terminalInput, SpiService& 
        spiService, SdService& sdService, 
        ArgTransformer& argTransformer, 
        UserInputManager& userInputManager, 
        BinaryAnalyzeManager& binaryAnalyzeManager, 
        SdCardShell& sdCardShell,
        SpiFlashShell& spiFlashShell,
        SpiEepromShell& spiEepromShell,
        HelpShell& helpShell
    ); 

    // Entry point for handle raw user command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for handle parsed instruction bytecodes
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure SPI is properly configured before use
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    SpiService& spiService;
    SdService& sdService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    BinaryAnalyzeManager& binaryAnalyzeManager;
    SdCardShell& sdCardShell;
    SpiFlashShell& spiFlashShell;
    SpiEepromShell& spiEepromShell;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;

    // Passive bus monitor
    void handleSniff();

    // Handle SPI flash operations
    void handleFlash(const TerminalCommand& cmd);

    // Handle EEPROM operations
    void handleEeprom(const TerminalCommand& cmd);

    // SD operations
    void handleSdCard();

    // Slave mode
    void handleSlave();

    // Configure SPI bus parameters
    void handleConfig();

    // Available commands
    void handleHelp();
};
