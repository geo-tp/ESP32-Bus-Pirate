#pragma once
#include "Interfaces/ITerminalView.h"
#include <string>
#include "Servers/WebSocketServer.h"
#include "Enums/TerminalTypeEnum.h"

class WebTerminalView : public ITerminalView {
public:
    WebTerminalView(WebSocketServer &server);

    void initialize() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void print(const std::string& text) override;
    void print(const uint8_t data) override;
    void println(const std::string& text) override;
    void printPrompt(const std::string& mode) override;
    void clear() override;
    void waitPress() override;
    
private:
    WebSocketServer server;
};
