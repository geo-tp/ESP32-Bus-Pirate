#include "InfraredController.h"
#include <M5Unified.h>

/*
Constructor
*/
InfraredController::InfraredController(
    ITerminalView&           view,
    IInput&                  terminalInput,
    InfraredService&         service,
    LittleFsService&         littleFsService,
    ArgTransformer&          argTransformer,
    InfraredRemoteTransformer& infraredRemoteTransformer,
    UserInputManager&        userInputManager,
    UniversalRemoteShell&    universalRemoteShell
)
    : terminalView(view),
      terminalInput(terminalInput),
      infraredService(service),
      littleFsService(littleFsService),
      argTransformer(argTransformer),
      infraredRemoteTransformer(infraredRemoteTransformer),
      userInputManager(userInputManager),
      universalRemoteShell(universalRemoteShell)
{}

/*
Entry point to handle Infrared command
*/
void InfraredController::handleCommand(const TerminalCommand& command) {
    if (command.getRoot() == "config")            handleConfig();
    else if (command.getRoot() == "send")         handleSend(command);
    else if (command.getRoot() == "receive")      handleReceive();
    else if (command.getRoot() == "devicebgone")  handleDeviceBgone();
    else if (command.getRoot() == "remote")       handleRemote();
    else if (command.getRoot() == "replay")       handleReplay(command);
    else if (command.getRoot() == "record")       handleRecord();
    else if (command.getRoot() == "load")         handleLoad(command);
    else if (command.getRoot() == "jam")          handleJam();
    else if (command.getRoot() == "setprotocol")  handleSetProtocol();
    else handleHelp();
}

/*
Send
*/
void InfraredController::handleSend(const TerminalCommand& command) {
    std::istringstream iss(command.getArgs());
    std::string subStr, cmdStr;
    iss >> subStr >> cmdStr;
    auto addrStr = command.getSubcommand();

    if (addrStr.empty() || subStr.empty() || cmdStr.empty()) {
        terminalView.println("Missing argument. Usage: send <device> <sub> <cmd>");
        return;
    }

    int device, subdevice, function;
    if (!argTransformer.parseInt(addrStr, device) ||
        !argTransformer.parseInt(subStr, subdevice) ||
        !argTransformer.parseInt(cmdStr, function)) {
        terminalView.println("Invalid number format. Use decimal or hex.");
        return;
    }

    InfraredCommand infraredCommand;
    infraredCommand.setDevice(device);
    infraredCommand.setSubdevice(subdevice);
    infraredCommand.setFunction(function);
    infraredCommand.setProtocol(state.getInfraredProtocol());

    for (int i = 0; i < 3; ++i) {
        infraredService.sendInfraredCommand(infraredCommand);
        delay(100);
    }

    terminalView.println("IR command sent with protocol " + InfraredProtocolMapper::toString(state.getInfraredProtocol()));
}

/*
Receive
*/
void InfraredController::handleReceive() {
    bool decode = userInputManager.readYesNo("Decode infrared signal?", true);

    terminalView.println("INFRARED Receive: Waiting for signal...");
    terminalView.println("Press [ENTER] to stop.\n");
    
    infraredService.startReceiver();

    while (true) {
        // Stop on ENTER
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("\nINFRARED Receive: Stopped by user.");
            break;
        }

        if (decode) {
            // Decode signal
            InfraredCommand cmd = infraredService.receiveInfraredCommand();
            if (cmd.getProtocol() != RAW) {
                terminalView.println("");
                terminalView.println("Infrared signal received:");
                terminalView.println("  Protocol : " + InfraredProtocolMapper::toString(cmd.getProtocol()));
                terminalView.println("  Device   : " + std::to_string(cmd.getDevice()));
                terminalView.println("  SubDev   : " + std::to_string(cmd.getSubdevice()));
                terminalView.println("  Command  : " + std::to_string(cmd.getFunction()));
                terminalView.println("");
                terminalView.println("INFRARED Receive: Waiting for next signal or press [ENTER] to exit.");
            }
        } else {
            // Raw mode
            std::vector<uint16_t> timings;
            uint32_t khz = 0;

            if (infraredService.receiveRaw(timings, khz)) {
                terminalView.println("\nRAW Timings: ");

                bool mark = true;
                for (uint16_t t : timings) {
                    terminalView.print(mark ? "+" : "-");
                    terminalView.print(std::to_string(t));
                    terminalView.print(" ");
                    mark = !mark;
                }
                terminalView.println("");
            }
        }
    }

    infraredService.stopReceiver();
}

