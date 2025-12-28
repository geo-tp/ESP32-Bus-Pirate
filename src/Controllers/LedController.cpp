#include "LedController.h"
#include <sstream>
#include <algorithm>

/*
Constructor
*/
LedController::LedController(ITerminalView& terminalView, IInput& terminalInput,
                             LedService& ledService, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput),
      ledService(ledService), argTransformer(argTransformer), userInputManager(userInputManager) {}

/*
Command
*/
void LedController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "fill") {
        handleFill(cmd);
    } else if (cmd.getRoot() == "scan") {
        handleScan();
    } else if (cmd.getRoot() == "set") {
        handleSet(cmd);
    } else if (cmd.getRoot() == "reset") {
        handleReset(cmd);
    } else if (cmd.getRoot() == "blink") {
        handleAnimation(cmd);
    } else if (cmd.getRoot() == "rainbow") {
        handleAnimation(cmd);
    } else if (cmd.getRoot() == "chase") {
        handleAnimation(cmd);
    } else if (cmd.getRoot() == "cycle") {
        handleAnimation(cmd);
    } else if (cmd.getRoot() == "wave") {
        handleAnimation(cmd);
    } else if (cmd.getRoot() == "config") {
        handleConfig();
    } else if (cmd.getRoot() == "setprotocol") {
        handleSetProtocol();
    } else {
        handleHelp();
    }
}

/*
Instructions
*/
void LedController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("[ERROR] LED instructions not implemented.");
}

/*
Scan
*/
void LedController::handleScan() {
    terminalView.println("\n  [INFO] LED protocol scan.");
    terminalView.println("         A short 'chase' animation will be played for each protocol.");
    terminalView.println("         Watch the LEDs: they should light up one by one in blue,");
    terminalView.println("         then turn off in sequence. If it looks correct, press [ENTER].");
    terminalView.println("         Otherwise, wait 3 seconds and it will try the next protocol.\n");

    terminalView.println("Which type of LED are you scanning?");
    terminalView.println("  1. Single-wire (DATA only)");
    terminalView.println("  2. Clocked (DATA + CLOCK)\n");

    // Ask for LED type
    uint8_t typeChoice = 0;
    while (true) {
        typeChoice = userInputManager.readValidatedUint8("Choice", 1);
        if (typeChoice == 1 || typeChoice == 2) break;
        terminalView.println("Invalid choice. Enter 1 or 2.");
    }

    // Define scanned protocol
    std::vector<std::string> protocols = (typeChoice == 1)
        ? LedService::getSingleWireProtocols()
        : LedService::getSpiChipsets();

    // Get saved pins and leds count
    uint8_t dataPin = state.getLedDataPin();
    uint8_t clockPin = state.getLedClockPin();
    uint16_t length = state.getLedLength();
    uint8_t brightness = state.getLedBrightness();
    
    // Run chase animation for each protocol until ENTER is pressed
    for (const auto& proto : protocols) {
        terminalView.println("Trying protocol: " + proto);
        ledService.configure(dataPin, clockPin, length, proto, brightness);
        ledService.resetLeds();

        terminalView.println(">>> PRESS [ENTER] if the LEDs chase in blue (auto-skip in 3s)...");

        // Show the animation for 3 sec or until ENTER is press
        unsigned long start = millis();
        while (millis() - start < 3000) {
            char key = terminalInput.readChar();
            // Found, save the protocol
            if (key == '\r' || key == '\n') {
                terminalView.print("\nLED: Protocol found: " + proto);
                terminalView.println(". Successfully saved to configuration.");
                state.setLedProtocol(proto);
                return;
            }
            ledService.runAnimation("chase");
        }
        ledService.resetLeds(); // in case of some anim persist in this mode

    }
    terminalView.println("\nLED: No protocol matched.");
    ensureConfigured();
}

/*
Fill
*/
void LedController::handleFill(const TerminalCommand& cmd) {
    std::vector<std::string> args = argTransformer.splitArgs(cmd.getArgs());

    // fill <r> <g> <b>
    if (!args.empty() &&
        argTransformer.isValidNumber(cmd.getSubcommand()) &&
        args.size() >= 2 &&
        argTransformer.isValidNumber(args[0]) &&
        argTransformer.isValidNumber(args[1])) {

        std::vector<std::string> full = {cmd.getSubcommand(), args[0], args[1]};
        CRGB rgb = parseFlexibleColor(full);
        ledService.fill(rgb);
        return;
    }

    // fill <hex|name>
    std::vector<std::string> colorArg = {cmd.getSubcommand()};
    CRGB rgb = parseFlexibleColor(colorArg);
    ledService.fill(rgb);
}

/*
Set
*/
void LedController::handleSet(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (args.empty()) {
        terminalView.println("Usage: set <index> <hex RGB color | r g b | name>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Error: Invalid index format.");
        return;
    }

    uint16_t index = argTransformer.parseHexOrDec(cmd.getSubcommand());
    CRGB rgb = parseFlexibleColor(args);

    ledService.set(index, rgb);
}

/*
Reset
*/
void LedController::handleReset(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        ledService.resetLeds();
        terminalView.println("LED: Reset all LEDs to default.");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("LED: Invalid syntax. Use:");
        terminalView.println("  reset");
        terminalView.println("  reset <led num>");
        return;
    }

    uint16_t index = argTransformer.parseHexOrDec(cmd.getSubcommand());
    ledService.set(index, CRGB::Black);
    terminalView.println("LED: Reset LED " + std::to_string(index));
}

