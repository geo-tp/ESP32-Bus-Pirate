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
    ArgTransformer& argTransformer,
    SysInfoShell& sysInfoShell
)
    : terminalView(terminalView),
      deviceView(deviceView),
      terminalInput(terminalInput),
      pinService(pinService),
      userInputManager(userInputManager),
      argTransformer(argTransformer),
      sysInfoShell(sysInfoShell)
{}

/*
Entry point for command
*/
void UtilityController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "help" || cmd.getRoot() == "h" || cmd.getRoot() == "?") handleHelp();
    else if (cmd.getRoot() == "P")                                               handleEnablePullups();
    else if (cmd.getRoot() == "p")                                               handleDisablePullups();
    else if (cmd.getRoot() == "logic")                                           handleLogicAnalyzer(cmd);
    else if (cmd.getRoot() == "system")                                          handleSystem();
    else {
        terminalView.println("Unknown command. Try 'help'.");
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
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: logic <pin>");
        return;
    }

    // Verify protected pin
    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    auto forbidden = state.getProtectedPins();
    if (std::find(forbidden.begin(), forbidden.end(), pin) != forbidden.end()) {
        terminalView.println("Logic Analyzer: This pin is protected or reserved.");
        return;
    }

    terminalView.println("\nLogic Analyzer: Monitoring pin " + std::to_string(pin) + "... Press [ENTER] to stop.");
    terminalView.println("Displaying waveform on the ESP32 screen...\n");


    pinService.setInput(pin);
    std::vector<uint8_t> buffer;
    buffer.reserve(240); // 240 samples

    unsigned long lastCheck = millis();
    deviceView.clear();
    deviceView.topBar("Logic Analyzer", false, false);

    while (true) {
        // Enter press
        if (millis() - lastCheck > 10) {
            lastCheck = millis();
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("Logic Analyzer: Stopped by user.");
                break;
            }
        }

        // Draw
        if (buffer.size() >= 240) {
            buffer.resize(240);
            deviceView.drawLogicTrace(pin, buffer);
            buffer.clear();
        }

        buffer.push_back(pinService.read(pin));
        delayMicroseconds(500);
    }
}

/*
System Information
*/
void UtilityController::handleSystem() {
    sysInfoShell.run();
}

