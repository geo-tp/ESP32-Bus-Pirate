#include "GuideShell.h"
#include "Managers/UserInputManager.h"
#include "Transformers/ArgTransformer.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"

GuideShell::GuideShell(ITerminalView& tv,
                       IInput& in,
                       UserInputManager& uim)
    : terminalView(tv)
    , terminalInput(in)
    , userInputManager(uim) {}

void GuideShell::run() {
    bool loop = true;
    
    while (loop) {
        terminalView.println("\n=== Guide Shell ===");
        int choice = userInputManager.readValidatedChoiceIndex("Select a topic", actions, actionsCount, actionsCount - 1);
        switch (choice) {
            case 0: cmdQuickStart(); break;
            case 1: cmdExamples(); break;
            case 2: cmdWebTerminal(); break;
            case 3: cmdInstructionSyntax(); break;
            case 4: cmdPythonAutomation(); break;
            case 5: cmdLittleFS(); break;
            case 6: cmdSafety(); break;
            default:
                loop = false;
                break;
        }
    }

    terminalView.println("Exiting Guide Shell...\n");
}

void GuideShell::cmdQuickStart() {
    terminalView.println("\n=== Quick Start ===\n");

    terminalView.println("What is it for:");
    terminalView.println("  Explore and debug hardware and radio buses.");
    terminalView.println("");

    terminalView.println("Modes:");
    terminalView.println("  Select the mode to work with");
    terminalView.println("  You can type 'mode' or 'm'.");
    terminalView.println("");

    terminalView.println("Command syntax:");
    terminalView.println("  <arg>  required argument");
    terminalView.println("  [arg]  optional argument");
    terminalView.println("");
    terminalView.println("  Example:");
    terminalView.println("    read <addr> <reg>");
    terminalView.println("    read 0x3C 0x00");
    terminalView.println("");
    terminalView.println("    dump <addr> [len]");
    terminalView.println("    dump 0x50 256");
    terminalView.println("");

    terminalView.println("Shell menus:");
    terminalView.println("  Some commands open a sub-menu.");
    terminalView.println("  Pick an action by index number.");
    terminalView.println("  Use Exit to return.");
    terminalView.println("");

    terminalView.println("Tip:");
    terminalView.println("  Use 'help' anywhere for full command lists.");
    terminalView.println("  See the Wiki for detailed workflows.");
    terminalView.println("  https://github.com/geo-tp/ESP32-Bus-Pirate/wiki");
}

void GuideShell::cmdExamples() {
    terminalView.println("\n=== Modes : common workflows ===\n");

    terminalView.println("[WIFI] Connect and explore:");
    terminalView.println("  mode wifi");
    terminalView.println("  scan");
    terminalView.println("  connect");
    terminalView.println("  status");
    terminalView.println("  nmap 192.168.1.10 -p 22");
    terminalView.println("  lookup mac 44:38:39:ff:ef:57");
    terminalView.println("  nc 192.168.1.12 80");
    terminalView.println("  ap MyHotspot 12345678");
    terminalView.println("  ping google.com ");
    terminalView.println("");

    terminalView.println("[I2C] Debug a module:");
    terminalView.println("  mode i2c");
    terminalView.println("  scan");
    terminalView.println("  ping 0x13");
    terminalView.println("  read 0x13 0x00      (read reg)");
    terminalView.println("  write 0x13 0x01 0x0 (write reg)");
    terminalView.println("  monitor 0x13 500    (watch changes)");
    terminalView.println("");

    terminalView.println("[DIO] Drive / observe pins:");
    terminalView.println("  mode dio");
    terminalView.println("  read 1");
    terminalView.println("  set 1 L");
    terminalView.println("  set 1 HIGH");
    terminalView.println("  set 1 O");
    terminalView.println("  toggle 1 250");
    terminalView.println("  sniff 1");
    terminalView.println("");

    terminalView.println("[UART] Serial target:");
    terminalView.println("  mode uart");
    terminalView.println("  scan");
    terminalView.println("  autobaud");
    terminalView.println("  write \"AT\"");
    terminalView.println("  read");
    terminalView.println("  spam Hello 1000");
    terminalView.println("");

    terminalView.println("Note:");
    terminalView.println("  Examples only.");
    terminalView.println("  See the Wiki for detailed workflows.");
    terminalView.println("  https://github.com/geo-tp/ESP32-Bus-Pirate/wiki");
}

