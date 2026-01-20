#include "PinService.h"
#

void PinService::setInput(uint8_t pin) {
    pinMode(pin, INPUT);
    pullState[pin] = NOPULL;
}

void PinService::setInputPullup(uint8_t pin) {
    pinMode(pin, INPUT_PULLUP);
    pullState[pin] = PULL_UP;
}

void PinService::setInputPullDown(uint8_t pin) {
    pinMode(pin, INPUT_PULLDOWN);
    pullState[pin] = PULL_DOWN;
}

void PinService::setOutput(uint8_t pin) {
    pinMode(pin, OUTPUT);
}

void PinService::setHigh(uint8_t pin) {
    setOutput(pin);  // force OUTPUT
    digitalWrite(pin, HIGH);
}

void PinService::setLow(uint8_t pin) {
    setOutput(pin);  // force OUTPUT
    digitalWrite(pin, LOW);
}

bool PinService::read(uint8_t pin) {
    return gpio_get_level((gpio_num_t)pin);
}

void PinService::togglePullup(uint8_t pin) {
    pullType enabled = pullState[pin];

    if (enabled == PULL_UP) {
        setInput(pin);
    } else {
        setInputPullup(pin);
    }
}

void PinService::togglePullDown(uint8_t pin) {
    pullType enabled = pullState[pin];

    if (enabled == PULL_DOWN) {
        setInput(pin);
    } else {
        setInputPullDown(pin);
    }
}

int PinService::readAnalog(uint8_t pin) {
    pinMode(pin, INPUT); 
    return analogRead(pin);
}

bool PinService::setupPwm(uint8_t pin, uint32_t freq, uint8_t dutyPercent) {
    if (dutyPercent > 100) dutyPercent = 100;

    int channel = pin % 8;

    // Find the highest compatible resolution
    int resolution = -1;
    for (int bits = 14; bits >= 1; --bits) {
        if (isPwmFeasible(freq, bits)) {
            if (ledcSetup(channel, freq, bits)) {  // ok
                resolution = bits;
                break;
            }
        }
    }
    if (resolution < 0) return false;

    ledcAttachPin(pin, channel);

    uint32_t dutyMax = (1UL << resolution) - 1;
    uint32_t dutyVal = ((uint32_t)dutyPercent * dutyMax) / 100U;
    ledcWrite(channel, dutyVal);
    return true;
}

void PinService::setServoAngle(uint8_t pin, uint8_t angle) {
  const int channel = 0;
  const int freq = 50; // Hz
  const int resolution = 14;  // max stable

  // setup et attach
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(pin, channel);

  // period and duty
  const uint32_t periodUs = 1000000UL / freq;    // 20000 us
  const uint32_t dutyMax  = (1UL << resolution) - 1;

  // map angle
  uint32_t pulseUs = map(angle, 0, 180, 1000, 2000);
  uint32_t dutyVal = (pulseUs * dutyMax) / periodUs;

  ledcWrite(channel, dutyVal);
}

bool PinService::isPwmFeasible(uint32_t freq, uint8_t resolutionBits) {
    if (resolutionBits < 1 || resolutionBits > 14) return false;

    const uint32_t baseClkHz     = 80000000UL;   // 80 MHz
    const uint32_t maxDivParam   = 0x3FFFF;      // limite
    if (freq == 0) return false;

    // div_param = baseClk / (freq * 2^resolution)
    uint64_t denom = (uint64_t)freq * (1ULL << resolutionBits);
    if (denom == 0) return false;

    uint32_t divParam = (uint32_t)(baseClkHz / denom);
    return (divParam >= 1 && divParam <= maxDivParam);
}

PinService::pullType PinService::getPullType(uint8_t pin){
    return(pullState[pin]);
}

std::vector<uint8_t> PinService::getConfiguredPullPins() {
    std::vector<uint8_t> pins;
    pins.reserve(pullState.size());

    for (const auto& entry : pullState) {
        pins.push_back(entry.first);
    }

    return pins;
}
