#ifdef DEVICE_CARDPUTER


#include "CardputerInput.h"

char CardputerInput::handler() {
    while(true) {
        // Update keyboard state
        M5Cardputer.update();
        
        if (!M5Cardputer.Keyboard.isChange()) {
            continue;
        }
    
        if (!M5Cardputer.Keyboard.isPressed()) {
            continue;
        }
    
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    
        if (status.enter) { // go to next menu
            return KEY_OK;
        }
        if (status.del) { 
            return KEY_DEL;
        }
        
        if(M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_LEFT)) { // go back to previous menu
            return KEY_ARROW_LEFT;
        }
    
        if(M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_RIGHT)) { // go to next menu
            return KEY_ARROW_RIGHT;
        }
    
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

    if (!M5Cardputer.Keyboard.isChange()) {
        return KEY_NONE;
    }

    if (!M5Cardputer.Keyboard.isPressed()) {
        return KEY_NONE;
    }

    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

    if (status.enter) {
        return KEY_OK;
    }
    if (status.del) { 
        return KEY_DEL;
    }
    
    if(M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_LEFT)) { // go back to previous menu
        return KEY_ARROW_LEFT;
    }

    if(M5Cardputer.Keyboard.isKeyPressed(KEY_ARROW_RIGHT)) { // go to next menu
        return KEY_ARROW_RIGHT;
    }

    for (auto c : status.word) {
        // Issue with %, the only key that requires 2 inputs to display
        if (c == '%') {
            return '5'; 
        }
        return c; // retourner le premier char saisi
    }
}

#endif
