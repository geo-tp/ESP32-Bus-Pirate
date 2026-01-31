#include "JtagController.h"

/*
Constructor
*/
JtagController::JtagController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    JtagService& jtagService,
    UserInputManager& userInputManager,
    HelpShell& helpShell
) : terminalView(terminalView),
    terminalInput(terminalInput),
    jtagService(jtagService),
    userInputManager(userInputManager),
    helpShell(helpShell) {}
/*
Entry point that handles JTAG commands
*/
void JtagController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") handleScan(cmd); 
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Scan
*/
void JtagController::handleScan(const TerminalCommand& cmd) {
    auto type = cmd.getSubcommand();

    if (type[0] == 's') handleScanSwd();
    else if (type[0] == 'j') handleScanJtag();
}

/*
Scan SWD
*/
void JtagController::handleScanSwd() {
    terminalView.println("JTAG: Scanning for SWD devices...");

    uint8_t swdio, swclk;
    uint32_t idcode;
    std::vector<uint8_t> swdCandidates = state.getJtagScanPins();

    bool found = jtagService.scanSwdDevice(swdCandidates, swdio, swclk, idcode);

    if (found) {
        terminalView.println("\n SWD device found!");
        terminalView.println("  • SWDIO  : GPIO " + std::to_string(swdio));
        terminalView.println("  • SWCLK  : GPIO " + std::to_string(swclk));
        terminalView.println("  • IDCODE : 0x" + std::to_string(idcode));
        terminalView.println("  ✅ SWD scan done.\n");
    } else {
        terminalView.println("\nJTAG: No SWD device found on available GPIOs.");
    }
}

/*
Scan JTAG
*/
void JtagController::handleScanJtag() {    
    terminalView.println("JTAG: Scanning for JTAG devices...");

    std::vector<uint8_t> jtagCandidates = state.getJtagScanPins();
    uint8_t tdi, tdo, tck, tms;
    int trst;
    std::vector<uint32_t> ids;

    bool found = jtagService.scanJtagDevice(
        jtagCandidates,
        tdi, tdo, tck, tms, trst,
        ids,
        true, // detect pullup
        nullptr // callback progression
    );

    if (found) {
        terminalView.println("\n JTAG device(s) found!");
        terminalView.println("  • TDI   : GPIO " + std::to_string(tdi));
        terminalView.println("  • TDO   : GPIO " + std::to_string(tdo));
        terminalView.println("  • TCK   : GPIO " + std::to_string(tck));
        terminalView.println("  • TMS   : GPIO " + std::to_string(tms));
        if (trst >= 0) {
            terminalView.println("  • TRST  : GPIO " + std::to_string(trst));
        }

        for (size_t i = 0; i < ids.size(); ++i) {
            char buf[11];
            snprintf(buf, sizeof(buf), "0x%08X", ids[i]);
            terminalView.println("  • IDCODE[" + std::to_string(i) + "] : " + buf);
        }

        terminalView.println("  ✅ Scan complete.\n");
    } else {
        terminalView.println("\nJTAG: No device found on available GPIOs.");
    }
}

/*
Config
*/
void JtagController::handleConfig() {
    terminalView.println("JTAG/SWD Configuration:");

    // Default
    const std::vector<uint8_t> defaultPins = state.getJtagScanPins();

    // Protected
    const std::vector<uint8_t> protectedPins = state.getProtectedPins();

    // Ask user for pin group
    std::vector<uint8_t> selectedPins = userInputManager.readValidatedPinGroup(
        "GPIOs to scan (SWD/JTAG)",
        defaultPins,
        protectedPins
    );

    // Save it
    state.setJtagScanPins(selectedPins);

    // Show confirmation
    terminalView.print("Pins set for the scan (SWD/JTAG): ");
    for (uint8_t pin : selectedPins) {
        terminalView.print(std::to_string(pin) + " ");
    }
    terminalView.println("\r\nJTAG/SWD configured.\n");
}

/*
Help
*/
void JtagController::handleHelp() {
    terminalView.println("\nUnknown command. Available JTAG commands:");
    helpShell.run(state.getCurrentMode(), false);
}

/*
Ensure Configuration
*/
void JtagController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }
}