/* 
DeviceBgone
*/
void InfraredController::handleDeviceBgone() {
    terminalView.println("Sending Device-B-Gone commands... Press [ENTER] to stop");

    for (const auto& cmdStruct : universalOnOff) {

        // Convert to InfraredCommand model
        auto cmd = InfraredCommand(
            cmdStruct.proto,
            cmdStruct.device,
            cmdStruct.subdevice,
            cmdStruct.function
        );
        
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("Infrared Device-B-Gone: Interrupted by user.");
            return;
        }

        for (int i = 0; i < 2; ++i) { // send 2x per command
            infraredService.sendInfraredCommand(cmd);
            delay(100);
        }

        terminalView.println(
            "Sent On/Off to protocol=" + InfraredProtocolMapper::toString(cmd.getProtocol()) +
            " device=" + std::to_string(cmd.getDevice()) +
            " sub=" + std::to_string(cmd.getSubdevice()) +
            " cmd=" + std::to_string(cmd.getFunction())
        );
    }

    terminalView.println("Device-B-Gone sequence completed.");
}

/*
Universal Remote
*/
void InfraredController::handleRemote() {
    universalRemoteShell.run();
}

/*
Replay
*/
void InfraredController::handleReplay(const TerminalCommand& command) {

    // Optional replay count
    uint32_t replayCount = 0; // 0 = infinite
    const std::string sub = command.getSubcommand();
    if (!sub.empty() && argTransformer.isValidNumber(sub)) {
        replayCount = argTransformer.toUint32(sub);
    }

    // Record frames
    std::vector<IRFrame> tape;
    if (!recordFrames(tape)) {
        return; // No frames captured
    }

    // Playback frames
    playbackFrames(tape, replayCount);
}

void InfraredController::handleRecord() {

    // Mount LittleFS
    if (!littleFsService.mounted()) {
        littleFsService.begin();
        if (!littleFsService.mounted()) {
            terminalView.println("INFRARED Record: LittleFS not mounted. Aborting.");
            return;
        } else {
            terminalView.println("INFRARED Record: LittleFS mounted.");
        }
    }

    // Space check: need at least 8 kb free
    constexpr size_t MIN_FREE_BYTES = 8 * 1024;
    size_t free = littleFsService.freeBytes();
    if (free < MIN_FREE_BYTES) {
        terminalView.println(
            "INFRARED Record: Not enough LittleFS space. Need >= 8KB free, have " +
            std::to_string(free) + " bytes."
        );
        return;
    }

    // Record decoded commands
    std::vector<InfraredFileRemoteCommand> cmds;
    cmds.reserve(64);

    terminalView.println("\nINFRARED Record: Waiting for frames (max 64)... Press [ENTER] to stop.\n");

    infraredService.startReceiver();

    while (true) {
        // Stop ENTER
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("\nINFRARED Record: Stopped.");
            break;
        }

        if (cmds.size() >= 64) {
            terminalView.println("\nINFRARED Record: Reached maximum of 64 saved commands, stopping.\n");
            break;
        }

        InfraredCommand decoded = infraredService.receiveInfraredCommand();

        // ignore RAW / invalid
        if (decoded.getProtocol() == RAW) {
            continue;
        }

        terminalView.println("");
        terminalView.println("Infrared signal received:");
        terminalView.println("  Protocol : " + InfraredProtocolMapper::toString(decoded.getProtocol()));
        terminalView.println("  Device   : " + std::to_string(decoded.getDevice()));
        terminalView.println("  SubDev   : " + std::to_string(decoded.getSubdevice()));
        terminalView.println("  Command  : " + std::to_string(decoded.getFunction()));
        terminalView.println("");

        // Save the command ?
        if (!userInputManager.readYesNo("Save this command?", true)) {
            terminalView.println("\nSkipped. Press [ENTER] to stop or wait for next signal...\n");
            continue;
        }

        // If yes, Ask function name
        std::string defFunc = "cmd_" + std::to_string(cmds.size() + 1);
        std::string funcName = userInputManager.readSanitizedString("Enter function name", defFunc, false);
        if (funcName.empty()) funcName = defFunc;

        // Build cmd
        InfraredFileRemoteCommand cmd;
        cmd.functionName = funcName;
        cmd.protocol     = decoded.getProtocol();

        // Address
        uint8_t device = static_cast<uint8_t>(decoded.getDevice() & 0xFF);
        uint8_t sub    = static_cast<uint8_t>((decoded.getSubdevice() < 0 ? 0 : decoded.getSubdevice()) & 0xFF);
        cmd.address     = (static_cast<uint16_t>(sub) << 8) | device;

        cmd.function    = static_cast<uint8_t>(decoded.getFunction() & 0xFF);

        // Unused for non-RAW
        cmd.rawData = nullptr;
        cmd.rawDataSize = 0;
        cmd.frequency = 0;
        cmd.dutyCycle = 0.0f;

        cmds.push_back(cmd);

        terminalView.println("\n✅ Saved '" + funcName + "'\n");
        terminalView.println("INFRARED Record: Waiting for next signal... Press [ENTER] to stop and save.\n");
    }

    infraredService.stopReceiver();

    if (cmds.empty()) {
        terminalView.println("INFRARED Record: No commands saved.\n");
        return;
    }

    // Ask filename
    std::string defName = "ir_record_" + std::to_string(millis() % 1000000); // court
    std::string fileBase = userInputManager.readSanitizedString("Enter file name", defName, false);
    if (fileBase.empty()) fileBase = defName;

    std::string path = "/" + fileBase;
    if (path.size() < 4 || path.substr(path.size() - 3) != ".ir") {
        path += ".ir";
    }

    // Serialize to file format
    std::string text = infraredRemoteTransformer.transformToFileFormat(fileBase, cmds);

    // Write to LittleFS
    if (!littleFsService.write(path, text)) {
        terminalView.println("INFRARED Record: Failed to write: " + path);
        return;
    }

    terminalView.println("\n✅ INFRARED Record: Saved file: " + path);
    terminalView.println("Use 'load' command or connect to Web Terminal to get the file.\n");
}

