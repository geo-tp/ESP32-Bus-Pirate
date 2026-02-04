#pragma once

#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "Interfaces/IInput.h"
#include <RotaryEncoder.h>
#include <Arduino.h>
// #include <Views/TembedDeviceView.h>

#ifdef DEVICE_TEMBEDS3CC1101
    #define TEMBED_PIN_ENCODE_A          4
    #define TEMBED_PIN_ENCODE_B          5
    #define TEMBED_PIN_SIDE_BTN          6
#else
    #define TEMBED_PIN_ENCODE_A          2
    #define TEMBED_PIN_ENCODE_B          1
    #define TEMBED_PIN_SIDE_BTN          0 // side button is reset
#endif

#define TEMBED_PIN_ENCODE_BTN            0

class TembedInput : public IInput {
public:
    TembedInput();

    char handler() override;
    char readChar() override;
    void waitPress(uint32_t timeoutMs) override;

    void tick();
    void checkShutdownRequest();
    void shutdownToDeepSleep();

private:
    RotaryEncoder encoder;
    // TembedDeviceView view;
    char lastInput;
    char lastButton;
    int lastPos;
    unsigned long pressStart;
};

#endif