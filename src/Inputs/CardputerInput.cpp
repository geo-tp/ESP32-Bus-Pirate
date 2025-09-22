#ifdef DEVICE_CARDPUTER

#include "CardputerInput.h"


char CardputerInput::handler() {
    while(true) {
        // Update keyboard state
        M5Cardputer.update();
        
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) {
            delay(50);
            return CARDPUTER_SPECIAL_ARROW_UP;
        }    
        if (M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) {
            delay(50);
            return CARDPUTER_SPECIAL_ARROW_DOWN;
        }
    
        if (!M5Cardputer.Keyboard.isChange()) {
            continue;
        }
    
        if (!M5Cardputer.Keyboard.isPressed()) {
            continue;
        }
    
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    
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

void CardputerInput::waitPress() {
  while(true){
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
      if (M5Cardputer.Keyboard.isPressed()) {
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        return;
      }
    }
    delay(10);
  }
}

char CardputerInput::readChar() {
    M5Cardputer.update();
    
    // Special arrows for scrolling, can be long pressed
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) {
        delay(50); // debounce
        return CARDPUTER_SPECIAL_ARROW_UP;
    }
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) {
        delay(50); // debounce
        return CARDPUTER_SPECIAL_ARROW_DOWN;
    }

    // State change
    if (!M5Cardputer.Keyboard.isChange()) return KEY_NONE;
    if (!M5Cardputer.Keyboard.isPressed()) return KEY_NONE;
    
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    
    if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_UP)) return KEY_ARROW_UP;
    if (status.fn && M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_DOWN)) return KEY_ARROW_DOWN;
    if (status.enter) return KEY_OK;
    if (status.del) return KEY_DEL;

    for (auto c : status.word) {
        delay(50); // debounce
        
        // Issue with %, the only key that requires 2 inputs to display
        if (c == '%') {
            return '5'; 
        }
        return c; // retourner le premier char saisi
    }

    return KEY_NONE;
}

#endif
