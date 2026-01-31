#include "ANetworkController.h"

/*
Constructor
*/
ANetworkController::ANetworkController(
    ITerminalView& terminalView, 
    IInput& terminalInput, 
    IInput& deviceInput,
    WifiService& wifiService, 
    WifiOpenScannerService& wifiOpenScannerService,
    EthernetService& ethernetService,
    SshService& sshService,
    NetcatService& netcatService,
    NmapService& nmapService,
    ICMPService& icmpService,
    NvsService& nvsService,
    HttpService& httpService,
    TelnetService& telnetService,
    ArgTransformer& argTransformer,
    JsonTransformer& jsonTransformer,
    UserInputManager& userInputManager,
    ModbusShell& modbusShell,
    HelpShell& helpShell
)
: terminalView(terminalView),
  terminalInput(terminalInput),
  deviceInput(deviceInput),
  wifiService(wifiService),
  wifiOpenScannerService(wifiOpenScannerService),
  ethernetService(ethernetService),
  sshService(sshService),
  netcatService(netcatService),
  nmapService(nmapService),
  icmpService(icmpService),
  nvsService(nvsService),
  httpService(httpService),
  telnetService(telnetService),
  argTransformer(argTransformer),
  jsonTransformer(jsonTransformer),
  userInputManager(userInputManager),
  modbusShell(modbusShell),
  helpShell(helpShell)
{
}

