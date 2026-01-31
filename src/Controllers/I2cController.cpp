#include "I2cController.h"

/*
Constructor
*/
I2cController::I2cController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    I2cService& i2cService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    I2cEepromShell& eepromShell
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      i2cService(i2cService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      eepromShell(eepromShell)
{}

/*
Entry point to handle I2C command
*/
void I2cController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") handleScan();
    else if (cmd.getRoot() == "sniff") handleSniff();
    else if (cmd.getRoot() == "ping") handlePing(cmd);
    else if (cmd.getRoot() == "identify") handleIdentify(cmd);
    else if (cmd.getRoot() == "write") handleWrite(cmd);
    else if (cmd.getRoot() == "read") handleRead(cmd);
    else if (cmd.getRoot() == "dump") handleDump(cmd);
    else if (cmd.getRoot() == "slave") handleSlave(cmd);
    else if (cmd.getRoot() == "glitch") handleGlitch(cmd);
    else if (cmd.getRoot() == "flood") handleFlood(cmd);
    else if (cmd.getRoot() == "jam") handleJam();
    else if (cmd.getRoot() == "eeprom") handleEeprom(cmd);
    else if (cmd.getRoot() == "recover") handleRecover();
    else if (cmd.getRoot() == "monitor") handleMonitor(cmd);
    else if (cmd.getRoot() == "swap") handleSwap();
    else if (cmd.getRoot() == "health") handleHealth(cmd);
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Entry point to handle I2C instruction
*/
void I2cController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = i2cService.executeByteCode(bytecodes);
    if (!result.empty()) {
        terminalView.println("I2C Read:\n");
        terminalView.println(result);
    }
}

/*
Scan
*/
void I2cController::handleScan() {
    terminalView.println("I2C Scan: Scanning I2C bus... Press [ENTER] to stop");
    terminalView.println("");
    bool found = false;

    for (uint8_t addr = 1; addr < 127; ++addr) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("I2C Scan: Cancelled by user.");
            return;
        }
        
        i2cService.beginTransmission(addr);
        if (i2cService.endTransmission() == 0) {
            std::stringstream ss;
            ss << "Found device at 0x" << std::hex << std::uppercase << (int)addr;
            terminalView.println(ss.str());
            found = true;
        }
    }

    if (!found) {
        terminalView.println("I2C Scan: No I2C devices found.");
    }
    terminalView.println("");
}

/*
Sniff
*/    
void I2cController::handleSniff() {
    terminalView.println("I2C Sniffer: Listening on SCL/SDA... Press [ENTER] to stop.\n");
    i2c_sniffer_begin(state.getI2cSclPin(), state.getI2cSdaPin()); // dont need freq to work
    i2c_sniffer_setup();

    std::string line;

    while (true) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        while (i2c_sniffer_available()) {
            char c = i2c_sniffer_read();

            if (c == '\n') {
                line += "  ";
                terminalView.println(line);
                line.clear();
            } else {
                line += c;
            }
        }
        delayMicroseconds(100);
    }

    i2c_sniffer_reset_buffer();
    i2c_sniffer_stop();
    i2cService.configure(state.getI2cSdaPin(), state.getI2cSclPin(), state.getI2cFrequency());
    terminalView.println("\n\nI2C Sniffer: Stopped.");
}

/*
Ping
*/
void I2cController::handlePing(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: ping <I2C address>");
        return;
    }

    const std::string& arg = cmd.getSubcommand();
    uint8_t address = 0;

    std::stringstream ss(arg);
    int temp = 0;

    // Detect hex prefix
    if (arg.rfind("0x", 0) == 0 || arg.rfind("0X", 0) == 0) {
        ss >> std::hex >> temp;
    } else {
        ss >> std::dec >> temp;
    }

    if (ss.fail() || temp < 0 || temp > 127) {
        terminalView.println("I2C Ping: Invalid address format. Use hex (e.g. 0x3C).");
        return;
    }

    address = static_cast<uint8_t>(temp);

    std::stringstream result;
    result << "Ping 0x" << std::hex << std::uppercase << (int)address << ": ";

    i2cService.beginTransmission(address);
    uint8_t i2cResult = i2cService.endTransmission();

    if (i2cResult == 0) {
        result << "I2C Ping: ACK received! Device is present.";
    } else {
        result << "I2C Ping: No response (NACK or error).";
    }

    terminalView.println(result.str());
}

