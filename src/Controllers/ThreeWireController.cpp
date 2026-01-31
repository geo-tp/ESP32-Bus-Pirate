#include "ThreeWireController.h"

/*
Constructor
*/
ThreeWireController::ThreeWireController(
    ITerminalView& terminalView,
    IInput& terminalInput,
    UserInputManager& userInputManager,
    ThreeWireService& threeWireService,
    ArgTransformer& argTransformer,
    ThreeWireEepromShell& threeWireEepromShell,
    HelpShell& helpShell)
  : terminalView(terminalView),
    terminalInput(terminalInput),
    userInputManager(userInputManager),
    threeWireService(threeWireService),
    argTransformer(argTransformer),
    threeWireEepromShell(threeWireEepromShell),
    helpShell(helpShell) {}

/*
Entry point for command
*/
void ThreeWireController::handleCommand(const TerminalCommand& cmd) {
    if      (cmd.getRoot() == "config")  handleConfig();
    else if (cmd.getRoot() == "eeprom")  handleEeprom(cmd);
    else                                 handleHelp();
}

/*
Entry point for instructions
*/
void ThreeWireController::handleInstruction(const std::vector<ByteCode>& bytecodes) {
    terminalView.println("Instruction handling not yet implemented");
}

/*
EEPROM
*/
void ThreeWireController::handleEeprom(const TerminalCommand& cmd) {
    threeWireEepromShell.run();
}

/*
Help
*/
void ThreeWireController::handleHelp() {
    terminalView.println("\nUnknown command. Available 3WIRE commands:");
    helpShell.run(state.getCurrentMode(), false);
}

/*
Config
*/
void ThreeWireController::handleConfig() {
    terminalView.println("3WIRE Configuration:");
    
    // Pins
    const auto& forbidden = state.getProtectedPins();
    uint8_t cs = userInputManager.readValidatedPinNumber("CS pin", state.getThreeWireCsPin(), forbidden);
    state.setThreeWireCsPin(cs);
    uint8_t sk = userInputManager.readValidatedPinNumber("SK pin", state.getThreeWireSkPin(), forbidden);
    state.setThreeWireSkPin(sk);
    uint8_t di = userInputManager.readValidatedPinNumber("DI pin", state.getThreeWireDiPin(), forbidden);
    state.setThreeWireDiPin(di);
    uint8_t doPin = userInputManager.readValidatedPinNumber("DO pin", state.getThreeWireDoPin(), forbidden);
    state.setThreeWireDoPin(doPin);

    // Configure the service, default value for eeprom
    threeWireService.configure(cs, sk, di, doPin, 46, state.isThreeWireOrg8());
    terminalView.println("3WIRE configured.\n");
    configured = true;
}

/*
Ensure configured
*/
void ThreeWireController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    }

    // Pins could have been used elsewhere, reconfigure the service
    auto cs = state.getThreeWireCsPin();
    auto sk = state.getThreeWireSkPin();
    auto di = state.getThreeWireDiPin();
    auto doPin = state.getThreeWireDoPin();
    auto modelId = state.getThreeWireEepromModelIndex(); 
    auto org8 = state.isThreeWireOrg8();
    threeWireService.configure(cs, sk, di, doPin, modelId, org8);
}