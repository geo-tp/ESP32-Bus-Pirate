#include "Controllers/UtilityController.h"

/*
Constructor
*/
UtilityController::UtilityController(ITerminalView& terminalView, IInput& terminalInput, PinService& pinService, UserInputManager& userInputManager)
    : terminalView(terminalView), terminalInput(terminalInput), pinService(pinService), userInputManager(userInputManager) {}

/*
Entry point for command
*/
void UtilityController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "help" || cmd.getRoot() == "h" || cmd.getRoot() == "?") {
        handleHelp();
    }

    else if (cmd.getRoot() == "P") {
        handleEnablePullups();
    }

    else if (cmd.getRoot() == "p") {
        handleDisablePullups();
    }

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

    for (int i = 0; i <= static_cast<int>(ModeEnum::JTAG); ++i) {
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

        default:
            terminalView.println("Pull-ups not applicable for this mode.");
            break;
    }
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
    terminalView.println("  mode <name>          - Set active mode");
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
    terminalView.println("  write id <8 bytes>   - Write ibutton ID");
    terminalView.println("  write sp <8 bytes>   - Write scratchpad");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0xAA r:8] ...       - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 3. UART:");
    terminalView.println("  scan                 - Auto baud detect");
    terminalView.println("  ping                 - Send and expect reply");
    terminalView.println("  read                 - Read at current baud");
    terminalView.println("  write <text>         - Send at current baud");
    terminalView.println("  bridge               - Full-duplex mode");
    terminalView.println("  glitch               - Timing attack");
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
    terminalView.println("  sniff                - View traffic");
    terminalView.println("  read <addr> <reg>    - Read register");
    terminalView.println("  write <a> <r> <val>  - Write register");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [0x13 0x4B 0x1]      - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 6. SPI:");
    terminalView.println("  sniff                - View traffic");
    terminalView.println("  sdcard               - SD operations");
    terminalView.println("  flash probe          - Identify chip");
    terminalView.println("  flash read           - Read content");
    terminalView.println("  flash write          - Write to flash");
    terminalView.println("  flash erase          - Erase flash");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  [..]                 - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 7. 2WIRE:");
    terminalView.println("  sniff                - View traffic");
    terminalView.println("  [..]                 - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 8. 3WIRE:");
    terminalView.println("  [..]                 - Instruction syntax");

    terminalView.println("");
    terminalView.println(" 9. DIO:");
    terminalView.println("  sniff <pin>          - Track toggle states");
    terminalView.println("  read <pin>           - Get pin state");
    terminalView.println("  set <pin> <H/L/I/O>  - Set pin state");
    terminalView.println("  pullup <pin>         - Set pin pullup");
    terminalView.println("  pwm <pin> freq <dut> - Set PWM on pin");
    terminalView.println("  reset <pin>          - Reset to default");

    terminalView.println("");
    terminalView.println(" 10. LED:");
    terminalView.println("  write i <r> <g> <b>  - Set LED color");
    terminalView.println("  setprotocol          - Select LED type");
    terminalView.println("  test                 - Run animation");
    terminalView.println("  config               - Configure settings");
    terminalView.println("  reset                - Turn off all LEDs");

    terminalView.println("");
    terminalView.println(" 11. INFRARED:");
    terminalView.println("  send <dev> sub <cmd> - Send IR signal");
    terminalView.println("  receive              - Receive IR signal");
    terminalView.println("  setprotocol          - Set IR protocol type");
    terminalView.println("  devicebgone          - OFF devices blast");
    terminalView.println("  config               - Configure settings");

    terminalView.println("");
    terminalView.println(" 12. USB:");
    terminalView.println("  stick                - Mount SD as USB");
    terminalView.println("  keyboard <text>      - Type over USB");
    terminalView.println("  mouse <x> <y>        - Move mouse cursor");
    terminalView.println("  mouse click          - Left click");
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
    terminalView.println("  keyboard <text>      - Type over Bluetooth");
    terminalView.println("  mouse <x> <y>        - Move mouse cursor");
    terminalView.println("  mouse click          - Mouse click");
    terminalView.println("  reset                - Reset interface");

    terminalView.println("");
    terminalView.println(" 14. WIFI:");
    terminalView.println("  scan                 - List Wi-Fi networks");
    terminalView.println("  ping <host>          - Ping a remote host");
    terminalView.println("  sniff                - Monitor Wi-Fi packets");
    terminalView.println("  connect <ssid> <pw>  - Connect to a network");
    terminalView.println("  spoof ap <mac>       - Spoof AP MAC");
    terminalView.println("  spoof sta <mac>      - Spoof Station MAC");
    terminalView.println("  status               - Show Wi-Fi status");
    terminalView.println("  disconnect           - Disconnect from Wi-Fi");
    terminalView.println("  ap <ssid> <password> - Set access point");
    terminalView.println("  webui                - Show the web UI IP");
    terminalView.println("  reset                - Reset interface");

    terminalView.println("");
    terminalView.println(" 15. JTAG:");
    terminalView.println("  scan                 - Scan JTAG/SWD pins");

    terminalView.println("");
    terminalView.println(" Instructions (available in most modes):");
    terminalView.println(" See documentation for instruction syntax.");
    terminalView.println("");
    terminalView.println(" Note: Use 'mode' to switch between modes");
    terminalView.println("");
}

bool UtilityController::isGlobalCommand(const TerminalCommand& cmd) {
    std::string root = cmd.getRoot();
    return (root == "help" || root == "h" || root == "?" ||
            root == "mode" || root == "m" || root == "l"
                           || root == "P" || root == "p");
}