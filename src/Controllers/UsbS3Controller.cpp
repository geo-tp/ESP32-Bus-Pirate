#include "UsbS3Controller.h"

/*
Constructor
*/
UsbS3Controller::UsbS3Controller(
    ITerminalView& terminalView,
    IInput& terminalInput,
    IInput& deviceInput,
    UsbS3Service& usbService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    HelpShell& helpShell
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      usbService(usbService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      helpShell(helpShell)
{}

/*
Entry point for command
*/
void UsbS3Controller::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "stick") handleUsbStick();
    else if (cmd.getRoot() == "keyboard") handleKeyboard(cmd);
    else if (cmd.getRoot() == "mouse") handleMouse(cmd);
    else if (cmd.getRoot() == "gamepad") handleGamepad(cmd);
    else if (cmd.getRoot() == "host") handleHost();
    else if (cmd.getRoot() == "reset") handleReset();
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Keyboard
*/
void UsbS3Controller::handleKeyboard(const TerminalCommand& cmd) {
    auto sub = cmd.getSubcommand();

    if (sub.empty()) handleKeyboardBridge();
    else if (sub == "bridge") handleKeyboardBridge();
    else handleKeyboardSend(cmd);
}

/*
Keyboard Send
*/
void UsbS3Controller::handleKeyboardSend(const TerminalCommand& cmd) {
    terminalView.println("USB Keyboard: Configuring...");
    usbService.keyboardBegin();
    terminalView.println("USB Keyboard: Initialize...");
    auto full = cmd.getArgs().empty() ? cmd.getSubcommand() : cmd.getSubcommand() + " " + cmd.getArgs();
    usbService.keyboardSendString(full);
    // usbService.reset();
    terminalView.println("USB Keyboard: String sent.");
}

/*
Keyboard Bridge
*/
void UsbS3Controller::handleKeyboardBridge() {
    terminalView.println("USB Keyboard Bridge: Sending all keys to USB HID.");
    usbService.keyboardBegin();

    auto sameHost = false;
    if (state.getTerminalMode() != TerminalTypeEnum::Standalone) {
        terminalView.println("\n[WARNING] If the USB device is plugged on the same host as");
        terminalView.println("          the terminal, it may cause looping issues with ENTER.");
        terminalView.println("          (That makes no sense to bridge your keyboard on the same host)\n");
        
        sameHost = userInputManager.readYesNo("Are you connected on the same host? (y/n)", true);
    
        if (sameHost) {
            terminalView.println("Same host, ENTER key will not be sent to USB HID.");
        }
    }    

    terminalView.println("USB Keyboard: Bridge started.. Press [ANY ESP32 BUTTON] to stop.");

    while (true) {
        // Esp32 button to stop
        char k = deviceInput.readChar();
        if (k != KEY_NONE) {
            terminalView.println("\r\nUSB Keyboard Bridge: Stopped by user.");
            break;
        }
        
        // Terminal to keyboard hid
        char c = terminalInput.readChar();

        // if we send \n in the terminal web browser
        // and the usb hid are on the same host
        // then it will loop infinitely
        if (c != KEY_NONE) { 
            if (c == '\n' && sameHost) continue;
            usbService.keyboardSendString(std::string(1, c));
            delay(20); // slow down looping
        }
    }
}

/*
Mouse Move
*/
void UsbS3Controller::handleMouseMove(const TerminalCommand& cmd) {
    int x, y = 0;

    // mouse move x y
    if (cmd.getSubcommand() == "move") {
        auto args = argTransformer.splitArgs(cmd.getArgs());
        if (args.size() < 2 ||
            argTransformer.isValidSignedNumber(args[0]) == false ||
            argTransformer.isValidSignedNumber(args[1]) == false) {
            terminalView.println("Usage: mouse move <x> <y>");
            return;
        }
        x = argTransformer.toClampedInt8(args[0]);
        y = argTransformer.toClampedInt8(args[1]);
    // mouse x y
    } else {
        if (argTransformer.isValidSignedNumber(cmd.getSubcommand()) == false ||
            argTransformer.isValidSignedNumber(cmd.getArgs()) == false) {
            terminalView.println("Usage: mouse <x> <y>");
            return;
        }
        x = argTransformer.toClampedInt8(cmd.getSubcommand());
        y = argTransformer.toClampedInt8(cmd.getArgs());
    }

    usbService.mouseMove(x, y);
    terminalView.println("USB Mouse: Moved by (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

/*
Mouse Click
*/
void UsbS3Controller::handleMouseClick() {
    // Left click
    usbService.mouseClick(1);
    delay(100);
    usbService.mouseRelease(1);
    terminalView.println("USB Mouse: Click sent.");
}

/*
Mouse
*/
void UsbS3Controller::handleMouse(const TerminalCommand& cmd)  {
    
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: mouse <x> <y>");
        terminalView.println("       mouse click");
        terminalView.println("       mouse jiggle [ms]");
        return;
    }

    terminalView.println("USB Mouse: Configuring HID...");
    usbService.mouseBegin();
    terminalView.println("USB Mouse: Initialize HID...");

    if (cmd.getSubcommand() == "click") handleMouseClick();
    else if (cmd.getSubcommand() == "jiggle") handleMouseJiggle(cmd);
    else handleMouseMove(cmd);
}

/*
Mouse Jiggle
*/
void UsbS3Controller::handleMouseJiggle(const TerminalCommand& cmd) {
    int intervalMs = 1000; // defaut

    if (!cmd.getArgs().empty() && argTransformer.isValidNumber(cmd.getArgs())) {
        auto intervalMs = argTransformer.parseHexOrDec32(cmd.getArgs());
    }

    terminalView.println("USB Mouse: Jiggle started (" + std::to_string(intervalMs) + " ms)... Press [ENTER] to stop.");

    while (true) {
        // Random moves
        int dx = (int)random(-127, 127);
        int dy = (int)random(-127, 127);
        if (dx == 0 && dy == 0) dx = 1;

        usbService.mouseMove(dx, dy);

        // wait interval while listening for ENTER
        unsigned long t0 = millis();
        while ((millis() - t0) < intervalMs) {
            auto c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("USB Mouse: Jiggle stopped.\n");
                return;
            }
            delay(10);
        }
    }
}

/*
Gamepad
*/
void UsbS3Controller::handleGamepad(const TerminalCommand& cmd) {
    terminalView.println("USB Gamepad: Configuring HID...");
    usbService.gamepadBegin();

    std::string subcmd = cmd.getSubcommand();
    std::transform(subcmd.begin(), subcmd.end(), subcmd.begin(), ::tolower);

    if (subcmd == "up" || subcmd == "down" || subcmd == "left" || subcmd == "right" ||
        subcmd == "a" || subcmd == "b") {
        
        usbService.gamepadPress(subcmd);
        terminalView.println("USB Gamepad: Key sent.");

    } else {
        terminalView.println("USB Gamepad: Unknown input. Try up, down, left, right, a, b");
    }
}

/*
Stick
*/
void UsbS3Controller::handleUsbStick() {
    terminalView.println("USB Stick: Starting... USB Drive can take 30sec to appear");
    usbService.storageBegin(state.getSdCardCsPin(), state.getSdCardClkPin(),
                            state.getSdCardMisoPin(), state.getSdCardMosiPin());

    if (usbService.isStorageActive()) {
        terminalView.println("\n ✅ USB Stick configured. Mounting drive... (Can take up to 30 sec)\n");
    } else {
        terminalView.println("\n ❌ USB Stick configuration failed. No SD card detected.\n");
    }
}

/*
Config
*/
void UsbS3Controller::handleConfig() {
    terminalView.println("USB Configuration:");

    auto confirm = userInputManager.readYesNo("Configure SD card pins for USB?", false);

    if (confirm) {
        const auto& forbidden = state.getProtectedPins();
    
        uint8_t cs = userInputManager.readValidatedPinNumber("SD Card CS pin", state.getSdCardCsPin(), forbidden);
        state.setSdCardCsPin(cs);
    
        uint8_t clk = userInputManager.readValidatedPinNumber("SD Card CLK pin", state.getSdCardClkPin(), forbidden);
        state.setSdCardClkPin(clk);
    
        uint8_t miso = userInputManager.readValidatedPinNumber("SD Card MISO pin", state.getSdCardMisoPin(), forbidden);
        state.setSdCardMisoPin(miso);
    
        uint8_t mosi = userInputManager.readValidatedPinNumber("SD Card MOSI pin", state.getSdCardMosiPin(), forbidden);
        state.setSdCardMosiPin(mosi);
    }
    terminalView.println("USB Configured.");

    if (state.getTerminalMode() == TerminalTypeEnum::Standalone) {
        terminalView.println("");
        return;
    };

    terminalView.println("\n[WARNING] If you're using USB Serial terminal mode,");
    terminalView.println("          using USB commands may interrupt the session.");
    terminalView.println("          Use Web UI or restart if connection is lost.\n");
}

/*
Host
*/
void UsbS3Controller::handleHost() {
    if (!usbService.isHostActive()) {
        terminalView.println("USB Host: Once started, you cannot use USB HID features until restart.");
        auto confirm = userInputManager.readYesNo("\nSwitch to host to connect USB Devices to the ESP32?", false);
        if (!confirm) {
            terminalView.println("USB Host: Action cancelled by user.\n");
            return;
        }
    }

    if (!usbService.usbHostBegin()) {
        terminalView.println("USB Host: Failed to start.\n");
        return;
    }

    terminalView.println("");
    terminalView.println("USB Host: Starting... Plug a USB device to the ESP32. Press [ENTER] to stop.\n");
    terminalView.println("[INFO] USB Host will print connected device info and descriptors.");
    terminalView.println("       This feature works only with USB *devices* (peripherals),");
    terminalView.println("       such as keyboards, mice, gamepads, USB adapters, etc.");
    terminalView.println("       Make sure your board provides 5V on the USB port.\n");
    while (true) {
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') break;

        std::string msg = usbService.usbHostTick();
        if (!msg.empty()) {
            terminalView.println(msg);
        }

    }

    // TODO: usbService.usbHostEnd(); can't be called because it will stop the host 
    // and make the USB interface instable, find a way to switch back to default mode
    terminalView.println("\nUSB Host: Stopped by user.\n");
}

/*
Reset
*/
void UsbS3Controller::handleReset() {
    usbService.reset();
    terminalView.println("USB Reset: Disable interfaces...");
}

/*
Help
*/
void UsbS3Controller::handleHelp() {
    helpShell.run(state.getCurrentMode(), false);
}

/*
Ensure Configuration
*/
void UsbS3Controller::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}