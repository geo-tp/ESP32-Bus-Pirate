#include "SpiEepromShell.h"

SpiEepromShell::SpiEepromShell(
    SpiService& spiService,
    ITerminalView& view,
    IInput& input,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    BinaryAnalyzeManager& binaryAnalyzeManager
) :
    spiService(spiService),
    terminalView(view),
    terminalInput(input),
    argTransformer(argTransformer),
    userInputManager(userInputManager),
    binaryAnalyzeManager(binaryAnalyzeManager)
{
}

void SpiEepromShell::run() {
    // Select EEPROM model
    int selectedModelIndex = userInputManager.readValidatedChoiceIndex("\nSelect EEPROM type", models, 0);
    if (selectedModelIndex < 0) {
        terminalView.println("Invalid selection. Aborting.\n");
        return;
    }

    // Get params
    eepromSize = memoryLengths[selectedModelIndex];
    pageSize = pageLengths[selectedModelIndex];
    eepromModel = models[selectedModelIndex];
    size_t p = eepromModel.find('|'); // Remove everything after the first '|'
    eepromModel.resize(p);
    bool isSmall = selectedModelIndex < 3; // Models 25X010, 25X020, 25X040 are small

    auto mosi = state.getSpiMOSIPin();
    auto miso = state.getSpiMISOPin();
    auto sclk = state.getSpiCLKPin();
    auto cs = state.getSpiCSPin();
    auto wp = 999; // Default write protect pin
    
    // Initialize EEPROM
    bool ok = spiService.initEeprom(mosi, miso, sclk, cs, pageSize, eepromSize, wp, isSmall);
    if (!ok) {
        terminalView.println("\nFailed to initialize EEPROM. Please check connections.");
        terminalView.println("HOLD pin must be connected to VCC to detect EEPROM.\n");
        return;
    }

    while (true) {
        terminalView.println("\n=== SPI EEPROM Shell ===");

        // Select action
        int index = userInputManager.readValidatedChoiceIndex("Select EEPROM action", actions, 0);

        // Quit
        if (index == -1 || actions[index] == " 🚪 Exit Shell") {
            terminalView.println("Exiting SPI EEPROM Shell...\n");
            break;
        }
        // Run selected action
        switch (index) {
            case 0: cmdProbe(); break;
            case 1: cmdAnalyze(); break;
            case 2: cmdRead();  break;
            case 3: cmdWrite(); break;
            case 4: cmdDump();  break;
            case 5: cmdDump(true); break;
            case 6: cmdErase(); break;
            default:
                terminalView.println("Unknown action.");
                break;
        }
    }
    spiService.closeEeprom();
}

void SpiEepromShell::cmdProbe() {
    terminalView.println("\n[INFO] Probing SPI EEPROM...");

    const bool ok = spiService.probeEeprom();

    if (ok) {
        terminalView.println("\n ✅ EEPROM detected.");
        terminalView.println(" Model     :" + eepromModel);
        terminalView.println(" Size      : " + std::to_string(eepromSize / 1024) + " Kbytes");
        terminalView.println(" Page Size : " + std::to_string(pageSize) + " bytes");
    } else {
        terminalView.println("\n ❌ No EEPROM found.");
    }
}

void SpiEepromShell::cmdRead() {
    terminalView.println("\n📖 Read EEPROM");

    auto addrStr = userInputManager.readValidatedHexString("Start address (e.g., 00FF00) ", 0, true);
    uint32_t addr = argTransformer.parseHexOrDec32("0x" + addrStr);

    if (addr >= eepromSize) {
        terminalView.println("\n ❌ Error: Start address is beyond EEPROM size.\n");
        return;
    }

    uint32_t count = userInputManager.readValidatedUint32("Number of bytes to read:", 16);
    if (addr + count > eepromSize) {
        count = eepromSize - addr;
    }

    terminalView.println("");
    const uint8_t bytesPerLine = 16;

    for (uint32_t i = 0; i < count; i += bytesPerLine) {
        uint8_t buffer[bytesPerLine];
        uint8_t lenToRead = std::min<uint32_t>(bytesPerLine, count - i);

        bool ok = spiService.readEepromBuffer(addr + i, buffer, lenToRead);
        if (!ok) {
            terminalView.println("\n ❌ Read failed at 0x" + argTransformer.toHex(addr + i, 6));
            return;
        }

        std::vector<uint8_t> line(buffer, buffer + lenToRead);
        std::string formattedLine = argTransformer.toAsciiLine(addr + i, line);
        terminalView.println(formattedLine);
    }

    terminalView.println("");
}


