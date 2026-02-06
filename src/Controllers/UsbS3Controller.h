#pragma once

#ifndef DEVICE_M5STICK

#include <sstream>
#include <Arduino.h>
#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "Models/TerminalCommand.h"
#include "Models/ByteCode.h"
#include "States/GlobalState.h"
#include "Transformers/ArgTransformer.h"
#include "Managers/UserInputManager.h"
#include "Interfaces/IUsbController.h"
#include "Interfaces/IUsbService.h"
#include "Shells/HelpShell.h"

class UsbS3Controller: public IUsbController {
public:
    // Constructor
    UsbS3Controller(
        ITerminalView& terminalView, 
        IInput& terminalInput, 
        IInput& deviceInput,
        IUsbService& usbService, 
        ArgTransformer& argTransformer, 
        UserInputManager& userInputManager,
        HelpShell& helpShell
    );

    // Entry point for handle raw terminal command
    void handleCommand(const TerminalCommand& cmd);

    // Ensure USB is configured before any action
    void ensureConfigured();

private:
    // Simulate USB mass storage mount
    void handleUsbStick();

    // Move the HID mouse cursor
    void handleMouseMove(const TerminalCommand& cmd);

    // Perform HID mouse click
    void handleMouseClick();

    // Moove the mouse cursor until ENTER is pressed
    void handleMouseJiggle(const TerminalCommand& cmd);

    // High-level mouse handler
    void handleMouse(const TerminalCommand& cmd);

    // Emulate gamepad events
    void handleGamepad(const TerminalCommand& cmd);

    // Handle keyboard events
    void handleKeyboard(const TerminalCommand& cmd);

    // Bridge keyboard input to USB HID
    void handleKeyboardBridge();

    // Send keyboard input via HID
    void handleKeyboardSend(const TerminalCommand& cmd);

    // Reset all USB states
    void handleReset();

    // Configure USB behavior
    void handleConfig();

    // Display help for USB commands
    void handleHelp();

    bool configured = false;
    ITerminalView& terminalView;
    IInput& terminalInput;
    IInput& deviceInput;
    IUsbService& usbService;
    ArgTransformer& argTransformer;
    GlobalState& state = GlobalState::getInstance();
    UserInputManager& userInputManager;
    HelpShell& helpShell;
};

#endif