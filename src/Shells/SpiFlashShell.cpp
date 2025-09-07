#include "SpiFlashShell.h"

SpiFlashShell::SpiFlashShell(
    SpiService& spiService,
    ITerminalView& view,
    IInput& input,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    BinaryAnalyzeManager& binaryAnalyzeManager
)
    : spiService(spiService),
      terminalView(view),
      terminalInput(input),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      binaryAnalyzeManager(binaryAnalyzeManager)
{
    // Nothing
}

void SpiFlashShell::run() {
    while (true) {
        terminalView.println("\n=== SPI Flash Shell ===");

        // Select action
        int index = userInputManager.readValidatedChoiceIndex("Select a SPI Flash action", actions, 0);

        // Quit
        if (index == -1 || actions[index] == "🚪 Exit Shell") {
            terminalView.println("Exiting SPI Flash Shell...\n");
            break;
        }

        // Dispatch
        switch (index) {
            case 0: cmdProbe();   break;
            case 1: cmdAnalyze(); break;
            case 2: cmdSearch();  break;
            case 3: cmdStrings(); break;
            case 4: cmdRead();    break;
            case 5: cmdWrite();   break;
            case 6: cmdDump();    break;
            case 7: cmdDump(true); break;
            case 8: cmdErase();   break;
            default:
                terminalView.println("Unknown action.\n");
                break;
        }
    }
}

/*
Flash Probe
*/
void SpiFlashShell::cmdProbe() {
    uint8_t id[3] = {0};
    spiService.readFlashIdRaw(id);

    std::stringstream idStr;
    terminalView.println("");
    idStr << "SPI Flash ID: "
          << std::hex << std::uppercase << std::setfill('0')
          << std::setw(2) << (int)id[0] << " "
          << std::setw(2) << (int)id[1] << " "
          << std::setw(2) << (int)id[2];
    terminalView.println(idStr.str());

    // Check for common invalid responses
    if ((id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00) ||
        (id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF)) {    
        terminalView.println("No SPI flash detected (bus error or missing chip).");
        return;
    }

    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);

    // Known in database
    if (chip) {
        terminalView.println("Manufacturer: " + std::string(chip->manufacturerName));
        terminalView.println("Model: " + std::string(chip->modelName));
        terminalView.println("Capacity: " +
            std::to_string(chip->capacityBytes / (1024UL * 1024UL)) + " MB\n");
        return;
    }

    // Fallback, not a known chip
    const char* manufacturer = findManufacturerName(id[0]);
    terminalView.println("Manufacturer: " + std::string(manufacturer));

    // Estimate Capacity
    uint32_t size = 1UL << id[2];
    std::stringstream sizeStr;
    if (size >= (1024 * 1024)) {
        sizeStr << (size / (1024 * 1024)) << " MB (guessed)";
    } else {
        sizeStr << size << " bytes (guessed)";
    }
    terminalView.println("Estimated capacity: " + sizeStr.str());
    terminalView.println("");
}

/*
Flash Analyze
*/
void SpiFlashShell::cmdAnalyze() {
    if (!checkFlashPresent()) return;

    // Start address if any
    uint32_t start = 0;
    terminalView.println("\nSPI Flash Analyze: SPI Flash from 0x00000000... Press [ENTER] to stop.");

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Analyze
    BinaryAnalyzeManager::AnalysisResult result = binaryAnalyzeManager.analyze(
        0,
        flashSize,
        [&](uint32_t addr, uint8_t* buf, uint32_t len) {
            spiService.readFlashData(addr, buf, len);
        }
    );

    // Calculate Summary
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
        terminalView.println("\n  No known file signatures found.");
    }

    terminalView.println("\n  SPI Flash Analyze: Done.\n");
}

/*
Flash Strings
*/
void SpiFlashShell::cmdStrings() {
    // Check chip presence
    if (!checkFlashPresent()) return;

    // Validate and parse args
    uint8_t minStringLen = userInputManager.readValidatedUint8("Min. length of the strings:", 10);

    terminalView.println("\nSPI Flash: Extracting strings... Press [ENTER] to stop.\n");


    const uint32_t blockSize = 512;
    uint8_t buffer[blockSize];
    std::string currentStr;
    uint32_t currentAddr = 0;
    uint32_t stringStartAddr = 0;
    bool inString = false;

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Read flash in chuncks
    for (uint32_t addr = 0; addr < flashSize; addr += blockSize) {
        spiService.readFlashData(addr, buffer, blockSize);

        // Read blocks
        for (uint32_t i = 0; i < blockSize; ++i) {
            uint8_t b = buffer[i];
            uint32_t absoluteAddr = addr + i;

            if (b >= 32 && b <= 126) {  // printable ASCII
                if (!inString) {
                    inString = true;
                    stringStartAddr = absoluteAddr;
                }
                currentStr += static_cast<char>(b);
            } else {
                if (inString && currentStr.length() >= minStringLen) {
                    terminalView.println(
                        "0x" + argTransformer.toHex(stringStartAddr, 6) + ": " + currentStr
                    );
                }
                currentStr.clear();
                inString = false;
            }

            // Quit if user presses ENTER
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nSPI Flash: Extraction cancelled by user.");
                return;
            }
        }
    }

    // if remaining string
    if (inString && currentStr.length() >= minStringLen) {
        terminalView.println(
            "0x" + argTransformer.toHex(stringStartAddr, 6) + ": " + currentStr
        );
    }

    terminalView.println("\nSPI Flash: String extraction complete.\n");
}

