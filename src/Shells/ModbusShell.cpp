#include "ModbusShell.h"

ModbusShell::ModbusShell(
    ITerminalView& view,
    IInput& input,
    ArgTransformer& argTransformer,
    UserInputManager& userInputManager,
    ModbusService& modbusService
)
: terminalView(view)
, terminalInput(input)
, argTransformer(argTransformer)
, userInputManager(userInputManager)
, modbusService(modbusService) 
{
}

void ModbusShell::run(const std::string& host, uint16_t port) {
    installModbusCallbacks();

    hostShown = host;
    portShown = port ? port : 502;

    if (!modbusService.setTarget(hostShown, portShown)) {
        terminalView.println("MODBUS: DNS/Target error.\n");
        return;
    }

    modbusService.begin(reqTimeoutMs, idleTimeoutMs, 4);
    terminalView.println("");

    bool start = true;
    while (start) {
        printHeader();
        auto choice = userInputManager.readValidatedChoiceIndex("Select a modbus action", actions, 0);

        switch (choice) {
            case 0: cmdReadHolding();         break;
            case 1: cmdWriteHolding();        break;
            case 2: cmdReadInputRegisters();  break;
            case 3: cmdReadCoils();           break;
            case 4: cmdWriteCoils();          break;
            case 5: cmdReadDiscreteInputs();  break;
            case 6: cmdMonitorHolding();      break;
            case 7: cmdSetUnit();             break;
            case 8: cmdConnect();             break;
            case 9: terminalView.println("Modbus shell closed.\n"); start = false; break;
        }
    }
    modbusService.clearCallbacks();
    modbusService.setTarget("", 0);
}

// ===== Actions =====

void ModbusShell::cmdConnect() {
    terminalView.print("Host or IP: ");
    std::string h = userInputManager.getLine();
    uint16_t p = userInputManager.readValidatedUint32("Port", 502);

    terminalView.println("Connecting to " + h + ":" + std::to_string(p) + " ...");
    if (!modbusService.setTarget(h, p)) {
        terminalView.println("DNS/Target error.\n");
        return;
    }
    hostShown = h;
    portShown = p;

    modbusService.begin(reqTimeoutMs, idleTimeoutMs, 4);

    terminalView.println(" ✅ OK.\n");
}

void ModbusShell::cmdSetUnit() {
    unitId  = userInputManager.readValidatedUint8("Unit ID (1-247)", unitId, 1, 247);
    terminalView.println(" ✅ OK.\n");
}

void ModbusShell::cmdReadHolding() {
    // Address and quantity
    uint16_t addr = userInputManager.readValidatedUint32("Start addr", 0);
    uint16_t qty = userInputManager.readValidatedUint32("Quantity", 8);

    // Read
    terminalView.println("Reading...\n");
    clearReply();
    Error e = modbusService.readHolding(unitId, addr, qty);
    if (e != SUCCESS) {
        ModbusError me(e);
        terminalView.println(std::string("Request error: ") + (const char*)me + "\n");
        return;
    }

    if (!waitReply(reqTimeoutMs + 1000)) {
        terminalView.println("Timeout.\n");
        return;
    }

    if (_reply.ok) {
        printRegs(_reply.regs, addr);
        terminalView.println("");
    } else if (_reply.fc & 0x80) {
        char buf[64]; snprintf(buf, sizeof(buf), "Exception 0x%02X\n", _reply.exception);
        terminalView.println(buf);
    } else {
        terminalView.println((_reply.error + "\n").c_str());
    }
}

void ModbusShell::cmdWriteHolding() {
    // Values
    int addr = userInputManager.readValidatedUint32("Start addr", 0);
    std::string line = userInputManager.readValidatedHexString("Enter 16 bit value(s) ", 0, true, 4);
    auto vals = argTransformer.parseHexList16(line);

    // Confirm
    bool confirm = userInputManager.readYesNo(
        "Write " + std::to_string(vals.size()) + " regs at addr " + std::to_string(addr) + " ?", false);
    if (!confirm) { terminalView.println("Aborted.\n"); return; }

    // Write
    terminalView.println("Writing...\n");
    clearReply();
    Error e = SUCCESS;
    if (vals.size() == 1) {
        e = modbusService.writeHoldingSingle(unitId, (uint16_t)addr, vals[0]);   // FC06
    } else {
        e = modbusService.writeHoldingMultiple(unitId, (uint16_t)addr, vals);    // FC16
    }
    if (e != SUCCESS) {
        ModbusError me(e);
        terminalView.println(std::string("Request error: ") + (const char*)me + "\n");
        return;
    }

    if (!waitReply(reqTimeoutMs + 1000)) {
        terminalView.println("Timeout.\n"); 
        return;
    }

    if (_reply.ok && (_reply.fc == 0x10 || _reply.fc == 0x06)) {
        terminalView.println(" ✅ OK.\n");
    } else if (_reply.fc & 0x80) {
        char buf[64]; snprintf(buf, sizeof(buf), "Exception 0x%02X\n", _reply.exception);
        terminalView.println(buf);
    } else {
        terminalView.println((_reply.error + "\n").c_str());
    }
}