/*
Write
*/
void I2cController::handleWrite(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    // addr
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: write <addr> [reg] [val]");
        return;
    }

    const std::string& addrStr = cmd.getSubcommand();

    if (!argTransformer.isValidNumber(addrStr)) {
        terminalView.println("Error: Invalid address. Use decimal or 0x-prefixed hex.");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(addrStr);

    // reg
    uint8_t reg = 0;
    if (args.size() >= 1) {
        if (!argTransformer.isValidNumber(args[0])) {
            terminalView.println("Error: Invalid register. Use decimal or 0x-prefixed hex.");
            return;
        }
        reg = argTransformer.parseHexOrDec(args[0]);
    } else {
        reg = (uint8_t)userInputManager.readValidatedUint32("Register (e.g. 0x00)", 0);
    }

    // val
    uint8_t val = 0;
    if (args.size() >= 2) {
        if (!argTransformer.isValidNumber(args[1])) {
            terminalView.println("Error: Invalid value. Use decimal or 0x-prefixed hex.");
            return;
        }
        val = argTransformer.parseHexOrDec(args[1]);
    } else {
        val = (uint8_t)userInputManager.readValidatedUint32("Value (e.g. 0x1E)", 0);
    }

    // Ping addr
    i2cService.beginTransmission(addr);
    uint8_t pingResult = i2cService.endTransmission();
    if (pingResult != 0) {
        terminalView.println("I2C Ping: 0x" + argTransformer.toHex(addr) + " no response. Aborting write.");
        return;
    }

    // Write
    i2cService.beginTransmission(addr);
    i2cService.write(reg);
    i2cService.write(val);
    i2cService.endTransmission(true);

    terminalView.println(
        "I2C Write: 0x" + argTransformer.toHex(val) +
        " -> reg 0x" + argTransformer.toHex(reg) +
        " @ dev 0x" + argTransformer.toHex(addr) + "."
    );
}

/*
Read
*/
void I2cController::handleRead(const TerminalCommand& cmd) {
    // addr 
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: read <addr> [reg]");
        return;
    }

    const std::string& addrStr = cmd.getSubcommand();
    if (!argTransformer.isValidNumber(addrStr)) {
        terminalView.println("Error: Invalid address. Use decimal or 0x-prefixed hex.");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(addrStr);

    // reg
    uint8_t reg = 0;
    if (!cmd.getArgs().empty()) {
        if (!argTransformer.isValidNumber(cmd.getArgs())) {
            terminalView.println("Error: Invalid register. Use decimal or 0x-prefixed hex.");
            return;
        }
        reg = argTransformer.parseHexOrDec(cmd.getArgs());
    } else {
        reg = (uint8_t)userInputManager.readValidatedUint32("Register (e.g. 0x00)", 0);
    }

    // Check I2C device presence
    i2cService.beginTransmission(addr);
    if (i2cService.endTransmission()) {
        terminalView.println("I2C Read: No device found at 0x" + argTransformer.toHex(addr));
        return;
    }

    // Write register address first
    i2cService.beginTransmission(addr);
    i2cService.write(reg);
    i2cService.endTransmission(false);

    i2cService.requestFrom(addr, 1);
    if (i2cService.available()) {
        uint8_t value = (uint8_t)i2cService.read();
        terminalView.println(
            "I2C Read: 0x" + argTransformer.toHex(value) +
            " (" + std::to_string((int)value) + ")" +
            " from reg 0x" + argTransformer.toHex(reg) +
            " @ dev 0x" + argTransformer.toHex(addr) + "."
        );
    } else {
        terminalView.println("I2C Read: No data available.");
    }
}

