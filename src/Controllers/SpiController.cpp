#include "SpiController.h"

/*
Constructor
*/
SpiController::SpiController(ITerminalView& terminalView, IInput& terminalInput, 
                             SpiService& spiService, SdService& sdService, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager, BinaryAnalyzeManager& binaryAnalyzeManager,
                             SdCardShell& sdCardShell, SpiFlashShell& spiFlashShell, SpiEepromShell& spiEepromShell)
    : terminalView(terminalView),
      terminalInput(terminalInput),
      spiService(spiService),
      sdService(sdService),
      argTransformer(argTransformer),
      userInputManager(userInputManager),
      binaryAnalyzeManager(binaryAnalyzeManager),
      sdCardShell(sdCardShell),
      spiFlashShell(spiFlashShell),
      spiEepromShell(spiEepromShell)
{}

/*
Entry point for command
*/
void SpiController::handleCommand(const TerminalCommand& cmd) {
    if      (cmd.getRoot() == "sniff")  handleSniff();
    else if (cmd.getRoot() == "sdcard") handleSdCard();
    else if (cmd.getRoot() == "slave")  handleSlave();
    else if (cmd.getRoot() == "flash")  handleFlash(cmd);
    else if (cmd.getRoot() == "eeprom") handleEeprom(cmd);
    else if (cmd.getRoot() == "help")   handleHelp();
    else if (cmd.getRoot() == "config") handleConfig();
    else handleHelp();
}

/*
Entry point for instructions
*/
void SpiController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    auto result = spiService.executeByteCode(bytecodes);
    if (!result.empty()) {
        terminalView.println("SPI Read:\n");
        terminalView.println(result);
    }
}

/*
Sniff
*/
void SpiController::handleSniff() {
    terminalView.println("SPI sniff [NYI]");
}

/*
Flash
*/
void SpiController::handleFlash(const TerminalCommand& cmd) {
    spiFlashShell.run();
}

/*
EEPROM
*/
void SpiController::handleEeprom(const TerminalCommand& cmd) {
    spiEepromShell.run();
    ensureConfigured();
}

/*
Slave
*/
void SpiController::handleSlave() {
    #ifdef DEVICE_M5STICK

    terminalView.println("SPI Slave: Not supported on M5Stick devices due to shared SPI bus.");
    return;

    #endif

    spiService.end(); // Stop master mode if active
    
    int sclk = state.getSpiCLKPin();
    int miso = state.getSpiMISOPin();
    int mosi = state.getSpiMOSIPin();
    int cs   = state.getSpiCSPin();

    terminalView.println("SPI Slave: In progress... Press [ENTER] to stop.");
    spiService.startSlave(sclk, miso, mosi, cs);

    terminalView.println("");
    terminalView.println("  [INFO]");
    terminalView.println("    SPI Slave mode listens passively on the SPI bus.");
    terminalView.println("    Any command sent by a SPI master will be captured and logged");
    terminalView.println("    Data is only captured when CS (chip select) is active.");
    terminalView.println("");

    while (true) {
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        // Read slave data from master
        auto packets = spiService.getSlaveData();
        for (const auto& packet : packets) {
            std::stringstream ss;
            ss << "[MOSI] ";
            for (uint8_t b : packet) {
                ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)b << " ";
            }
            terminalView.println(ss.str());
        }
    }
    spiService.stopSlave(sclk, miso, mosi, cs);
    terminalView.println("\nSPI Slave: Stopped by user.\n");
}

/*
SD Card
*/
void SpiController::handleSdCard() {
    terminalView.println("SD Card: Mounting...");
    delay(500);

    // Configure
    spiService.end();
    bool success = sdService.configure(
        state.getSpiCLKPin(),
        state.getSpiMISOPin(),
        state.getSpiMOSIPin(),
        state.getSpiCSPin()
    );

    if (!success) {
        terminalView.println("SD Card: Mount failed. Check config and wiring and try again.\n");
        return;
    }
    
    // SD Shell
    terminalView.println("SD Card: Mounted successfully. Loading...\n");
    sdCardShell.run();

    // Reconfigure
    sdService.end();
    spiService.end();
    ensureConfigured();
}

/*
Help
*/
void SpiController::handleHelp() {
    terminalView.println("");
    terminalView.println("Unknown SPI command. Usage:");
    terminalView.println("  sniff");
    terminalView.println("  sdcard");
    terminalView.println("  slave");
    terminalView.println("  flash");
    terminalView.println("  eeprom");
    terminalView.println("  config");
    terminalView.println("  raw instructions, e.g: [0x9F r:3]");
    terminalView.println("");
}

/*
Config
*/
void SpiController::handleConfig() {
    terminalView.println("\nSPI Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t mosi = userInputManager.readValidatedPinNumber("MOSI pin", state.getSpiMOSIPin(), forbidden);
    state.setSpiMOSIPin(mosi);

    uint8_t miso = userInputManager.readValidatedPinNumber("MISO pin", state.getSpiMISOPin(), forbidden);
    state.setSpiMISOPin(miso);

    uint8_t sclk = userInputManager.readValidatedPinNumber("SCLK pin", state.getSpiCLKPin(), forbidden);
    state.setSpiCLKPin(sclk);

    uint8_t cs = userInputManager.readValidatedPinNumber("CS pin", state.getSpiCSPin(), forbidden);
    state.setSpiCSPin(cs);

    uint32_t freq = userInputManager.readValidatedUint32("Frequency", state.getSpiFrequency());
    state.setSpiFrequency(freq);

    spiService.configure(mosi, miso, sclk, cs, freq);

    terminalView.println("SPI configured.\n");
}

/*
Ensure SPI is configured
*/
void SpiController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }

    spiService.end();
    sdService.end();
    // Reconfigure, user could have used these pins for another mode
    uint8_t sclk = state.getSpiCLKPin();
    uint8_t miso = state.getSpiMISOPin();
    uint8_t mosi = state.getSpiMOSIPin();
    uint8_t cs   = state.getSpiCSPin();
    int freq   = state.getSpiFrequency();
    spiService.configure(mosi, miso, sclk, cs, freq);
}
