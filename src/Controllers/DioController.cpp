#include "Controllers/DioController.h"
#include <sstream>
#include <algorithm>

/*
Constructor
*/
DioController::DioController(ITerminalView& terminalView, IInput& terminalInput, PinService& pinService, ArgTransformer& argTransformer)
    : terminalView(terminalView), terminalInput(terminalInput), pinService(pinService), argTransformer(argTransformer) {}

/*
Entry point to handle a DIO command
*/
void DioController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "sniff") handleSniff(cmd);
    else if (cmd.getRoot() == "read")   handleReadPin(cmd); 
    else if (cmd.getRoot() == "set")    handleSetPin(cmd);
    else if (cmd.getRoot() == "pullup") handlePullup(cmd);
    else if (cmd.getRoot() == "pulldown") handlePulldown(cmd);
    else if (cmd.getRoot() == "pwm")    handlePwm(cmd);
    else if (cmd.getRoot() == "toggle") handleTogglePin(cmd);
    else if (cmd.getRoot() == "pulse")  handlePulse(cmd);
    else if (cmd.getRoot() == "measure") handleMeasure(cmd);
    else if (cmd.getRoot() == "servo")  handleServo(cmd);
    else if (cmd.getRoot() == "reset")  handleResetPin(cmd);
    else                                handleHelp();
    
}

/*
Read
*/
void DioController::handleReadPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: read <pin>");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Read")) return;
    int value = pinService.read(pin);
    terminalView.println("Pin " + std::to_string(pin) + " = " + std::to_string(value) + (value ? " (HIGH)" : " (LOW)"));
}

/*
Set
*/
void DioController::handleSetPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: set <pin> <IN/OUT/HI/LOW>");
        return;
    }

    std::string arg = cmd.getArgs();
    if (arg.empty()) {
        terminalView.println("DIO Set: Missing mode (IN/OUT/HI/LOW).");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Set")) return;
    char c = std::toupper(arg[0]);

    switch (c) {
        case 'I':
            pinService.setInput(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to INPUT");
            break;
        case 'O':
            pinService.setOutput(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to OUTPUT");
            break;
        case 'H':
        case '1':
            pinService.setOutput(pin);
            pinService.setHigh(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to HIGH");
            break;
        case 'L':
        case '0':
            pinService.setOutput(pin);
            pinService.setLow(pin);
            terminalView.println("DIO Set: Pin " + std::to_string(pin) + " set to LOW");
            break;
        default:
            terminalView.println("Unknown command. Use I, O, H (1), or L (0).");
            break;
    }
}

/*
Pullup
*/
void DioController::handlePullup(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: pullup <pin>");
        return;
    }

    int pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Pullup")) return;
    pinService.setInputPullup(pin);

    terminalView.println("DIO Pullup: Set on pin " + std::to_string(pin));
}

/*
Pulldown
*/
void DioController::handlePulldown(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: pulldown <pin>");
        return;
    }

    int pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Pulldown")) return;
    pinService.setInputPullDown(pin);

    terminalView.println("DIO Pulldown: Set on pin " + std::to_string(pin));
}

/*
Sniff
*/
void DioController::handleSniff(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || !argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: sniff <pin>");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Sniff")) return;
    PinService::pullType pull =  pinService.getPullType(pin);
    
    switch (pull)
    {
    case PinService::NOPULL:
        pinService.setInput(pin);
        break;
    case PinService::PULL_UP:
        pinService.setInputPullup(pin);
        break;
    case PinService::PULL_DOWN:
        pinService.setInputPullDown(pin);
        break;
    
    default:
        break;
    }
    

    terminalView.println("DIO Sniff: Pin " + std::to_string(pin) + "... Press [ENTER] to stop");
    
    int last = pinService.read(pin);
    terminalView.println("Initial state: " + std::to_string(last));

    unsigned long lastCheck = millis();
    while (true) {
        // check ENTER press
        if (millis() - lastCheck > 10) {
            lastCheck = millis();
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("DIO Sniff: Stopped.");
                break;
            }
        }

        // check pin state
        int current = pinService.read(pin);
        if (current != last) {
            std::string transition = (last == 0 && current == 1)
                ? "LOW  -> HIGH"
                : "HIGH -> LOW";
            terminalView.println("Pin " + std::to_string(pin) + ": " + transition);
            last = current;
        }
    }
}

