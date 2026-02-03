#pragma once

#include "Interfaces/IInput.h"
#include "Arduino.h"

class S3DevKitInput : public IInput {
public:
    S3DevKitInput();

    char mapButton();
    char readChar();
    char handler();
    void waitPress(uint32_t timeoutMs);
};
