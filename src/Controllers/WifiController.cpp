#include "Controllers/WifiController.h"
#include "Vendors/wifi_atks.h"

/*
Entry point for command
*/
void WifiController::handleCommand(const TerminalCommand &cmd)
{
    const auto &root = cmd.getRoot();

    if (root == "connect") handleConnect(cmd);
    else if (root == "disconnect") handleDisconnect(cmd);
    else if (root == "status") handleStatus(cmd);
    else if (root == "ap") handleAp(cmd);
    else if (root == "spoof") handleSpoof(cmd);
    else if (root == "scan") handleScan(cmd);
    else if (root == "probe") handleProbe();
    else if (root == "ping") handlePing(cmd);
    else if (root == "sniff") handleSniff(cmd);
    else if (root == "webui") handleWebUi(cmd);
    else if (root == "ssh") handleSsh(cmd);
    else if (root == "telnet") handleTelnet(cmd);
    else if (root == "nc") handleNetcat(cmd);
    else if (root == "nmap") handleNmap(cmd);
    else if (root == "modbus") handleModbus(cmd);
    else if (root == "http") handleHttp(cmd);
    else if (root == "lookup") handleLookup(cmd);
    else if (root == "discovery") handleDiscovery(cmd);
    else if (root == "reset") handleReset();
    else if (root == "deauth") handleDeauth(cmd);
    else handleHelp();
}

/*
Connect
*/
void WifiController::handleConnect(const TerminalCommand &cmd)
{
    std::string ssid;
    std::string password;
    auto args = argTransformer.splitArgs(cmd.getSubcommand());

    // No args provided, we need to check saved creds or scan and select networks
    if (cmd.getSubcommand().empty()) {

        // Check saved creds
        nvsService.open();
        ssid = nvsService.getString(state.getNvsSsidField());
        password = nvsService.getString(state.getNvsPasswordField());
        nvsService.close();
        auto confirmation = false;

        // Creds found
        if (!ssid.empty() && !password.empty()) {
            confirmation = userInputManager.readYesNo(
                "WiFi: Use saved credentials for " + ssid + "? (Y/n)", true
            );
        } 

        // Select network if no creds or not confirmed
        if (!confirmation) {
            terminalView.println("Wifi: Scanning for available networks...");
            auto networks = wifiService.scanNetworks();
            int selectedIndex = userInputManager.readValidatedChoiceIndex("\nSelect Wi-Fi network", networks, 0);
            ssid = networks[selectedIndex];
            terminalView.println("Selected SSID: " + ssid);
            terminalView.print("Password: ");
            password = userInputManager.getLine();
        }

    // Args provided
    } else  {
        // Concatenate subcommand and args
        std::string full = cmd.getSubcommand() + " " + cmd.getArgs();
    
        // Find the last space to separate SSID and password
        size_t pos = full.find_last_of(' ');
        if (pos == std::string::npos || pos == full.size() - 1) {
            terminalView.println("Usage: connect <ssid> <password>");
            return;
        }
        ssid = full.substr(0, pos);
        password = full.substr(pos + 1);
    }

    terminalView.println("WiFi: Connecting to " + ssid + "...");

    wifiService.setModeApSta();
    wifiService.connect(ssid, password);
    if (wifiService.isConnected()) {
        terminalView.println("");
        terminalView.println("WiFi: Connected to Wi-Fi!");
        terminalView.println("      Reset the device and choose WiFi Web,");
        terminalView.println("      if you want to use the web based CLI");
        terminalView.println("");
        terminalView.println("[BAREBONE] To launch the WebUI without a screen:");
        terminalView.println("  1. Reset the device (don’t hold the board button during boot)");
        terminalView.println("  1. Once the device is powered, you have 3 seconds to press the board button");
        terminalView.println("  3. The built-in LED shows the following status:");
        terminalView.println("     • Blue  = No Wi-Fi credentials saved.");
        terminalView.println("     • White = Connecting in progress");
        terminalView.println("     • Green = Connected, open the WebUI in a browser");
        terminalView.println("     • Red   = Connection failed, try connect again with serial");
        terminalView.println("");
        terminalView.println("WiFi Web UI: http://" + wifiService.getLocalIP());

        // Save creds
        nvsService.open();
        nvsService.saveString(state.getNvsSsidField(), ssid);
        nvsService.saveString(state.getNvsPasswordField(), password);
        nvsService.close();
    } else {
        terminalView.println("WiFi: Connection failed.");
        wifiService.reset();
        delay(100);
    }
}

