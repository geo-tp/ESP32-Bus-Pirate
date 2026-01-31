#pragma once

#include <sstream> 
#include <string>
#include <algorithm>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/I2cService.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Vendors/i2c_sniffer.h"
#include "Shells/I2cEepromShell.h"
#include "Shells/HelpShell.h"
#include "Data/I2cKnownAdresses.h"

class I2cController {
public:
    // Constructor
    I2cController(ITerminalView& terminalView, IInput& terminalInput, I2cService& i2cService, ArgTransformer& argTransformer, UserInputManager& userInputManager, I2cEepromShell& eepromShell, HelpShell& helpShell);

    // Entry point for I2C command
    void handleCommand(const TerminalCommand& cmd);

    // Entry point for compiled bytecode instructions
    void handleInstruction(const std::vector<ByteCode>& bytecodes);

    // Ensure I2C is configured before use
    void ensureConfigured();
    
private:
    struct Stats {
        int tries = 0;
        int ok = 0;
        int nack = 0;
        uint32_t minUs = 0xFFFFFFFF;
        uint32_t maxUs = 0;
        uint32_t sumUs = 0;

        void add(bool success, uint32_t dt) {
            tries++;
            if (success) {
                ok++;
                sumUs += dt;
                if (dt < minUs) minUs = dt;
                if (dt > maxUs) maxUs = dt;
            } else {
                nack++;
            }
        }

        uint32_t avgUs() const { return (ok > 0) ? (sumUs / (uint32_t)ok) : 0; }
        uint32_t jitterUs() const { return (ok > 0) ? (maxUs - minUs) : 0; }
    };

    ITerminalView& terminalView;
    IInput& terminalInput;
    I2cService& i2cService;
    ArgTransformer& argTransformer;
    UserInputManager& userInputManager;
    I2cEepromShell& eepromShell;
    HelpShell& helpShell;
    GlobalState& state = GlobalState::getInstance();
    bool configured = false;
    
    // Ping an I2C address
    void handlePing(const TerminalCommand& cmd);

    // Scan the I2C bus for devices
    void handleScan();

    // Start sniffing I2C traffic passively
    void handleSniff();

    // Read data from an I2C device
    void handleRead(const TerminalCommand& cmd);

    // Write data to an I2C device
    void handleWrite(const TerminalCommand& cmd);

    // Emulate I2C slave device logging master command
    void handleSlave(const TerminalCommand& cmd);

    // Attempt to glitch an I2C device
    void handleGlitch(const TerminalCommand& cmd);

    // Flood an I2C device with commands
    void handleFlood(const TerminalCommand& cmd);
    
    // Jam the I2C bus with noise
    void handleJam();

    // Configure I2C parameters
    void handleConfig();

    // Swap SDA and SCL pins
    void handleSwap();

    // Monitor I2C device registers
    void handleMonitor(const TerminalCommand& cmd);

    // I2C EEPROM operations
    void handleEeprom(const TerminalCommand& cmd);

    // Show I2C help message
    void handleHelp();

    // Recover I2C bus if stuck
    void handleRecover();

    // Identify I2C device based on address
    void handleIdentify(const TerminalCommand& cmd);

    // Perform health timing test
    void handleHealth(const TerminalCommand& cmd);

    // Discover devices and identify them
    void handleDiscover();

    // Dump I2C registers content
    void handleDump(const TerminalCommand& cmd);
    void performRegisterRead(uint8_t addr, uint16_t, uint16_t len,
                            std::vector<uint8_t>& values, std::vector<bool>& valid);
    void performRawRead(uint8_t addr, uint16_t, uint16_t len,
                        std::vector<uint8_t>& values, std::vector<bool>& valid);
    void printHexDump(uint16_t, uint16_t len,
                    const std::vector<uint8_t>& values, const std::vector<bool>& valid);
    std::string identifyToString(uint8_t address, bool includeHeader = false);
};
