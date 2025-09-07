#include "WebTerminalView.h"

WebTerminalView::WebTerminalView(WebSocketServer& server)
    : server(server) {}

void WebTerminalView::initialize() {}

void WebTerminalView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {
    const std::string msg = "Welcome to Web CLI\n";
    server.sendText(msg);
}

void WebTerminalView::print(const std::string& text) {
    server.sendText(text);
}

void WebTerminalView::print(const uint8_t data) {
    server.sendText(std::to_string(data)); // Convert byte to string
}

void WebTerminalView::println(const std::string& text) {
    server.sendText(text + "\n");
}

void WebTerminalView::printPrompt(const std::string& mode) {
    const std::string prompt = mode + "> ";
    server.sendText(prompt);
}

void WebTerminalView::clear() {
    server.sendText("[Screen cleared]\n"); // ph
}

void WebTerminalView::waitPress() {
    server.sendText("\n\nPress any key to start...");
}