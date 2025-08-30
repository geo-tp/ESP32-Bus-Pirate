#include "OneWireEepromShell.h"

OneWireEepromShell::OneWireEepromShell(
    ITerminalView& view,
    IInput& input,
    OneWireService& oneWireService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    BinaryAnalyzeManager& binaryAnalyzeManager
) :
    terminalView(view),
    terminalInput(input),
    oneWireService(oneWireService),
    argTransformer(argTransformer),
    userInputManager(userInputManager),
    binaryAnalyzeManager(binaryAnalyzeManager)
{
}

void OneWireEepromShell::run() {
    cmdProbe();

    while (true) {
        terminalView.println("\n=== DS24/28 EEPROM Shell ===");
        int index = userInputManager.readValidatedChoiceIndex("Select EEPROM action", actions, 0);
        if (index == -1 || actions[index] == " 🚪 Exit Shell") break;

        switch (index) {
            case 0: cmdProbe(); break;
            case 1: cmdAnalyze(); break;
            case 2: cmdRead();  break;
            case 3: cmdWrite(); break;
            case 4: cmdDump();  break;
            case 5: cmdErase(); break;
            default: terminalView.println("Invalid action."); break;
        }
    }
    terminalView.println("Exiting 1-Wire EEPROM Shell...\n");
}

void OneWireEepromShell::cmdProbe() {
    terminalView.println("\nProbing 1-Wire EEPROM...");
    uint8_t id[8];

    if (!oneWireService.eeprom2431Probe(id)) {
        terminalView.println("\n ❌ No supported EEPROM detected");
        terminalView.println("    (DS2431, DS2433, DS28E01)");
        return;
    }

    if (!oneWireService.getEepromModelInfos(id, eepromModel, eepromSize, eepromPageSize)) {
        uint8_t familyCode = id[0];
        terminalView.println("\n ⚠️ Unknown family code: 0x" + argTransformer.toHex(familyCode, 2));
        eepromModel = "Unknown";
        eepromSize = 128;
        eepromPageSize = 8;
    }

    terminalView.println("\n ✅ Detected EEPROM: " + eepromModel + " (0x" + argTransformer.toHex(id[0], 2) + ")");
    terminalView.println(" Size: " + std::to_string(eepromSize) + " bytes | Page size: " + std::to_string(eepromPageSize) + " bytes");
}

void OneWireEepromShell::cmdRead() {
    terminalView.println("\n📖 Read EEPROM");

    auto addr = userInputManager.readValidatedUint32("Start address", 0);

    if (addr >= eepromSize) {
        terminalView.println("\n ❌ Invalid address.");
        return;
    }

    uint16_t len = userInputManager.readValidatedUint32("Number of bytes to read", 16);
    if (addr + len > eepromSize) len = eepromSize - addr;

    terminalView.println("");
    for (uint16_t i = 0; i < len; ++i) {
        uint8_t value = oneWireService.eeprom2431ReadByte(addr + i);
        terminalView.println("  [0x" + argTransformer.toHex(addr + i, 2) + "] = " + argTransformer.toHex(value, 2));
    }
}

void OneWireEepromShell::cmdWrite() {
    terminalView.println("\n✏️  Write EEPROM (page-wise)");

    uint16_t addr = userInputManager.readValidatedUint32("Start address", 0);
    if (addr >= eepromSize) {
        terminalView.println("\n ❌ Invalid start address.");
        return;
    }

    std::string hexStr = userInputManager.readValidatedHexString("Enter hex bytes (e.g., AA BB CC ...):", 0, true);
    std::vector<uint8_t> data = argTransformer.parseHexList(hexStr);

    if (addr + data.size() > eepromSize) {
        terminalView.println("\n ❌ Data exceeds EEPROM size.");
        return;
    }

    auto confirm = userInputManager.readYesNo("Confirm write at address 0x" + argTransformer.toHex(addr, 2) + "?", false);

    if (!confirm) {
        terminalView.println("\n ❌ Write cancelled.");
        return;
    }

    // Write row by row
    size_t offset = 0;
    while (offset < data.size()) {
        size_t chunkSize = std::min((size_t)8, data.size() - offset);
        uint16_t absoluteAddr = addr + offset;

        // Copy to 8-byte buffer
        uint8_t buffer[8] = {0};
        memcpy(buffer, data.data() + offset, chunkSize);

        // Calculate rowAddress = absoluteAddr / 8
        uint8_t rowAddr = absoluteAddr / 8;

        // Write the row
        bool ok = oneWireService.eeprom2431WriteRow(rowAddr, buffer, true);
        if (!ok) {
            terminalView.println("\n ❌ Write failed at row " + std::to_string(rowAddr));
            return;
        }

        offset += chunkSize;
    }

    terminalView.println("\n ✅ EEPROM write complete.");
}

void OneWireEepromShell::cmdDump() {
    terminalView.println("\n🗃️ EEPROM Dump: Reading entire memory...\n");

    const uint8_t bytesPerLine = 16;

    for (uint16_t addr = 0; addr < eepromSize; addr += bytesPerLine) {
        std::vector<uint8_t> line = oneWireService.eeprom2431Dump(addr, bytesPerLine);
        std::string formattedLine = argTransformer.toAsciiLine(addr, line);
        terminalView.println(formattedLine);

        // User interruption
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') {
            terminalView.println("\n ❌ Dump cancelled by user.");
            return;
        }
    }

    terminalView.println("\n ✅ EEPROM Dump Done.");
}

void OneWireEepromShell::cmdErase() {
    terminalView.println("\n💣 EEPROM Erase: Writing 0x00 to entire memory...");
    if (!userInputManager.readYesNo("Confirm erase?", false)) {
        terminalView.println("Erase cancelled.");
        return;
    }

    uint8_t buffer[8];
    memset(buffer, 0x00, sizeof(buffer));

    for (uint16_t addr = 0; addr < eepromSize; addr += 8) {
        uint8_t rowAddr = addr / 8;

        bool ok = oneWireService.eeprom2431WriteRow(rowAddr, buffer, true);
        if (!ok) {
            terminalView.println("\n ❌ Erase failed at row " + std::to_string(rowAddr));
            return;
        }
    }

    terminalView.println("\n ✅ EEPROM Erase Done.");
}

void OneWireEepromShell::cmdAnalyze() {
    terminalView.println("\n📊 Analyze 1-Wire EEPROM...");

    // Analyze chunked
    auto result = binaryAnalyzeManager.analyze(
        0, // Start address
        eepromSize,
        // Fetch function
        [&](uint32_t addr, uint8_t* buf, uint32_t len) {
            auto chunk = oneWireService.eeprom2431Dump(addr, len);
            memcpy(buf, chunk.data(), len);
        },
        32 // Block size
    );

    // Format summary and display results
    auto summary = binaryAnalyzeManager.formatAnalysis(result);
    terminalView.println(summary);

    if (!result.foundSecrets.empty()) {
        terminalView.println("\n  Detected secrets:");
        for (const auto& s : result.foundSecrets) terminalView.println("    " + s);
    }

    if (!result.foundFiles.empty()) {
        terminalView.println("\n  Detected file signatures:");
        for (const auto& f : result.foundFiles) terminalView.println("    " + f);
    } else {
        terminalView.println("\n  No known file signatures found.");
    }

    terminalView.println("\n ✅ Analyze complete.");
}