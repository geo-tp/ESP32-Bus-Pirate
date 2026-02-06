#include "Controllers/BluetoothController.h"

/*
Constructor
*/
BluetoothController::BluetoothController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    IInput& deviceInput,
    BluetoothService& bluetoothService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    HelpShell& helpShell
) : terminalView(terminalView),
    terminalInput(terminalInput),
    deviceInput(deviceInput),
    bluetoothService(bluetoothService),
    argTransformer(argTransformer),
    userInputManager(userInputManager),
    helpShell(helpShell) {}

/*
Entry point for BT command
*/
void BluetoothController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if      (root == "scan")     handleScan();
    else if (root == "pair")     handlePair(cmd);
    else if (root == "spoof")    handleSpoof(cmd);
    else if (root == "sniff")    handleSniff(cmd);
    else if (root == "status")   handleStatus();
    else if (root == "server")   handleServer(cmd);
    else if (root == "keyboard") handleKeyboard(cmd);
    else if (root == "mouse")    handleMouse(cmd);
    else if (root == "reset")    handleReset();
    else handleHelp();
}

/*
Scan
*/
void BluetoothController::handleScan() {
    terminalView.println("Bluetooth Scan: In progress for 10 sec...\n");

    auto lines = bluetoothService.scanDevices(10);
    if (lines.empty()) {
        terminalView.println("Bluetooth Scan: No devices");
        return;
    }

    for (const auto& line : lines) {
        terminalView.println("  " + line + "\n");
    }
}

/*
Pair
*/
void BluetoothController::handlePair(const TerminalCommand& cmd) {
    bluetoothService.switchToMode(BluetoothMode::CLIENT);
    std::string addr = cmd.getSubcommand();
    if (addr.empty()) {
        terminalView.println("Usage: pair <mac address>");
        return;
    }

    terminalView.println("Bluetooth Pair: Attempting " + addr + "...");

    auto services = bluetoothService.connectTo(addr);
    if (!services.empty()) {
        terminalView.println("Bluetooth Pair: Successfully connected.");
        terminalView.println("Bluetooth Pair: Services discovered:");
        for (const auto& uuid : services) {
            terminalView.println("  - " + uuid);
        }
    } else {
        terminalView.println("Bluetooth Pair: Failed to connect to " + addr);
    }
}

/*
Status
*/
void BluetoothController::handleStatus() {
    terminalView.println("Bluetooth Status:");

    if (bluetoothService.getMode() == BluetoothMode::NONE) {
        terminalView.println("  Mode: Not initialized");
        return;
    } else if (bluetoothService.getMode() == BluetoothMode::CLIENT) {
        terminalView.println("  Mode: Client");
    } else {
        terminalView.println("  Mode: Server");
    }

    terminalView.println("  Connected: " + std::string(bluetoothService.isConnected() ? "Yes" : "No"));

    std::string mac = bluetoothService.getMacAddress();
    if (!mac.empty()) {
        terminalView.println("  MAC Address: " + mac);
    } else {
        terminalView.println("  MAC Address: Unknown");
    }
}

/*
Sniff
*/
void BluetoothController::handleSniff(const TerminalCommand& cmd) {
    terminalView.println("Bluetooth Sniff: Started... Press [ENTER] to stop.\n");

    bluetoothService.switchToMode(BluetoothMode::CLIENT);
    BluetoothService::startPassiveBluetoothSniffing();

    unsigned long lastPull = 0;

    while (true) {
        // Enter press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        // Show paquets if any
        if (millis() - lastPull > 200) { 
            auto logs = BluetoothService::getBluetoothSniffLog();
            for (const auto& line : logs) {
                terminalView.println(line);
            }
            lastPull = millis();
        }

        delay(10);
    }

    BluetoothService::stopPassiveBluetoothSniffing();
    terminalView.println("Bluetooth Sniff: Stopped by user.\n");
}

/*
Server
*/
void BluetoothController::handleServer(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() == BluetoothMode::SERVER && bluetoothService.isConnected()) {
        terminalView.println("Bluetooth Server: Already Started");
        return;
    }

    std::string name = cmd.getSubcommand();
    if (name.empty()) {
        name = "Bus-Pirate-Bluetooth";
    }

    terminalView.println("Bluetooth Server: Starting BLE HID server as \"" + name + "\"...");
    bluetoothService.startServer(name);
    terminalView.println("→ You can now pair from your phone or computer.");
}

/*
Keyboard
*/
void BluetoothController::handleKeyboard(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() != BluetoothMode::SERVER) {
        terminalView.println("Bluetooth Keyboard: Start the server before sending data");
        return;
    }

    auto sub = cmd.getSubcommand();
    if (sub.empty() || sub == "bridge") {
        handleKeyboardBridge();
        return;
    }

    // if spaces in text, cmd.getArgs() is not empty
    auto full = cmd.getArgs().empty() ? 
                        cmd.getSubcommand() : 
                        cmd.getSubcommand() + " " + cmd.getArgs();

    bluetoothService.sendKeyboardText(full);
    terminalView.println("Bluetooth Keyboard: String sent.");
}

