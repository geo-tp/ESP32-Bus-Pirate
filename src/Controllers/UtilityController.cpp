#include "Controllers/UtilityController.h"

/*
Constructor
*/
UtilityController::UtilityController(
    ITerminalView& terminalView,
    IDeviceView& deviceView,
    IInput& terminalInput,
    PinService& pinService,
    UserInputManager& userInputManager,
    PinAnalyzeManager& pinAnalyzeManager,
    ArgTransformer& argTransformer,
    SysInfoShell& sysInfoShell,
    GuideShell& guideShell,
    HelpShell& helpShell
)
    : terminalView(terminalView),
      deviceView(deviceView),
      terminalInput(terminalInput),
      pinService(pinService),
      userInputManager(userInputManager),
      pinAnalyzeManager(pinAnalyzeManager),
      argTransformer(argTransformer),
      sysInfoShell(sysInfoShell),
      guideShell(guideShell),
      helpShell(helpShell)
{}

/*
Entry point for command
*/
void UtilityController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "help" || cmd.getRoot() == "h" || cmd.getRoot() == "?") handleHelp();
    else if (cmd.getRoot() == "P")                                               handleEnablePullups();
    else if (cmd.getRoot() == "p")                                               handleDisablePullups();
    else if (cmd.getRoot() == "logic")                                           handleLogicAnalyzer(cmd);
    else if (cmd.getRoot() == "analogic")                                        handleAnalogic(cmd);
    else if (cmd.getRoot() == "system")                                          handleSystem();
    else if (cmd.getRoot() == "guide")                                           handleGuide();
    else if (cmd.getRoot() == "man")                                             handleGuide();
    else if (cmd.getRoot() == "wizard")                                          handleWizard(cmd);
    else {
        helpShell.run(state.getCurrentMode(), false);
    }
}

/*
Mode Change
*/
ModeEnum UtilityController::handleModeChangeCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() != "mode" && cmd.getRoot() != "m") {
        terminalView.println("Invalid command for mode change.");
        return ModeEnum::None;
    }

    if (!cmd.getSubcommand().empty()) {
        ModeEnum newMode = ModeEnumMapper::fromString(cmd.getSubcommand());
        if (newMode != ModeEnum::None) {
            terminalView.println("Mode changed to " + ModeEnumMapper::toString(newMode));
            terminalView.println(""); 
            return newMode;
        } else {
            terminalView.println("Unknown mode: " + cmd.getSubcommand());
            return ModeEnum::None;
        }
    }

    return handleModeSelect();
}

/*
Mode Select
*/
ModeEnum UtilityController::handleModeSelect() {
    terminalView.println("");
    terminalView.println("Select mode:");
    std::vector<ModeEnum> modes;

    for (int i = 0; i < static_cast<int>(ModeEnum::COUNT); ++i) {
        ModeEnum mode = static_cast<ModeEnum>(i);
        std::string name = ModeEnumMapper::toString(mode);
        if (!name.empty()) {
            modes.push_back(mode);
            terminalView.println("  " + std::to_string(modes.size()) + ". " + name);
        }
    }

    terminalView.println("");
    terminalView.print("Mode Number > ");
    auto modeNumber = userInputManager.readModeNumber();

    if (modeNumber == -1) {
        terminalView.println("");
        terminalView.println("");
        terminalView.println("Invalid input.");
        return ModeEnum::None;
    } else if (modeNumber >= 1 && modeNumber <= modes.size()) {
        ModeEnum selected = modes[modeNumber - 1];
        if (static_cast<int>(selected) > 9) {
            terminalView.println(""); // Hack to render correctly on web terminal
        }
        terminalView.println("");
        terminalView.println("Mode changed to " + ModeEnumMapper::toString(selected));
        terminalView.println("");
        return selected;
    } else {
        terminalView.println("");
        terminalView.println("Invalid mode number.");
        terminalView.println("");
        return ModeEnum::None;
    }
}