bool InfraredController::recordFrames(std::vector<IRFrame>& tape) {
    tape.clear();
    tape.reserve(MAX_IR_FRAMES);

    terminalView.println("INFRARED Replay: Recording raw IR frames (max 64)... Press [ENTER] to stop.\n");

    // Start the capture
    infraredService.startReceiver();
    uint32_t lastMillis = millis();
    while (true) {
        // Stop if Enter pressed
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') break;

        // Max frames reached
        if (tape.size() >= MAX_IR_FRAMES) {
            terminalView.println("\nINFRARED Replay: Reached maximum of 64 frames, stopping recording...\n");
            break;
        }

        // Attempt to capture
        std::vector<uint16_t> timings;
        uint32_t khz = 0;
        if (infraredService.receiveRaw(timings, khz)) {
            const uint32_t now = millis();
            const uint32_t gap = tape.empty() ? 0u : (now - lastMillis);
            lastMillis = now;

            tape.push_back(IRFrame{ std::move(timings), khz, gap });
            terminalView.println(
                "  📥 Captured frame #" + std::to_string(tape.size()) +
                " (gap " + std::to_string(gap) + " ms, carrier " + std::to_string(khz) + " kHz)"
            );
        }
    }
    infraredService.stopReceiver();

    // Nothing
    if (tape.empty()) {
        terminalView.println("INFRARED Replay: No frames captured. Nothing to replay.");
        return false;
    }

    return true;
}

