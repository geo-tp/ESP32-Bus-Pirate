#pragma once

#if defined(DEVICE_STICKS3)


#include <M5Unified.h>
#include <Arduino.h>
#include "Interfaces/IInput.h"
#include "InputKeys.h"

class StickInput : public IInput {
public:
    StickInput();

    char handler() override;     // Lecture bloquante
    char readChar() override;    // Lecture non bloquante
    void waitPress(uint32_t timeoutMs) override;   // Attend une pression

private:
    char mapButton();
};

#endif