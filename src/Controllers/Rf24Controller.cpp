#include "Controllers/Rf24Controller.h"

/*
Entry point for rf24 commands
*/
void Rf24Controller::handleCommand(const TerminalCommand& cmd) {
    const std::string& root = cmd.getRoot();

    if (root == "help") handleHelp();
    else if (root == "config")     handleConfig();
    else if (root == "sniff")      handleSniff();
    else if (root == "scan")       handleScan();
    else if (root == "sweep")      handleSweep();
    else if (root == "jam")        handleJam();
    else if (root == "setchannel") handleSetChannel();
    else                           handleHelp();
}

/*
Ensure NRF24 is configured before use
*/
void Rf24Controller::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
        return;
    }

    uint8_t ce = state.getRf24CePin();
    uint8_t csn = state.getRf24CsnPin();
    uint8_t sck = state.getRf24SckPin();
    uint8_t miso = state.getRf24MisoPin();
    uint8_t mosi = state.getRf24MosiPin();
    rf24Service.configure(csn, ce, sck, miso, mosi);
}

/*
Sniff
*/
void Rf24Controller::handleSniff() {
    terminalView.println("RF24: Sniffing Channel " + std::to_string(rf24Service.getChannel()) + "... Press [ENTER] to stop.\n");

    rf24Service.initRx();
    rf24Service.startListening();
    while (true) {
        auto c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        if (rf24Service.available()) {
            uint8_t tmp[32] = {0};
            if (rf24Service.receive(tmp, sizeof(tmp))) {

                // Display hex and ASCII
                for (int row = 0; row < 32; row += 16) {
                    // Hex
                    for (int i = 0; i < 16; i++) {
                        terminalView.print(argTransformer.toHex(tmp[row + i], 2) + " ");
                    }

                    terminalView.print(" | ");

                    // ASCII
                    for (int i = 0; i < 16; i++) {
                        char c = (tmp[row + i] >= 32 && tmp[row + i] <= 126) ? tmp[row + i] : '.';
                        terminalView.print(std::string(1, c));
                    }

                    terminalView.println("");
                }
            }
        }
    }
    rf24Service.stopListening();
    rf24Service.flushRx();
    terminalView.println("\nRF24: Sniff stopped by user.\n");
}

/*
Scan
*/
void Rf24Controller::handleScan() {
    uint32_t dwell = 128; // µs 
    uint8_t levelHold[126 + 1] = {0}; // 0..200
    uint8_t threshold = userInputManager.readValidatedUint8("High threshold (10..200)?", 20, 10, 200);
    int bestCh = -1;
    uint8_t bestVal = 0;
    const uint8_t DECAY = 6;  // decay per sweep
    
    terminalView.println("RF24: Scanning channel 0 to 125... Press [ENTER] to stop.\n");
    
    rf24Service.initRx();
    while (true) {
        // Cancel
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        // Sweep
        for (uint8_t ch = 0; ch <= 125; ++ch) {
            rf24Service.setChannel(ch);
            rf24Service.startListening();
            delayMicroseconds(dwell);
            rf24Service.stopListening();

            // Measure
            uint8_t instant = 0;
            if (rf24Service.testRpd())          instant = 200;
            else if (rf24Service.testCarrier()) instant = 120;

            // Decrease hold level
            if (levelHold[ch] > DECAY) levelHold[ch] -= DECAY;
            else levelHold[ch] = 0;

            // Get max
            if (instant > levelHold[ch]) levelHold[ch] = instant;

            if (levelHold[ch] >= threshold) {
                // Save best
                if (levelHold[ch] > bestVal) {
                    bestVal = levelHold[ch];
                    bestCh = ch;
                }

                // Log
                terminalView.println(
                    "  Detect: Channel=" + std::to_string(ch) +
                    "  Freq=" + std::to_string(2400 + ch) + " MHz" +
                    "  Level=" + std::to_string(levelHold[ch])
                );
            }
        }
    }

    // Result
    terminalView.println("");
    if (bestCh >= 0) {
        // Log best
        terminalView.println(
            "Best channel: ch=" + std::to_string(bestCh) +
            "  f=" + std::to_string(2400 + bestCh) + " MHz" +
            "  peakLevel=" + std::to_string(bestVal)
        );

        // Ask to apply to config
        if (userInputManager.readYesNo("Save best channel to config?", true)) {
            rf24Service.setChannel((uint8_t)bestCh);
            terminalView.println("RF24: Channel set to " + std::to_string(bestCh) + ".\n");
        } else {
            terminalView.println("RF24: Channel not changed.\n");
        }
    } else {
        terminalView.println("\nRF24: No activity detected.\n");
    }
}

