#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "TembedInput.h"
#include "Inputs/InputKeys.h"
#include <esp_sleep.h>
#include <Arduino.h>

TembedInput::TembedInput()
    : encoder(TEMBED_PIN_ENCODE_A, TEMBED_PIN_ENCODE_B, RotaryEncoder::LatchMode::TWO03),
      lastInput(KEY_NONE),
      lastPos(0),
      lastButton(false),
      pressStart(0)
{
    encoder.setPosition(0);
    pinMode(TEMBED_PIN_ENCODE_BTN, INPUT_PULLUP);
    pinMode(TEMBED_PIN_SIDE_BTN, INPUT_PULLUP);
}

void TembedInput::tick() {
    encoder.tick();

    int pos = encoder.getPosition();
    if (pos < lastPos) {
        lastInput = KEY_ARROW_LEFT;
        lastPos = pos;
    } else if (pos > lastPos) {
        lastInput = KEY_ARROW_RIGHT;
        lastPos = pos;
    } else if (!digitalRead(TEMBED_PIN_ENCODE_BTN) && !lastButton) {
        lastInput = KEY_OK;
        lastButton = true;
    } else if (digitalRead(TEMBED_PIN_ENCODE_BTN)) {
        lastButton = false;
    }

    checkShutdownRequest(); // Check if OK/side button is long pressed
}

char TembedInput::readChar() {
    tick();
    char c = lastInput;
    lastInput = KEY_NONE;
    return c;
}

char TembedInput::handler() {
    while (true) {
        char c = readChar();
        if (c != KEY_NONE) return c;
        delay(5);
    }
}

void TembedInput::waitPress(uint32_t timeoutMs) {
    uint32_t start = millis();
    while (true) {
        if (readChar() != KEY_NONE) return;
        if (timeoutMs > 0 && (millis() - start) >= timeoutMs) return;
        delay(5);
    }
}

void TembedInput::checkShutdownRequest() {
    if (!digitalRead(TEMBED_PIN_ENCODE_BTN) || !digitalRead(TEMBED_PIN_SIDE_BTN)) {
        unsigned long start = millis();

        // Wait 3sec press
        for (int i = 3; i > 0; --i) {
            // Verify if it's still pressed
            for (int j = 0; j < 10; ++j) {
                // Released
                if (digitalRead(TEMBED_PIN_ENCODE_BTN) && digitalRead(TEMBED_PIN_SIDE_BTN)) return;
                delay(100);
            }
        }

        // If we are here, then the button was pressed 3sec
        shutdownToDeepSleep();
    }
}

void TembedInput::shutdownToDeepSleep() {
    view.shutDown();
    delay(3000);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)TEMBED_PIN_SIDE_BTN, 0);
    esp_deep_sleep_start();
}

#endif
