#include "CanController.h"

CanController::CanController(ITerminalView& terminalView, IInput& terminalInput, UserInputManager& userInputManager,
                             CanService& canService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), userInputManager(userInputManager),
      canService(canService), argTransformer(argTransformer) {}

/*
Entry point for CAN commands
*/
void CanController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "sniff")          handleSniff();
    else if (cmd.getRoot() == "send")      handleSend(cmd);
    else if (cmd.getRoot() == "receive")   handleReceive(cmd);
    else if (cmd.getRoot() == "status")    handleStatus();
    else if (cmd.getRoot() == "config")    handleConfig();
    else handleHelp();
}

/*
Sniff all CAN frames
*/
void CanController::handleSniff() {
    canService.reset();
    
    terminalView.println("CAN Sniff: Waiting for frame... Press [ENTER] to stop.\n");
    
    unsigned long lastFrameTime = millis();
    while (true) {
        auto frame = canService.readFrameAsString();

        // Received frame
        if (!frame.empty()) {
            terminalView.println(" 📥 " + frame);
            lastFrameTime = millis();  // reset timer
        }

        // Reset CAN if no frame for 3 seconds
        if (millis() - lastFrameTime > 3000) {
            canService.reset();
            lastFrameTime = millis();
        }

        // Abort if ENTER is pressed
        char ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("\nCan Sniff: Stopped by user.");
            break;
        }
    }
}

/*
Status of the CAN controller
*/
void CanController::handleStatus() {

    std::string status = canService.getStatus();
    terminalView.println("\n  CAN Status:");
    terminalView.println(status);
}

/*
Send a CAN frame with specific ID
*/
void CanController::handleSend(const TerminalCommand& cmd) {

    int id;
    if (!cmd.getSubcommand().empty() && argTransformer.isValidNumber(cmd.getSubcommand())) {
        id = argTransformer.parseHexOrDec16(cmd.getSubcommand());
    } else {
        // Ask user for ID
        id = userInputManager.readValidatedCanId("Filter CAN ID", 0x123);
    }
    
    // Check max value allowed for an id
    if (id > 0x7FF) {
        terminalView.println("\n❌ Only 11-bit standard IDs are supported (max 0x7FF).");
        return;
    }

    // Ask for data bytes
    terminalView.println("Enter bytes separated by space (e.g. '01 02 0A FF'):");
    std::string hexString = userInputManager.readValidatedHexString("", 0, true);

    // Convert hex string to byte vector
    std::vector<uint8_t> data = argTransformer.parseHexList(hexString);

    if (canService.sendFrame(id, data)) {
        terminalView.println("\nCAN Send: ✅ Frame sent to 0x" + argTransformer.toHex(id, 3));
    } else {
        terminalView.println("\nCAN Send: ❌ Failed to send frame to 0x" + argTransformer.toHex(id, 3));
    }
}

/*
Receive CAN frames with filtering by frame ID
*/
void CanController::handleReceive(const TerminalCommand& cmd) {
    terminalView.println("CAN Receive: Filtered by ID");

    int id;
    if (!cmd.getSubcommand().empty() && argTransformer.isValidNumber(cmd.getSubcommand())) {
        id = argTransformer.parseHexOrDec16(cmd.getSubcommand());
    } else {
        // Ask user for ID
        id = userInputManager.readValidatedCanId("Filter CAN ID", 0x123);
    }
    
    // Check max value allowed
    if (id > 0x7FF) {
        terminalView.println("\n❌ Only 11-bit standard IDs are supported.");
        return;
    }

    // Filter by ID
    canService.setFilter(id);
    
    // Flush internal buffer
    canService.flush();

    terminalView.println("Waiting for CAN frame with ID 0x" + argTransformer.toHex(id, 3) + "... Press [ENTER] to stop.\n");

    unsigned long lastFrameTime = millis();
    while (true) {
        std::string frameStr = canService.readFrameAsString();

        // Received frame
        if (!frameStr.empty()) {
            terminalView.println(" 📥 " + frameStr);
            lastFrameTime = millis();  // reset timer
        }

        // Reset CAN if no frame for 3 seconds
        if (millis() - lastFrameTime > 3000) {
            canService.reset();
            lastFrameTime = millis();
        }

        // Abort if ENTER is pressed
        char ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            terminalView.println("\nCan Receive: Stopped by user.");
            break;
        }
    }

    // Reset filter
    canService.reset();
}

/*
Help message for CAN commands
*/
void CanController::handleHelp() {
    terminalView.println("Available CAN commands:");
    terminalView.println("  sniff");
    terminalView.println("  send [id]");
    terminalView.println("  receive [id]");
    terminalView.println("  status");
    terminalView.println("  config");
}

/*
Configure the CAN controller
*/
void CanController::handleConfig() {

    terminalView.println("\nCAN Configuration:");
    terminalView.println("\nMake sure you are using an MCP2515 CAN module.\n");
    
    const auto& forbidden = state.getProtectedPins();

    // CS pin is fixed, no need to configure
    uint8_t cs = state.getCanCspin();
    terminalView.print("MCP2515 CS pin is fixed to: " + std::to_string(cs));
    terminalInput.waitPress();
    terminalView.println("");

    // Configure SCK
    uint8_t sck = userInputManager.readValidatedPinNumber("MCP2515 SCK pin", state.getCanSckPin(), forbidden);
    state.setCanSckPin(sck);

    // Configure SI (MOSI)
    uint8_t si = userInputManager.readValidatedPinNumber("MCP2515 SI (MOSI) pin", state.getCanSiPin(), forbidden);
    state.setCanSiPin(si);

    // Configure SO (MISO)
    uint8_t so = userInputManager.readValidatedPinNumber("MCP2515 SO (MISO) pin", state.getCanSoPin(), forbidden);
    state.setCanSoPin(so);

    // Configure bitrate
    uint32_t kbps = userInputManager.readValidatedUint32("Speed in kbps", state.getCanKbps());
    uint32_t adjusted = canService.closestSupportedBitrate(kbps);
    state.setCanKbps(adjusted);
    if (adjusted != kbps) {
        terminalView.println("⚠️  Requested bitrate " + std::to_string(kbps) + " kbps is not supported. Using " + std::to_string(adjusted) + " kbps instead.");
    }

    // Apply configuration
    canService.configure(cs, sck, so, si, kbps);

    // Test MCP2515 responsiveness
    auto probeOk = canService.probe();
    if (!probeOk) {
        terminalView.println("\n ❌ MCP2515 CAN configuration failed. Please check your wiring.\n");
        return;
    }
    terminalView.println("\n ✅ MCP2515 CAN configured.\n");
}

/*
Ensure CAN is configured before any operation
*/
void CanController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Always reapply config in case pins were reassigned elsewhere
    canService.configure(
        state.getCanCspin(),
        state.getCanSckPin(),
        state.getCanSoPin(),
        state.getCanSiPin(),
        state.getCanKbps()
    );
}

