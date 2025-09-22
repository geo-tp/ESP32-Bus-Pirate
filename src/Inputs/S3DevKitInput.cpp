#include "S3DevKitInput.h"
#include "InputKeys.h"

#define BOOT_BUTTON_PIN 0

S3DevKitInput::S3DevKitInput() {
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
}

char S3DevKitInput::mapButton() {
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        return KEY_OK;
    }
    return KEY_NONE;
}

char S3DevKitInput::readChar() {
    return mapButton();
}

char S3DevKitInput::handler() {
    char c = KEY_NONE;
    while ((c = mapButton()) == KEY_NONE) {
        delay(5);
    }
    return c;
}

void S3DevKitInput::waitPress() {
    while (mapButton() == KEY_NONE) {
        delay(5);
    }
}