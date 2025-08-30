#pragma once

#include "Services/OneWireService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Managers/BinaryAnalyzeManager.h"

class OneWireEepromShell {
public:
    OneWireEepromShell(
        ITerminalView& view,
        IInput& input,
        OneWireService& oneWireService,
        ArgTransformer& argTransformer,
        UserInputManager& userInputManager,
        BinaryAnalyzeManager& binaryAnalyzeManager
    );

    void run();

private:
    void cmdProbe();
    void cmdRead();
    void cmdWrite();
    void cmdDump();
    void cmdErase();
    void cmdAnalyze();

    OneWireService& oneWireService;
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    BinaryAnalyzeManager& binaryAnalyzeManager;

    const std::vector<std::string> actions = {
        " 🔍 Probe",
        " 📊 Analyze",
        " 📖 Read",
        " ✏️  Write",
        " 🗃️  Dump",
        " 💣 Erase",
        " 🚪 Exit Shell"
    };

    std::string eepromModel = "DS2431"; // default
    uint8_t eepromPageSize = 8;
    uint16_t eepromSize = 128;
};