/*
Config
*/
void I2cController::handleConfig() {
    terminalView.println("I2C Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t sda = userInputManager.readValidatedPinNumber("SDA pin", state.getI2cSdaPin(), forbidden);
    state.setI2cSdaPin(sda);

    uint8_t scl = userInputManager.readValidatedPinNumber("SCL pin", state.getI2cSclPin(), forbidden);
    state.setI2cSclPin(scl);

    uint32_t freq = userInputManager.readValidatedUint32("Frequency", state.getI2cFrequency());
    state.setI2cFrequency(freq);

    i2cService.configure(sda, scl, freq);

    terminalView.println("I2C configured.\n");
}

/*
Slave
*/
void I2cController::handleSlave(const TerminalCommand& cmd) {
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: slave <addr>");
        return;
    }

    // Parse arg
    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint8_t sda = state.getI2cSdaPin();
    uint8_t scl = state.getI2cSclPin();

    // Validate arg
    if (addr < 0x08 || addr > 0x77) {
        terminalView.println("I2C Slave: Invalid address. Must be between 0x08 and 0x77.");
        return;
    }

    terminalView.println("I2C Slave: Listening on address 0x" + argTransformer.toHex(addr) +
                         "... Press [ENTER] to stop.\n");
    
    // Start slave
    i2cService.clearSlaveLog();
    i2cService.beginSlave(addr, sda, scl);

    std::vector<std::string> lastLog;
    while (true) {
        // Enter press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        // Get master log from slave and display it
        auto currentLog = i2cService.getSlaveLog();
        if (currentLog.size() > lastLog.size()) {
            for (size_t i = lastLog.size(); i < currentLog.size(); ++i) {
                terminalView.println(currentLog[i]);
            }
            lastLog = currentLog;
        }
        delay(1);
    }

    // Close slave
    i2cService.endSlave();
    ensureConfigured();
    terminalView.println("\nI2S Slave: Stopped by user.");
}

/*
Dump
*/
void I2cController::handleDump(const TerminalCommand& cmd) {
    // Validate sub
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: dump <addr> [length]");
        return;
    }

    // Parse addr
    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint16_t start = 0x00;
    uint16_t len = 256;

    // Check I2C device presence
    i2cService.beginTransmission(addr);
    if (i2cService.endTransmission()) {
        terminalView.println("I2C Dump: No device found at " + cmd.getSubcommand());
        return;
    }
    
    // Validate and parse arg
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (args.size() >= 1 && argTransformer.isValidNumber(args[0])) {
        len = argTransformer.parseHexOrDec16(args[0]);
    }

    std::vector<uint8_t> values(len, 0xFF);
    std::vector<bool> valid(len, false);

    // Device registers are readable
    if (i2cService.isReadableDevice(addr, start)) {
        terminalView.println("I2C Dump: 0x" + argTransformer.toHex(addr) +
                             " from 0x" + argTransformer.toHex(start) +
                             " for " + std::to_string(len) + " bytes... Press [ENTER] to stop.\n");

        performRegisterRead(addr, start, len, values, valid);

    // Not readable
    } else {
        terminalView.println("I2C Dump: Device at 0x" + argTransformer.toHex(addr) +
                             " may not use standard register access — trying raw read...");

        performRawRead(addr, start, len, values, valid);
    }

    // Not able to read any data
    if (std::all_of(valid.begin(), valid.end(), [](bool b) { return !b; })) {
        terminalView.println("I2C Dump: Unable to read any data — device NACKed or unsupported protocol.\n");
        return;
    }

    printHexDump(start, len, values, valid);
}

void I2cController::performRegisterRead(uint8_t addr, uint16_t start, uint16_t len,
                                        std::vector<uint8_t>& values, std::vector<bool>& valid) {
    const uint8_t CHUNK_SIZE = 16;
    const bool use16bitAddr = (start + len - 1) > 0xFF;
    int consecutiveErrors = 0;

    for (uint16_t offset = 0; offset < len; offset += CHUNK_SIZE) {
        if (consecutiveErrors >= 3) {
            terminalView.println("I2C Dump: Aborted after 3 consecutive errors.");
            return;
        }

        uint16_t reg = start + offset;
        uint8_t toRead = (offset + CHUNK_SIZE <= len) ? CHUNK_SIZE : (len - offset);

        // Write register address (1 or 2 bytes)
        i2cService.beginTransmission(addr);
        if (use16bitAddr) {
            i2cService.write((reg >> 8) & 0xFF); // MSB
            i2cService.write(reg & 0xFF);        // LSB
        } else {
            i2cService.write((uint8_t)(reg & 0xFF));
        }
        bool writeOk = (i2cService.endTransmission(false) == 0);  // No stop
        if (!writeOk) {
            consecutiveErrors++;
            continue;
        }

        // Read chunk
        uint8_t received = i2cService.requestFrom(addr, toRead, true);
        if (received == toRead) {
            for (uint8_t i = 0; i < toRead; ++i) {
                char key = terminalInput.readChar();
                if (key == '\r' || key == '\n') {
                    terminalView.println("I2C Dump: Cancelled by user.");
                    return;
                }

                if (i2cService.available()) {
                    values[offset + i] = i2cService.read();
                    valid[offset + i] = true;
                }
            }
            consecutiveErrors = 0;
        } else {
            while (i2cService.available()) i2cService.read();  // Flush
            consecutiveErrors++;
        }

        delay(1);
    }
}