/*
Keyboard Bridge
*/
void BluetoothController::handleKeyboardBridge() {
    terminalView.println("Bluetooth Keyboard Bridge: Sending all keys to BLE HID.");

    bool sameHost = false;
    if (state.getTerminalMode() != TerminalTypeEnum::Standalone) {

        terminalView.println("\n[WARNING] If the BLE device is plugged on the same host as");
        terminalView.println("          the terminal, it may cause looping issues with ENTER.");
        terminalView.println("          (That makes no sense to bridge your keyboard on the same host)\n");
    
        sameHost = userInputManager.readYesNo("Are you connected on the same host? (y/n)", true);
        if (sameHost) {
            terminalView.println("Same host, ENTER key will not be sent to BLE HID.");
        }
    }

    terminalView.println("Bluetooth Keyboard: Bridge started.. Press [ANY ESP32 BUTTON] to stop.");

    while (true) {
        // Stop if any esp32 button pressed
        char k = deviceInput.readChar();
        if (k != KEY_NONE) {
            terminalView.println("\r\nBluetooth Keyboard Bridge: Stopped by user.");
            break;
        }

        // Relay terminal -> BLE HID
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            if (c == '\n' && sameHost) continue;
            bluetoothService.sendKeyboardText(std::string(1, c));
            delay(20); 
        }
    }
}

/*
Mouse
*/
void BluetoothController::handleMouse(const TerminalCommand& cmd) {
    if (bluetoothService.getMode() != BluetoothMode::SERVER) {
        terminalView.println("Bluetooth Mouse: Start the server before sending data");
        return;
    }

    // TEMPORARY TODO: Mouse HID report disabled.
    // Creating multiple HID input reports with the same characteristic UUID
    // currently causes the second report to be attached without a valid service
    // in NimBLE, leading to a crash on notify().
    // Until the root cause is fixed in the BLE HID stack, we only enable
    // a single HID report (keyboard).
    terminalView.println("Bluetooth Mouse: HID report currently unavailable due to issues.\n");
    return;

    // mouse click
    if (cmd.getSubcommand() == "click") {
        bluetoothService.clickMouse();
        terminalView.println("Bluetooth Mouse: Click sent.");
        return;
    }

    // mouse jiggle
    if (cmd.getSubcommand() == "jiggle") {
        handleMouseJiggle(cmd);
        return;
    }

    auto args = argTransformer.splitArgs(cmd.getArgs());

    // mouse move x y
    if (args.size() == 2 && cmd.getSubcommand() == "move" &&
        argTransformer.isValidSignedNumber(args[0]) &&
        argTransformer.isValidSignedNumber(args[1])) {

        int8_t x = argTransformer.toClampedInt8(args[0]);
        int8_t y = argTransformer.toClampedInt8(args[1]);

        bluetoothService.mouseMove(x, y);
        terminalView.println("Bluetooth Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
        return;
    }
    
    // mouse x y
    if (args.size() != 1 ||
        !argTransformer.isValidSignedNumber(cmd.getSubcommand()) ||
        !argTransformer.isValidSignedNumber(args[0])) {
        terminalView.println("Usage: mouse <x> <y> or mouse click");
        return;
    }

    int8_t x = argTransformer.toClampedInt8(cmd.getSubcommand());
    int8_t y = argTransformer.toClampedInt8(args[0]);

    bluetoothService.mouseMove(x, y);
    terminalView.println("Bluetooth Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

/*
Mouse Jiggle
*/
void BluetoothController::handleMouseJiggle(const TerminalCommand& cmd) {
    int intervalMs = 1000; // default

    // Optional interval arg
    const std::string& arg = cmd.getArgs();
    if (!arg.empty() && argTransformer.isValidNumber(arg)) {
        intervalMs = argTransformer.parseHexOrDec32(arg);
    }

    terminalView.println(
        "Bluetooth Mouse: Jiggle started (" + std::to_string(intervalMs) +
        " ms)... Press [ENTER] to stop."
    );

    while (true) {
        // Random move
        int8_t dx = (int8_t)random(-127, 127);
        int8_t dy = (int8_t)random(-127, 127);
        if (dx == 0 && dy == 0) dx = 1;

        bluetoothService.mouseMove(dx, dy);
        delay(30);

        // Wait for interval while listening for ENTER
        unsigned long t0 = millis();
        while ((millis() - t0) < (unsigned long)intervalMs) {
            int c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("Bluetooth Mouse: Jiggle stopped.\n");
                return;
            }
            delay(10);
        }
    }
}

/*
Spoof
*/
void BluetoothController::handleSpoof(const TerminalCommand& cmd) {
    std::string mac = cmd.getSubcommand();
    if (bluetoothService.isConnected() || bluetoothService.getMode() != BluetoothMode::NONE) {
        terminalView.println("Bluetooth Spoof: You must set the address before init Bluetooth. Use 'reset' command");
        return;
    }

    if (mac.empty()) {
        terminalView.println("Usage: spoof <mac address>");
        return;
    }

    bool success = bluetoothService.spoofMacAddress(mac);
    if (success) {
        terminalView.println("Bluetooth Spoof: MAC address overridden to " + mac);
    } else {
        terminalView.println("Bluetooth Spoof: Failed to set MAC address.");
    }
}

/*
Reset
*/
void BluetoothController::handleReset() {
    bluetoothService.stopServer();
    terminalView.println("Bluetooth: Reset complete.");
}

/*
Config
*/
void BluetoothController::handleConfig() {
    #ifdef DEVICE_M5STICK
        bluetoothService.releaseBtClassic();
    #endif
}

/*
Help
*/
void BluetoothController::handleHelp() {
    terminalView.println("\nUnknown command. Available Bluetooth commands:");
    helpShell.run(state.getCurrentMode(), false);
}

/*
Ensure Config
*/
void BluetoothController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}
