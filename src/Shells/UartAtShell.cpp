#include "UartAtShell.h"
#include <sstream>
#include <cstdio>
#include <regex>

UartAtShell::UartAtShell(ITerminalView& terminalView,
                         IInput& terminalInput,
                         UserInputManager& userInputManager,
                         ArgTransformer& argTransformer,
                         UartService& uartService)
: terminalView(terminalView),
  terminalInput(terminalInput),
  userInputManager(userInputManager),
  argTransformer(argTransformer),
  uartService(uartService) {}

void UartAtShell::run() {
    while (true){
        terminalView.println("\n=== UART AT Shell ===");
        
        // Select the mode
        AtMode mode{};
        if (!selectMode(mode)) {
            terminalView.println("Exiting UART AT Shell...\n");
            return;
        }

        // Loop action on the mode
        actionLoop(mode);
    }
}

void UartAtShell::actionLoop(AtMode& mode) {
    while (true) {
        const auto& actions = getAtActionsFor(mode);
        const AtActionItem* chosen = nullptr;

        terminalView.println("\n=== UART AT Shell ===");

        // Select an action
        selectAction(actions, chosen);

        // Back
        if (!chosen) {
            terminalView.println("Going back to mode selection...\n");

            break;
        }

        // Ask for args if needed
        std::string cmd;
        if (!buildCommandFromArgs(chosen->command, chosen->args, chosen->args_count, cmd)) {
            terminalView.println("⚠️  Command canceled.\n");
            continue;
        }

        // Confirm if destructive cmd
        if (!confirmIfDestructive(*chosen)) {
            terminalView.println("⚠️  Destructive command canceled.\n");
            continue;
        }

        // Send the command
        terminalView.println("Send: " + cmd + " ... Waiting for response");
        auto response = sendAt(cmd);

        // Response
        terminalView.println("\n=== Response ===");
        if (response.empty()) {
            terminalView.println("\nNo response from the device.\n");
        } else {
            std::string responseFormatted = "\n" + response;
            terminalView.println(responseFormatted.c_str());
        }
    }
}

bool UartAtShell::selectMode(AtMode& outMode) {
    std::vector<std::string> items;
    items.reserve(kAtModesCount + 1); // + "Exit"

    // Chaque mode
    for (std::size_t i = 0; i < kAtModesCount; ++i) {
        std::string label = joinLabel(kAtModes[i].emoji, kAtModes[i].name);
        if (i < 9) label = " " + label; // alignement visuel 1..9
        items.push_back(std::move(label));
    }

    // Option Exit (toujours en dernier)
    static constexpr const char* kExitLabel = "🚪  Exit Shell";
    items.emplace_back(kExitLabel);

    // Sélection
    int index = userInputManager.readValidatedChoiceIndex("Select AT mode", items, items.size() - 1);
    if (index < 0) return false;

    const std::size_t uindex = static_cast<std::size_t>(index);

    // Choix "Exit" ou hors bornes
    if (uindex >= kAtModesCount) return false;

    outMode = kAtModes[uindex].mode;
    return true;
}

bool UartAtShell::selectAction(AtActionSlice actions, const AtActionItem*& outAction) {
    std::vector<std::string> items;
    items.reserve(actions.size + 1); // + Back

    std::size_t i = 0;
    for (; i < actions.size; ++i) {
        const auto& a = actions.data[i];
        auto label = joinLabel(a.emoji, a.label, a.command);
        if (i < 9) label = " " + label;
        items.push_back(std::move(label));
    }

    // Option "Back"
    items.push_back(i > 9 ? "↩️   Back" : " ↩️   Back");

    int index = userInputManager.readValidatedChoiceIndex("Select command", items, items.size() - 1);
    if (index < 0) return false;

    if (static_cast<std::size_t>(index) == actions.size) {
        return false; // Back
    }

    outAction = &actions.data[static_cast<std::size_t>(index)];
    return true;
}

std::string UartAtShell::buildPromptText(const AtActionArg& a, size_t idx) const {
    std::string label = a.name ? std::string(a.name) : ("arg#" + std::to_string(idx + 1));
    std::string p = "Enter " + label;
    if (a.hint && *a.hint) {
        p += " (e.g. ";
        p += a.hint;
        p += ")";
    }
    if (!a.required && a.defaultValue) {
        p += " [default: ";
        p += a.defaultValue;
        p += "]";
    }
    p += ": ";
    return p;
}

std::string UartAtShell::readUserLine(const std::string& prompt) {
    terminalView.print(prompt);
    return userInputManager.getLine();
}

bool UartAtShell::isInChoices(const std::string& v, const char* choices) const {
    if (!choices) return false;
    std::string vv = argTransformer.toLower(v);
    std::string cs = argTransformer.toLower(choices);

    size_t start = 0;
    while (true) {
        size_t bar = cs.find('|', start);
        std::string tok = cs.substr(start, (bar == std::string::npos) ? (cs.size() - start) : (bar - start));
        if (vv == tok) return true;
        if (bar == std::string::npos) return false;
        start = bar + 1;
    }
}