void I2cController::performRawRead(uint8_t addr, uint16_t start,
                                   uint16_t len,
                                   std::vector<uint8_t>& values,
                                   std::vector<bool>& valid) {
    values.assign(len, 0xFF);
    valid.assign(len, false);

    terminalView.println("I2C Dump: Trying read raw...");

    // Write start register
    i2cService.beginTransmission(addr);
    i2cService.write(start);
    if (i2cService.endTransmission(false) != 0) {
        return;  // NACK
    }

    // Read len from register addr
    uint16_t received = i2cService.requestFrom(addr, (uint8_t)len, true);
    for (uint16_t i = 0; i < received && i < len; ++i) {
        auto key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("I2C Dump: Cancelled by user.");
            return;
        }
        if (i2cService.available()) {
            values[i] = i2cService.read();
            valid[i] = true;
        }
    }

    while (i2cService.available()) i2cService.read();
}

void I2cController::printHexDump(uint16_t start, uint16_t len,
                                 const std::vector<uint8_t>& values, const std::vector<bool>& valid) {
    for (uint16_t lineStart = 0; lineStart < len; lineStart += 16) {
        std::string line;
        char addrStr[8];
        snprintf(addrStr, sizeof(addrStr), "%02X:", start + lineStart);
        line += addrStr;

        for (uint8_t i = 0; i < 16; ++i) {
            uint16_t idx = lineStart + i;
            if (idx < len) {
                if (valid[idx]) {
                    char hex[4];
                    snprintf(hex, sizeof(hex), " %02X", values[idx]);
                    line += hex;
                } else {
                    line += " ??";
                }
            } else {
                line += "   ";
            }
        }

        line += "  ";

        for (uint8_t i = 0; i < 16; ++i) {
            uint16_t idx = lineStart + i;
            if (idx < len && valid[idx]) {
                char c = values[idx];
                line += (c >= 32 && c <= 126) ? c : '.';
            } else {
                line += '.';
            }
        }

        terminalView.println(line);
    }
    terminalView.println("");
}

/*
Identify
*/
void I2cController::handleIdentify(const TerminalCommand& cmd) {
    // Validate subcommand
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: identify <addr>");
        return;
    }

    // Parse I2C address
    uint8_t address = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint16_t start = 0x00;
    uint16_t len = 256;

    std::stringstream ss;
    ss << "\n\r 📟 I2C 0x" + argTransformer.toHex(address) + " Identification Result\n";

    // Search for known addresses
    bool matchFound = false;
    for (size_t i = 0; i < i2cknownAddressesCount; ++i) {
        if (i2cKnownAddresses[i].address == address) {
            matchFound = true;
            ss << "\r  ➤ Could be: - [" << i2cKnownAddresses[i].type << "] " << i2cKnownAddresses[i].component << "\n";
        }
    }

    if (!matchFound) {
        ss << "\r  ➤ No match found for address 0x" << argTransformer.toHex(address) << "\n";
    }

    terminalView.println(ss.str());
}

/*
Recover
*/
void I2cController::handleRecover() {
    uint8_t sda = state.getI2cSdaPin();
    uint8_t scl = state.getI2cSclPin();
    uint32_t freq = state.getI2cFrequency();

    terminalView.println("I2C Reset: Attempting to recover I2C bus...");

    // Release I2C bus
    i2cService.end();
    // 16 clock pulse + STOP condition
    bool success = i2cService.i2cBitBangRecoverBus(scl, sda, freq);
    // Reconfigure I2C
    i2cService.configure(sda, scl, freq);

    if (success) {
        terminalView.println("\nI2C Reset: SDA released. Bus recovery successful.");
    } else {
        terminalView.println("\nI2C Reset: SDA still LOW after recovery, bus may remain stuck.");
    }
}

