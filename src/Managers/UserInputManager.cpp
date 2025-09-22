#include "UserInputManager.h"

std::string UserInputManager::getLine(bool onlyNumber /* = false */) {
    std::string result;
    size_t cursorIndex = 0;

    while (true) {
        char c = terminalInput.handler();

        if (c == CARDPUTER_SPECIAL_ARROW_UP) {
            terminalView.print(std::string(1, CARDPUTER_SPECIAL_ARROW_UP));
            continue;
        }

        if (c == CARDPUTER_SPECIAL_ARROW_DOWN) {
            terminalView.print(std::string(1, CARDPUTER_SPECIAL_ARROW_DOWN));
            continue;
        }

        // Enter
        if (c == '\r' || c == '\n') break;

        // Backspace
        if ((c == '\b' || c == 127) && cursorIndex > 0) {
            cursorIndex--;
            result.erase(cursorIndex, 1);
            terminalView.print("\b \b");
            continue;
        }

        // Only number
        if (onlyNumber && !std::isdigit(c)) {
            continue;
        }

        // Printable
        if (isprint(c)) {
            result.insert(cursorIndex, 1, c);
            cursorIndex++;
            terminalView.print(std::string(1, c));
        }
    }

    terminalView.println("");
    return result;
}

uint8_t UserInputManager::readValidatedUint8(const std::string& label, uint8_t def, uint8_t min, uint8_t max) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        if (argTransformer.isValidNumber(input)) {
            uint8_t val = argTransformer.toUint8(input);
            if (val >= min && val <= max) return val;
        }

        terminalView.println("Invalid input. Must be " + std::to_string(min) + "-" + std::to_string(max));
    }
}

uint8_t UserInputManager::readValidatedUint8(const std::string& label, uint8_t defaultVal) {
    return readValidatedUint8(label, defaultVal, 0, 255);
}

uint32_t UserInputManager::readValidatedUint32(const std::string& label, uint32_t def) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        if (argTransformer.isValidNumber(input)) {
            return argTransformer.toUint32(input);
        }

        terminalView.println("Invalid number.");
    }
}

