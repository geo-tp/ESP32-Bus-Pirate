#pragma once

#ifdef DEVICE_M5STAMPS3

#include "Interfaces/IInput.h"
#include <M5Unified.h>

class StampS3Input : public IInput {
public:
    StampS3Input();

    char readChar() override;
    char handler() override;
    void waitPress(uint32_t timeoutMs) override;

private:
    char mapButton();
};

#endif