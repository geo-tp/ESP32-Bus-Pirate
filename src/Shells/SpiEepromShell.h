#pragma once

#include "Services/SpiService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Managers/BinaryAnalyzeManager.h"
#include "States/GlobalState.h"

class SpiEepromShell {
public:
    SpiEepromShell(
        SpiService& spiService,
        ITerminalView& view,
        IInput& input,
        ArgTransformer& argTransformer,
        UserInputManager& userInputManager,
        BinaryAnalyzeManager& binaryAnalyzeManager
    );

    void run();

private:
    SpiService& spiService;
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    BinaryAnalyzeManager& binaryAnalyzeManager;
    GlobalState& state = GlobalState::getInstance();
    uint32_t eepromSize = 8192; // default
    uint16_t pageSize = 64; // default
    std::string eepromModel;

    inline static const char* actions[] = {
        " 🔍 Probe EEPROM",
        " 📊 Analyze EEPROM",
        " 📖 Read bytes",
        " ✏️  Write bytes",
        " 🗃️  Dump ASCII",
        " 🗃️  Dump RAW",
        " 💣 Erase EEPROM",
        " 🚪 Exit Shell"
    };

    static constexpr size_t actionsCount = sizeof(actions) / sizeof(actions[0]);

    inline static const char* models[] = {
        " 25X010 | 128 B  | 8 B page   | 1 B addr | blk: 0b | max 10 MHz",
        " 25X020 | 256 B  | 8 B page   | 1 B addr | blk: 0b | max 10 MHz",
        " 25X040 | 512 B  | 8 B page   | 1 B addr | blk: 1b | max 10 MHz",
        " 25X080 | 1 KB   | 16 B page  | 2 B addr | blk: 0b | max 10 MHz",
        " 25X160 | 2 KB   | 16 B page  | 2 B addr | blk: 0b | max 10 MHz",
        " 25X320 | 4 KB   | 32 B page  | 2 B addr | blk: 0b | max 10 MHz",
        " 25X640 | 8 KB   | 32 B page  | 2 B addr | blk: 0b | max 10 MHz",
        " 25X128 | 16 KB  | 64 B page  | 2 B addr | blk: 0b | max 10 MHz",
        " 25X256 | 32 KB  | 64 B page  | 2 B addr | blk: 0b | max 10 MHz",
        "25X512 | 64 KB  | 128 B page | 2 B addr | blk: 0b | max 10 MHz",
        "25X1024| 128 KB | 256 B page | 3 B addr | blk: 0b | max 10 MHz",
        "25XM01 | 128 KB | 256 B page | 3 B addr | blk: 0b | max 10 MHz",
        "25XM02 | 256 KB | 256 B page | 3 B addr | blk: 0b | max 5 MHz",
        "25XM04 | 512 KB | 256 B page | 3 B addr | blk: 0b | max 8 MHz"
    };
    static constexpr size_t modelsCount = sizeof(models) / sizeof(models[0]);

    std::vector<uint32_t> memoryLengths = {
        128,      // 25X010
        256,      // 25X020
        512,      // 25X040
        1024,     // 25X080
        2048,     // 25X160
        4096,     // 25X320
        8192,     // 25X640
        16384,    // 25X128
        32768,    // 25X256
        65536,    // 25X512
        131072,   // 25X1024
        131072,   // 25XM01
        262144,   // 25XM02
        524288    // 25XM04
    };

    std::vector<uint16_t> pageLengths = {
        8,     // 25X010
        8,     // 25X020
        8,     // 25X040
        16,    // 25X080
        16,    // 25X160
        32,    // 25X320
        32,    // 25X640
        64,    // 25X128
        64,    // 25X256
        128,   // 25X512
        256,   // 25X1024
        256,   // 25XM01
        256,   // 25XM02
        256    // 25XM04
    };

    void cmdProbe();
    void cmdRead();
    void cmdWrite();
    void cmdDump(bool raw = false);
    void cmdErase();
    void cmdAnalyze();
};