/*
Flash Search
*/
void SpiFlashShell::cmdSearch() {
    // Check chip presence
    if (!checkFlashPresent()) return;

    auto startAddr = 0;

    // Search pattern
    terminalView.print("Enter string search pattern: ");
    std::string pattern = userInputManager.getLine();

    terminalView.println("\nSearching for \"" + pattern + "\" in SPI flash from 0x" + argTransformer.toHex(startAddr, 6) + "... Press [ENTER] to stop.\n");

    const uint32_t blockSize = 512;
    const uint32_t contextSize = 16;  // characters before and after
    uint8_t buffer[blockSize + 32];

    // Get flash size
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashSize = chip ? chip->capacityBytes : spiService.calculateFlashCapacity(id[2]);

    // Read flash in chunks
    for (uint32_t addr = startAddr; addr < flashSize; addr += blockSize - pattern.size()) {
        spiService.readFlashData(addr, buffer, blockSize + pattern.size() - 1);
        
        // Read block
        for (uint32_t i = 0; i <= blockSize; ++i) {
            if (i + pattern.size() > blockSize + pattern.size() - 1) break;

            bool match = true;
            for (size_t j = 0; j < pattern.size(); ++j) {
                if (buffer[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            
            // Found a matching string
            if (match) {
                uint32_t matchAddr = addr + i;
                std::string context;

                // Before the pattern
                for (int j = (int)i - (int)contextSize; j < (int)i; ++j) {
                    if (j >= 0) {
                        char c = (char)buffer[j];
                        context += (isprint(c) ? c : '.');
                    }
                }

                // Pattern
                context += "[";
                for (size_t j = 0; j < pattern.size(); ++j) {
                    char c = (char)buffer[i + j];
                    context += (isprint(c) ? c : '.');
                }
                context += "]";

                // After the pattern
                for (uint32_t j = i + pattern.size(); j < i + pattern.size() + contextSize && j < blockSize + pattern.size(); ++j) {
                    char c = (char)buffer[j];
                    context += (isprint(c) ? c : '.');
                }

                terminalView.println("0x" + argTransformer.toHex(matchAddr, 6) + ": " + context);
            }

            // Allow user to interrupt
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nSPI Flash Search: Cancelled by user.\n");
                return;
            }
        }
    }

    terminalView.println("\nSearch complete.");
}

/*
Flash Read
*/
void SpiFlashShell::cmdRead() {
    // Check chip presence
    if (!checkFlashPresent()) return;
    
    auto addrStr = userInputManager.readValidatedHexString("Start address (e.g., 00FF00) ", 0, true);
    auto address = argTransformer.parseHexOrDec16("0x" + addrStr);
    uint32_t count = userInputManager.readValidatedUint32("Number of bytes to read:", 16);

    // Read flash in chunks
    terminalView.println("SPI Flash Read: In progress... Press [ENTER] to stop");
    terminalView.println("");
    readFlashInChunks(address, count);
    terminalView.println("");
}

/*
Flash Read In Chunks
*/
void SpiFlashShell::readFlashInChunks(uint32_t address, uint32_t length) {
    uint8_t buffer[1024];
    uint32_t remaining = length;
    uint32_t currentAddr = address;

    // Display chunks
    while (remaining > 0) {
        uint32_t chunkSize = (remaining > 1024) ? 1024 : remaining;
        spiService.readFlashData(currentAddr, buffer, chunkSize);

        for (uint32_t i = 0; i < chunkSize; i += 16) {
            std::stringstream line;

            // Address
            line << std::hex << std::uppercase << std::setfill('0')
                 << std::setw(6) << (currentAddr + i) << ": ";

            // Hex
            for (uint32_t j = 0; j < 16; ++j) {
                if (i + j < chunkSize) {
                    line << std::setw(2) << (int)buffer[i + j] << " ";
                } else {
                    line << "   ";
                }
            }

            // ASCII
            line << " ";
            for (uint32_t j = 0; j < 16; ++j) {
                if (i + j < chunkSize) {
                    char c = static_cast<char>(buffer[i + j]);
                    line << (isprint(c) ? c : '.');
                }
            }
            terminalView.println(line.str());

            // Check user ENTER to stop
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nRead interrupted by user.");
                return;
            }
        }

        currentAddr += chunkSize;
        remaining -= chunkSize;
    }
}

void SpiFlashShell::readFlashInChunksRaw(uint32_t address, uint32_t length) {
    uint8_t buffer[1024];
    uint32_t remaining = length;
    uint32_t current   = address;

    while (remaining > 0) {
        uint32_t n = (remaining > sizeof(buffer)) ? sizeof(buffer) : remaining;
        spiService.readFlashData(current, buffer, n);
        for (uint32_t i = 0; i < n; ++i) {
            terminalView.print(buffer[i]);
        }
        current   += n;
        remaining -= n;
    }
}

uint32_t SpiFlashShell::readFlashCapacity() {
    // Verify flash capacity
    uint8_t id[3];
    spiService.readFlashIdRaw(id);
    const FlashChipInfo* chip = findFlashInfo(id[0], id[1], id[2]);
    uint32_t flashCapacity = 0;
    if (chip) {
        flashCapacity = chip->capacityBytes;
    } else {
        flashCapacity = spiService.calculateFlashCapacity(id[2]);
        std::stringstream capStr;
        capStr << "Estimated capacity from ID: " << (flashCapacity >> 20) << " MB";
        terminalView.println(capStr.str());
    }

    return flashCapacity;   
}

/*
Flash Write
*/
void SpiFlashShell::cmdWrite() {
    // Vérifie présence
    if (!checkFlashPresent()) return;

    // Adresse
    auto addrStr = userInputManager.readValidatedHexString("Start address (e.g., 00FF00) ", 0, true);
    auto addr = argTransformer.parseHexOrDec16("0x" + addrStr);

    std::vector<uint8_t> data;

    // Demander si on veut écrire une chaîne ASCII
    if (userInputManager.readYesNo("Write an ASCII string?", true)) {
        terminalView.println("Enter ASCII string (supports \\n, \\x41, etc):");
        std::string ascii = userInputManager.getLine();
        std::string decoded = argTransformer.decodeEscapes(ascii);
        data.assign(decoded.begin(), decoded.end());
    } else {
        // Liste d'octets hexadécimaux
        std::string hexStr = userInputManager.readValidatedHexString("Enter byte values (e.g., 01 A5 FF...) ", 0, true);
        data = argTransformer.parseHexList(hexStr);
    }

    // Confirmation
    if (!userInputManager.readYesNo("SPI Flash Write: Confirm write operation?", false)) {
        terminalView.println("SPI Flash Write: Cancelled.\n");
        return;
    }

    // Validation
    if (data.empty()) {
        terminalView.println("SPI Flash Write: Invalid data format.");
        return;
    }

    // Écriture
    terminalView.println("Writing " + std::to_string(data.size()) + " byte(s) at address 0x" +
                         argTransformer.toHex(addr, 6));

    uint32_t freq = state.getSpiFrequency();
    spiService.writeFlashPatch(addr, data, freq);

    terminalView.println("SPI Flash Write: Complete.\n");
}

/*
Flash Erase
*/
void SpiFlashShell::cmdErase() {
    // Check chip presence
    if (!checkFlashPresent()) return;
    
    terminalView.println("");
    if (!userInputManager.readYesNo("SPI Flash Erase: Erase entire flash memory?", false)) {
        terminalView.println("SPI Flash Erase: Cancelled.\n");
        return;
    }

    uint32_t freq = state.getSpiFrequency();
    const uint32_t sectorSize = 4096; // standard
    uint32_t flashSize = readFlashCapacity();

    // Erase sectors and display progression
    const uint32_t totalSectors = flashSize / sectorSize;
    terminalView.print("In progress");
    for (uint32_t i = 0; i < totalSectors; ++i) {
        uint32_t addr = i * sectorSize;
        spiService.eraseFlashSector(addr, freq);

        // Display a dot
        if (i % 64 == 0) terminalView.print(".");
    }

    terminalView.println("\r\nSPI Flash Erase: Complete.\n");
}

/*
Flash Dump
*/
void SpiFlashShell::cmdDump(bool raw) {
    if (!checkFlashPresent()) return;

    terminalView.println("\nSPI Flash: Full dump from 0x000000... Press [ENTER] to stop.\n");

    if (raw) {
        auto confirm = userInputManager.readYesNo("The raw mode is for python scripting, Continue?", false);
        if (!confirm) return;
    }

    // Get flash size
    uint32_t flashSize = readFlashCapacity();

    // Chunk read
    if (raw) readFlashInChunksRaw(0, flashSize); 
    else readFlashInChunks(0, flashSize);

    terminalView.println("\nSPI Flash Dump: Done.\n");
}


/*
Check Chip
*/
bool SpiFlashShell::checkFlashPresent() {
    uint8_t id[3];
    spiService.readFlashIdRaw(id);

    bool invalid = (id[0] == 0xFF && id[1] == 0xFF && id[2] == 0xFF) ||
                   (id[0] == 0x00 && id[1] == 0x00 && id[2] == 0x00);

    if (invalid) {
        terminalView.println("No SPI flash detected (bus error or missing chip).\n");
        return false;
    }

    return true;
}