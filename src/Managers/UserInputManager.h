#pragma once

#include <string>
#include <algorithm>
#include <cctype>

#include <Interfaces/ITerminalView.h>
#include <Interfaces/IInput.h>
#include <Transformers/ArgTransformer.h>
#include <Enums/ModeEnum.h>

class UserInputManager {
public:
    UserInputManager(ITerminalView& view, IInput& input, ArgTransformer& transformer)
        : terminalView(view), terminalInput(input), argTransformer(transformer) {}

    std::string getLine();
    uint8_t readValidatedUint8(const std::string& label, uint8_t def, uint8_t min, uint8_t max);
    uint8_t readValidatedUint8(const std::string& label, uint8_t defaultVal);
    uint32_t readValidatedUint32(const std::string& label, uint32_t def);
    char readCharChoice(const std::string& label, char def, const std::vector<char>& allowed);
    bool readYesNo(const std::string& label, bool def);
    uint8_t readModeNumber();
    uint8_t readValidatedPinNumber(const std::string& label, uint8_t def, uint8_t min, uint8_t max,  const std::vector<uint8_t>& forbiddenPins);
    uint8_t readValidatedPinNumber(const std::string& label, uint8_t def,  const std::vector<uint8_t>& forbiddenPins);
    std::vector<uint8_t> readValidatedPinGroup(
        const std::string& label,
        const std::vector<uint8_t>& defaultPins,
        const std::vector<uint8_t>& protectedPins
    );
    std::string readValidatedHexString(const std::string& label, size_t numBytes, bool ignoreLen = false, size_t digitsPerItem = 2);
    uint16_t readValidatedCanId(const std::string& label, uint16_t defaultValue);
    int readValidatedChoiceIndex(const std::string& label, const std::vector<std::string>& choices, int defaultIndex = 0);
private:
    ITerminalView& terminalView;
    IInput& terminalInput;
    ArgTransformer& argTransformer;
};
