#pragma once

#ifdef DEVICE_CARDPUTER

#include <map>
#include <M5Cardputer.h>
#include "Interfaces/IInput.h"
#include "InputKeys.h"

class CardputerInput : public IInput {
public:
    char handler() override;
    void waitPress(uint32_t timeoutMs) override;
    char readChar() override;
};


#endif