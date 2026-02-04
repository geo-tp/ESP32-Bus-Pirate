#pragma once
#include <string>

enum class TerminalTypeEnum {
    SerialPort,
    WiFiAp,
    WiFiClient,
    Standalone,
    None
};

class TerminalTypeEnumMapper {
public:
    static std::string toString(TerminalTypeEnum type) {
        switch (type) {
            case TerminalTypeEnum::SerialPort:  return "USB Serial";
            case TerminalTypeEnum::WiFiAp:      return "WiFi AP";
            case TerminalTypeEnum::WiFiClient:  return "WiFi Web";
            case TerminalTypeEnum::Standalone:  return "Standalone";
            case TerminalTypeEnum::None:        return "None";
            default:                            return "Unknown";
        }
    }
};
