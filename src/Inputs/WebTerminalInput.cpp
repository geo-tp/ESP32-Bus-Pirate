#include "WebTerminalInput.h"

WebTerminalInput::WebTerminalInput(WebSocketServer& server) : server(server) {}

char WebTerminalInput::handler() {
    return server.readCharBlocking();
}

char WebTerminalInput::readChar() {
    return server.readCharNonBlocking();
}

void WebTerminalInput::waitPress(uint32_t timeoutMs) {
    (void)timeoutMs; // currently not used
    server.readCharBlocking();
}