void GuideShell::cmdWebTerminal() {
    terminalView.println("\n=== Web terminal ===\n");

    terminalView.println("Use the web interface:");
    terminalView.println(" mode wifi");
    terminalView.println(" connect");
    terminalView.println(" reboot, pick WiFi Web");
    terminalView.println("");

    terminalView.println("Barebone boards (no screen):");
    terminalView.println("  Boot Wi-Fi Web mode:");
    terminalView.println("    • Reset device");
    terminalView.println("    • Press board button < 3s");
    terminalView.println("    • LED status:");
    terminalView.println("       White : connecting");
    terminalView.println("       Blue  : no Wi-Fi saved");
    terminalView.println("       Green : connected");
    terminalView.println("       Red   : failed");
    terminalView.println("");
    terminalView.println("  Important:");
    terminalView.println("    Do not hold BOOT on power");
    terminalView.println("");

    terminalView.println("Notes:");
    terminalView.println("  • Some commands cut sessions");
    terminalView.println("    ex: wifi disconnect with Web Terminal");
    terminalView.println("    ex: usb reset / usb mode with Serial Terminal");
    terminalView.println("");
    terminalView.println("Tip:");
    terminalView.println("  Use Serial for heavy output.");
    terminalView.println("  ex: sniffers (I2C/1WIRE).");
}

void GuideShell::cmdInstructionSyntax() {
    terminalView.println("\n=== Instruction syntax [ ... ] ===\n");

    terminalView.println("Purpose:");
    terminalView.println("  Send low-level bus actions.");
    terminalView.println("");

    terminalView.println("How it works:");
    terminalView.println("  Everything inside [ ] is an instruction.");
    terminalView.println("  Parsed into byte-level actions.");
    terminalView.println("  Execution depends on the active mode.");
    terminalView.println("");

    terminalView.println("Common examples:");
    terminalView.println("  [0xAA 0xBB]          write bytes");
    terminalView.println("  [r:4]                read 4 bytes");
    terminalView.println("  [\"ABC\"]            write ASCII string");
    terminalView.println("  [d:10]               delay 10 us");
    terminalView.println("  [D:1]                delay 1 ms");
    terminalView.println("");

    terminalView.println("Combined examples:");
    terminalView.println("  [0xA0 d:10, r:2 0xB1 r]");
    terminalView.println("    write, delay, read 2 bytes, write, read");
    terminalView.println("");
    terminalView.println("  [d:100 D:2]");
    terminalView.println("    delay 100 us, then 2 ms");
    terminalView.println("");
    terminalView.println("  [\"AT\" d:10 r:255]");
    terminalView.println("    write AT, wait, read reply");
    terminalView.println("");
}

void GuideShell::cmdPythonAutomation() {
    terminalView.println("\n=== Python Automation (Serial) ===\n");
    terminalView.println("Repo: ESP32-Bus-Pirate-Scripts");
    terminalView.println("https://github.com/geo-tp/ESP32-Bus-Pirate-Scripts");
    terminalView.println("");
    terminalView.println("Minimal example:");
    terminalView.println("  bp = BusPirate.auto_connect()");
    terminalView.println("  bp.start()");
    terminalView.println("  bp.change_mode(\"dio\")");
    terminalView.println("  bp.send(\"set 1 LOW\")");
    terminalView.println("  response = bp.receive_all(2)");
    terminalView.println("  bp.stop()");
}

void GuideShell::cmdLittleFS() {
    terminalView.println("\n=== LittleFS / Web UI Files ===\n");
    terminalView.println("LittleFS is a small filesystem stored in flash.");
    terminalView.println("Use the Web UI: Files button to Upload/Download/Delete.");
    terminalView.println("You can, for example, load and record infrared files.");
    terminalView.println("");
    terminalView.println("Limits:");
    terminalView.println("  • ~4.5 MB usable on boards with 8MB flash (approx).");
    terminalView.println("  • Flashing a different firmware may overwrite files.");
}

void GuideShell::cmdSafety() {
    terminalView.println("\n=== Safety / Voltage ===\n");
    terminalView.println("Use 3.3V / 5V only.");
    terminalView.println("Do not connect higher voltage peripherals (risk of ESP32 damage).");
}