bool UartAtShell::validateAndFormat(const AtActionArg& a, const std::string& raw, std::string& out) const {
    if (raw.empty()) { terminalView.println("❌ This field is required."); return false; }

    switch (a.type) {
        case AtArgType::Phone:
        case AtArgType::String:
            out = raw;
            return true;

        case AtArgType::Uint:
            if (!argTransformer.isValidNumber(raw)) {
                terminalView.println("❌ Unsigned integer expected.");
                return false;
            }
            out = std::to_string(argTransformer.toUint32(raw));
            return true;

        case AtArgType::Int: {
            int iv = 0;
            if (!argTransformer.parseInt(raw, iv)) {
                terminalView.println("❌ Signed integer expected.");
                return false;
            }
            out = std::to_string(iv);
            return true;
        }

        case AtArgType::Bool01:
            if (raw == "0" || raw == "1") { out = raw; return true; }
            terminalView.println("❌ Enter 0 or 1.");
            return false;

        case AtArgType::HexBytes: {
            auto bytes = argTransformer.parseHexList(raw);
            if (bytes.empty()) {
                terminalView.println("❌ Hex bytes expected (e.g. \"01 AA 03\").");
                return false;
            }
            out.clear();
            for (size_t i = 0; i < bytes.size(); ++i) {
                if (i) out += ' ';
                out += argTransformer.toHex(bytes[i], 2);
            }
            return true;
        }

        case AtArgType::Choice:
            if (!isInChoices(raw, a.choices)) {
                terminalView.println("❌ Invalid choice.");
                return false;
            }
            out = raw;
            return true;

        case AtArgType::Regex:
            try {
                if (!a.pattern) return false;
                if (!std::regex_match(raw, std::regex(a.pattern))) {
                    terminalView.println("❌ Invalid format.");
                    return false;
                }
                out = raw;
                return true;
            } catch (...) {
                terminalView.println("❌ Bad regex.");
                return false;
            }
    }
    return false;
}

bool UartAtShell::acquireArgValue(const AtActionArg& a, size_t idx, std::string& accepted, bool& hasValue) {
    // Reset outputs
    accepted.clear();
    hasValue = false;

    // Keep prompting until we get a valid value, or skip if optional with no default
    while (true) {
        const std::string prompt = buildPromptText(a, idx);
        std::string raw = readUserLine(prompt);

        // If empty input
        if (raw.empty()) {
            // Optional with default -> take default
            if (!a.required && a.defaultValue) {
                accepted = a.defaultValue;
                hasValue = true;
                return true;
            }
            // Optional without default -> skip value entirely
            if (!a.required && !a.defaultValue) {
                // keep hasValue = false, accepted = ""
                return true; // skip
            }
            // Required -> inform and loop
            terminalView.println("❌ This field is required.");
            continue;
        }

        // Non-empty input: validate
        if (validateAndFormat(a, raw, accepted)) {
            hasValue = true;
            return true;
        }
        // else: validation printed an error, loop again
    }
}

std::string UartAtShell::placeholderFor(size_t idx) const {
    return "%" + std::to_string(idx + 1);
}

void UartAtShell::applyArgToCommand(std::string& cmd, size_t idx, const std::string& accepted, bool hasValue) const {
    const std::string ph = placeholderFor(idx);
    size_t pos = cmd.find(ph);

    if (pos != std::string::npos) {
        cmd.replace(pos, ph.size(), accepted);
        return;
    }

    // No placeholder in template: append only if we actually have a value
    if (hasValue) {
        cmd += (idx ? "," : " ");
        cmd += accepted;
    }
}

bool UartAtShell::buildCommandFromArgs(const char* commandTemplate,
                                       const AtActionArg* args,
                                       std::size_t argCount,
                                       std::string& outCmd) {
    if (args == nullptr || argCount == 0) {
        outCmd = commandTemplate;
        return true;
    }

    std::string cmd = commandTemplate;

    for (std::size_t i = 0; i < argCount; ++i) {
        const AtActionArg& a = args[i];
        std::string accepted;
        bool hasValue = false;

        if (!acquireArgValue(a, i, accepted, hasValue)) {
            // Si tu veux pouvoir annuler : return false;
        }

        applyArgToCommand(cmd, i, accepted, hasValue);
    }

    outCmd = std::move(cmd);
    return true;
}

bool UartAtShell::confirmIfDestructive(const AtActionItem& action) {
    if (!action.destructive) return true;

    terminalView.println("⚠️  This action can be destructive: " + std::string(action.label));
    std::vector<std::string> choices = { "No, cancel", "Yes, proceed" };
    int c = userInputManager.readValidatedChoiceIndex("Are you sure?", choices, 0);
    return (c == 1);
}

std::string UartAtShell::sendAt(const std::string& cmd, uint32_t timeoutMs /*=500*/) {
    // uartService.flush();

    // Envoi
    uartService.write(cmd);
    uartService.write("\r\n");
    
    const uint32_t start = millis();
    std::string resp = "";
    uint32_t lastByteTs = start;
    
    // Read everything for timeoutMs
    while (millis() - start < timeoutMs) {
        while (uartService.available() > 0) {
            char c = uartService.read();
            resp.push_back(c);
        }
        delay(1);
    }

    return resp;
}

std::string UartAtShell::joinLabel(const char* emoji, const char* text, const char* rawCmd) {
    std::string s;
    if (emoji && *emoji) { s += emoji; s += "  "; }
    if (rawCmd && *rawCmd) {
        s += rawCmd;
        s+= " - ";
    }
    if (text && *text)  { s += text; }
    return s;
}