/*
Disconnect
*/
void WifiController::handleDisconnect(const TerminalCommand &cmd)
{
    wifiService.disconnect();
    terminalView.println("WiFi: Disconnected.");
}

/*
Status
*/
void WifiController::handleStatus(const TerminalCommand &cmd)
{
    auto ssid     = wifiService.getSsid();     if (ssid.empty()) ssid = "N/A";
    auto bssid    = wifiService.getBssid();    if (bssid.empty()) bssid = "N/A";
    auto hostname = wifiService.getHostname(); if (hostname.empty()) hostname = "N/A";

    terminalView.println("\n=== Wi-Fi Status ===");
    terminalView.println("Mode         : " + std::string(wifiService.getWifiModeRaw() == WIFI_MODE_AP ? "Access Point" : "Station"));
    terminalView.println("AP MAC       : " + wifiService.getMacAddressAp());
    terminalView.println("STA MAC      : " + wifiService.getMacAddressSta());
    terminalView.println("IP           : " + wifiService.getLocalIp());
    terminalView.println("Subnet       : " + wifiService.getSubnetMask());
    terminalView.println("Gateway      : " + wifiService.getGatewayIp());
    terminalView.println("DNS1         : " + wifiService.getDns1());
    terminalView.println("DNS2         : " + wifiService.getDns2());
    terminalView.println("Hostname     : " + hostname);

    terminalView.println("SSID         : " + ssid);
    terminalView.println("BSSID        : " + bssid);
    terminalView.println("Prov enabled : " + std::string(wifiService.isProvisioningEnabled() ? "Yes" : "No"));

    const int status = wifiService.getWifiStatusRaw();
    if (status == 3 /* WL_CONNECTED */) {
        terminalView.println("RSSI         : " + std::to_string(wifiService.getRssi()) + " dBm");
        terminalView.println("Channel      : " + std::to_string(wifiService.getChannel()));
    } else {
        terminalView.println("RSSI         : N/A");
        terminalView.println("Channel      : N/A");
    }

    terminalView.println("Mode         : " + std::string(wifiService.wifiModeToStr(wifiService.getWifiModeRaw())));
    terminalView.println("Status       : " + std::string(wifiService.wlStatusToStr(status)));
    terminalView.println("Prov enabled : " + std::string(wifiService.isProvisioningEnabled() ? "Yes" : "No"));
    terminalView.println("====================\n");
}

/*
Access Point
*/
void WifiController::handleAp(const TerminalCommand &cmd)
{
    auto ssid = cmd.getSubcommand();

    if (ssid.empty())
    {
        terminalView.println("Usage: ap <ssid> <password>");
        terminalView.println("       ap spam");
        return;
    }

    if (ssid == "spam") {
        handleApSpam();
        return;
    }

    auto full = cmd.getSubcommand() + " " + cmd.getArgs();

    // Find the last space to separate SSID and password
    size_t pos = full.find_last_of(' ');
    if (pos == std::string::npos || pos == full.size() - 1) {
        terminalView.println("Usage: connect <ssid> <password>");
        return;
    }
    ssid = full.substr(0, pos);
    auto password = full.substr(pos + 1);

    // Already connected, mode AP+STA
    if (wifiService.isConnected())
    {
        wifiService.setModeApSta();
    }
    else
    {
        wifiService.setModeApOnly();
    }

    if (wifiService.startAccessPoint(ssid, password))
    {
        terminalView.println("WiFi: Access Point started with SSID " + ssid);
        terminalView.println("AP IP: " + wifiService.getApIp());

        auto nvsSsidField = state.getNvsSsidField();
        auto nvsPasswordField = state.getNvsPasswordField();
        auto ssid = nvsService.getString(nvsSsidField, "");
        auto password = nvsService.getString(nvsPasswordField, "");

        // Try to reconnect to saved WiFi
        if (!ssid.empty() && !password.empty())
        {
            wifiService.connect(ssid, password);
        }

        if (wifiService.isConnected())
        {
            terminalView.println("STA IP: " + wifiService.getLocalIp());
        }
    }
    else
    {
        terminalView.println("WiFi: Failed to start Access Point.");
    }
}

