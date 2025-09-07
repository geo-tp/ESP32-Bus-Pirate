#pragma once
#include <string>
#include <vector>
#include <Enums/TerminalTypeEnum.h>

class ITerminalView {
public:
    virtual ~ITerminalView() = default;
    
    // Initialize the terminal
    virtual void initialize() = 0;

    // Show welcome message with logo and infos
    virtual void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) = 0;

    // Print to the terminal
    virtual void print(const std::string& text) = 0;
    virtual void print(const uint8_t data) = 0;
    virtual void println(const std::string& text) = 0;
    virtual void printPrompt(const std::string& mode = "HIZ") = 0;

    // Wait press
    virtual void waitPress() = 0;

    // Clear the terminal
    virtual void clear() = 0;
};