/*
Help
*/
void UtilityController::handleHelp() {
    terminalView.println("");
    terminalView.println("   +=== Help: Available Commands ===+");
    terminalView.println("");

    terminalView.println(" General:");
    terminalView.println("  help                 - Show this help");
    terminalView.println("  system               - Show system infos");
    terminalView.println("  mode <name>          - Set active mode");
    terminalView.println("  logic <pin>          - Logic analyzer");
    terminalView.println("  P                    - Enable pull-up");
    terminalView.println("  p                    - Disable pull-up");

    terminalView.println("");
    terminalView.println(" 1. HiZ:");
    terminalView.println("  (default mode)       - All lines disabled");

    terminalView.println("");
    terminalView.println(" 2. 1WIRE:");
    terminalView.println("  scan                 - Scan 1-Wire devices");
    terminalView.println("  ping                 - Ping 1-Wire device");
    terminalView.println("  sniff                - View 1-Wire traffic");
    terminalView.println("  read                 - Read ID + SP");
    terminalView.println("  write id [8 bytes]   - Write device ID");
    terminalView.println("  write sp [8 bytes]   - Write scratchpad");
    terminalView.println("  temp                 - Read temperature");
    terminalView.println("  ibutton              - iButton operations");
    terminalView.println("  eeprom               - EEPROM operations");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0xAA r:8] ...       - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 3. UART:");
    terminalView.println("  scan                 - Auto baud detect");
    terminalView.println("  ping                 - Send and expect reply");
    terminalView.println("  read                 - Read at current baud");
    terminalView.println("  write <text>         - Send at current baud");
    terminalView.println("  bridge               - Full-duplex mode");
    terminalView.println("  at                   - AT commands operations");
    terminalView.println("  spam <text> <ms>     - Write text every ms");
    terminalView.println("  glitch               - Timing attack");
    terminalView.println("  xmodem <send> <path> - Send file via XMODEM");
    terminalView.println("  xmodem <recv> <path> - Receive file via XMODEM");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  ['Hello'] [r:64]...  - Instruction syntax");
 
    terminalView.println("");
    terminalView.println(" 4. HDUART:");
    terminalView.println("  bridge               - Half-duplex I/O");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0x1 D:10 r:255]     - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 5. I2C:");
    terminalView.println("  scan                 - Find devices");
    terminalView.println("  ping <addr>          - Check ACK");
    terminalView.println("  identify <addr>      - Identify device");
    terminalView.println("  sniff                - View traffic");
    terminalView.println("  slave <addr>         - Emulate I2C device");
    terminalView.println("  read <addr> <reg>    - Read register");
    terminalView.println("  write <a> <r> <val>  - Write register");
    terminalView.println("  dump <addr> [len]    - Read all registers");
    terminalView.println("  glitch <addr>        - Run attack sequence");
    terminalView.println("  flood <addr>         - Saturate target I/O");
    terminalView.println("  monitor <addr> [ms]  - Monitor register changes");
    terminalView.println("  eeprom [addr]        - I2C EEPROM operations");
    terminalView.println("  recover              - Attempt bus recovery");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0x13 0x4B 0x1]      - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 6. SPI:");
    terminalView.println("  sniff                - View traffic");
    terminalView.println("  sdcard               - SD operations");
    terminalView.println("  slave                - Emulate SPI slave");
    terminalView.println("  flash                - SPI Flash operations");
    terminalView.println("  eeprom               - SPI EEPROM operations");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0x9F r:3]           - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 7. 2WIRE:");
    terminalView.println("  sniff                - View 2WIRE traffic");
    terminalView.println("  smartcard            - Smartcard operations");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0xAB r:4]           - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 8. 3WIRE:");
    terminalView.println("  eeprom               - 3WIRE EEPROM operations");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 9. DIO:");
    terminalView.println("  sniff <pin>          - Track toggle states");
    terminalView.println("  read <pin>           - Get pin state");
    terminalView.println("  set <pin> <H/L/I/O>  - Set pin state");
    terminalView.println("  pullup <pin>         - Set pin pullup");
    terminalView.println("  pulse <pin> <us>     - Send pulse on pin");
    terminalView.println("  servo <pin> <angle>  - Set servo angle");
    terminalView.println("  pwm <pin> freq <dut> - Set PWM on pin");
    terminalView.println("  toggle <pin> <ms>    - Toggle pin periodically");
    terminalView.println("  measure <pin> [ms]   - Calculate frequency");
    terminalView.println("  analog <pin>         - Read analog value");
    terminalView.println("  reset <pin>          - Reset to default");

    terminalView.println("");
    terminalView.println(" 10. LED:");
    terminalView.println("  scan                 - Try to detect LEDs type");
    terminalView.println("  fill <color>         - Fill all LEDs with a color");
    terminalView.println("  set <index> <color>  - Set specific LED color");
    terminalView.println("  blink                - Blink all LEDs");
    terminalView.println("  rainbow              - Rainbow animation");
    terminalView.println("  chase                - Chasing light effect");
    terminalView.println("  cycle                - Cycle through colors");
    terminalView.println("  wave                 - Wave animation");
    terminalView.println("  reset                - Turn off all LEDs");
    terminalView.println("  setprotocol          - Select LED protocol");
    terminalView.println("  config               - Configure LED settings");

    terminalView.println("");
    terminalView.println(" 11. INFRARED:");
    terminalView.println("  send <dev> sub <cmd> - Send IR signal");
    terminalView.println("  receive              - Receive IR signal");
    terminalView.println("  setprotocol          - Set IR protocol type");
    terminalView.println("  devicebgone          - OFF devices blast");
    terminalView.println("  remote               - Universal remote commands");
    terminalView.println("  replay [count]       - Replay recorded IR frames");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 12. USB:");
    terminalView.println("  stick                - Mount SD as USB");
    terminalView.println("  keyboard             - Start keyboard bridge");
    terminalView.println("  mouse <x> <y>        - Move mouse cursor");
    terminalView.println("  mouse click          - Left click");
    terminalView.println("  mouse jiggle [ms]    - Random mouse moves");
    terminalView.println("  gamepad <key>        - Press button");
    terminalView.println("  reset                - Reset interface");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 13. BLUETOOTH:");
    terminalView.println("  scan                 - Discover devices");
    terminalView.println("  pair <mac>           - Pair with a device");
    terminalView.println("  sniff                - Sniff Bluetooth data");
    terminalView.println("  spoof <mac>          - Spoof mac address");
    terminalView.println("  status               - Show current status");
    terminalView.println("  server               - Create an HID server");
    terminalView.println("  keyboard             - Start keyboard bridge");
    terminalView.println("  mouse <x> <y>        - Move mouse cursor");
    terminalView.println("  mouse click          - Mouse click");
    terminalView.println("  mouse jiggle [ms]    - Random mouse moves");
    terminalView.println("  reset                - Reset interface");

    terminalView.println("");
    terminalView.println(" 14. WIFI:");
    terminalView.println("  scan                 - List Wi-Fi networks");
    terminalView.println("  connect              - Connect to a network");
    terminalView.println("  ping <host>          - Ping a remote host");
    terminalView.println("  discovery            - Discover network devices");
    terminalView.println("  sniff                - Monitor Wi-Fi packets");
    terminalView.println("  probe                - Search for net access");
    terminalView.println("  spoof ap <mac>       - Spoof AP MAC");
    terminalView.println("  spoof sta <mac>      - Spoof Station MAC");
    terminalView.println("  status               - Show Wi-Fi status");
    terminalView.println("  deauth <ssid>        - Deauthenticate hosts");
    terminalView.println("  disconnect           - Disconnect from Wi-Fi");
    terminalView.println("  ap <ssid> <password> - Set access point");
    terminalView.println("  ap spam              - Spam random beacons");
    terminalView.println("  ssh <h> <u> <p> [p]  - Open SSH session");
    terminalView.println("  telnet <host> [port] - Open telnet session");
    terminalView.println("  nc <host> <port>     - Open netcat session");
    terminalView.println("  nmap <h> [-p ports]  - Scan host ports");
    terminalView.println("  modbus <host> [port] - Modbus TCP operations");
    terminalView.println("  http get <url>       - HTTP(s) GET request");
    terminalView.println("  http analyze <url>   - Get analysis report");
    terminalView.println("  lookup mac|ip <addr> - Lookup MAC or IP address");
    terminalView.println("  webui                - Show the web UI IP");
    terminalView.println("  reset                - Reset interface");

    terminalView.println("");
    terminalView.println(" 15. JTAG:");
    terminalView.println("  scan swd             - Scan SWD pins");
    terminalView.println("  scan jtag            - Scan JTAG pins");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 16. I2S:");
    terminalView.println("  play <freq> [ms]     - Play sine wave for ms");
    terminalView.println("  record               - Read mic continuously");
    terminalView.println("  test <speaker|mic>   - Run basic audio tests");
    terminalView.println("  reset                - Reset to default");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 17. CAN:");
    terminalView.println("  sniff                - Print all received frames");
    terminalView.println("  send [id]            - Send frame with given ID");
    terminalView.println("  receive [id]         - Capture frames with ID");
    terminalView.println("  status               - State of the CAN controller");
    terminalView.println("  config               - Configure MCP2515 settings");

    terminalView.println("");
    terminalView.println(" 18. ETHERNET:");
    terminalView.println("  connect              - Connect using DHCP");
    terminalView.println("  status               - Show ETH status");
    terminalView.println("  ping <host>          - Ping a remote host");
    terminalView.println("  discovery            - Discover network devices");
    terminalView.println("  ssh <h> <u> <p> [p]  - Open SSH session");
    terminalView.println("  telnet <host> [port] - Open telnet session");
    terminalView.println("  nc <host> <port>     - Open netcat session");
    terminalView.println("  nmap <h> [-p ports]  - Scan host ports");
    terminalView.println("  modbus <host> [port] - Modbus TCP operations");
    terminalView.println("  http get <url>       - HTTP(s) GET request");
    terminalView.println("  http analyze <url>   - Get analysis report");
    terminalView.println("  lookup mac|ip <addr> - Lookup MAC or IP address");
    terminalView.println("  reset                - Reset interface");
    terminalView.println("  config               - Configure W5500 settings");

    terminalView.println("");
    terminalView.println(" 19. SUBGHZ:");
    terminalView.println("  scan                 - Search frequencies");
    terminalView.println("  sniff                - Raw frame sniffing");
    terminalView.println("  setfrequency         - Set operating frequency");
    terminalView.println("  config               - Configure CC1101 settings");

    terminalView.println("");
    terminalView.println(" 20. RFID:");
    terminalView.println("  read                 - Read RFID tag data");
    terminalView.println("  write                - Write UID/Block to tag");
    terminalView.println("  clone                - Clone Mifare UID");
    terminalView.println("  erase                - Erase RFID tag");
    terminalView.println("  config               - Configure PN532 settings");

    terminalView.println("");
    terminalView.println(" Instructions (available in most modes):");
    terminalView.println(" See documentation for instruction syntax.");
    terminalView.println("");
    terminalView.println(" Note: Use 'mode' to switch between modes");
    terminalView.println("");
}

bool UtilityController::isGlobalCommand(const TerminalCommand& cmd) {
    std::string root = cmd.getRoot();
    return (root == "help"  || root == "h" || root == "?" ||
            root == "mode"  || root == "m" || root == "l" ||
            root == "logic" || root == "P" || root == "p") || 
            root == "system";
}