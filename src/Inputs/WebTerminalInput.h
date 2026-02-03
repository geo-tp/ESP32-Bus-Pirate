#pragma once
#include "Interfaces/IInput.h"
#include "Servers/WebSocketServer.h"

class WebTerminalInput : public IInput {
public:
    WebTerminalInput(WebSocketServer& server);

    char handler() override;
    char readChar() override;
    void waitPress(uint32_t timeoutMs) override;

private:
    WebSocketServer& server;
};