/*
AP Spam
*/
void WifiController::handleApSpam()
{
    terminalView.println("WiFi: Starting beacon spam... Press [ENTER] to stop.");
    while (true)
    {
        beaconCreate(""); // func from Vendors/wifi_atks.h

        // Enter press to stop
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n') break;
        delay(10);
    }

    terminalView.println("WiFi: Beacon spam stopped.\n");
}

/*
Scan
*/
void WifiController::handleScan(const TerminalCommand &)
{
    terminalView.println("WiFi: Scanning for networks...");
    delay(300);

    auto networks = wifiService.scanDetailedNetworks();

    for (const auto &net : networks)
    {
        std::string line = "  SSID: " + net.ssid;
        line += " | Sec: " + wifiService.encryptionTypeToString(net.encryption);
        line += " | BSSID: " + net.bssid;
        line += " | CH: " + std::to_string(net.channel);
        line += " | RSSI: " + std::to_string(net.rssi) + " dBm";
        if (net.open)
            line += " [open]";
        if (net.vulnerable)
            line += " [vulnerable]";
        if (net.hidden)
            line += " [hidden]";

        terminalView.println(line);
    }

    if (networks.empty())
    {
        terminalView.println("WiFi: No networks found.");
    }
}

/*
Probe
*/
void WifiController::handleProbe() 
{
    terminalView.println("WIFI: Starting probe for internet access on open networks...");
    terminalView.println("\n[WARNING] This will try to connect to surrounding open networks.\n");

    // Confirm before starting
    auto confirmation = userInputManager.readYesNo("Start Wi-Fi probe to find internet access?", false);
    if (!confirmation) {
        terminalView.println("WIFI: Probe cancelled.\n");
        return;
    }

    // Stop any existing probe
    if (wifiOpenScannerService.isOpenProbeRunning()) {
        wifiOpenScannerService.stopOpenProbe();
    }
    wifiOpenScannerService.clearProbeLog();

    // Start the open probe service
    if (!wifiOpenScannerService.startOpenProbe()) {
        terminalView.println("WIFI: Failed to start probe.\n");
        return;
    }

    terminalView.println("WIFI: Probe for internet access... Press [ENTER] to stop.\n");

    // Start the open probe task
    while (wifiOpenScannerService.isOpenProbeRunning()) {
        // Display logs
        auto batch = wifiOpenScannerService.fetchProbeLog();
        for (auto& ln : batch) {
            terminalView.println(ln.c_str());
        }

        // Enter Press to stop
        int ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') {
            wifiOpenScannerService.stopOpenProbe();
            break;
        }

        delay(10);
    }

    // Flush final logs
    for (auto& ln : wifiOpenScannerService.fetchProbeLog()) {
        terminalView.println(ln.c_str());
    }
    terminalView.println("WIFI: Open-Wifi probe ended.\n");
}

/*
Sniff
*/
void WifiController::handleSniff(const TerminalCommand &cmd)
{
    terminalView.println("WiFi Sniffing started... Press [ENTER] to stop.\n");

    wifiService.startPassiveSniffing();
    wifiService.switchChannel(1);

    uint8_t channel = 1;
    unsigned long lastHop = 0;
    unsigned long lastPull = 0;

    while (true)
    {
        // Enter Press
        char key = terminalInput.readChar();
        if (key == '\r' || key == '\n')
            break;

        // Read sniff data
        if (millis() - lastPull > 20)
        {
            auto logs = wifiService.getSniffLog();
            for (const auto &line : logs)
            {
                terminalView.println(line);
            }
            lastPull = millis();
        }

        // Switch channel every 100ms
        if (millis() - lastHop > 100)
        {
            channel = (channel % 13) + 1; // channel 1 to 13
            wifiService.switchChannel(channel);
            lastHop = millis();
        }

        delay(5);
    }

    wifiService.stopPassiveSniffing();
    terminalView.println("WiFi Sniffing stopped.\n");
}

