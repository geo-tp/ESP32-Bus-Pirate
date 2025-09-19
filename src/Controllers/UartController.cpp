#include "UartController.h"

/*
Constructor
*/
UartController::UartController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    IInput& deviceInput,
    UartService& uartService,
    SdService& sdService,
    HdUartService& hdUartService,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    UartAtShell& uartAtShell
)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      deviceInput(deviceInput),
      uartService(uartService),
      sdService(sdService),
      hdUartService(hdUartService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      uartAtShell(uartAtShell) 
{}


/*
Entry point for command
*/
void UartController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "scan") handleScan();
    else if (cmd.getRoot() == "ping") handlePing();
    else if (cmd.getRoot() == "read") handleRead();
    else if (cmd.getRoot() == "write") handleWrite(cmd);
    else if (cmd.getRoot() == "bridge") handleBridge();
    else if (cmd.getRoot() == "at") handleAtCommand(cmd);
    else if (cmd.getRoot() == "spam") handleSpam(cmd);
    else if (cmd.getRoot() == "glitch") handleGlitch();
    else if (cmd.getRoot() == "xmodem") handleXmodem(cmd);
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Entry point for instructions
*/
void UartController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = uartService.executeByteCode(bytecodes);
    terminalView.println("");
    terminalView.print("UART Read: ");
    if (!result.empty()) {
        terminalView.println("");
        terminalView.println("");
        terminalView.println(result);
        uartService.clearUartBuffer();
    } else {
        terminalView.print("No data");
    }
    terminalView.println("");
}

/*
Bridge
*/
void UartController::handleBridge() {
    terminalView.println("Uart Bridge: In progress... Press [ANY ESP32 BUTTON] to stop.\n");
    while (true) {
        // Read from UART and print to terminal
        if (uartService.available()) {
            char c = uartService.read();
            terminalView.print(std::string(1, c));
        }

        // Read from user input and write to UART
        char c = terminalInput.readChar();
        if (c != KEY_NONE) {
            uartService.write(c);
        }
        
        // Read from device input and stop bridge if any
        c = deviceInput.readChar();
        if (c != KEY_NONE) {  
            terminalView.println("\nUart Bridge: Stopped by user.");
            break;
        }
    }
}

/*
Read
*/
void UartController::handleRead() {
    terminalView.println("UART Read: Streaming until [ENTER] is pressed...");
    uartService.flush();

    while (true) {
        // Stop if ENTER is pressed
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') {
            terminalView.println("");
            terminalView.println("UART Read: Stopped by user.");
            break;
        }

        // Print UART data as it comes
        while (uartService.available() > 0) {
            char c = uartService.read();
            terminalView.print(std::string(1, c));
        }
    }
}

/*
AT Command shell
*/
void UartController::handleAtCommand(const TerminalCommand& cmd) {
    #ifdef DEVICE_M5STICK
        terminalView.println("UART AT: No flash space available on M5Stick for AT command shell.");
        return;
        
    #else 
        uartAtShell.run();
    #endif
}

/*
Write
*/
void UartController::handleWrite(TerminalCommand cmd) {
    std::string raw = cmd.getSubcommand() + cmd.getArgs();
    std::string decoded = argTransformer.decodeEscapes(raw);
    uartService.print(decoded);
    terminalView.println("UART Write: Text sent at baud " + std::to_string(state.getUartBaudRate()));
}

/*
Ping
*/
void UartController::handlePing() { 
    std::string response;
    unsigned long start = millis();
    size_t probeIndex = 0;

    terminalView.println("UART Ping: Probing for 5 seconds...");
    uartService.clearUartBuffer();

    while (millis() - start < 5000) {
        // Envoi progressif
        if (probeIndex < probes.size()) {
            uartService.write(probes[probeIndex]);
            probeIndex++;
        }

        // Lecture continue
        char c;
        while (uartService.available() > 0) {
            c = uartService.read();
            response += c;
        }

        delay(10);
    }

    // Analyse ASCII simple
    size_t asciiCount = 0;
    std::string result = "";
    for (char c : response)
        if (isprint(static_cast<unsigned char>(c)) || isspace(c)) {
            asciiCount++;
            result += c;
        }

    if (asciiCount < 5) {
        terminalView.println("UART Ping: No response.");
        return;
    }

    terminalView.println("UART Response: ");
    terminalView.println("");
    terminalView.println(result);
    terminalView.println("");

    terminalView.println("UART Ping: Device detected");
}

