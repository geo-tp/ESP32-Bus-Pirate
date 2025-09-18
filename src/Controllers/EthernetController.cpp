#include "Controllers/EthernetController.h"

/*
Entry point for command
*/
void EthernetController::handleCommand(const TerminalCommand& cmd) {
    const auto& root = cmd.getRoot();

    if      (root == "config")    handleConfig();
    else if (root == "connect")   handleConnect();
    else if (root == "nc")        handleNetcat(cmd);
    else if (root == "nmap")      handleNmap(cmd);
    else if (root == "discovery") handleDiscovery(cmd);
    else if (root == "ping")      handlePing(cmd);
    else if (root == "ssh")       handleSsh(cmd);
    else if (root == "telnet")    handleTelnet(cmd);
    else if (root == "modbus")    handleModbus(cmd);
    else if (root == "http")      handleHttp(cmd);
    else if (root == "lookup")    handleLookup(cmd);
    else if (root == "status")    handleStatus();
    else if (root == "reset")     handleReset();
    else                          handleHelp();
}

/*
Connect using DHCP
*/
void EthernetController::handleConnect() {

    unsigned long timeoutMs = 5000;

    terminalView.println("Ethernet: DHCP…");
    if (!ethernetService.beginDHCP(timeoutMs)) {
        if (!ethernetService.linkUp()) {
            terminalView.println("Ethernet: No link (cable unplugged).");
        } else {
            terminalView.println("Ethernet: DHCP failed.");
        }
        return;
    }

    terminalView.println("\n=== Ethernet: Connected via DHCP ===");
    terminalView.println("  IP   : " + ethernetService.getLocalIP());
    terminalView.println("  GATE : " + ethernetService.getGatewayIp());
    terminalView.println("  MASK : " + ethernetService.getSubnetMask());
    terminalView.println("  DNS  : " + ethernetService.getDns());
    terminalView.println("==============================\n");
}

/*
Config W5500
*/
void EthernetController::handleConfig() {
    terminalView.println("Ethernet (W5500) Configuration:");

    const auto& forbidden = state.getProtectedPins();
    uint8_t defCS   = state.getEthernetCsPin();
    uint8_t defRST  = state.getEthernetRstPin();
    uint8_t defSCK  = state.getEthernetSckPin();
    uint8_t defMISO = state.getEthernetMisoPin();
    uint8_t defMOSI = state.getEthernetMosiPin();
    uint8_t defIRQ  = state.getEthernetIrqPin();
    uint32_t defHz  = state.getEthernetFrequency();

    // User input for configuration
    uint8_t cs   = userInputManager.readValidatedPinNumber("W5500 CS pin",   defCS,   forbidden);
    uint8_t sck  = userInputManager.readValidatedPinNumber("W5500 SCK pin",  defSCK,  forbidden);
    uint8_t miso = userInputManager.readValidatedPinNumber("W5500 MISO pin", defMISO, forbidden);
    uint8_t mosi = userInputManager.readValidatedPinNumber("W5500 MOSI pin", defMOSI, forbidden);
    uint8_t irq  = userInputManager.readValidatedPinNumber("W5500 IRQ pin",  defIRQ,  forbidden);

    // RST optional
    bool useReset = userInputManager.readYesNo(
        "Use a RESET (RST) pin?",
        false
    );

    uint8_t rst = 255;
    if (useReset) {
        rst = userInputManager.readValidatedPinNumber("W5500 RST pin", rst, forbidden);
    }

    // Frequency SPI
    uint32_t hz = userInputManager.readValidatedUint32("SPI frequency (Hz)", defHz);

    // MAC addr (optional)
    std::string macStr;
    std::array<uint8_t,6> mac = state.getEthernetMac();

    auto confirmation = userInputManager.readYesNo(
        "Use a custom MAC address?",
        false
    );

    // Ask for MAC if confirmed
    if (confirmation) {
        macStr = userInputManager.readValidatedHexString(
            "MAC (DE AD BE EF 00 42)",
            6,
            false
        );

        argTransformer.parseMac(macStr, mac);
    }

    // Save
    state.setEthernetCsPin(cs);
    state.setEthernetSckPin(sck);
    state.setEthernetMisoPin(miso);
    state.setEthernetMosiPin(mosi);
    state.setEthernetRstPin(useReset ? rst : 255);
    state.setEthernetIrqPin(irq);
    state.setEthernetFrequency(hz);
    state.setEthernetMac(mac);

    // Configure
    bool confirm = ethernetService.configure(
        cs,
        (useReset ? rst : -1),
        sck,
        miso,
        mosi,
        irq,
        hz,
        mac
    );

    if (confirm) {
        terminalView.println("\n ✅ W5500 Ethernet configured.\n");
        return;
    }

    terminalView.println("\n ❌ W5500 Ethernet configuration failed. Check your wiring.\n");
}

/*
W55000 Status
*/
void EthernetController::handleStatus() {
    const bool link      = ethernetService.linkUp();
    const bool connected = ethernetService.isConnected();

    const std::string mac = ethernetService.getMac();
    const std::string ip  = ethernetService.getLocalIP();
    const bool hasIp      = (ip != "0.0.0.0");

    terminalView.println("\n=== Ethernet Status ===");
    terminalView.println(std::string("  Link    : ") + (link ? "UP" : "DOWN"));
    terminalView.println(std::string("  MAC     : ") + mac);

    if (connected) {
        terminalView.println(std::string("  IP     : ") + ip);
        terminalView.println(std::string("  Mask   : ") + ethernetService.getSubnetMask());
        terminalView.println(std::string("  GW     : ") + ethernetService.getGatewayIp());
        terminalView.println(std::string("  DNS    : ") + ethernetService.getDns());
    } else if (link && !hasIp) {
        terminalView.println("  IP      : (waiting for DHCP)");
    } else if (!link) {
        terminalView.println("  IP      : (no link)");
    } else {
        terminalView.println(std::string("  IP      : ") + ip);
    }
    terminalView.println("========================\n");
}

/*
Reset
*/
void EthernetController::handleReset()
{
    ethernetService.hardReset();
    terminalView.println("Ethernet: Interface reset. Disconnected. [NYI].");
}

/*
Help
*/
void EthernetController::handleHelp() {
    terminalView.println("Ethernet commands:");
    terminalView.println("  status");
    terminalView.println("  connect");
    ANetworkController::handleHelp();
    terminalView.println("  reset");
    terminalView.println("  config");
}

/*
Ensure W5500 is configured
*/
void EthernetController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    // Reconfigure in case these pins have been used somewhere else
    auto cs = state.getEthernetCsPin();
    auto sck = state.getEthernetSckPin();
    auto miso = state.getEthernetMisoPin();
    auto mosi = state.getEthernetMosiPin();
    auto rst = state.getEthernetRstPin();
    auto irq = state.getEthernetIrqPin();
    auto frequency = state.getEthernetFrequency();
    auto mac = state.getEthernetMac();

    ethernetService.configure(cs, rst, sck, miso, mosi, irq, frequency, mac);
}
