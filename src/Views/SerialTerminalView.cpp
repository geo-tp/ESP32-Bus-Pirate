#include "SerialTerminalView.h"

void SerialTerminalView::initialize() {
    Serial.begin(baudrate); // Serial USB CDC, to the PC
    while (!Serial) {
        delay(10);
    }
}

void SerialTerminalView::welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) {

    GlobalState& state = GlobalState::getInstance();
    std::string version = state.getVersion();

    Serial.println("   ____              _ __  _           _       ");
    Serial.println("  | __ ) _   _ ___  | '_ \\(_)_ __ __ _| |_ ___ ");
    Serial.println("  |  _ \\| | | / __| | |_) | | '__/ _` | __/ _ \\");
    Serial.println("  | |_) | |_| \\__ \\ | .__/| | | | (_| | ||  __/");
    Serial.println("  |____/ \\__,_|___/ |_|   |_|_|  \\__,_|\\__\\___|");
    Serial.println();
    Serial.println("             ESP32 SWISS ARMY KNIFE");
/*
    Serial.println("  ____                    _           _       ");
    Serial.println(" | __ ) _   _ ___   _ __ (_)_ __ __ _| |_ ___ ");
    Serial.println(" |  _ \\| | | / __| | '_ \\| | '__/ _` | __/ _ \\");
    Serial.println(" | |_) | |_| \\__ \\ | |_) | | | | (_| | ||  __/");
    Serial.println(" |____/ \\__,_|___/ | .__/|_|_|  \\__,_|\\__\\___|");
    Serial.println("                   |_|                        ");
    */
    Serial.println();
    Serial.printf("     Version %s           Ready to board\n", version.c_str());
    Serial.println("");
    Serial.println(" Type 'mode' to start or 'help' for commands");
    Serial.println("");

}

void SerialTerminalView::print(const std::string& text) {
    Serial.print(text.c_str());
}

void SerialTerminalView::print(const uint8_t data) {
    Serial.write(data);
}

void SerialTerminalView::println(const std::string& text) {
    Serial.println(text.c_str());
}

void SerialTerminalView::printPrompt(const std::string& mode) {
    if (!mode.empty()) {
        Serial.print(mode.c_str());
        Serial.print("> ");
    } else {
        Serial.print("> ");
    }
}

void SerialTerminalView::clear() {
    Serial.write(27);  // ESC
    Serial.print("[2J"); // erase screen
    Serial.write(27);
    Serial.print("[H");  // default cursor pos
}

void SerialTerminalView::waitPress() {
    Serial.println("\n\n\rPress any key to start...");
}

void SerialTerminalView::setBaudrate(unsigned long baud) {
    baudrate = baud;
}