/*
Glitch
*/
void I2cController::handleGlitch(const TerminalCommand& cmd) {
    // Validate arg
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: glitch <addr>");
        return;
    }

    // Parse and get I2C default config
    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint8_t scl = state.getI2cSclPin();
    uint8_t sda = state.getI2cSdaPin();
    uint32_t freqHz = state.getI2cFrequency();

    // Check I2C device presence
    i2cService.beginTransmission(addr);
    if (i2cService.endTransmission()) {
        terminalView.println("I2C Glitch: No device found at " + cmd.getSubcommand());
        return;
    }

    terminalView.println("I2C Glitch: Attacking device at 0x" + argTransformer.toHex(addr) + "...\n");
    delay(500);

    terminalView.println(" 1. Flooding with random junk...");
    i2cService.floodRandom(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 2. Flooding START sequences...");
    i2cService.floodStart(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 3. Over-read (read more bytes than expected)...");
    i2cService.overReadAttack(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 4. Reading invalid/unmapped registers...");
    i2cService.invalidRegisterRead(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 5. Simulating clock stretch confusion...");
    i2cService.simulateClockStretch(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 6. Rapid START/STOP sequences...");
    i2cService.rapidStartStop(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 7. Glitching ACK phase...");
    i2cService.glitchAckInjection(addr, freqHz, scl, sda);
    delay(50);

    terminalView.println(" 8. Injecting random noise on SCL/SDA...");
    i2cService.randomClockPulseNoise(scl, sda, freqHz);
    delay(50);

    ensureConfigured();
    terminalView.println("\nI2C Glitch: Done. Target may be unresponsive or corrupted.");
}

/*
Flood
*/
void I2cController::handleFlood(const TerminalCommand& cmd) {
    // Validate arg
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: flood <addr>");
        return;
    }

    // Parse arg
    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    
    // Check device presence
    i2cService.beginTransmission(addr);
    if (i2cService.endTransmission()) {
        terminalView.println("I2C Flood: No device found at " + cmd.getSubcommand());
        return;
    }
    
    terminalView.println("I2C Flood: Streaming read to 0x" + argTransformer.toHex(addr) + "... Press [ENTER] to stop.");
    while (true) {
        // Enter to stop
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("\nI2C Flood: Stopped by user.");
            break;
        }

        // Random register address
        uint8_t reg = esp_random() & 0xFF;

        // Transmit only register
        i2cService.beginTransmission(addr);
        i2cService.write(reg);
        i2cService.endTransmission(true);
    }
}

/*
Jam
*/
void I2cController::handleJam() {
    uint8_t scl = state.getI2cSclPin();
    uint8_t sda = state.getI2cSdaPin();
    uint32_t freqHz = state.getI2cFrequency();

    terminalView.println("I2C Jam: Perturbing bus SCL/SDA... Press [ENTER] to stop.\n");

    // Release I2C bus
    i2cService.end();

    while (true) {
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;

        i2cService.injectRandomGlitch(scl, sda, freqHz);
    }

    // Try recovering bus after jamming
    i2cService.i2cBitBangRecoverBus(scl, sda, freqHz);

    // Reconfigure I2C
    ensureConfigured();
    terminalView.println("\nI2C Jam: Stopped by user.\n");
}

/*
Monitor
*/
void I2cController::handleMonitor(const TerminalCommand& cmd) {
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: monitor <addr> [delay_ms]");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    uint16_t len = 256;
    uint32_t delayMs = 500;

    // Optional delay
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (!args.empty() && argTransformer.isValidNumber(args[0])) {
        delayMs = argTransformer.parseHexOrDec32(args[0]);
    }

    // Check device presence
    i2cService.beginTransmission(addr);
    if (i2cService.endTransmission()) {
        terminalView.println("I2C Monitor: No device found at 0x" + argTransformer.toHex(addr));
        return;
    }

    terminalView.println("I2C Monitor: Monitoring register changes at 0x" + argTransformer.toHex(addr) + "... Press [ENTER] to stop.\n");

    std::vector<uint8_t> prev(len, 0xFF);
    std::vector<uint8_t> curr(len, 0xFF);
    std::vector<bool> valid(len, false);

    // First read to initialize prev
    if (i2cService.isReadableDevice(addr, 0x00)) {
        performRegisterRead(addr, 0x00, len, prev, valid);
    } else {
        performRawRead(addr, 0x00, len, prev, valid);
    }

    while (true) {
        // Try register read
        if (i2cService.isReadableDevice(addr, 0x00)) {
            performRegisterRead(addr, 0x00, len, curr, valid);
        } else {
            performRawRead(addr, 0x00, len, curr, valid);
        }

        // Compare and show changes
        for (uint16_t i = 0; i < len; ++i) {
            if (valid[i] && curr[i] != prev[i]) {
                std::stringstream ss;
                ss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << i
                   << ": 0x" << std::setw(2) << (int)prev[i]
                   << " -> 0x" << std::setw(2) << (int)curr[i];
                terminalView.println(ss.str());
                prev[i] = curr[i];
            }
        }

        // Check for user input to stop
        uint32_t elapsed = 0;
        while (elapsed < delayMs) {
            char key = terminalInput.readChar();
            if (key == '\r' || key == '\n') {
                terminalView.println("\nI2C Monitor: Stopped by user.");
                return;
            }
            delay(10);
            elapsed += 10;
        }
    }

    terminalView.println("\nI2C Monitor: Stopped.");
}

/*
EEPROM
*/
void I2cController::handleEeprom(const TerminalCommand& cmd) {
    uint8_t addr = 0x50; // Default EEPROM I2C address

    auto sub = cmd.getSubcommand();
    if (!sub.empty()) {
        if (!argTransformer.isValidNumber(sub)) {
            terminalView.println("Usage: eeprom [addr]");
            return;
        }

        auto parsed = argTransformer.parseHexOrDec(sub);
        if (parsed < 0x03 || parsed > 0x77) { // plage valide I2C 7-bit
            terminalView.println("❌ Invalid I2C address. Must be between 0x03 and 0x77.");
            return;
        }

        addr = parsed;
    }

    eepromShell.run(addr);
    ensureConfigured();
}

/*
Help
*/
void I2cController::handleHelp() {
    terminalView.println("Unknown I2C command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  ping <addr>");
    terminalView.println("  identify <addr>");
    terminalView.println("  sniff");
    terminalView.println("  slave <addr>");
    terminalView.println("  read <addr> [reg]");
    terminalView.println("  write <addr> [reg] [val]");
    terminalView.println("  dump <addr> [len]");
    terminalView.println("  glitch <addr>");
    terminalView.println("  flood <addr>");
    terminalView.println("  health <addr>");
    terminalView.println("  jam");
    terminalView.println("  recover");
    terminalView.println("  monitor <addr> [delay_ms]");
    terminalView.println("  eeprom [addr]");
    terminalView.println("  swap");
    terminalView.println("  config");
    terminalView.println("  raw instructions, e.g: [0x13 0x4B r:8]");
}

/*
Swap SDA and SCL pins
*/
void I2cController::handleSwap() {
    uint8_t sda = state.getI2cSdaPin();
    uint8_t scl = state.getI2cSclPin();

    // Swap in state
    state.setI2cSdaPin(scl);
    state.setI2cSclPin(sda);

    // Reconfigure I2C with swapped pins
    i2cService.configure(state.getI2cSdaPin(), state.getI2cSclPin(), state.getI2cFrequency());

    terminalView.println(
        "I2C Swap: SDA/SCL swapped. SDA=" + std::to_string(state.getI2cSdaPin()) +
        " SCL=" + std::to_string(state.getI2cSclPin())
    );
    terminalView.println("");
}

/*
Health
*/
void I2cController::handleHealth(const TerminalCommand& cmd) {
    // Validate subcommand (addr)
    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: health <addr>");
        return;
    }

    uint8_t addr = argTransformer.parseHexOrDec(cmd.getSubcommand());
    terminalView.println("I2C Health: Analyzing @ 0x" + argTransformer.toHex(addr) + "... Press [ENTER] to stop.\n");

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

    auto stoppedByUser = [&]() -> bool {
        char key = terminalInput.readChar();
        return (key == '\r' || key == '\n');
    };

    /* PING ANALYZE */
    terminalView.println("[ACK latency (ping)]");

    // Warm up
    uint32_t warmDt = 0;
    (void)i2cService.ping(addr, true, &warmDt);
    delay(1);

    const int PING_TRIES = 50;
    Stats ping;

    for (int i = 0; i < PING_TRIES; ++i) {
        if (stoppedByUser()) {
            terminalView.println("\nI2C Health: Stopped by user.");
            return;
        }

        uint32_t dt = 0;
        bool ok = i2cService.ping(addr, true, &dt);
        ping.add(ok, dt);

        delay(1);
    }

    if (ping.ok == 0) {
        terminalView.println("  ❌ No ACK received.\n");
        return;
    }

    terminalView.println("  Tries: " + std::to_string(PING_TRIES) +
                         "   Ok: " + std::to_string(ping.ok) +
                         "   Nack: " + std::to_string(ping.nack));
    terminalView.println("  Min: " + std::to_string(ping.minUs) + " µs   Avg: " +
                         std::to_string(ping.avgUs()) + " µs   Max: " +
                         std::to_string(ping.maxUs) + " µs\n\r  Jitter: " +
                         std::to_string(ping.jitterUs()) + " µs\n");

    /* REGISTER READ ANALYZE */
    terminalView.println("[Register read probe]");

    // Probe support by trying a single read of reg 0x00
    uint8_t probeVal = 0;
    uint32_t probeDt = 0;
    bool regSupported = i2cService.readReg(addr, 0x00, &probeVal, &probeDt);

    terminalView.println(std::string("  Supported: ") + (regSupported ? "yes" : "no\n"));

    const int READ_TRIES = 50;
    Stats rd;

    if (regSupported) {
        // Include the probe result in stats (optional, but coherent)
        rd.add(true, probeDt);

        for (int i = 1; i < READ_TRIES; ++i) {
            if (stoppedByUser()) {
                terminalView.println("\nI2C Health: Stopped by user.");
                return;
            }

            uint8_t val = 0;
            uint32_t dt = 0;
            bool ok = i2cService.readReg(addr, 0x00, &val, &dt);
            rd.add(ok, dt);

            delay(1);
        }

        if (rd.ok > 0) {
            terminalView.println("  Tries: " + std::to_string(READ_TRIES) +
                                 "   Ok: " + std::to_string(rd.ok) +
                                 "   Nack: " + std::to_string(rd.nack));
            terminalView.println("  Min: " + std::to_string(rd.minUs) + " µs   Avg: " +
                                 std::to_string(rd.avgUs()) + " µs   Max: " +
                                 std::to_string(rd.maxUs) + " µs\n\r  Jitter: " +
                                 std::to_string(rd.jitterUs()) + " µs\n");
        } else {
            terminalView.println("  ❌ No successful register reads\n");
        }
    }

    /* REPORT */
    terminalView.println("[Health report for 0x" + argTransformer.toHex(addr) + "]");

    uint32_t freq = state.getI2cFrequency();
    uint32_t bitUs = (freq > 0) ? (1000000UL / freq) : 10; // fallback 10us

    uint32_t pingJ = ping.jitterUs();
    uint32_t readJ = (rd.ok > 0) ? rd.jitterUs() : 0;

    uint32_t pingThr = 20 * bitUs;
    uint32_t readThr = 40 * bitUs;

    bool pingStable = (ping.ok > 0) && (ping.nack == 0) && (pingJ <= pingThr);
    terminalView.println(pingStable ? "  ✅ Stable ACK" : "  ⚠️  ACK variability");

    if (!regSupported) {
        terminalView.println("  Skipped reads (non-standard device)");
    } else {
        bool readStable = (rd.ok > 0) && (rd.nack == 0) && (readJ <= readThr);
        terminalView.println(readStable ? "  ✅ Stable reads" : "  ⚠️  Read variability");
    }

    terminalView.println("  Freq=" + std::to_string(freq) + " Hz  bit=" + std::to_string(bitUs) + " µs");

    if (ping.nack == 0 && pingJ > (200 * bitUs)) {
        terminalView.println("  No NACKs but high latency variance.");
    }
    if (rd.ok > 0 && rd.nack == 0 && readJ > (300 * bitUs)) {
        terminalView.println("  Reads are reliable but variable.");
    }

    terminalView.println("");
}

/*
Config
*/
void I2cController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // User could have set the same pin to a different usage
    // eg. select I2C then select UART then select I2C
    // Always reconfigure pins before use
    i2cService.end();
    uint8_t sda = state.getI2cSdaPin();
    uint8_t scl = state.getI2cSclPin();
    uint32_t freq = state.getI2cFrequency();
    i2cService.configure(sda, scl, freq);
}