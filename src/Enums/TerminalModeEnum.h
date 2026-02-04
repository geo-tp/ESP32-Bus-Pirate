#pragma once
#include <string>

enum class TerminalMode {
    SerialPort,
    Web,
    None
};

class TerminalModeEnumMapper {
public:
    static std::string toString(TerminalMode mode) {
        switch (mode) {
            case TerminalMode::SerialPort: return "Serial";
            case TerminalMode::Web:    return "Web   ";
            case TerminalMode::None:   return "None  ";
            default:                   return "Unknown";
        }
    }
};
