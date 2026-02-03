#ifdef DEVICE_M5STAMPS3

#include "Inputs/StampS3Input.h"
#include "Inputs/InputKeys.h"

StampS3Input::StampS3Input() {
    M5.begin();
}

char StampS3Input::mapButton() {
    M5.update();

    if (M5.BtnA.wasPressed()) return KEY_OK;

    return KEY_NONE;
}

char StampS3Input::readChar() {
    return mapButton();
}

char StampS3Input::handler() {
    char c = KEY_NONE;
    while ((c = mapButton()) == KEY_NONE) {
        delay(10);
    }
    return c;
}

void StampS3Input::waitPress(uint32_t timeoutMs) {
    (void)timeoutMs; // currently not used
    while (mapButton() == KEY_NONE) {
        delay(10);
    }
}

#endif