void ModbusShell::cmdMonitorHolding() {
    uint16_t addr = userInputManager.readValidatedUint32("Start addr", 0);
    uint16_t qty = userInputManager.readValidatedUint32("Quantity", 8);
    uint32_t period = userInputManager.readValidatedUint32("Period ms", 500);

    terminalView.println("Monitoring... Press [ENTER] to stop.\n");

    std::vector<uint16_t> last;
    bool stop = false;
    while (true) {
        char c = terminalInput.readChar();
        if (c == '\r' || c == '\n') { terminalView.println("Stopped.\n"); break; }

        clearReply();
        modbusService.readHolding(unitId, addr, qty);

        // Wait for response while handling user input
        const uint32_t t0 = millis();
        while ((millis() - t0) < reqTimeoutMs) {
            if (_reply.ready) break;

            // enter press to stop
            char k = terminalInput.readChar();
            if (k == '\r' || k == '\n') { 
                stop = true; 
                break; 
            }
            delay(5);
        }

        if (_reply.ok && _reply.regs != last) {
            printRegs(_reply.regs, addr);
            terminalView.println("");
            last = _reply.regs;
        }
    }
}

void ModbusShell::cmdReadInputRegisters() {
  uint16_t addr = userInputManager.readValidatedUint32("Start addr (Input Reg)", 0);
  uint16_t qty  = userInputManager.readValidatedUint32("Quantity (max 125)", 1);

  terminalView.println("Reading (FC04)...\n");
  clearReply();
  Error e = modbusService.readInputRegisters(unitId, addr, qty);
  if (e != SUCCESS) { ModbusError me(e); terminalView.println(std::string("Request error: ") + (const char*)me + "\n"); return; }

  if (!waitReply(reqTimeoutMs + 1000)) { terminalView.println("Timeout.\n"); return; }

  if (_reply.ok) { printRegs(_reply.regs, (uint16_t)addr); terminalView.println(""); }
  else if (_reply.fc & 0x80) { char buf[64]; snprintf(buf, sizeof(buf), "Exception 0x%02X\n", _reply.exception); terminalView.println(buf); }
  else { terminalView.println((_reply.error + "\n").c_str()); }
}

void ModbusShell::cmdReadCoils() {
    uint16_t addr = userInputManager.readValidatedUint32("Start addr (Coils)", 0);
    uint16_t qty  = userInputManager.readValidatedUint32("Quantity", 8);

    terminalView.println("Reading (FC01)...\n");
    clearReply();
    Error e = modbusService.readCoils(unitId, addr, qty);
    if (e != SUCCESS) { ModbusError me(e); terminalView.println(std::string("Request error: ") + (const char*)me + "\n"); return; }

    if (!waitReply(reqTimeoutMs + 1000)) { terminalView.println("Timeout.\n"); return; }
    if (!_reply.ok) {
        // display error
        terminalView.println((_reply.error + "\n").c_str());
        if (!_reply.raw.empty()) {
            std::string hex;
            for (auto b : _reply.raw) { char x[4]; snprintf(x, sizeof(x), "%02X ", b); hex += x; }
            terminalView.println("Raw resp: " + hex + "\n");
        }
        return;
    }

    const size_t need = (qty + 7) / 8;
    if (_reply.coilBytes.size() < need) {
        char msg[128];
        snprintf(msg, sizeof(msg),
                "Short response: need %u byte(s) for %u coils, got %u.\n",
                (unsigned)need, (unsigned)qty, (unsigned)_reply.coilBytes.size());
        terminalView.println(msg);
        // hexdump
        if (!_reply.raw.empty()) {
            std::string hex;
            for (auto b : _reply.raw) { char x[4]; snprintf(x, sizeof(x), "%02X ", b); hex += x; }
            terminalView.println("Raw resp: " + hex + "\n");
        }
        return;
    }
}

