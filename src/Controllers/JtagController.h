#pragma once

// Placeholder implementation for JTAG controller

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"

class JtagController {
public:
    // Constructor
    JtagController(ITerminalView& terminalView, IInput& terminalInput);

    // Entry point for dispatch incoming JTAG command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure configuration is done before running commands
    void ensureConfigured();

private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    bool configured = false;

    // Perform a JTAG device scan (to be implemented)
    void handleScan();

    // Run JTAG configuration routine (stub)
    void handleConfig();
};