/*
Pwm
*/
void DioController::handlePwm(const TerminalCommand& cmd) {
    auto sub = cmd.getSubcommand();
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (!sub.empty() && args.size() != 2) {
        terminalView.println("DIO PWM: Invalid syntax. Use:");
        terminalView.println("  pwm <pin> <frequency> <duty>");
        return;
    }

    if (!argTransformer.isValidNumber(sub) ||
        !argTransformer.isValidNumber(args[0]) ||
        !argTransformer.isValidNumber(args[1])) {
        terminalView.println("DIO PWM: All arguments must be valid numbers.");
        return;
    }

    uint8_t pin = argTransformer.toUint8(sub);
    if (!isPinAllowed(pin, "PWM")) return;

    uint32_t freq = argTransformer.toUint32(args[0]);
    uint8_t duty = argTransformer.toUint8(args[1]);

    if (duty > 100) {
        terminalView.println("DIO PWM: Duty cycle must be between 0 and 100.");
        return;
    }

    bool ok = pinService.setupPwm(pin, freq, duty);
    if (!ok) {
        terminalView.println("DIO PWM: Cannot generate " + std::to_string(freq) +
                            " Hz. Try a higher frequency or use toggle command.");
        return;
    }

    terminalView.println("DIO PWM: Pin " + std::to_string(pin) +
                         " (" + std::to_string(freq) + "Hz, " +
                         std::to_string(duty) + "% duty).");
}

/*
Measure edges
*/
void DioController::handleMeasure(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: edgecount <pin> [duration_ms]");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("DIO Measure: Invalid pin number.");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Measure")) return;

    uint32_t durationMs = 1000;
    if (!args.empty() && argTransformer.isValidNumber(args[0])) {
        durationMs = std::min(argTransformer.toUint32(args[0]), 5000u);
        if (durationMs == 5000) {
            terminalView.println("Note: Duration limited to 5000 ms max.");
        }
    }

    terminalView.println("DIO EdgeCount: Sampling pin " + std::to_string(pin) +
                         " for " + std::to_string(durationMs) + " ms...");

    PinService::pullType pull =  pinService.getPullType(pin);
    
    switch (pull)
    {
    case PinService::NOPULL:
        pinService.setInput(pin);
        break;
    case PinService::PULL_UP:
        pinService.setInputPullup(pin);
        break;
    case PinService::PULL_DOWN:
        pinService.setInputPullDown(pin);
        break;
    
    default:
        break;
    }
    
    int last = pinService.read(pin);
    uint32_t rising = 0, falling = 0;

    unsigned long startMs = millis();

    while (millis() - startMs < durationMs) {
        int current = pinService.read(pin);
        if (current != last) {
            if (last == 0 && current == 1) ++rising;
            else if (last == 1 && current == 0) ++falling;
            last = current;
        }
    }

    terminalView.println("");
    terminalView.println(" Results:");
    terminalView.println("  • Rising edges:     " + std::to_string(rising));
    terminalView.println("  • Falling edges:    " + std::to_string(falling));

    uint32_t totalEdges = rising + falling;
    float freqHz = (totalEdges / 2.0f) / (durationMs / 1000.0f);

    std::ostringstream oss;
    oss.precision(2);
    oss << std::fixed << "  • Approx. frequency: " << freqHz << " Hz\n";
    terminalView.println(oss.str());
}

/*
Toggle
*/
void DioController::handleTogglePin(const TerminalCommand& cmd) {
    
    auto args = argTransformer.splitArgs(cmd.getArgs());
    
    if (cmd.getSubcommand().empty() || args.empty()) {
        terminalView.println("Usage: toggle <pin> <ms>");
        return;
    }
    
    if (!argTransformer.isValidNumber(cmd.getSubcommand()) ||
    !argTransformer.isValidNumber(args[0])) {
        terminalView.println("DIO Toggle: Invalid arguments.");
        return;
    }
    
    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Toggle")) return;

    uint32_t intervalMs = argTransformer.toUint32(args[0]);
    
    pinService.setOutput(pin);
    bool state = false;

    terminalView.println("\nDIO Toggle: Pin " + std::to_string(pin) + " every " + std::to_string(intervalMs) + "ms...Press [ENTER] to stop.");
    terminalView.println("");

    unsigned long lastToggle = millis();
    unsigned long lastCheck = millis();

    while (true) {
        unsigned long now = millis();

        // check ENTER press every 10ms
        if (now - lastCheck > 10) {
            lastCheck = now;
            char c = terminalInput.readChar();
            if (c == '\r' || c == '\n') {
                terminalView.println("DIO Toggle: Stopped.");
                break;
            }
        }

        // toggle pin at defined interval
        if (now - lastToggle >= intervalMs) {
            lastToggle = now;
            state = !state;
            if (state) {
                pinService.setHigh(pin);
            } else {
                pinService.setLow(pin);
            }
        }
    }
}

