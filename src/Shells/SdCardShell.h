#pragma once

#include <string>
#include <sstream>
#include "Services/SdService.h"
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/CommandHistoryManager.h"
#include "Managers/UserInputManager.h"

class SdCardShell {
public:
    SdCardShell(SdService& sdService, ITerminalView& view, IInput& input,  ArgTransformer& argTransformer, UserInputManager& userInputManager);
    void run();

private:
    SdService& sd;
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
    std::string currentDir;
    CommandHistoryManager commandHistoryManager;
    UserInputManager& userInputManager;

    void executeCommand(const std::string& input);

    // Command handlers
    void cmdLs();
    void cmdCd(std::istringstream& iss);
    void cmdMkdir(std::istringstream& iss);
    void cmdTouch(std::istringstream& iss);
    void cmdRm(std::istringstream& iss);
    void cmdCat(std::istringstream& iss);
    void cmdEcho(std::istringstream& iss);
    void cmdHelp();

    // Path utils
    std::string normalizePath(const std::string& path);
    std::string resolveRelativePath(const std::string& base, const std::string& arg);
};