char UserInputManager::readCharChoice(const std::string& label, char def, const std::vector<char>& allowed) {
    while (true) {
        terminalView.print(label + " [" + def + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        char c = toupper(input[0]);
        if (std::find(allowed.begin(), allowed.end(), c) != allowed.end()) return c;

        terminalView.println("Invalid choice.");
    }
}

bool UserInputManager::readYesNo(const std::string& label, bool def) {
    while (true) {
        terminalView.print(label + " [" + (def ? "y" : "n") + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        char c = tolower(input[0]);
        if (c == 'y') return true;
        if (c == 'n') return false;

        terminalView.println("Please answer y or n.");
    }
}

uint8_t UserInputManager::readModeNumber() {
    std::string inputDigit;
    size_t cursorIndex = 0;

    while (true) {
        char c = terminalInput.handler();

        if (c == CARDPUTER_SPECIAL_ARROW_UP) {
            terminalView.print(std::string(1, CARDPUTER_SPECIAL_ARROW_UP));
            continue;
        }

        if (c == CARDPUTER_SPECIAL_ARROW_DOWN) {
            terminalView.print(std::string(1, CARDPUTER_SPECIAL_ARROW_DOWN));
            continue;
        }

        // Enter
        if (c == '\r' || c == '\n') {
            terminalView.println("");
            break;
        }

        // Backspace
        if ((c == '\b' || c == 127) && cursorIndex > 0) {
            cursorIndex--;
            inputDigit.erase(cursorIndex, 1);
            terminalView.print("\b \b");
            continue;
        }

        // Only number
        if (std::isdigit(c)) {
            inputDigit.insert(cursorIndex, 1, c);
            cursorIndex++;
            terminalView.print(std::string(1, c));
        }
    }

    if (inputDigit.empty()) {
        return -1;
    }

    return std::stoi(inputDigit);
}

uint8_t UserInputManager::readValidatedPinNumber(const std::string& label, uint8_t def, uint8_t min, uint8_t max, const std::vector<uint8_t>& forbiddenPins) {
    while (true) {
        int val = readValidatedUint8(label, def, min, max);
        if (std::find(forbiddenPins.begin(), forbiddenPins.end(), val) != forbiddenPins.end()) {
            terminalView.println("This pin is reserved/protected and cannot be used.");
            continue;
        }
        return val;
    }
}

uint8_t UserInputManager::readValidatedPinNumber(const std::string& label, uint8_t def, const std::vector<uint8_t>& forbiddenPins) {
    return readValidatedPinNumber(label, def, 0, 48, forbiddenPins);
}

std::vector<uint8_t> UserInputManager::readValidatedPinGroup(
    const std::string& label,
    const std::vector<uint8_t>& defaultPins,
    const std::vector<uint8_t>& protectedPins
) {
    while (true) {
        // Default pins
        std::string defaultStr;
        for (size_t i = 0; i < defaultPins.size(); ++i) {
            if (i > 0) defaultStr += " ";
            defaultStr += std::to_string(defaultPins[i]);
        }
        
        // Display default list [1 2 3 ...]
        terminalView.print(label + " [" + defaultStr + "]: ");

        // Get user input
        std::string input = getLine();

        // Empty, we keep default pins
        if (input.empty()) {
            return defaultPins;
        }

        std::stringstream ss(input);
        std::vector<uint8_t> pins;
        int val;
        
        // Validate
        bool valid = true;
        while (ss >> val) {
            // Invalid
            if (val < 0 || val > 48) {
                terminalView.println("Invalid pin: " + std::to_string(val));
                valid = false;
                break;
            }
            // Protected
            if (std::find(protectedPins.begin(), protectedPins.end(), val) != protectedPins.end()) {
                terminalView.println("Pin " + std::to_string(val) + " is protected/reserved.");
                valid = false;
                break;
            }
            pins.push_back(static_cast<uint8_t>(val));
        }

        // Valid new pins
        if (valid && !pins.empty()) {
            return pins;
        }

        terminalView.println("Please enter valid, non-protected GPIOs separated by spaces.");
    }
}

std::string UserInputManager::readValidatedHexString(
    const std::string& label,
    size_t numItems,
    bool ignoreLen,
    size_t digitsPerItem /* = 2 */)
{
    while (true) {
        terminalView.print(label + "(hex): ");
        std::string input = getLine();

        // Remove space
        input.erase(std::remove_if(input.begin(), input.end(),
                    [](unsigned char c){ return std::isspace(c); }), input.end());

        // Empty ?
        if (input.empty()) {
            if (ignoreLen) {
                // Default value (one item at 0)
                return (digitsPerItem == 2) ? "00" : "0000";
            }
            terminalView.println("❌ Input cannot be empty.");
            continue;
        }

        // Verify hex
        bool valid = std::all_of(input.begin(), input.end(),
                        [](unsigned char c){ return std::isxdigit(c); });
        if (!valid) {
            terminalView.println("❌ Invalid characters. Only hexadecimal digits (0-9, A-F) are allowed.");
            continue;
        }

        // Verify length
        if (ignoreLen) {
            if (input.length() % digitsPerItem != 0) {
                terminalView.println("❌ Length must be a multiple of " + std::to_string(digitsPerItem) + " hex digits.");
                continue;
            }
        } else {
            const size_t expected = numItems * digitsPerItem;
            if (input.length() != expected) {
                terminalView.println("❌ Invalid length. Expected " + std::to_string(expected) + " hex digits.");
                continue;
            }
        }

        // Inject spaces between items (every digitsPerItem characters)
        std::string spaced;
        for (size_t i = 0; i < input.length(); i += digitsPerItem) {
            if (i > 0) spaced += ' ';
            spaced += input.substr(i, digitsPerItem);
        }

        return spaced;
    }
}

uint16_t UserInputManager::readValidatedCanId(const std::string& label, uint16_t defaultValue) {
    while (true) {
        terminalView.print(label + " (hex, max 3 digits) [default: " + argTransformer.toHex(defaultValue, 3) + "]: ");
        std::string input = getLine();

        if (input.empty()) return defaultValue;

        // Remove spaces
        input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());

        // Allow "0x" prefix
        if (input.rfind("0x", 0) == 0 || input.rfind("0X", 0) == 0) {
            input = input.substr(2);  // supprime le "0x"
        }

        // Check if input is valid hex
        bool valid = std::all_of(input.begin(), input.end(), [](char c) {
            return std::isxdigit(static_cast<unsigned char>(c));
        });

        if (!valid) {
            terminalView.println("❌ Invalid characters. Use hex digits only (0-9, A-F).");
            continue;
        }

        // Check length
        if (input.length() > 3) {
            terminalView.println("❌ Too long. Standard CAN ID must be ≤ 0x7FF (3 hex digits).");
            continue;
        }

        // Convert
        uint16_t id = std::stoul(input, nullptr, 16);

        // Check max value
        if (id > 0x7FF) {
            terminalView.println("❌ Value exceeds standard 11-bit CAN ID (max 0x7FF).");
            continue;
        }

        return id;
    }
}

int UserInputManager::readValidatedChoiceIndex(const std::string& label, const std::vector<std::string>& choices, int defaultIndex) {
    // Display choices
    terminalView.println(label + ":");
    for (size_t i = 0; i < choices.size(); ++i) {
        std::string prefix = (i == defaultIndex) ? "*" : " ";
        terminalView.println("  [" + std::to_string(i + 1) + "] " + prefix + choices[i]);
    }

    // Ask for index
    terminalView.print("Enter index (default " + std::to_string(defaultIndex + 1) + "): ");
    std::string input = getLine(true); // only numbers
    input = argTransformer.toLower(argTransformer.filterPrintable(input));

    // Default
    if (input.empty()) return defaultIndex;

    // Validate index
    int index;
    if (!argTransformer.parseInt(input, index) || index < 1 || index > (int)choices.size()) {
        terminalView.println("❌ Invalid choice. Using default.");
        return defaultIndex;
    }

    return index - 1; // Convert to 0 based index
}

int UserInputManager::readValidatedInt(const std::string& label, int def, int min /* = -127 */, int max /* = 127 */) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        int val = 0;
        if (argTransformer.parseInt(input, val) && val >= min && val <= max) {
            return val;
        }
        terminalView.println("Invalid input. Must be " + std::to_string(min) + "-" + std::to_string(max));
    }
}

