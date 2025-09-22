#include "RfidController.h"

/*
Constructor
*/
RfidController::RfidController(
    ITerminalView& view,
    IInput& input,
    RfidService& rfidService,
    UserInputManager& uim,
    ArgTransformer& transformer
) : terminalView(view),
    terminalInput(input),
    rfidService(rfidService),
    userInputManager(uim),
    argTransformer(transformer) {}

/*
Entry point for command
*/
void RfidController::handleCommand(const TerminalCommand& cmd) {
    const std::string root = cmd.getRoot();

    if (root == "read")             handleRead(cmd);
    else if (root == "write")       handleWrite(cmd);
    else if (root == "clone")       handleClone(cmd);
    else if (root == "erase")       handleErase(cmd);
    else if (root == "config")      handleConfig();
    else                            handleHelp();
}

/*
Read
*/
void RfidController::handleRead(const TerminalCommand&) {
    auto modes = rfidService.getTagTypes();
    int mode = userInputManager.readValidatedChoiceIndex("Select tag type", modes, 0);
    
    const uint32_t PRINT_INTERVAL_MS = 300;
    uint32_t lastPrint = 0;
    
    terminalView.println("\nRFID Read: Waiting for tag... Press [ENTER] to stop.\n");
    while (true) {
        // User enter press
        int ch = terminalInput.readChar(); 
        if (ch == '\n' || ch == '\r') break;

        uint32_t now = millis();
        if ((now - lastPrint) >= PRINT_INTERVAL_MS) {
            lastPrint = now;

            // Read and display tag infos if any
            int rc = rfidService.read(mode);
            if (rc == RFIDInterface::SUCCESS) {
                terminalView.println(std::string(" [TAG] UID   : ") + rfidService.uid());
                terminalView.println(std::string("       ATQA  : ") + rfidService.atqa());
                terminalView.println(std::string("       SAK   : ") + rfidService.sak());
                terminalView.println(std::string("       Type  : ") + rfidService.piccType() + "\n");
            }
        }
        delay(1);
    }

    terminalView.println("\nRFID Read: Done.\n");
}

/*
Write
*/
void RfidController::handleWrite(const TerminalCommand&) {
    std::vector<std::string> choices = {
        " UID (magic card)",
        " Block/Page data"
    };

    int sel = userInputManager.readValidatedChoiceIndex("Choose write option", choices, 0);

    if (sel == 0) handleWriteUid();
    else          handleWriteBlock();
}

/*
Write UID
*/
void RfidController::handleWriteUid() {
    terminalView.println("RFID Write UID: This requires a MAGIC MIFARE Classic card (rewritable block 0).");

    // UID, variable length 4/7/10 bytes
    std::string uidHex = userInputManager.readValidatedHexString(
        "UID (4, 7 or 10 bytes)", /*numBytes*/0, /*ignoreLen*/true, /*digitsPerItem*/2
    );

    // Validate length
    std::string uidHexClean = uidHex;
    uidHexClean.erase(std::remove(uidHexClean.begin(), uidHexClean.end(), ' '), uidHexClean.end());
    size_t uidLen = uidHexClean.size() / 2;
    if (!(uidLen == 4 || uidLen == 7 || uidLen == 10)) {
        terminalView.println("Invalid UID length. Must be 4, 7, or 10 bytes.\n");
        return;
    }
    rfidService.setUid(uidHexClean);

    // SAK (1 byte hex)
    std::string sakHex = userInputManager.readValidatedHexString("SAK (1 byte, e.g., 08)", 1, false, 2);
    rfidService.setSak(sakHex);

    // ATQA (2 bytes hex)
    std::string atqaHex = userInputManager.readValidatedHexString("ATQA (2 bytes, e.g., 00 04)", 2, false, 2);
    rfidService.setAtqa(atqaHex);

    // Parse defined fields
    rfidService.parseData();

    // Wait for user to place the card
    terminalView.println("RFID Write UID: Place the MAGIC card. Press [ENTER] to cancel.\n");
    while (true) {
        // User enter press
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("RFID Write UID: Stopped by user.\n");
            return;
        }

        // Write UID
        int rc = rfidService.clone(false); // dont check sak
        if (rc == RFIDInterface::SUCCESS) {
            terminalView.println("RFID Write UID: Done.\n");
            return;
        } else if (rc == RFIDInterface::TAG_NOT_PRESENT) {
            delay(5);
            continue;
        } else {
            terminalView.println(" -> " + rfidService.statusMessage(rc));
            terminalView.println("RFID Write: UID write works when block 0 is rewritable.");
            terminalView.println("");
            return;
        }
    }
}

/*
Write Block
*/
void RfidController::handleWriteBlock() {
    // Tag type
    std::vector<std::string> modes = rfidService.getTagTypes();
    int mode = userInputManager.readValidatedChoiceIndex("Target tag type", modes, 0);

    // Size per item (block/page)
    size_t bytesPerItem = 16;
    if (mode == 0) {
        // MIFARE Classic or NTAG/Ultralight
        auto fam = rfidService.getMifareFamily();
        int famIdx = userInputManager.readValidatedChoiceIndex("Target tag family", fam, 0);
        bytesPerItem = (famIdx == 0) ? 16 : 4;
    } else {
        // FeliCa
        bytesPerItem = 16;
    }

    // Block/page index
    int index = userInputManager.readValidatedInt(
        (bytesPerItem == 16) ? "Block index" : "Page index",
        /*def*/ 4, /*min*/ 0, /*max*/ 4095
    );
    std::string hex = userInputManager.readValidatedHexString(
        (bytesPerItem == 16) ? "Data (16 bytes)" : "Data (4 bytes)",
        /*numBytes*/ bytesPerItem, /*ignoreLen*/ false, /*digitsPerItem*/ 2
    );

    // Load payload
    std::string dump = "Page " + std::to_string(index) + ": " + hex + "\n";
    rfidService.loadDump(dump);

    // Wait and write
    terminalView.println("RFID Write: Approach the target tag. Press [ENTER] to stop.\n");
    while (true) {
        // Cancel
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("RFID Write: Stopped by user.\n");
            return;
        }

        // Write
        int rc = rfidService.write(mode);
        if (rc == RFIDInterface::SUCCESS) {
            terminalView.println("RFID Write: Done.\n");
            return;
        } else if (rc == RFIDInterface::TAG_NOT_PRESENT) {
            // keep trying until a tag is detected
            delay(5);
            continue;
        } else {
            terminalView.println(" -> " + rfidService.statusMessage(rc));
            terminalView.println("");
            return;
        }
    }
}