void InfraredController::playbackFrames(const std::vector<IRFrame>& tape, uint32_t replayCount) {
    if (replayCount == 0) {
        terminalView.println("\nINFRARED Replay: Playing back with original delays. Press [ENTER] to stop.\n");
    } else {
        terminalView.println("\nINFRARED Replay: Playing back " + std::to_string(replayCount) +
                             " time(s) with original delays. Press [ENTER] to stop.\n");
    }

    // Loop through the frames and send them
    uint32_t playedLoops = 0;
    while (true) {
        if (replayCount > 0 && playedLoops >= replayCount) break;

        for (size_t i = 0; i < tape.size(); ++i) {
            const auto& f = tape[i];

            // Check for Enter press and wait for gap
            uint32_t start = millis();
            while (millis() - start < f.gapMs) {
                char c = terminalInput.readChar();
                if (c == '\r' || c == '\n') {
                    terminalView.println("\nINFRARED Replay: Stopped by user.");
                    return;
                }
                delay(1);
            }

            // Log and send frame
            terminalView.println(
                "  📤 Sending frame #" + std::to_string(i) +
                " (gap " + std::to_string(f.gapMs) + " ms, carrier " + std::to_string(f.khz) + " kHz)"
            );
            infraredService.sendRaw(f.timings, f.khz);
        }
        ++playedLoops;
    }

    terminalView.println("\nINFRARED Replay: Completed (" + std::to_string(playedLoops) + " loop(s)).");
}

/*
Set protocol
*/
void InfraredController::handleSetProtocol() {
    terminalView.println("");
    terminalView.println("Select Infrared Protocol:");

    std::vector<InfraredProtocolEnum> protocols;

    for (int i = 0; i <= static_cast<int>(RAW); ++i) {
        InfraredProtocolEnum proto = static_cast<InfraredProtocolEnum>(i);
        std::string name = InfraredProtocolMapper::toString(proto);

        // avoid double name
        if (!name.empty() && 
            std::find_if(protocols.begin(), protocols.end(),
                [proto](InfraredProtocolEnum e) { return InfraredProtocolMapper::toString(e) == InfraredProtocolMapper::toString(proto); }) == protocols.end()) {
            protocols.push_back(proto);
            terminalView.println("  " + std::to_string(protocols.size()) + ". " + name);
        }
    }

    terminalView.println("");
    terminalView.print("Protocol Number > ");

    std::string inputStr;
    while (true) {
        char c = terminalInput.handler();
        if (c == '\r' || c == '\n') {
            terminalView.println("");
            break;
        }

        if (c == CARDPUTER_SPECIAL_ARROW_DOWN || 
            c == CARDPUTER_SPECIAL_ARROW_UP) {
            terminalView.print(std::string(1, c));
            continue;
        }        

        if (std::isdigit(c)) {
            if (c != CARDPUTER_SPECIAL_ARROW_DOWN || c != CARDPUTER_SPECIAL_ARROW_UP) {
                inputStr += c;
            }
            terminalView.print(std::string(1, c));
        } else {
            terminalView.println("\nInvalid input: only digits allowed.");
            return;
        }
    }

    if (inputStr.empty()) {
        terminalView.println("No input given.");
        return;
    }

    int index = std::stoi(inputStr);
    if (index >= 1 && index <= static_cast<int>(protocols.size())) {
        InfraredProtocolEnum selected = protocols[index - 1];
        GlobalState::getInstance().setInfraredProtocol(selected);
        terminalView.println("Protocol changed to " + InfraredProtocolMapper::toString(selected));
    } else {
        terminalView.println("Invalid protocol number.");
    }
}

/*
Load
*/
void InfraredController::handleLoad(TerminalCommand const& command) {
    if (!littleFsService.mounted()) {
        littleFsService.begin();
        return;
    }

    // Get IR files from LittleFS
    auto files = littleFsService.listFiles(/*root*/ "/", ".ir");
    if (files.empty()) {
        terminalView.println("INFRARED: No .ir files found in LittleFS root ('/').");
        return;
    }

    // Select file
    terminalView.println("\n=== '.ir' files in LittleFS ===");
    uint16_t idxFile = userInputManager.readValidatedChoiceIndex("File number", files, 0);
    const std::string& chosen = files[idxFile];

    // Check size
    int MAX_FILE_SIZE = 32 * 1024; // 32 KB
    auto fileSize = littleFsService.getFileSize("/" + chosen);
    if (fileSize == 0 || fileSize > MAX_FILE_SIZE) {
        terminalView.println("\nINFRARED: File size invalid (>32KB): " + chosen);
        return;
    }

    // Load file content
    std::string text;
    if (!littleFsService.readAll("/" + chosen, text)) {
        terminalView.println("\nINFRARED: Failed to read file: " + chosen);
        return;
    }

    // Verify format
    if (!infraredRemoteTransformer.isValidInfraredFile(text)) {
        terminalView.println("\nINFRARED: Unrecognized .ir format or empty: " + chosen);
        return;
    }

    // Extract commands
    auto cmds = infraredRemoteTransformer.transformFromFileFormat(text);
    if (cmds.empty()) {
        terminalView.println("\nINFRARED: No commands found in: " + chosen);
        return;
    }

    // Cmds names
    auto cmdStrings = infraredRemoteTransformer.extractFunctionNames(cmds);
    cmdStrings.push_back("Exit File"); // for exit option

    while (true) {
        // Select command
        terminalView.println("\n=== Commands in file '" + chosen + "' ===");
        uint8_t idxCmd = userInputManager.readValidatedChoiceIndex("Command number", cmdStrings, 0);
        if (idxCmd == cmdStrings.size()-1) {
            terminalView.println("Exiting command send...\n");
            break;
        }

        // Send
        infraredService.sendInfraredFileCommand(cmds[idxCmd]);
        terminalView.println("\n ✅  Sent command '" + cmds[idxCmd].functionName + "' from file '" + chosen + "'");
    }
}

