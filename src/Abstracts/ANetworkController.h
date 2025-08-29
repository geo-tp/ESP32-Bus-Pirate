#pragma once

#include <Arduino.h>
#include <string>
#include <ESP32Ping.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Services/WifiService.h"
#include "Services/WifiOpenScannerService.h"
#include "Services/EthernetService.h"
#include "Services/SshService.h"
#include "Services/NetcatService.h"
#include "Services/NmapService.h"
#include "Services/ICMPService.h"
#include "Services/NvsService.h"
#include "Services/TelnetService.h"
#include "Services/HttpService.h"
#include "Services/ModbusService.h"
#include "Transformers/ArgTransformer.h"
#include "Transformers/JsonTransformer.h"
#include "Managers/UserInputManager.h"
#include "States/GlobalState.h"
#include "Models/TerminalCommand.h"
#include "Shells/ModbusShell.h"

class ANetworkController {
public:

    ANetworkController(
        ITerminalView& terminalView, 
        IInput& terminalInput, 
        IInput& deviceInput,
        WifiService& wifiService, 
        WifiOpenScannerService& wifiOpenNetworkService,
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
        ModbusShell& modbusShell
    );

protected:
    void handleNetcat(const TerminalCommand& cmd);
    void handleNmap(const TerminalCommand& cmd);
    void handleSsh(const TerminalCommand& cmd);
    void handlePing(const TerminalCommand& cmd);
    void handleDiscovery(const TerminalCommand& cmd);
    void handleTelnet(const TerminalCommand& cmd);
    void handleModbus(const TerminalCommand& cmd);
    void handleHelp();

    // HTTP
    void handleHttp(const TerminalCommand &cmd);
    void handleHttpGet(const TerminalCommand &cmd);
    void handleHttpAnalyze(const TerminalCommand &cmd);
    
    // Lookup
    void handleLookup(const TerminalCommand& cmd);
    void handleLookupMac(const TerminalCommand& cmd);
    void handleLookupIp(const TerminalCommand& cmd);


protected:
    ITerminalView&     terminalView;
    IInput&            terminalInput;
    IInput&            deviceInput;

    WifiService&       wifiService;
    EthernetService&   ethernetService;

    NvsService&        nvsService;

    WifiOpenScannerService& wifiOpenScannerService;
    SshService&        sshService;
    NetcatService&     netcatService;
    NmapService&       nmapService;
    ICMPService&       icmpService;
    HttpService&       httpService;
    TelnetService&     telnetService;

    ModbusShell&       modbusShell;

    ArgTransformer&    argTransformer;
    JsonTransformer&   jsonTransformer;
    UserInputManager&  userInputManager;
    GlobalState&       globalState = GlobalState::getInstance();
};