/*
Pullup: p
*/
void UtilityController::handleDisablePullups() {
    auto mode = state.getCurrentMode();
    switch (mode) {
        case ModeEnum::SPI:
            pinService.setInput(state.getSpiMISOPin());
            terminalView.println("SPI: Pull-ups disabled on MISO");
            break;

        case ModeEnum::I2C:
            pinService.setInput(state.getI2cSdaPin());
            pinService.setInput(state.getI2cSclPin());
            terminalView.println("I2C: Pull-ups disabled on SDA, SCL.");
            break;

        case ModeEnum::OneWire:
            pinService.setInput(state.getOneWirePin());
            terminalView.println("1-Wire: Pull-up disabled on DQ.");
            break;

        case ModeEnum::UART:
            pinService.setInput(state.getUartRxPin());
            terminalView.println("UART: Pull-ups disabled on RX.");
            break;

        case ModeEnum::HDUART:
            pinService.setInput(state.getHdUartPin());
            terminalView.println("HDUART: Pull-up disabled on IO pin.");
            break;

        case ModeEnum::TwoWire:
            pinService.setInput(state.getTwoWireIoPin());
            terminalView.println("2-WIRE: Pull-up disabled on DATA pin.");
            break;

        case ModeEnum::JTAG:
            for (auto pin : state.getJtagScanPins()) {
                pinService.setInput(pin);
            }
            terminalView.println("JTAG: Pull-ups disabled on all scan pins.");
            break;

        default:
            terminalView.println("Pull-ups not applicable for this mode.");
            break;
    }
}

/*
Pullup P
*/
void UtilityController::handleEnablePullups() {
    auto mode = state.getCurrentMode();
    switch (mode) {
        case ModeEnum::SPI:
            pinService.setInput(state.getSpiMISOPin());
            pinService.setInputPullup(state.getSpiMISOPin());
            terminalView.println("SPI: Pull-up enabled on MISO.");
            break;

        case ModeEnum::I2C:
            pinService.setInputPullup(state.getI2cSdaPin());
            pinService.setInputPullup(state.getI2cSclPin());
            terminalView.println("I2C: Pull-ups enabled on SDA, SCL.");
            break;

        case ModeEnum::OneWire:
            pinService.setInputPullup(state.getOneWirePin());
            terminalView.println("1-Wire: Pull-up enabled on DQ.");
            break;

        case ModeEnum::UART:
            pinService.setInputPullup(state.getUartRxPin());
            terminalView.println("UART: Pull-up enabled on RX.");
            break;

        case ModeEnum::HDUART:
            pinService.setInputPullup(state.getHdUartPin());
            terminalView.println("HDUART: Pull-up enabled on IO pin.");
            break;

        case ModeEnum::TwoWire:
            pinService.setInputPullup(state.getTwoWireIoPin());
            terminalView.println("2-WIRE: Pull-up enabled on DATA pin.");
            break;

        case ModeEnum::JTAG:
            for (auto pin : state.getJtagScanPins()) {
                pinService.setInputPullup(pin);
            }
            terminalView.println("JTAG: Pull-ups enabled on all scan pins.");
            break;

        default:
            terminalView.println("Pull-ups not applicable for this mode.");
            break;
    }
}

