#pragma once

#include <Interfaces/IInput.h>
#include <Arduino.h>
#include <vector>

class SerialTerminalInput : public IInput {
public:
    char handler() override;
    void waitPress(uint32_t timeoutMs) override;
    char readChar() override;
};