void ModbusShell::cmdWriteCoils() {
    uint16_t addr = userInputManager.readValidatedUint32("Start addr (Coils)", 0);

    terminalView.println("Enter bits (0/1) either as '1 0 1 1' or '1011'");
    terminalView.print("> ");
    std::string line = userInputManager.getLine();
    if (line.empty()) { terminalView.println("Cancelled.\n"); return; }

    // Parse bits
    auto bits = argTransformer.parse01List(line);
    if (bits.empty()) { terminalView.println("No valid bit found.\n"); return; }

    // Confirm
    if (!userInputManager.readYesNo("Write " + std::to_string(bits.size()) +
                                    " coil(s) at addr " + std::to_string(addr) + " ?", false)) {
        terminalView.println("Aborted.\n"); return;
    }

    terminalView.println("Writing coils...\n");
    clearReply();
    Error e = SUCCESS;

    if (bits.size() == 1) {
        // FC05
        e = modbusService.writeSingleCoil(unitId, addr, bits[0] != 0);
    } else {
        // FC0F
        auto packed = argTransformer.packLsbFirst(bits);
        e = modbusService.writeMultipleCoils(unitId, addr, packed, bits.size());
    }
    if (e != SUCCESS) { ModbusError me(e); terminalView.println(std::string("Request error: ") + (const char*)me + "\n"); return; }
    if (!waitReply(reqTimeoutMs + 1000)) { terminalView.println("Timeout.\n"); return; }
    if (!_reply.ok) { terminalView.println((_reply.error + "\n").c_str()); return; }

    terminalView.println(" ✅ OK.\n");
    clearReply();
}

void ModbusShell::cmdReadDiscreteInputs() {
  uint16_t addr = userInputManager.readValidatedUint32("Start addr (Discrete Inputs)", 0);
  uint16_t qty  = userInputManager.readValidatedUint32("Quantity", 8);

  terminalView.println("Reading (FC02)...\n");
  clearReply();
  Error e = modbusService.readDiscreteInputs(unitId, addr, qty);
  if (e != SUCCESS) { ModbusError me(e); terminalView.println(std::string("Request error: ") + (const char*)me + "\n"); return; }

  if (!waitReply(reqTimeoutMs + 1000)) { terminalView.println("Timeout.\n"); return; }

  if (_reply.ok) { printCoils(_reply.coilBytes, addr, qty); terminalView.println(""); }
  else if (_reply.fc & 0x80) { char buf[64]; snprintf(buf, sizeof(buf), "Exception 0x%02X\n", _reply.exception); terminalView.println(buf); }
  else { terminalView.println((_reply.error + "\n").c_str()); }
}

// === Helpers ===

void ModbusShell::installModbusCallbacks() {
  modbusService.setOnReply([this](const ModbusService::Reply& r, uint32_t token){
    _reply.fc       = r.fc;
    _reply.ok       = r.ok;
    _reply.exception= r.exception;
    _reply.error    = r.error;
    _reply.regs     = r.regs;
    _reply.ready    = true;
  });
}

bool ModbusShell::waitReply(uint32_t timeoutMs) {
    const uint32_t deadline = millis() + timeoutMs;
    while (millis() < deadline) {
        if (_reply.ready) return true;
        delay(10);
    }
    return _reply.ready;
}

void ModbusShell::printHeader() {
    terminalView.println("=== Modbus Shell ===");
    terminalView.println(
        "Target: " + (hostShown.empty() ? std::string("<not set>") : hostShown) + ":" + std::to_string(portShown) +
        " | Unit: " + std::to_string(unitId)
    );
    terminalView.println("");
}

void ModbusShell::printRegs(const std::vector<uint16_t>& regs, uint16_t baseAddr) {
    for (size_t i=0;i<regs.size();++i) {
        const uint16_t v = regs[i];
        char line[96];
        snprintf(line, sizeof(line), "R[%u] (addr %u) = 0x%04X  (%5u)",
                 (unsigned)i, (unsigned)(baseAddr + i), v, v);
        terminalView.println(line);
    }
}

void ModbusShell::printCoils(const std::vector<uint8_t>& coilBytes, uint16_t baseAddr, uint16_t qty) {

    for (uint16_t i = 0; i < qty; ++i) {
        const size_t iByte = i >> 3;        // i / 8
        const uint8_t iBit = i & 0x07;      // i % 8
        uint8_t bit = 0;
        if (iByte < coilBytes.size()) {
            bit = (coilBytes[iByte] >> iBit) & 0x01; // LSB-first
        }
        char line[64];
        snprintf(line, sizeof(line), "C[%u] (addr %u) = %u",
                    (unsigned)i, (unsigned)(baseAddr + i), (unsigned)bit);
        terminalView.println(line);
    }
}