/*
ICMP Ping
*/
void ANetworkController::handlePing(const TerminalCommand &cmd)
{
    if (!wifiService.isConnected() && !ethernetService.isConnected()) {
        terminalView.println("Ping: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    const std::string host = cmd.getSubcommand();
    if (host.empty() || host == "-h" || host == "--help") {
        terminalView.println(icmpService.getPingHelp());
        return;
    }   
    
    #ifndef DEVICE_M5STICK

    auto args = argTransformer.splitArgs(cmd.getArgs());
    int pingCount = 5, pingTimeout = 1000, pingInterval = 200;

    for (int i=0;i<args.size();i++) {
        if (args[i].empty()) continue; // Skip empty args
        auto argument = args[i];
        if (argument == "-h" || argument == "--help") {
            terminalView.println(icmpService.getPingHelp());
            return;
        } else if (argument == "-c") {
            if (++i < args.size()) {
                if (!argTransformer.parseInt(args[i], pingCount) || args[i].empty()) {
                    terminalView.println("Invalid count value.");
                    return;
                }
            }
        } else if (argument == "-t") {
            if (++i < args.size()) {
                if (!argTransformer.parseInt(args[i], pingTimeout) || args[i].empty()) {
                    terminalView.println("Invalid timeout value.");
                    return;
                }
            }
        } else if (argument == "-i") {
            if (++i < args.size()) {
                if (!argTransformer.parseInt(args[i], pingInterval) || args[i].empty()) {
                    terminalView.println("Invalid interval value.");
                    return;
                }
            }
        }
    }

    icmpService.startPingTask(host, pingCount, pingTimeout, pingInterval);
    while (!icmpService.isPingReady())
        vTaskDelay(pdMS_TO_TICKS(50));

    terminalView.print(icmpService.getReport());


    #else  

    // Using ESP32Ping library to avoid IRAM overflow

    const unsigned long t0 = millis();
    const bool ok = Ping.ping(host.c_str(), 1);
    const unsigned long t1 = millis();
    if (ok) {
        terminalView.println("Ping: " + host + " successful, " + std::to_string(t1 - t0) + " ms");
    } else {
        terminalView.println("Ping: " + host + " failed.");
    }

    #endif
}

/*
Discovery  
*/
void ANetworkController::handleDiscovery(const TerminalCommand &cmd)
{
    bool wifiConnected = wifiService.isConnected();
    bool ethConnected = ethernetService.isConnected();
    phy_interface_t phy_interface = phy_interface_t::phy_none;

    // Which interface to scan
    auto mode = globalState.getCurrentMode();
    if (wifiConnected && mode == ModeEnum::WiFi){
        phy_interface = phy_interface_t::phy_wifi;
    }
    else if (ethConnected && mode == ModeEnum::ETHERNET) {
        phy_interface = phy_interface_t::phy_eth;
    }
    else {
        terminalView.println("Discovery: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    const std::string deviceIP = phy_interface == phy_interface_t::phy_wifi ? wifiService.getLocalIP() : ethernetService.getLocalIP();
    icmpService.startDiscoveryTask(deviceIP);

    while (!icmpService.isDiscoveryReady()) {
        // Display logs
        auto batch = icmpService.fetchICMPLog();
        for (auto& line : batch) {
            terminalView.println(line);
        }

        // Enter Press to stop
        int terminalKey = terminalInput.readChar();
        if (terminalKey == '\n' || terminalKey == '\r') {
            icmpService.stopICMPService();
            break;
        }
        char deviceKey = deviceInput.readChar();
        if (deviceKey == KEY_OK) {
            icmpService.stopICMPService();
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }

    delay(500);
    // Flush final logs
    for (auto& line : icmpService.fetchICMPLog()) {
        terminalView.println(line);
    }

    ICMPService::clearICMPLogging();
    icmpService.clearDiscoveryFlag();
    //terminalView.println(icmpService.getReport());
}

/*
Netcat
*/
void ANetworkController::handleNetcat(const TerminalCommand& cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("Netcat: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }
    // Args: nc <host> <port>
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 1) {
        terminalView.println("Usage: nc <host> <port>");
        return;
    }

    std::string host = cmd.getSubcommand();
    std::string portStr = args[0];

    if (!argTransformer.isValidNumber(portStr)) {
        terminalView.println("Netcat: Invalid port number.");
        return;
    }
    int port = argTransformer.parseHexOrDec16(portStr);
    if (port < 1 || port > 65535) {
        terminalView.println("Netcat: Port must be between 1 and 65535.");
        return;
    }

    terminalView.println("Netcat: Connecting to " + host + " with port " + portStr + "...");
    netcatService.startTask(host, 0, port, true);

    // Wait for connection
    unsigned long start = millis();
    while (!netcatService.isConnected() && millis() - start < 5000) {
        delay(50);
    }

    if (!netcatService.isConnected()) {
        terminalView.println("\r\nNetcat: Connection failed.");
        netcatService.close();
        return;
    }

    terminalView.println("Netcat: Connected. Shell started... Press [ANY ESP32 BUTTON] to stop.\n");

    while (true) {
        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE)
            break;

        char terminalKey = terminalInput.readChar();
        if (terminalKey == KEY_NONE) {
            std::string out = netcatService.readOutputNonBlocking();
            if (!out.empty()) terminalView.print(out);
            delay(10);
            continue;
        }

        netcatService.writeChar(terminalKey);
        terminalView.print(std::string(1, terminalKey)); // local echo
        if (terminalKey == '\r' || terminalKey == '\n') terminalView.println("");

        std::string output = netcatService.readOutputNonBlocking();
        if (!output.empty()) terminalView.print(output);
        delay(10);
    }

    netcatService.close();
    terminalView.println("\r\n\nNetcat: Session closed.");
}

/*
Nmap
*/
void ANetworkController::handleNmap(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("Nmap: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    auto args = argTransformer.splitArgs(cmd.getArgs());

    // Parse args
    // Parse hosts first
    auto hosts_arg = cmd.getSubcommand();
    
    // First helper invoke
    if (hosts_arg.compare("-h") == 0 || hosts_arg.compare("--help") == 0  || hosts_arg.empty()){
        terminalView.println(nmapService.getHelpText());
        return;
    }

    if(!nmapService.parseHosts(hosts_arg)) {
        terminalView.println("Nmap: Invalid host.");
        return;
    }

    // Check the first char of args is '-'
    if (!args.empty() && (args[0].empty() || args[0][0] != '-')) {
        terminalView.println("Nmap: Options must start with '-' (ex: -p 22)");
        return;
    }

    nmapService.setArgTransformer(argTransformer);
    auto tokens = argTransformer.splitArgs(cmd.getArgs());
    auto options = NmapService::parseNmapArgs(tokens);
    this->nmapService.setOptions(options);
    
    // Second helper
    if (options.help) {
        terminalView.println(nmapService.getHelpText());
        return;
    }

    if (options.hasTrash){
        // TODO handle this better
        //terminalView.println("Nmap: Invalid options.");
    }

    if (options.hasPort) {
        nmapService.setLayer4(options.tcp);
        // Parse ports
        if (!nmapService.parsePorts(options.ports)) {
            terminalView.println("Nmap: invalid -p value. Use 80,22,443 or 1000-2000.");
            return;
        }
    } else {
        nmapService.setLayer4(options.tcp);
        // Set the most popular ports
        nmapService.setDefaultPorts(options.tcp);
        terminalView.println("Nmap: Using top 100 common ports (may take a few seconds)");
    }

    // Re-use it for ICMP pings
    nmapService.setICMPService(&icmpService);
    nmapService.startTask(options.verbosity);
    
    while(!nmapService.isReady()){
        delay(100);
    }

    terminalView.println(nmapService.getReport());
    nmapService.clean();
    
    terminalView.println("\r\n\nNmap: Scan finished.");
}

/*
SSH
*/
void ANetworkController::handleSsh(const TerminalCommand &cmd)
{
    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("SSH: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    // Check args
    auto args = argTransformer.splitArgs(cmd.getArgs());
    if (cmd.getSubcommand().empty() || args.size() < 2)
    {
        terminalView.println("Usage: ssh <host> <user> <password> [port]");
        return;
    }

    // Check port
    int port = 22;
    if (args.size() == 3)
    {
        if (argTransformer.isValidNumber(args[2]))
        {
            port = argTransformer.parseHexOrDec16(args[2]);
        }
    }

    std::string host = cmd.getSubcommand();
    std::string user = args[0];
    std::string pass = args[1];

    // Connect, start the ssh task
    terminalView.println("SSH: Connecting to " + host + " as " + user + " with port " + std::to_string(port) + "...");
    sshService.startTask(host, user, pass, false, port);

    // Wait 5sec for connection success
    unsigned long start = millis();
    while (!sshService.isConnected() && millis() - start < 5000)
    {
        delay(500);
    }

    // Can't connect
    if (!sshService.isConnected())
    {
        terminalView.println("\r\nSSH: Connection failed.");
        sshService.close();
        return;
    }

    // Connected, start the bridge loop
    terminalView.println("SSH: Connected. Shell started... Press [ANY ESP32 KEY] to stop.\n");
    while (true)
    {
        char terminalKey = terminalInput.readChar();
        if (terminalKey != KEY_NONE)
            sshService.writeChar(terminalKey);

        char deviceKey = deviceInput.readChar();
        if (deviceKey != KEY_NONE)
            break;

        std::string output = sshService.readOutputNonBlocking();
        if (!output.empty())
            terminalView.print(output);

        delay(10);
    }

    // Close SSH
    sshService.close();
    terminalView.println("\r\n\nSSH: Session closed.");
}

/*
HTTP
*/
void ANetworkController::handleHttp(const TerminalCommand &cmd)
{
    #ifndef DEVICE_M5STICK

    // Check connection
    if (!wifiService.isConnected() && !ethernetService.isConnected())
    {
        terminalView.println("HTTP: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    const auto sub = cmd.getSubcommand();

    // http get <url>
    if (sub == "get" && !cmd.getArgs().empty()) {
        handleHttpGet(cmd);
        return;
    // PH for POST, PUT, DELETE
    } else if (sub == "post" || sub == "put" || sub == "delete") {
        terminalView.println("HTTP: Only GET implemented for now.");
        return;
    // http analyze <url>
    } else if (sub == "analyze") {
        handleHttpAnalyze(cmd);
        return;
    // http <url>
    } else if (!sub.empty() && cmd.getArgs().empty()) {
        handleHttpGet(cmd);
        return;
    } else {
        terminalView.println("Usage: http <get|post|put|delete> <url>");
    }

    #else

    terminalView.println("HTTP: M5Stick is not supported.");

    #endif
}

/*
HTTP GET
*/
void ANetworkController::handleHttpGet(const TerminalCommand &cmd)
{
    if (cmd.getSubcommand() == "get" && cmd.getArgs().empty())
    {
        terminalView.println("Usage: http get <url>");
        return;
    }

    // Support for http <url> or http get <url>
    auto arg = cmd.getArgs().empty() ? cmd.getSubcommand() : cmd.getArgs();
    std::string url = argTransformer.ensureHttpScheme(arg);

    terminalView.println("HTTP: Sending GET request to " + url + "...");
    httpService.startGetTask(url, 10000, 8192, true, 30000);

    // Wait until timeout or response is ready
    const unsigned long deadline = millis() + 10000;
    while (!httpService.isResponseReady() && millis() < deadline) {
        delay(50);
    }

    if (httpService.isResponseReady()) {
        terminalView.println("\n========== HTTP GET =============");
        terminalView.println(argTransformer.normalizeLines(httpService.lastResponse()));
        terminalView.println("=================================\n");

    } else {
        terminalView.println("\nHTTP: Error, request timed out");
    }

    httpService.reset();
}

/*
HTTP Analayze
*/
void ANetworkController::handleHttpAnalyze(const TerminalCommand& cmd)
{
    if (cmd.getArgs().empty()) {
        terminalView.println("Usage: http analyze <url>");
        return;
    }

    // Ensure URL has HTTP scheme and then extract host
    const std::string url  = argTransformer.ensureHttpScheme(cmd.getArgs());
    const std::string host = argTransformer.extractHostFromUrl(url);
    std::vector<std::string> lines;
    std::string resp;

    // === urlscan.io (last public scan) ====
    const std::string urlscanUrl =
        "https://urlscan.io/api/v1/search?datasource=scans&q=page.domain:" + host + "&size=1";

    terminalView.println("HTTP Analyze: " + urlscanUrl + " (latest public scan)...");
    resp = httpService.fetchJson(urlscanUrl, 8192);
    terminalView.println("\n===== URLSCAN LATEST =====");
    lines = jsonTransformer.toLines(jsonTransformer.dechunk(resp));
    for (auto& l : lines) terminalView.println(l);
    terminalView.println("==========================\n");


    // === ssllabs.com ====
    const std::string ssllabsUrl =
        "https://api.ssllabs.com/api/v3/analyze?host=" + url;
        

    terminalView.println("HTTP Analyze: " + ssllabsUrl + " (SSL Labs)...");
    resp = httpService.fetchJson(ssllabsUrl, 16384);

    terminalView.println("\n===== SSL LABS =====");
    lines = jsonTransformer.toLines(jsonTransformer.dechunk(resp));
    for (auto& l : lines) terminalView.println(l);
    terminalView.println("====================\n");
    httpService.reset();

    // ==== W3C HTML Validator (optional) ====
    auto confirm = userInputManager.readYesNo("\nAnalyze with the W3C Validator?", false);
    if (confirm) {
        const std::string w3cUrl =
            "https://validator.w3.org/nu/?out=json&doc=" + url;

        terminalView.println("Analyze: " + w3cUrl + " (W3C validator)...");
        resp = httpService.fetchJson(w3cUrl, 16384);
        terminalView.println("\n===== W3C RESULT =====");
        lines = jsonTransformer.toLines(jsonTransformer.dechunk(resp));
        for (auto& l : lines) terminalView.println(l);
        terminalView.println("======================\n");
        httpService.reset();
    }
    terminalView.println("\nHTTP Analyze: Finished.");
}

/*
Lookup
*/
void ANetworkController::handleLookup(const TerminalCommand& cmd)
{
    # ifndef DEVICE_M5STICK

    if (!wifiService.isConnected() && !ethernetService.isConnected()) {
        terminalView.println("Lookup: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    const std::string sub = cmd.getSubcommand();
    if (sub == "mac") {
        handleLookupMac(cmd);
    } else if (sub == "ip") {
        handleLookupIp(cmd);
    } else {
        terminalView.println("Usage: lookup mac <addr>");
        terminalView.println("       lookup ip <addr or url>");
    }

    #else

    terminalView.println("Lookup: M5Stick is not supported.");

    #endif
}

/*
Lookup MAC
*/
void ANetworkController::handleLookupMac(const TerminalCommand& cmd)
{
    if (cmd.getArgs().empty()) {
        terminalView.println("Usage: lookup mac <mac addr>");
        return;
    }

    const std::string mac = cmd.getArgs();
    const std::string url = "https://api.maclookup.app/v2/macs/" + mac;

    terminalView.println("Lookup MAC: " + url + " ...");

    std::string resp = httpService.fetchJson(url, 1024 * 4);

    terminalView.println("\n===== MAC LOOKUP =====");
    auto lines = jsonTransformer.toLines(resp);
    for (auto& l : lines) {
        terminalView.println(l);
    }
    terminalView.println("======================\n");

    httpService.reset();
}

/*
Lookup IP info
*/
void ANetworkController::handleLookupIp(const TerminalCommand& cmd)
{
    if (cmd.getArgs().empty()) {
        terminalView.println("Usage: lookup ip <addr or url>");
        return;
    }

    const std::string target = cmd.getArgs();
    const std::string url = "http://ip-api.com/json/" + target;
    const std::string url2 = "https://isc.sans.edu/api/ip/" + target + "?json";
    std::vector<std::string> lines;
    std::string resp;

    terminalView.println("Lookup IP: " + url + " ...");

    resp = httpService.fetchJson(url, 1024 * 4);
    terminalView.println("\n===== IP LOOKUP =====");
    lines = jsonTransformer.toLines(resp);
    for (auto& l : lines) terminalView.println(l);
    terminalView.println("=====================");

    resp = httpService.fetchJson(url2, 1024 * 4);
    lines = jsonTransformer.toLines(resp);
    for (auto& l : lines) terminalView.println(l);
    terminalView.println("=====================\n");

    httpService.reset();
}

/*
Telnet
*/
void ANetworkController::handleTelnet(const TerminalCommand &cmd)
{
    if (!wifiService.isConnected() && !ethernetService.isConnected()) {
        terminalView.println("TELNET: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: telnet <host> [port]");
        return;
    }

    // Get host and port
    const std::string host = cmd.getSubcommand();
    uint16_t port = 23; // default telnet
    if (argTransformer.isValidNumber(cmd.getArgs())) {
        port = argTransformer.parseHexOrDec16(cmd.getArgs());
    }

    // Connect to telnet
    terminalView.println("TELNET: Connecting to " + host + " on port " + std::to_string(port) + "...");
    if (!telnetService.connectTo(host, static_cast<uint16_t>(port), 3000)) {
        terminalView.println("TELNET: Connection failed: " + telnetService.lastError());
        return;
    }

    // Connection success
    terminalView.println("TELNET: Connected. Shell started... Press [ANY ESP32 KEY] to stop.\n");
    while (true)
    {
        // terminal to telnet
        char k = terminalInput.readChar();
        if (k != KEY_NONE) telnetService.writeChar(k);

        // device button press to stop
        if (deviceInput.readChar() != KEY_NONE) break;

        // telnet to terminal
        telnetService.poll();
        std::string out = telnetService.readOutputNonBlocking();
        if (!out.empty()) terminalView.print(out);

        delay(5);
    }

    telnetService.close();
    terminalView.println("\r\n\nTELNET: Session closed.");
}

/*
Modbus
*/
void ANetworkController::handleModbus(const TerminalCommand &cmd)
{
    // Verify connection
    if (!wifiService.isConnected() && !ethernetService.isConnected()) {
        terminalView.println("Modbus: You must be connected to Wi-Fi or Ethernet. Use 'connect' first.");
        return;
    }

    // Verify host
    const std::string host = cmd.getSubcommand();
    if (host.empty()) {
        terminalView.println("Usage: modbus <host> [port]");
        return;
    }

    // Port
    uint16_t port = 502; // default modbus
    if (argTransformer.isValidNumber(cmd.getArgs())) {
        port = argTransformer.parseHexOrDec16(cmd.getArgs());
    }

    // Start shell
    terminalView.println("Starting Modbus shell...");
    modbusShell.run(host, port);
}