int UserInputManager::readValidatedChoiceIndex(const std::string& label, const std::vector<int>& choices, int defaultIndex) {
    std::vector<std::string> strChoices;
    strChoices.reserve(choices.size());
    for (int val : choices) {
        strChoices.push_back(std::to_string(val));
    }
    return readValidatedChoiceIndex(label, strChoices, defaultIndex);
}

int UserInputManager::readValidatedChoiceIndex(const std::string& label,
                                               const std::vector<float>& choices,
                                               int defaultIndex) {
    std::vector<std::string> strChoices;
    strChoices.reserve(choices.size());

    for (float f : choices) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(2) << f; 
        strChoices.push_back(oss.str());
    }
    return readValidatedChoiceIndex(label, strChoices, defaultIndex);
}

float UserInputManager::readValidatedFloat(const std::string& label,
                                           float def,
                                           float min,
                                           float max) {
    while (true) {
        terminalView.print(label + " [" + std::to_string(def) + "]: ");
        std::string input = getLine();
        if (input.empty()) return def;

        // Remove spaces
        input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());

        try {
            float v = std::stof(input);
            if (v >= min && v <= max) return v;
        } catch (...) {
            // fallthrough
        }
        terminalView.println("Invalid input. Must be " + std::to_string(min) + " .. " + std::to_string(max));
    }
}