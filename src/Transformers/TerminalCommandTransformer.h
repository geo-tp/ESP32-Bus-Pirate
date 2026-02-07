#pragma once
#include <string>
#include <Models/TerminalCommand.h>

class TerminalCommandTransformer {
public:
    TerminalCommand transform(const std::string& raw) const;
private:
    std::string normalizeRaw(const std::string& raw) const;
    void autoCorrectRoot(TerminalCommand& cmd) const;
    void autoCorrectSubCommand(TerminalCommand& cmd) const;
    int scoreTightEditDistance(const std::string& a, const char* b) const;

    // aliases for protocols
    struct Alias { const char* from; const char* to; };
    inline static const Alias aliases[] = {
        {"onewire",   "1wire"},
        {"1w",        "1wire"},
        {"twowire",   "2wire"},
        {"2w",        "2wire"},
        {"threewire", "3wire"},
        {"3w",        "3wire"},
        {"i2",        "i2c"},
        {"serial",    "uart"},
        {"hd",        "hduart"},
        {"eth",       "ethernet"},
        {"bt",        "bluetooth"},
        {"ble",       "bluetooth"},
        {"ir",        "infrared"},
        {"sub",       "subghz"},
        {"rf",        "rf24"},
        {"nfc",       "rfid"},

        {nullptr, nullptr}
    };
};
