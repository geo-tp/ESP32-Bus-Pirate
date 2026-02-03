#ifdef DEVICE_CARDPUTER

#include "CardputerInput.h"


char CardputerInput::handler() {
    while(true) {
        // Update keyboard state
        M5Cardputer.update();
        
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        
        // Special arrows for scrolling, can be long pressed
        if (!status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) {
            delay(50); // debounce
            return CARDPUTER_SPECIAL_ARROW_UP;
        }
        if (!status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) {
            delay(50); // debounce
            return CARDPUTER_SPECIAL_ARROW_DOWN;
        }
    
        if (!M5Cardputer.Keyboard.isChange()) {
            continue;
        }
    
        if (!M5Cardputer.Keyboard.isPressed()) {
            continue;
        }
        
        if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) return KEY_ARROW_UP;
        if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) return KEY_ARROW_DOWN;
        if (status.enter) return KEY_OK;
        if (status.del) return KEY_DEL;

        for (auto c : status.word) {
            // Issue with %, the only key that requires 2 inputs to display
            if (c == '%') {
                return '5'; 
            }
            return c; // retourner le premier char saisi
        }
    
        delay(10); // debounce
        return KEY_NONE;
    }
}

void CardputerInput::waitPress(uint32_t timeoutMs) {
    uint32_t start = millis();

    for (;;) {
        char c = readChar();
        if (c != KEY_NONE) {
            return;
        }

        if (timeoutMs > 0 && (millis() - start) >= timeoutMs) {
            return;
        }

        delay(5);
    }
}

char CardputerInput::readChar() {
    M5Cardputer.update();
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    
    // Special arrows for scrolling, can be long pressed
    if (!status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) {
        delay(50); // debounce
        return CARDPUTER_SPECIAL_ARROW_UP;
    }
    if (!status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) {
        delay(50); // debounce
        return CARDPUTER_SPECIAL_ARROW_DOWN;
    }

    // State change
    if (!M5Cardputer.Keyboard.isChange()) return KEY_NONE;
    if (!M5Cardputer.Keyboard.isPressed()) return KEY_NONE;

    // ; . keys inversed
    if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) return KEY_ARROW_UP;
    if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) return KEY_ARROW_DOWN;

    if (status.enter) return KEY_OK;
    if (status.del) return KEY_DEL;
    if (status.tab) return KEY_TAB_CUSTOM;

    for (auto c : status.word) {
        delay(5); // debounce
        
        // Issue with %, the only key that requires 2 inputs to display
        if (c == '%') {
            return '5'; 
        }
        return c; // return first char pressed
    }

    return KEY_NONE;
}

#endif
