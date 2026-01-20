#pragma once

#if defined(DEVICE_M5STICK) || defined(DEVICE_STICKS3)


#include <M5Unified.h>
#include <Arduino.h>
#include "Interfaces/IInput.h"
#include "InputKeys.h"

// M5Stick C PLUS 2 inputs
class StickInput : public IInput {
public:
    StickInput();

    char handler() override;     // Lecture bloquante
    char readChar() override;    // Lecture non bloquante
    void waitPress() override;   // Attend une pression

private:
    char mapButton();
};

#endif