/*
Spoof
*/
void WifiController::handleSpoof(const TerminalCommand &cmd)
{
    auto mode = cmd.getSubcommand();
    auto mac = cmd.getArgs();

    if (mode.empty() && mac.empty())
    {
        terminalView.println("Usage: spoof sta <mac>");
        terminalView.println("       spoof ap <mac>");
        return;
    }

    WifiService::MacInterface iface = (mode == "sta")
                                          ? WifiService::MacInterface::Station
                                          : WifiService::MacInterface::AccessPoint;

    terminalView.println("WiFi: Spoofing " + mode + " MAC to " + mac + "...");

    bool ok = wifiService.spoofMacAddress(mac, iface);

    if (ok)
    {
        terminalView.println("WiFi: MAC spoofed successfully.");
    }
    else
    {
        terminalView.println("WiFi: Failed to spoof MAC.");
    }
}

/*
Reset
*/
void WifiController::handleReset()
{
    wifiService.reset();
    terminalView.println("WiFi: Interface reset. Disconnected.");
}

/*
Web Interface
*/
void WifiController::handleWebUi(const TerminalCommand &)
{
    if (wifiService.isConnected())
    {
        auto ip = wifiService.getLocalIP();
        terminalView.println("");
        terminalView.println("[WARNING] If you're connected via serial,");
        terminalView.println("          the web UI will not be active.");
        terminalView.println("          Reset the device and choose WiFi Web.");
        terminalView.println("");
        terminalView.println("[BAREBONE] To launch the WebUI without a screen:");
        terminalView.println("  1. Reset the device (don’t hold the board button during boot)");
        terminalView.println("  1. Once the device is powered, you have 3 seconds to press the board button");
        terminalView.println("  3. The built-in LED shows the following status:");
        terminalView.println("     • Blue  = No Wi-Fi credentials saved.");
        terminalView.println("     • White = Connecting in progress");
        terminalView.println("     • Green = Connected, open the WebUI in your browser.");
        terminalView.println("     • Red   = Connection failed, try connect again with serial");
        terminalView.println("");
        terminalView.println("WiFi Web UI: http://" + ip);
    }
    else
    {
        terminalView.println("WiFi Web UI: Not connected. Connect first to see address.");
    }
}

/*
Config
*/
void WifiController::handleConfig()
{
    if (state.getTerminalMode() == TerminalTypeEnum::Standalone) return;

    terminalView.println("[WARNING] If you're connected via Web CLI,");
    terminalView.println("          executing Wi-Fi commands may cause ");
    terminalView.println("          the terminal session to disconnect.");
    terminalView.println("          Don't use: sniff, probe, connect, scan, spoof...");
    terminalView.println("          Use USB serial or restart if connection is lost.\n");
}

/*
Help
*/
void WifiController::handleHelp()
{
    terminalView.println("WiFi commands:");
    terminalView.println("  scan");
    terminalView.println("  connect");;
    terminalView.println("  sniff");
    terminalView.println("  probe");
    terminalView.println("  spoof sta <mac>");
    terminalView.println("  spoof ap <mac>");
    terminalView.println("  deauth [ssid]");
    terminalView.println("  status");
    terminalView.println("  disconnect");
    terminalView.println("  ap <ssid> <password>");
    terminalView.println("  ap spam");
    ANetworkController::handleHelp();
    terminalView.println("  webui");
    terminalView.println("  reset");
}

/*
Ensure Configuration
*/
void WifiController::ensureConfigured()
{
    if (!configured)
    {
        handleConfig();
        configured = true;
    }
}

/*
Deauthenticate stations attack
*/
void WifiController::handleDeauth(const TerminalCommand &cmd)
{   
    auto target = cmd.getSubcommand();
    
    // Select network if no target provided
    if (target.empty()) {
        terminalView.println("Wifi: Scanning for available networks...");
        auto networks = wifiService.scanNetworks();
        int selectedIndex = userInputManager.readValidatedChoiceIndex("\nSelect Wi-Fi network", networks, 0);
        target = networks[selectedIndex];
    }

    // if the SSID have space in name, e.g "Router Wifi"
    if (!cmd.getArgs().empty())
    {
        target += " " + cmd.getArgs();
    }

    terminalView.println("WiFi: Sending deauth to \"" + target + "\"...");

    bool ok = wifiService.deauthApBySsid(target);

    if (ok)
        terminalView.println("WiFi: Deauth frames sent.");
    else
        terminalView.println("WiFi: SSID not found.");
}
