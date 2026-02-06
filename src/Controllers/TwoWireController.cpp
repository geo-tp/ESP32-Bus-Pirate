#include "TwoWireController.h"

/*
Constructor
*/
TwoWireController::TwoWireController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    UserInputManager& userInputManager,
    TwoWireService& twoWireService,
    SmartCardShell& smartCardShell,
    HelpShell& helpShell
)
    : terminalView(terminalView)
    , terminalInput(terminalInput)
    , userInputManager(userInputManager)
    , twoWireService(twoWireService)
    , smartCardShell(smartCardShell)
    , helpShell(helpShell)
{
}

/*
Entry point for 2WIRE command
*/
void TwoWireController::handleCommand(const TerminalCommand& cmd) {
    if      (cmd.getRoot() == "config")    handleConfig();
    else if (cmd.getRoot() == "sniff")     handleSniff();
    else if (cmd.getRoot() == "smartcard") handleSmartCard(cmd);
    else                                   handleHelp();
}

/*
Entry point for 2WIRE instruction
*/
void TwoWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("[TODO] Instruction support for 2WIRE not yet implemented.");
}

/*
Sniff
*/
void TwoWireController::handleSniff() {
    ensureConfigured();
    terminalView.println("2WIRE Sniffer: Running on CLK/IO... Press [ENTER] to stop\r\n");

    if (!twoWireService.startSniffer()) {
        terminalView.println("Failed to start sniffer (check pins/config).");
        return;
    }

    // Event types
    const uint8_t EVT_START = 1;
    const uint8_t EVT_STOP  = 2;
    const uint8_t EVT_DATA  = 3;

    std::vector<uint8_t> frame; // Accumulates bytes between START and STOP
    bool running = true;

    while (running) {
        // Drain events produced by the sniffer
        uint8_t t, d;
        while (twoWireService.getNextSniffEvent(t, d)) {
            if (t == EVT_START) {
                frame.clear();
            }
            else if (t == EVT_DATA) {
                frame.push_back(d);
            }
            else if (t == EVT_STOP) {
                // Print cmd on a single line
                if (frame.size() == 3) {
                    //  3 bytes => command (OP, A, B)
                    uint8_t op = frame[0], A = frame[1], B = frame[2];
                    const char* name = "UNKNOWN";
                    switch (op) {
                        case 0x30: name = "READ_MAIN";        break;
                        case 0x31: name = "READ_SECURITY";     break;
                        case 0x34: name = "READ_PROTECTION";   break;
                        case 0x33: name = "COMPARE_PSC_BYTE";  break;
                        case 0x38: name = "WRITE_MAIN";        break;
                        case 0x39: name = "WRITE_SECURITY";    break;
                        case 0x3C: name = "WRITE_PROTECTION";  break;
                    }
                    char line[96];
                    snprintf(line, sizeof(line),
                             "CMD %-16s : [%02X %02X %02X]\r\n",
                             name, op, A, B);
                    terminalView.print(line);
                } else {
                    // Otherwise, consider it a response
                    terminalView.print("RESP data            :");
                    for (size_t i = 0; i < frame.size(); ++i) {
                        char buf[6];
                        snprintf(buf, sizeof(buf), " %02X", frame[i]);
                        terminalView.print(buf);
                    }
                    terminalView.print("\r\n");
                }
                frame.clear();
            }
        }

        // Exit on [ENTER]
        int ch = terminalInput.readChar();
        if (ch == '\r' || ch == '\n') {
            running = false;
        }
    }

    twoWireService.stopSniffer();
    terminalView.println("\r\n2WIRE Sniffer: Stopped by user.");
}

/*
Smartcard
*/
void TwoWireController::handleSmartCard(const TerminalCommand& cmd) {
    smartCardShell.run();
}

/*
Configuration
*/
void TwoWireController::handleConfig() {
    terminalView.println("2WIRE Configuration:");
    const auto& forbidden = state.getProtectedPins();

    uint8_t clk = userInputManager.readValidatedPinNumber("CLK pin", state.getTwoWireClkPin(), forbidden);
    state.setTwoWireClkPin(clk);

    uint8_t io = userInputManager.readValidatedPinNumber("IO pin", state.getTwoWireIoPin(), forbidden);
    state.setTwoWireIoPin(io);

    uint8_t rst = userInputManager.readValidatedPinNumber("RST pin", state.getTwoWireRstPin(), forbidden);
    state.setTwoWireRstPin(rst);

    twoWireService.configure(clk, io, rst);

    terminalView.println("2WIRE configuration applied.\n");
}

/*
Help
*/
void TwoWireController::handleHelp() {
    terminalView.println("\nUnknown command. Available 2WIRE commands:");
    helpShell.run(state.getCurrentMode(), false);
}

/*
Ensure Configuration
*/
void TwoWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    } 

    twoWireService.configure(
        state.getTwoWireClkPin(),
        state.getTwoWireIoPin(),
        state.getTwoWireRstPin()
    );
}