/*
Scan
*/
void UartController::handleScan() {
    terminalView.println("UART Scan: in progress... Press [ENTER] to cancel");
    terminalView.println("");
    terminalView.println("[INFO]");
    terminalView.println("  The UART scanner attempts to detect the correct baudrate");
    terminalView.println("  by iteratively switching speeds, sending predefined probes");
    terminalView.println("");

    uartService.clearUartBuffer();
    scanCancelled = false;

    for (int baud : baudrates) {
        if (scanCancelled) return;
        if (scanAtBaudrate(baud)) {
            state.setUartBaudRate(baud);
            uartService.switchBaudrate(baud);
            terminalView.println("");
            terminalView.println("UART Scan: Setting baudrate to UART config.");
            terminalView.println("UART Scan: Baudrate detected " + std::to_string(baud));
            terminalView.println("");
            return;
        }
    }

    uartService.switchBaudrate(state.getUartBaudRate()); // restore previous
    terminalView.println("Uart Scan: No device detected.");
    terminalView.println("");
}

bool UartController::scanAtBaudrate(int baud) {
    const size_t maxResponseSize = 8192;
    uartService.switchBaudrate(baud);
    terminalView.println("→ Testing baudrate " + std::to_string(baud));
    uartService.clearUartBuffer();

    std::string response;
    size_t asciiCount = 0;
    size_t probeIndex = 0;
    unsigned long start = millis();

    while (millis() - start < 1500) {
        if (checkScanCancelled()) return false;
        sendNextProbe(probeIndex);
        updateResponse(response, asciiCount, maxResponseSize);

        if (isValidResponse(response, asciiCount)) {
            terminalView.println("");
            terminalView.println("Preview:");
            auto cleaned = argTransformer.filterPrintable(response.substr(0, 100));
            terminalView.println(cleaned + "...");
            return true;
        }

        delay(10);
    }

    return false;
}

bool UartController::checkScanCancelled() {
    char key = terminalInput.readChar();
    if (key == '\r' || key == '\n') {
        terminalView.println("UART Scan: Cancelled by user.");
        uartService.switchBaudrate(state.getUartBaudRate()); // restore previous
        scanCancelled = true;
        return true;
    }
    return false;
}

void UartController::sendNextProbe(size_t& probeIndex) {
    if (probeIndex < probes.size()) {
        uartService.write(probes[probeIndex]);
        probeIndex++;
    }
}

void UartController::updateResponse(std::string& response, size_t& asciiCount, size_t maxSize) {
    unsigned long readStart = millis();
    const unsigned long readTimeout = 150;  // ms
    while (uartService.available() > 0 && millis() - readStart < readTimeout) {
        char c = uartService.read();

        if (response.length() >= maxSize) {
            char dropped = response.front();
            if (isprint(dropped) || isspace(dropped)) asciiCount--;
            response.erase(0, 1);
        }

        if (isprint(c) || isspace(c)) asciiCount++;
        response += c;
    }
}

bool UartController::isValidResponse(const std::string& response, size_t asciiCount) {
    if (response.empty()) return false;

    float ratio = static_cast<float>(asciiCount) / response.size();
    float entropy = computeEntropy(response);

    bool plausibleLength = response.length() >= 32;
    bool readableEnough = ratio >= 0.85f;
    bool entropyOK = entropy >= 3.0f && entropy <= 7.5f;

    return plausibleLength && readableEnough && entropyOK;
}

float UartController::computeEntropy(const std::string& data) {
    std::unordered_map<char, size_t> freq;
    for (char c : data)
        freq[c]++;

    float entropy = 0.0f;
    for (auto& p : freq) {
        float prob = static_cast<float>(p.second) / data.size();
        entropy -= prob * std::log2(prob);
    }
    return entropy;
}