/*
Reset
*/
void DioController::handleResetPin(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty()) {
        terminalView.println("Usage: reset <pin>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("DIO Reset: Invalid pin number.");
        return;
    }

    uint8_t pin = argTransformer.toUint8(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Reset")) return;

    // Detacher PWM
    ledcDetachPin(pin);

    // Reset Pullup
    pinService.setInput(pin);

    terminalView.println("DIO Reset: Pin " + std::to_string(pin) + " to INPUT (no pull-up, no PWM).");
}

void DioController::handleServo(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || cmd.getArgs().empty()) {
        terminalView.println("Usage: servo <pin> <angle>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand()) ||
        !argTransformer.isValidNumber(cmd.getArgs())) {
        terminalView.println("DIO Servo: Invalid arguments.");
        return;
    }

    uint8_t pin = argTransformer.parseHexOrDec(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Servo")) return;

    uint8_t angle = argTransformer.parseHexOrDec(cmd.getArgs());
    pinService.setServoAngle(pin, angle);
    terminalView.println("DIO Servo: Set pin " + std::to_string(pin) + " to angle " + std::to_string(angle) + ".");
}

/*
Pulse
*/
void DioController::handlePulse(const TerminalCommand& cmd) {
    if (cmd.getSubcommand().empty() || cmd.getArgs().empty()) {
        terminalView.println("Usage: pulse <pin> <duration_us>");
        return;
    }

    if (!argTransformer.isValidNumber(cmd.getSubcommand()) ||
        !argTransformer.isValidNumber(cmd.getArgs())) {
        terminalView.println("DIO Pulse: Invalid arguments.");
        return;
    }

    uint8_t pin = argTransformer.parseHexOrDec(cmd.getSubcommand());
    if (!isPinAllowed(pin, "Pulse")) return;

    uint32_t durationUs = argTransformer.toUint32(cmd.getArgs());

    // Configure en sortie
    pinService.setOutput(pin);

    pinService.setHigh(pin);
    delayMicroseconds(durationUs);
    pinService.setLow(pin);

    terminalView.println("DIO Pulse: Pin " + std::to_string(pin) +
                         " HIGH for " + std::to_string(durationUs) + " µs.");
}

/*
Help
*/
void DioController::handleHelp() {
    terminalView.println("Unknown DIO command. Usage:");
    terminalView.println("  sniff <pin>");
    terminalView.println("  read <pin>");
    terminalView.println("  set <pin> <H/L/I/O>");
    terminalView.println("  pullup <pin>");
    terminalView.println("  pulldown <pin>");
    terminalView.println("  pwm <pin> <freq> <duty>");
    terminalView.println("  servo <pin> <angle>");
    terminalView.println("  measure <pin> [ms]");
    terminalView.println("  pulse <pin> <us>");
    terminalView.println("  toggle <pin> <ms>");
    terminalView.println("  reset <pin>");
}

bool DioController::isPinAllowed(uint8_t pin, const std::string& context) {
    const auto& protectedPins = state.getProtectedPins();
    
    if (std::find(protectedPins.begin(), protectedPins.end(), pin) != protectedPins.end()) {
        terminalView.println("DIO " + context + ": Pin " + std::to_string(pin) + " is protected and cannot be used.");
        return false;
    }

    if (pin > 48 /* max pin for S3 */) {
        terminalView.println("DIO " + context + ": Pin " + std::to_string(pin) + " is out of range (0-48).");
        return false;
    }

    if (pin < 0) {
        terminalView.println("DIO " + context + ": Pin " + std::to_string(pin) + " is invalid.");
        return false;
    }

    return true;
}

std::vector<std::string> DioController::buildPullConfigLines() {
    std::vector<std::string> lines;

    auto pins = pinService.getConfiguredPullPins();
    std::sort(pins.begin(), pins.end());

    size_t total = 0;

    for (uint8_t pin : pins) {
        auto pull = pinService.getPullType(pin);

        if (pull == PinService::PULL_UP) {
            lines.push_back("GPIO " + std::to_string(pin) + " PULLUP");
        } 
        else if (pull == PinService::PULL_DOWN) {
            lines.push_back("GPIO " + std::to_string(pin) + " PULLDOWN");
        } else {
            continue;
        }

        ++total;

        // Limit to 4 displayable lines
        if (lines.size() == 4) {
            if (pins.size() > total) {
                lines.back() += " ...";
            }
            break;
        }
    }

    return lines;
}