/*
Config
*/
void LedController::handleConfig() {
    terminalView.println("LED Configuration:");
    
    // Get default
    const auto& forbidden = state.getProtectedPins();
    uint8_t defaultDataPin = state.getLedDataPin();
    uint8_t defaultClockPin = state.getLedClockPin();
    uint16_t defaultLength = state.getLedLength();

    // LEDs pins, locked because FastLED needs them at compile time
    terminalView.println("[WARNING] Data pin cannot be changed. Data pin set to : " + std::to_string(defaultDataPin));
    terminalView.println("[WARNING] Clock pin cannot be changed. Clock pin set to: " + std::to_string(defaultClockPin));

    // LEDs count
    uint16_t length = userInputManager.readValidatedUint32("Number of LEDs", defaultLength);
    if (length <= 0) length = 1;

    // LED Brightness
    uint8_t defaultBrightness = state.getLedBrightness();
    uint8_t brightness = userInputManager.readValidatedUint8("Brightness (0–255)", defaultBrightness);

    // LED Protocol
    auto selectedProtocol = state.getLedProtocol();
    terminalView.println("Current protocol: '" + selectedProtocol + "'");
    terminalView.println("You can change it with 'setprotocol'");
    terminalView.println("or try to detect it automatically with 'scan'");

    // Configure
    ledService.configure(defaultDataPin, defaultClockPin, length, selectedProtocol, brightness);
    ledService.resetLeds();
    terminalView.println("LEDs configured.\n");

    // Update state
    state.setLedLength(length);
    state.setLedBrightness(brightness);
    state.setLedProtocol(selectedProtocol);
}

/*
Animation
*/
void LedController::handleAnimation(const TerminalCommand& cmd) {
    const auto& validTypes = ledService.getSupportedAnimations();

    // Check correct anim type
    const std::string& type = cmd.getRoot();
    if (std::find(validTypes.begin(), validTypes.end(), type) == validTypes.end()) {
        terminalView.println("LED: Unknown animation type: " + type);
        return;
    }
    
    // Run anim until user ENTER press
    terminalView.println("LED: Playing animation: " + type + "... Press [ENTER] to stop.");
    while (true) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("\nLED: Animation stopped.");
            break;
        }

        ledService.runAnimation(type);
    }
}

/*
Set Protocol
*/
void LedController::handleSetProtocol() {
    terminalView.println("\nSet LED Protocol:");

    std::vector<std::string> oneWire = LedService::getSingleWireProtocols();
    std::vector<std::string> spiChipsets = LedService::getSpiChipsets();
    std::vector<std::string> allProtocols;
    size_t index = 1;

    // Show single wire protocols
    terminalView.println("  -- Single-wire protocols (DATA only) --");
    for (const auto& proto : oneWire) {
        terminalView.println("  " + std::to_string(index++) + ". " + proto);
        allProtocols.push_back(proto);
    }

    // Show clocked
    terminalView.println("  -- Clocked chipsets (DATA + CLOCK) --");
    for (const auto& proto : spiChipsets) {
        terminalView.println("  " + std::to_string(index++) + ". " + proto);
        allProtocols.push_back(proto);
    }

    // Convert saved protocol to index
    std::string currentProtocol = state.getLedProtocol();
    auto it = std::find(allProtocols.begin(), allProtocols.end(), currentProtocol);
    size_t currentIndex = (it != allProtocols.end()) ? std::distance(allProtocols.begin(), it) + 1 : 1;


    // Ask for protocol index
    terminalView.println("");
    uint8_t choice = 0;
    while (true) {
        choice = userInputManager.readValidatedUint8("Choice", currentIndex);
        if (choice >= 1 && choice <= allProtocols.size()) break;
        terminalView.println("Invalid choice. Try again.");
    }

    // Configure
    std::string selectedProtocol = allProtocols[choice - 1];
    state.setLedProtocol(selectedProtocol);
    ensureConfigured();
    terminalView.println("LED protocol changed to " + selectedProtocol);
}

/*
Help
*/
void LedController::handleHelp() {
    terminalView.println("Unknown LED command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  fill blue");
    terminalView.println("  set 1 red");
    terminalView.println("  blink");
    terminalView.println("  rainbow");
    terminalView.println("  chase");
    terminalView.println("  cycle");
    terminalView.println("  wave");
    terminalView.println("  reset [led num]");
    terminalView.println("  setprotocol");
    terminalView.println("  config");
}

/*
Ensure Configuration
*/
void LedController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Reconfigure
    std::string protocol = state.getLedProtocol();
    uint8_t data = state.getLedDataPin();
    uint8_t clock = state.getLedClockPin();
    uint16_t length = state.getLedLength();
    uint8_t brightness = state.getLedBrightness();
    ledService.configure(data, clock, length, protocol, brightness);
}

/*
Utils
*/
CRGB LedController::parseFlexibleColor(const std::vector<std::string>& args) {
    if (args.empty()) return CRGB::Black;

    // 3 nombres
    if (args.size() >= 3 &&
        argTransformer.isValidNumber(args[0]) &&
        argTransformer.isValidNumber(args[1]) &&
        argTransformer.isValidNumber(args[2])) {

        uint8_t r = argTransformer.toUint8(args[0]);
        uint8_t g = argTransformer.toUint8(args[1]);
        uint8_t b = argTransformer.toUint8(args[2]);
        return CRGB(r, g, b);
    }

    // #RRGGBB ou 0xRRGGBB
    const std::string& token = args[0];
    if (token.rfind("#", 0) == 0 || token.rfind("0x", 0) == 0 || token.rfind("0X", 0) == 0) {
        return ledService.parseHtmlColor(token);
    }

    // blue, red, etc.
    return ledService.parseStringColor(token);
}