/*
Spam
*/
void UartController::handleSpam(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || cmd.getArgs().empty()) {
        terminalView.println("Usage: spam <text> <ms>");
        return;
    }

    // Find the last space to separate SSID and password
    std::string full = cmd.getSubcommand() + " " + cmd.getArgs();
    size_t pos = full.find_last_of(' ');
    if (pos == std::string::npos || pos == full.size() - 1) {
        terminalView.println("Usage: spam <text> <ms>");
        return;
    }
    auto textRaw = full.substr(0, pos);
    auto msRaw = full.substr(pos + 1);

    std::string text = argTransformer.decodeEscapes(textRaw);

    if (!argTransformer.isValidNumber(msRaw)) {
        terminalView.println("Usage: spam <text> <ms>");
        return;
    }

    uint32_t delayMs = argTransformer.toUint32(msRaw);
    unsigned long lastSend = 0;

    terminalView.println(
        "UART Spam: Sending \"" + text + 
        "\" every " + std::to_string(delayMs) + 
        " ms at baud " + std::to_string(state.getUartBaudRate()) + 
        "... Press [ENTER] to stop."
    );

    while (true) {
        // Stop if ENTER pressed
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') {
            terminalView.println("\nUART Spam: Stopped by user.");
            break;
        }

        // Send if delay elapsed
        unsigned long now = millis();
        if (now - lastSend >= delayMs) {
            uartService.print(text);
            lastSend = now;
        }

        delay(1);
    }
}

/*
Xmodem
*/
void UartController::handleXmodem(const TerminalCommand& cmd) {
    std::string action = cmd.getSubcommand();
    std::string path = cmd.getArgs();

    if (action.empty()) {
        terminalView.println("Usage: xmodem <recv/send> <path>");
        return;
    }

    if (path.empty()) {
        terminalView.println("Error: missing path argument (ex: /file.txt)");
        return;
    }
    
    // Normalize path
    if (!path.empty() && path[0] != '/') {
        path = "/" + path;
    }

    terminalView.println("\nXmodem Configuration:");

    // Xmodem block size
    uint8_t defaultBlockSize = uartService.getXmodemBlockSize();
    uint8_t blockSize = userInputManager.readValidatedUint8("Block size (typ. 128 or 1024)", defaultBlockSize, 1, 128);
    uartService.setXmodemBlockSize(blockSize);

    // Xmodem id size
    uint8_t defaultIdSize = uartService.getXmodemIdSize();
    uint8_t idSize = userInputManager.readValidatedUint8("Block ID size in bytes (1-4)", defaultIdSize, 1, 4);
    uartService.setXmodemIdSize(idSize);

    // Xmodem CRC
    bool useCrc = userInputManager.readYesNo("Use CRC?", true);
    uartService.setXmodemCrc(useCrc);

    terminalView.println("\nXmodem configured\n");

    if (action == "recv") {
        handleXmodemReceive(path);
    } else if (action == "send") {
        handleXmodemSend(path);
    } else {
        terminalView.println("Usage: xmodem <recv/send> <path>");
    }
}

void UartController::handleXmodemSend(const std::string& path) {
    // Open SD with SPI pin
    auto sdMounted = sdService.configure(state.getSpiCLKPin(), state.getSpiMISOPin(), 
                    state.getSpiMOSIPin(), state.getSpiCSPin());

    //Check SD mounted
    if (!sdMounted) {
        terminalView.println("UART XMODEM: No SD card detected. Check SPI pins");
        return;
    }

    // Open the file
    File file = sdService.openFileRead(path);
    if (!file) {
        terminalView.println("UART XMODEM: Could not open file");
        return;
    }

    // Infos
    terminalView.println(" [INFO]  No progress bar will be shown on WEBUI.");
    terminalView.println("         Progress bar is only visible over USB Serial.");
    terminalView.println("         Please be patient during the transfer.\n");
    std::stringstream ss;
    ss << "         Estimated duration: ~" 
    << (uint32_t)((file.size() * 10.0) / state.getUartBaudRate()) 
    << " seconds.\n";
    terminalView.println(ss.str());

    // Send it
    terminalView.println("UART XMODEM: Sending...");
    bool ok = uartService.xmodemSendFile(file);
    file.close();
    sdService.end();

    // Result
    terminalView.println(ok ? "\nUART XMODEM: Success, file is transferred" : "\nUART XMODEM: Failed to transfer file");

    // Close Xmodem
    uartService.end();
    ensureConfigured();

    // Close SD
    sdService.end();
}

