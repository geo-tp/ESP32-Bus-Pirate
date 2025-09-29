#pragma once

#include <string>
#include <vector>
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "Transformers/InstructionTransformer.h"
#include "Enums/ModeEnum.h"
#include "Providers/DependencyProvider.h"
#include "Enums/ByteCodeEnum.h"
#include "Enums/TerminalTypeEnum.h"
#include "Interfaces/ITerminalView.h"

class ActionDispatcher {
public:
    // Constructor with dependency injection
    explicit ActionDispatcher(DependencyProvider& provider);

    // Initialize
    void setup(TerminalTypeEnum terminalType, std::string terminaInfos);

    // Main loop that handles user input
    void run();

    // Process raw user action
    void dispatch(const std::string& raw);

private:
    DependencyProvider& provider;
    GlobalState& state = GlobalState::getInstance();

    // Handle a command
    void dispatchCommand(const TerminalCommand& cmd);

    // Handle a sequence of bytecode instructions
    void dispatchInstructions(const std::vector<Instruction>& instructions);

    // Read user input with cursor support
    std::string getUserAction();

    // Handle ANSI escape sequences (arrows, etc.)
    bool handleEscapeSequence(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode);

    // Handle Cardputer arrows
    bool handleCardputerEscapeSequence(char c, size_t& cursorIndex, std::string& inputLine, const std::string& mode);

    // Handle backspace logic
    bool handleBackspace(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode);

    // Handle printable characters insertion
    bool handlePrintableChar(char c, std::string& inputLine, size_t& cursorIndex, const std::string& mode);

    // Handle Enter key and dispatch line
    bool handleEnterKey(char c, const std::string& inputLine);

    // Switch to a different input mode
    void setCurrentMode(ModeEnum newMode);
};
