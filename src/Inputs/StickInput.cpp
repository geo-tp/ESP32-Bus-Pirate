#if defined(DEVICE_STICKS3)

#include "StickInput.h"

StickInput::StickInput() {
    M5.begin();
}

char StickInput::mapButton() {
    M5.update();

    if (M5.BtnA.wasPressed()) return KEY_OK;
    if (M5.BtnB.wasPressed()) return KEY_ARROW_LEFT;
    if (M5.BtnPWR.wasPressed()) return KEY_ARROW_RIGHT;

    return KEY_NONE;
}

char StickInput::readChar() {
    return mapButton();
}

char StickInput::handler() {
    char c = KEY_NONE;
    while ((c = mapButton()) == KEY_NONE) {
        delay(10);
    }
    return c;
}

void StickInput::waitPress(uint32_t timeoutMs) {
    uint32_t start = millis();
    for (;;) {
        char c = mapButton();
        if (c != KEY_NONE) {
            return;
        }

        if (timeoutMs > 0 && (millis() - start) >= timeoutMs) {
            return;
        }
        delay(5);
    }
}

#endif