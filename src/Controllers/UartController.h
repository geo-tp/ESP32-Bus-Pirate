#pragma once

#include <vector>
#include <string>
#include "HardwareSerial.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Services/UartService.h"
#include "Services/HdUartService.h"
#include "Services/SdService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Shells/UartAtShell.h"
#include "Shells/HelpShell.h"
#include "Shells/UartEmulationShell.h"

class UartController {
public:
    // Constructor
    UartController(ITerminalView& terminalView, 
                   IInput& terminalInput,
                   IInput& deviceInput,
                   UartService& uartService, 
                   SdService& sdService,
                   HdUartService& hdUartService, 
                   ArgTransformer& argTransformer,
                   UserInputManager& userInputManager,
                   UartAtShell& uartAtShell,
                   HelpShell& helpShell,
                   UartEmulationShell& uartEmulationShell);
    
    // Entry point for UART command
    void handleCommand(const TerminalCommand& cmd);

    //  Entry point for handle parsed bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);
    
    // Ensure UART is configured before use
    void ensureConfigured();
    
private:
    // Start bidirectional UART bridge
    void handleBridge();
    
    // Perform a simple read
    void handleRead();
    
    // Send probes to get a response
    void handlePing();
    
    // Write data to UART
    void handleWrite(TerminalCommand cmd);

    // Handle AT commands
    void handleAtCommand(const TerminalCommand& cmd);

    // Configure UART settings
    void handleConfig();

    // Display available commands
    void handleHelp();

    // Send glitch pattern over uart
    void handleGlitch();

    // Swap RX and TX pins
    void handleSwap();

    // Scan for active UART pins
    void handleScan();

    // Find the right baudrate
    void handleAutoBaud();

    // Spam given input every given ms
    void handleSpam(const TerminalCommand& cmd);

    // Xmodem cmd handler
    void handleXmodem(const TerminalCommand& cmd);

    // Receive file from xmodem
    void handleXmodemReceive(const std::string& path);

    // Send file to xmodem
    void handleXmodemSend(const std::string& path);

    // Handle UART emulation shell
    void handleEmulation();
    
    ITerminalView& terminalView;
    IInput& terminalInput;
    IInput& deviceInput;
    UartService& uartService;
    SdService& sdService;
    HdUartService& hdUartService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    UartAtShell& uartAtShell;
    HelpShell& helpShell;
    UartEmulationShell& uartEmulationShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    bool scanCancelled = false;

    // Predefined probe commands for ping
    std::vector<std::string> probes = {
        // Generic AT commands
        "AT\r\n",
        "ATE0\r\n", "ATE1\r\n", "ATI\r\n", "ATI0\r\n", "ATI1\r\n", "ATI2\r\n",
        "AT+GMR\r\n", "AT+VERSION?\r\n", "AT+HELP\r\n",

        // Modem/GSM specific
        "AT+CSQ\r\n", "AT+CREG?\r\n", "AT+CGMI\r\n", "AT+CGMM\r\n",
        "AT+CGMR\r\n", "AT+CGSN\r\n",

        // GPS modules
        "$PMTK?\r\n", "$GPTXT\r\n",

        // Miscellaneous
        "help\r\n", "?\r\n", "\r\n",
        "\x1B", "\x03", "\x04", "\x1A", "\x11", "\x13"  // ESC, Ctrl+C/Z/D, XON/XOFF
    };
};