/*
Config
*/
void InfraredController::handleConfig() {
    terminalView.println("\nInfrared Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t txPin = userInputManager.readValidatedPinNumber("Infrared TX pin", state.getInfraredTxPin(), forbidden);
    uint8_t rxPin = userInputManager.readValidatedPinNumber("Infrared RX pin", state.getInfraredRxPin(), forbidden);

    state.setInfraredTxPin(txPin);
    state.setInfraredRxPin(rxPin);
    infraredService.configure(txPin, rxPin);

    // Protocol
    auto selectedProtocol = InfraredProtocolMapper::toString(state.getInfraredProtocol());
    terminalView.println("Current protocol: '" + selectedProtocol + "'");
    terminalView.println("You can change it with 'setprotocol'");

    // Infrared need EXT power on Sticks3
    #ifdef DEVICE_STICKS3
    {
        auto confirm = userInputManager.readYesNo(
            "Enable power for built-in IR on StickS3?", true
        );
        const uint32_t t0 = millis();
        bool done = false;

        while (millis() - t0 < 1000) { // retry for 1 sec
            M5.Power.setExtOutput(confirm);

            // Done when current state matches requested state
            if (M5.Power.getExtOutput() == confirm) {
                done = true;
                break;
            }
            delay(100);
        }

        pinMode(state.getInfraredRxPin(), INPUT_PULLUP);
    }
    #endif

    terminalView.println("Infrared configured.\n");
}

/*
Jam
*/
void InfraredController::handleJam() {
    // Mode
    std::vector<std::string> modes = infraredService.getJamModeStrings();
    uint16_t midx = userInputManager.readValidatedChoiceIndex("Select Jam Mode", modes, 0);

    // kHz
    uint16_t khz = 38;
    if (modes[midx] == "carrier") {
        std::vector<std::string> khzChoices = infraredService.getCarrierStrings();
        uint16_t kidx = userInputManager.readValidatedChoiceIndex("Select Carrier kHz", khzChoices, 3);
        khz = (uint16_t)std::stoi(khzChoices[kidx]);
    }

    // density
    uint8_t density =  userInputManager.readValidatedInt("Density (1-20)", 10, 1, 20);

    terminalView.println("\nINFRARED Jam: Sending random signals...");
    terminalView.println("Press [ENTER] to stop.");

    uint32_t sweepIdx = 0;
    uint32_t bursts = 0;

    while (true) {
        // Stop ENTER
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("\nINFRARED Jam: Stopped by user.");
            break;
        }

        infraredService.sendJam(midx, khz, sweepIdx, density);
        bursts++;
    }
}

/*
Help
*/
void InfraredController::handleHelp() {
    terminalView.println("Unknown INFRARED command. Usage:");
    terminalView.println("  send <addr> <subadd> <cmd>");
    terminalView.println("  receive");
    terminalView.println("  setprotocol");
    terminalView.println("  devicebgone");
    terminalView.println("  remote");
    terminalView.println("  replay");
    terminalView.println("  record");
    terminalView.println("  load");
    terminalView.println("  jam");
    terminalView.println("  config");
}

void InfraredController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Always reconfigure before use
    uint8_t tx = state.getInfraredTxPin();
    uint8_t rx = state.getInfraredRxPin();
    infraredService.configure(tx, rx);
}