/*
Jam
*/
void Rf24Controller::handleJam() {
    auto confirm = userInputManager.readYesNo("RF24 Jam: This will transmit random signals. Continue?", false);
    if (!confirm) return;

    // List of group names
    std::vector<std::string> groupNames;
    groupNames.reserve(RF24_GROUP_COUNT);
    for (size_t i = 0; i < RF24_GROUP_COUNT; ++i) groupNames.emplace_back(RF24_GROUPS[i].name);
    groupNames.emplace_back("Full range (0..125)");

    // Select group
    int choice = userInputManager.readValidatedChoiceIndex("Select band to jam:", groupNames, 0);

    // Get group
    const Rf24ChannelGroup* group = nullptr;
    if (choice >= (int)RF24_GROUP_COUNT) {
        // Full range
        uint8_t fullRangeChannels[126];
        for (uint8_t i = 0; i <= 125; ++i) fullRangeChannels[i] = i;
        static const Rf24ChannelGroup fullRangeGroup = {
            " Full range",
            fullRangeChannels,
            126
        };
        group = &fullRangeGroup;
    } else {
        group = &RF24_GROUPS[choice];
    }
    
    terminalView.println("\nRF24: Sweeping noise on channels... Press [ENTER] to stop.");

    rf24Service.stopListening();
    rf24Service.setDataRate(RF24_2MBPS);
    rf24Service.powerUp();
    rf24Service.setPowerMax();

    // Jam loop
    bool run = true;
    while (run) {
        for (size_t i = 0; i < group->count; ++i) {
            // Cancel
            int k = terminalInput.readChar();
            if (k == '\n' || k == '\r') { run = false; break; }

            // Sweep
            rf24Service.setChannel(group->channels[i]);
        }
    }

    rf24Service.flushTx();
    rf24Service.powerDown();
    terminalView.println("RF24: Jam stopped by user.\n");
}

/*
Sweep
*/
void Rf24Controller::handleSweep() {
    // Params
    int dwellMs  = userInputManager.readValidatedInt("Hold time per channel (ms)", 10, 10, 1000);
    int samples  = userInputManager.readValidatedInt("Samples per channel", 80, 1, 100);
    int thrPct   = userInputManager.readValidatedInt("Activity threshold (%)", 1, 0, 100);

    terminalView.println("\nRF24 Sweep: channels 0–125"
                         " | hold=" + std::to_string(dwellMs) + " ms"
                         " | samples=" + std::to_string(samples) +
                         " | thr=" + std::to_string(thrPct) + "%... Press [ENTER] to stop.\n");

    bool run = true;
    while (run) {
        for (int ch = 0; ch <= 125 && run; ++ch) {
            // Cancel
            char c = terminalInput.readChar();
            if (c == '\n' || c == '\r') { run = false; break; }

            rf24Service.setChannel(static_cast<uint8_t>(ch));

            // Analyze activity
            int hits = 0;
            for (int s = 0; s < samples; ++s) {
                rf24Service.startListening();
                delayMicroseconds((dwellMs * 1000) / samples);
                rf24Service.stopListening();
                if (rf24Service.testRpd()) {
                    hits += 2;
                }

                if  (rf24Service.testCarrier()) {
                    hits++;
                }
            }

            int activityPct = (hits * 100) / samples;
            if (activityPct > 100) activityPct = 100;

            // Log if above threshold
            if (activityPct >= thrPct) {
                terminalView.println(
                    "  Channel " + std::to_string(ch) + " (" +
                    std::to_string(2400 + ch) + " MHz)" +
                    "  activity=" + std::to_string(activityPct) + "%"
                );
            }
        }
    }

    rf24Service.flushRx();
    terminalView.println("\nRF24 Sweep: Stopped by user.\n");
}

/*
Set Channel
*/
void Rf24Controller::handleSetChannel() {
    uint8_t ch = userInputManager.readValidatedUint8("Channel (0..125)?", 76, 0, 125);
    rf24Service.setChannel(ch);
    terminalView.println("RF24: Channel set to " + std::to_string(ch) + ".");
}

/*
NRF24 Configuration
*/
void Rf24Controller::handleConfig() {
    uint8_t csn = userInputManager.readValidatedInt("NRF24 CSN pin?", state.getRf24CsnPin());
    uint8_t sck  = userInputManager.readValidatedInt("NRF24 SCK pin?", state.getRf24SckPin());
    uint8_t miso = userInputManager.readValidatedInt("NRF24 MISO pin?", state.getRf24MisoPin());
    uint8_t mosi = userInputManager.readValidatedInt("NRF24 MOSI pin?", state.getRf24MosiPin());
    uint8_t ce  = userInputManager.readValidatedInt("NRF24 CE pin?", state.getRf24CePin());
    state.setRf24CsnPin(csn);
    state.setRf24SckPin(sck);
    state.setRf24MisoPin(miso);
    state.setRf24MosiPin(mosi);
    state.setRf24CePin(ce);

    bool ok = rf24Service.configure(csn, ce, sck, miso, mosi);

    configured = true; // consider configured even if not detected to avoid re-asking
    terminalView.println(ok ? "\n ✅ NRF24 detected and configured.\n" : "\n ❌ NRF24 not detected. Check wiring.\n");
}

/*
Help
*/
void Rf24Controller::handleHelp() {
    terminalView.println("RF24 commands:");
    terminalView.println("  scan");
    terminalView.println("  sniff");
    terminalView.println("  sweep");
    terminalView.println("  jam");
    terminalView.println("  setchannel");
    terminalView.println("  config");
}