/*
Erase
*/
void RfidController::handleErase(const TerminalCommand&) {
    // Confirm
    bool confirm = userInputManager.readYesNo(
        "RFID Erase: This operation will ERASE the tag. Continue?", false
    );
    if (!confirm) { terminalView.println("Aborted.\n"); return; }

    terminalView.println("RFID Erase: Approach the tag to erase... Press [ENTER] to stop.\n");

    while (true) {
        // Cancel
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("RFID Erase: Stopped by user.\n");
            return;
        }

        // Try erasing
        int rc = rfidService.erase();
        if (rc == RFIDInterface::SUCCESS) {
            terminalView.println("RFID Erase: Done.\n");
            return;
        } else if (rc != RFIDInterface::TAG_NOT_PRESENT) {
            // Any other error -> report and stop
            terminalView.println(" -> " + rfidService.statusMessage(rc));
            terminalView.println("RFID Erase: Failed to erase tag.\n");
            terminalView.println("");

            return;
        }
    }
}

/*
Clone
*/
void RfidController::handleClone(const TerminalCommand&) {
    auto modes = rfidService.getTagTypes();
    int mode = userInputManager.readValidatedChoiceIndex("Select tag type", modes, 0);

    terminalView.println("\nRFID UID Clone: Wait for Mifare SOURCE tag... Press [ENTER] to stop.\n");

    const uint32_t PRINT_INTERVAL_MS = 300;
    uint32_t lastPrint = 0;
    bool haveSource = false;

    while (true) {
        // Cancel
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') return;

        uint32_t now = millis();
        if ((uint32_t)(now - lastPrint) >= PRINT_INTERVAL_MS) {
            lastPrint = now;

            // Read
            int rc = rfidService.read(mode);
            if (rc == RFIDInterface::SUCCESS) {
                haveSource = true;
                terminalView.println(std::string(" [SRC] UID   : ") + rfidService.uid());
                terminalView.println(std::string("       ATQA  : ") + rfidService.atqa());
                terminalView.println(std::string("       SAK   : ") + rfidService.sak());
                terminalView.println(std::string("       Type  : ") + rfidService.piccType() + "\n");
                break;
            }
        }
        delay(1);
    }

    if (!haveSource) {
        terminalView.println("\nRFID UID Clone: No SOURCE tag detected.\n");
        return;
    }

    // Confirm
    terminalView.println("\nPut TARGET card on the PN532.");
    bool proceed = userInputManager.readYesNo("Ready to clone?", true);
    if (!proceed) { terminalView.println("RFID UID Clone: Cancelled by user.\n"); return; }
    terminalView.println("RFID UID Clone: Wait for Mifare TARGET tag... Press [ENTER] to stop.");

    // Cloning
    while (true) {
        // Cancel
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("RFID UID Clone: Stopped by user.\n");
            return;
        }

        int rc = rfidService.clone();
        if (rc == RFIDInterface::SUCCESS) {
            terminalView.println(" -> Success");
            terminalView.println("RFID UID Clone: Done.\n");
            return;
        } else if (rc == RFIDInterface::TAG_NOT_PRESENT) {
            // target not detected
            delay(5);
            continue;
        } else {
            // other error: report and stop
            terminalView.println(" -> " + rfidService.statusMessage(rc));
            terminalView.println("RFID UID Clone: Cloning may require a 'magic card'.");
            terminalView.println("");
            return;
        }
    }
}

/*
Config
*/
void RfidController::handleConfig() {
    terminalView.println("RFID Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t sda = userInputManager.readValidatedPinNumber("PN532 SDA pin", state.getRfidSdaPin(), forbidden);
    state.setRfidSdaPin(sda);

    uint8_t scl = userInputManager.readValidatedPinNumber("PN532 SCL pin", state.getRfidSclPin(), forbidden);
    state.setRfidSclPin(scl);

    // Configure + begin
    rfidService.configure(sda, scl);
    bool ok = rfidService.begin();

    if (!ok) {
        terminalView.println("\n ❌ RFID: PN532 failed to initialize. Check wiring.\n");
        configured = false;
    } else {
        terminalView.println("\n ✅ RFID: PN532 module detected and initialized.\n");
        configured = true;
    }
}

/*
Help
*/
void RfidController::handleHelp() {
    terminalView.println("RFID commands:");
    terminalView.println("  read");
    terminalView.println("  write");
    terminalView.println("  clone");
    terminalView.println("  erase");
    terminalView.println("  config");
}

/*
Ensure Configuration
*/
void RfidController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Reapply
    rfidService.configure(state.getRfidSdaPin(), state.getRfidSclPin());
    rfidService.begin();
}