/*
Logic
*/
void UtilityController::handleLogicAnalyzer(const TerminalCommand& cmd) {

    uint16_t tDelay = 500;
    uint16_t inc = 100; // tDelay will be decremented/incremented by inc
    uint8_t step = 1; // step of the trace display kind of a zoom

    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: logic <pin>");
        return;
    }

    // Verify protected pin
    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    /*
    auto forbidden = state.getProtectedPins();
    if (std::find(forbidden.begin(), forbidden.end(), pin) != forbidden.end()) {
        terminalView.println("Logic Analyzer: This pin is protected or reserved.");
        return;
    }
*/
    if (state.isPinProtected(pin)) {
        terminalView.println("Logic Analyzer: This pin is protected or reserved.");
        return;
    }
    terminalView.println("\nLogic Analyzer: Monitoring pin " + std::to_string(pin) + "... Press [ENTER] to stop.");
    terminalView.println("Displaying waveform on the ESP32 screen...\n");


    pinService.setInput(pin);
    std::vector<uint8_t> buffer;
    buffer.reserve(320); // 320 samples

    unsigned long lastCheck = millis();
    deviceView.clear();
    deviceView.topBar("Logic Analyzer", false, false);

    while (true) {
        // Enter press
        if (millis() - lastCheck > 10) {
            lastCheck = millis();
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                // fdufnews 2025/10/24 added to restore cursor position when leaving
                if (state.getTerminalMode() == TerminalTypeEnum::SerialPort)
                    terminalView.print("\n\n\n\n\r"); // 4 lines down to place cursor just under the logic trace
                terminalView.println("Logic Analyzer: Stopped by user.");
                break;
            }
            if (c == 's'){
                if (tDelay > inc){
                    tDelay -= inc;
                    terminalView.println("delay : " + std::to_string(tDelay) + "\n");
                }
            };
            if (c == 'S'){
                if (tDelay < 10000){
                    tDelay += inc;
                    terminalView.println("delay : " + std::to_string(tDelay) + "\n");
                }
            };
            if (c == 'z'){
                if (step > 1){
                    step--;
                    terminalView.println("step : " + std::to_string(step) + "\n");
                }
            };
            if (c == 'Z'){
                if (step < 4){
                    step++;
                    terminalView.println("step : " + std::to_string(step) + "\n");
                }
            };
        }

        // Draw
        if (buffer.size() >= 320) {
            buffer.resize(320);
            deviceView.drawLogicTrace(pin, buffer, step);

            // The poor man's drawLogicTrace() on terminal
            // draws a 132 samples sub part of the buffer to speed up the things
            if (state.getTerminalMode() == TerminalTypeEnum::SerialPort){
                terminalView.println("");
                uint8_t pos = 0;
                for(size_t i = 0; i < 132; i++, pos++){
                     terminalView.print(buffer[i]?"-":"_");
                }
                terminalView.print("\r\x1b[A");  // Up 1 line to put cursor at the correct place for the next draw
            }
            buffer.clear();
        }

        buffer.push_back(pinService.read(pin));
        delayMicroseconds(tDelay);
    }
}

/*
Analogic
*/
void UtilityController::handleAnalogic(const TerminalCommand& cmd) {
    uint16_t tDelay = 500;
    uint16_t inc = 100; // tDelay will be decremented/incremented by inc
    uint8_t step = 1; // step of the trace display kind of a zoom

    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: analogic <pin>");
        return;
    }

    // Verify protected pin
    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (state.isPinProtected(pin)) {
        terminalView.println("Analogic: This pin is protected or reserved.");
        return;
    }
    if (!state.isPinAnalog(pin)){
        terminalView.println("Analogic: This pin is not an analog one");
        return;
    };

    terminalView.println("\nAnalogic: Monitoring pin " + std::to_string(pin) + "... Press [ENTER] to stop.");
    terminalView.println("Displaying waveform on the ESP32 screen...\n");


    pinService.setInput(pin);
    std::vector<uint8_t> buffer;
    buffer.reserve(320); // 320 samples

    unsigned long lastCheck = millis();
    deviceView.clear();
    deviceView.topBar("Analog plotter", false, false);
    int count = 0;
    while (true) {
        // Enter press
        if (millis() - lastCheck > 10) {
            lastCheck = millis();
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("\nAnalogic: Stopped by user.");
                break;
            }
            if (c == 's'){
                if (tDelay > inc){
                    tDelay -= inc;
                    terminalView.println("\ndelay : " + std::to_string(tDelay) + "\n");
                }
            };
            if (c == 'S'){
                if (tDelay < 10000){
                    tDelay += inc;
                    terminalView.println("\ndelay : " + std::to_string(tDelay) + "\n");
                }
            };
            if (c == 'z'){
                if (step > 1){
                    step--;
                    terminalView.println("\nstep : " + std::to_string(step) + "\n");
                }
            };
            if (c == 'Z'){
                if (step < 4){
                    step++;
                    terminalView.println("\nstep : " + std::to_string(step) + "\n");
                }
            };
            count++;
            if ((count > 50) && (state.getTerminalMode() != TerminalTypeEnum::Standalone)){
                int raw = pinService.readAnalog(pin);
                float voltage = (raw / 4095.0f) * 3.3f;

                std::ostringstream oss;
                oss << "   Analog pin " << static_cast<int>(pin)
                    << ": " << raw
                    << " (" << voltage << " V)";
                terminalView.println(oss.str());
                count = 0;
            }
        }

        // Draw
        if (buffer.size() >= 320) {
            buffer.resize(320);
            deviceView.drawAnalogicTrace(pin, buffer, step);
            buffer.clear();
        }

        buffer.push_back(pinService.readAnalog(pin) >> 4); // convert the readAnalog() value to a uint8_t (4096 ==> 256)
        delayMicroseconds(tDelay);
    }
}

