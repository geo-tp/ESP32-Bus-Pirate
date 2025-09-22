#include "HorizontalSelector.h"

HorizontalSelector::HorizontalSelector(IDeviceView& display, IInput& input)
    : display(display), input(input) {}

int HorizontalSelector::select(
    const std::string& title, 
    const std::vector<std::string>& options, 
    const std::string& description1, 
    const std::string& description2) {

    int currentIndex = 0;
    int lastIndex = -1;

    display.topBar(title, false, false);

    while (true) {
        if (lastIndex != currentIndex) {
            display.horizontalSelection(options, currentIndex, description1, description2);
            lastIndex = currentIndex;
        }

        char key = input.handler();

        switch (key) {
            case KEY_ARROW_LEFT:
                currentIndex = (currentIndex > 0) ? currentIndex - 1 : options.size() - 1;
                break;
            case KEY_ARROW_RIGHT:
                currentIndex = (currentIndex < options.size() - 1) ? currentIndex + 1 : 0;
                break;
            case KEY_OK:
                return currentIndex;
            default:
                break;
        }
    }
}

int HorizontalSelector::selectHeadless() {
    int selected = 1;  // default

    // 3 sec to press the button
    const unsigned long timeout = millis() + 3000;
    while (millis() < timeout) {
        char c = input.readChar();
        if (c == KEY_OK) {
            selected = 0; 
            break;
        }
        delay(10);
    }

    return selected;
}