void SpiEepromShell::cmdWrite() {
    terminalView.println("\n✏️  Write EEPROM");

    uint32_t addr = userInputManager.readValidatedUint32("Start address:", 0);

    if (userInputManager.readYesNo("Write an ASCII string?", true)) {
        terminalView.print("Enter ASCII string: ");
        std::string input = userInputManager.getLine();
        std::string decoded = argTransformer.decodeEscapes(input);
        bool ok = spiService.writeEepromBuffer(addr, (const uint8_t*)decoded.data(), decoded.size());
        terminalView.println(ok ? "\n ✅ Write OK" : "\n ❌ Write failed");
    } else {
        std::string hexStr = userInputManager.readValidatedHexString("Enter hex bytes (e.g., AA BB CC) ", 0, true);
        std::vector<uint8_t> data = argTransformer.parseHexList(hexStr);
        bool ok = spiService.writeEepromBuffer(addr, data.data(), data.size());
        terminalView.println(ok ? "\n ✅ Write OK" : "\n ❌ Write failed");
    }
}

void SpiEepromShell::cmdDump(bool raw) {
    terminalView.println("\n🗃️ EEPROM Dump: Reading entire memory...");

    if (raw) {
        auto confirm = userInputManager.readYesNo("The raw dump is for python scripts. Continue?", false);
        if (!confirm) return;
    }

    const uint32_t totalSize = eepromSize;
    const uint32_t lineSize  = 16;
    uint8_t buffer[lineSize];

    for (uint32_t addr = 0; addr < totalSize; addr += lineSize) {
        // Read
        bool ok = spiService.readEepromBuffer(addr, buffer, lineSize);
        if (!ok) {
            if (!raw) {
                terminalView.println("\n ❌ Read failed at 0x" + argTransformer.toHex(addr, 6));
            }
            return;
        }

        if (raw) {
            // Mode RAW
            for (uint32_t i = 0; i < lineSize; i++) terminalView.print(buffer[i]);
        } else {
            // Mode ASCII 
            std::vector<uint8_t> line(buffer, buffer + lineSize);
            std::string formattedLine = argTransformer.toAsciiLine(addr, line);
            terminalView.println(formattedLine);

            // Cancel
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\n ❌ Dump cancelled by user.");
                return;
            }
        }
    }

    if (!raw) {
        terminalView.println("\n ✅ EEPROM Dump Done.");
    }
}

void SpiEepromShell::cmdErase() {
    terminalView.println("\n💣 EEPROM Erase: Writing 0xFF to entire memory...");

    if (!userInputManager.readYesNo("Confirm erase?", false)) {
        terminalView.println("Erase cancelled.");
        return;
    }

    const uint32_t totalSize = eepromSize;
    const uint32_t blockSize = 64;
    uint8_t ff[blockSize];
    std::fill_n(ff, blockSize, 0xFF);
    
    terminalView.print("Erasing");
    for (uint32_t addr = 0; addr < totalSize; addr += blockSize) {
        bool ok = spiService.writeEepromBuffer(addr, ff, blockSize);
        if (!ok) {
            terminalView.println("\n ❌ Write failed at 0x" + argTransformer.toHex(addr, 6));
            return;
        }

        // Progression feedback
        if (addr % 1024 == 0) terminalView.print(".");
    }

    terminalView.println("\r\n\n ✅ EEPROM Erase Done.");
}

void SpiEepromShell::cmdAnalyze() {
    terminalView.println("\nSPI EEPROM Analyze: from 0x00000000... Press [ENTER] to stop.");

    if (!spiService.probeEeprom()) {
        terminalView.println("\n ❌ No EEPROM found. Aborting.");
        return;
    }

    // Analyze EEPROM in chunks
    auto result = binaryAnalyzeManager.analyze(
        0,
        eepromSize,
        [&](uint32_t addr, uint8_t* buf, uint32_t len) {
            if (!spiService.readEepromBuffer(addr, buf, len)) {
                memset(buf, 0xFF, len);
            }
        }
    );

    // Summary
    auto summary = binaryAnalyzeManager.formatAnalysis(result);
    terminalView.println(summary);

    // Secrets
    if (!result.foundSecrets.empty()) {
        terminalView.println("\n  Detected sensitive patterns:");
        for (const auto& entry : result.foundSecrets) {
            terminalView.println("    " + entry);
        }
    }

    // Files
    if (!result.foundFiles.empty()) {
        terminalView.println("\n  Detected file signatures:");
        for (const auto& entry : result.foundFiles) {
            terminalView.println("    " + entry);
        }
    } else {
        terminalView.println("\n No known file signatures found.");
    }

    terminalView.println("\n ✅ SPI EEPROM Analyze: Done.");
}