/*
System Information
*/
void UtilityController::handleSystem() {
    sysInfoShell.run();
}

/*
Firmware Guide (man)
*/
void UtilityController::handleGuide() {
    guideShell.run();
}

/*
Wizard
*/
void UtilityController::handleWizard(const TerminalCommand& cmd) {
    // Validate pin argument
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: wizard <pin>");
        return;
    }

    // Verify protected pin
    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (state.isPinProtected(pin)) {
        terminalView.println("Wizard: This pin is protected or reserved.");
        return;
    }

    terminalView.println("\nWizard: Please wait, analyzing pin " + std::to_string(pin) + "... Press [ENTER] to stop.\n");
    pinAnalyzeManager.begin(pin);
    const bool doPullTest = false; // TODO: add argument to enable pull test if needed

    while (true) {
        // Check for ENTER press to stop
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("\nWizard: Stopped by user.");
            break;
        }

        // Sample pin
        for (int i = 0; i < 2048; i++) {
            pinAnalyzeManager.sample();
        }

        // Check if it's time to report and report activity
        if (pinAnalyzeManager.shouldReport(millis())) {
            auto report = pinAnalyzeManager.buildReport(doPullTest);
            terminalView.print(pinAnalyzeManager.formatWizardReport(pin, report));
            pinAnalyzeManager.resetWindow();
            terminalView.println("Wizard: Analyzing pin " + std::to_string(pin) + "... Press [ENTER] to stop.\n");
        }
    }

    // Cleanup buffers
    pinAnalyzeManager.end();
}

/*
Help
*/
void UtilityController::handleHelp() {
    helpShell.run(state.getCurrentMode());
}

bool UtilityController::isGlobalCommand(const TerminalCommand& cmd) {
    std::string root = cmd.getRoot();

    return (root == "mode"  || root == "m" || root == "l" ||
            root == "logic" || root == "analogic" || root == "P" || root == "p") || 
            root == "system" || root == "guide" || root == "man" || root == "wizard" ||
            root == "help" || root == "h" || root == "?";
}

bool UtilityController::isScreenCommand(const TerminalCommand& cmd) {
   return cmd.getRoot() == "config" || cmd.getRoot() == "setprotocol" || cmd.getRoot() == "trace"
       || cmd.getRoot() == "pullup" || cmd.getRoot() == "pulldown" || cmd.getRoot() == "reset"
       || cmd.getRoot() == "swap" || cmd.getRoot() == "logic" || cmd.getRoot() == "analogic";
}