void UartController::handleXmodemReceive(const std::string& path) {
    // Open sd card with SPI pin
    auto sdMounted = sdService.configure(state.getSpiCLKPin(), state.getSpiMISOPin(), 
                    state.getSpiMOSIPin(), state.getSpiCSPin());

    //Check SD mounted
    if (!sdMounted) {
        terminalView.println("UART XMODEM: No SD card detected. Check SPI pins");
        return;
    }

    // Create target file
    File file = sdService.openFileWrite(path);
    if (!file) {
        terminalView.println("UART XMODEM: Could not create file.");
        return;
    }

    // Infos
    terminalView.println("");
    terminalView.println("  [INFO] XMODEM receive mode is blocking.");
    terminalView.println("         No progress bar will be shown on WEBUI.");
    terminalView.println("         Progress bar is only visible over USB Serial.");
    terminalView.println("         The device will wait for incoming data");
    terminalView.println("         for up to 2 minutes. Once started,");
    terminalView.println("         the transfer must complete before exiting.\n");

    // Receive
    terminalView.println("UART XMODEM: Receiving...");
    bool ok = uartService.xmodemReceiveToFile(file);
    file.close();
    sdService.end();

    // Result
    terminalView.println("");
    terminalView.println(ok ? 
        ("UART XMODEM: Receive OK, File saved to " + path) :
        "UART XMODEM: Receive failed");

    // Close Xmodem
    uartService.end();
    ensureConfigured();

    // Close SD
    if (!ok) {
        sdService.deleteFile(path); // delete created file
    }
    sdService.end();
}

/*
Config
*/
void UartController::handleConfig() {
    terminalView.println("");
    terminalView.println("UART Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t rxPin = userInputManager.readValidatedPinNumber("RX pin number", state.getUartRxPin(), forbidden);
    state.setUartRxPin(rxPin);

    uint8_t txPin = userInputManager.readValidatedPinNumber("TX pin number", state.getUartTxPin(), forbidden);
    state.setUartTxPin(txPin);

    uint32_t baud = userInputManager.readValidatedUint32("Baud rate", state.getUartBaudRate());
    state.setUartBaudRate(baud);

    uint8_t dataBits = userInputManager.readValidatedUint8("Data bits (5-8)", state.getUartDataBits(), 5, 8);
    state.setUartDataBits(dataBits);

    char defaultParity = state.getUartParity().empty() ? 'N' : state.getUartParity()[0];
    char parityChar = userInputManager.readCharChoice("Parity (N/E/O)", defaultParity, {'N', 'E', 'O'});
    state.setUartParity(std::string(1, parityChar));

    uint8_t stopBits = userInputManager.readValidatedUint8("Stop bits (1 or 2)", state.getUartStopBits(), 1, 2);
    state.setUartStopBits(stopBits);

    bool inverted = userInputManager.readYesNo("Inverted?", state.isUartInverted());
    state.setUartInverted(inverted);

    uint32_t config = uartService.buildUartConfig(dataBits, parityChar, stopBits);
    state.setUartConfig(config);
    uartService.configure(baud, config, rxPin, txPin, inverted);

    terminalView.println("UART configuration applied.");
    terminalView.println("");
}

/*
Help
*/
void UartController::handleHelp() {
    terminalView.println("");
    terminalView.println("Unknown UART command. Usage:");
    terminalView.println("  scan");
    terminalView.println("  ping");
    terminalView.println("  read");
    terminalView.println("  write <text>");
    terminalView.println("  bridge");
    terminalView.println("  at");
    terminalView.println("  spam <text> <ms>");
    terminalView.println("  glitch");
    terminalView.println("  xmodem recv <dest path>");
    terminalView.println("  xmodem send <file path>");
    terminalView.println("  config");
    terminalView.println("  raw instructions, ['AT' D:100 r:128]");
    terminalView.println("");
}

/*
Glitch
*/
void UartController::handleGlitch() {
    terminalView.println("Uart Glicher: Not Yet Implemented");
}

/*
Ensure Config
*/
void UartController::ensureConfigured() {
    // hdUartService.end() // It crashed the app for some reasons, not rly needed

    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // User could have set the same pin to a different usage
    // eg. select UART, then select I2C, then select UART
    // Always reconfigure pins before use
    uartService.end();

    uint8_t rx = state.getUartRxPin();
    uint8_t tx = state.getUartTxPin();
    uint32_t baud = state.getUartBaudRate();
    uint32_t config = state.getUartConfig();
    bool inverted = state.isUartInverted();

    uartService.configure(baud, config, rx, tx, inverted);
}