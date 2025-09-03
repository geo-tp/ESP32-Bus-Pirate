#include "SubGhzController.h"

/*
Entry point for commands
*/
void SubGhzController::handleCommand(const TerminalCommand& cmd) {
    const std::string root = cmd.getRoot();

    if (root == "sniff")             handleSniff(cmd);
    else if (root == "scan")         handleScan(cmd);
    else if (root == "setfrequency") handleSetFrequency();
    else if (root == "setfreq")      handleSetFrequency();
    else if (root == "replay")       handleReplay(cmd);
    else if (root == "jam")          handleJam(cmd);
    else if (root == "config")       handleConfig();
    else                             handleHelp();
}

/*
Sniff for signals
*/
void SubGhzController::handleSniff(const TerminalCommand&) {
    float f = state.getSubGhzFrequency();
    uint32_t count = 0;

    if (!subGhzService.applySniffProfile(f)) {
        terminalView.println("SUBGHZ: Not configured. Run 'config' first.");
        return;
    }
    
    terminalView.println("SUBGHZ Sniff: Frequency @ " + std::to_string(f) + " MHz... Press [ENTER] to stop\n");
    
    subGhzService.startRawSniffer(state.getSubGhzGdoPin());
    while (true) {
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') {
            break;
        }

        auto raw = subGhzService.readRawPulses();
        for (auto &line : raw) {
            count++;
            terminalView.println(line);
        }
    }
    subGhzService.stopRawSniffer();

    terminalView.println("\nSUBGHZ Sniff: Done. " + std::to_string(count) + " frames\n");
}

/*
Scan for frequencies
*/
void SubGhzController::handleScan(const TerminalCommand& cmd) {
    const auto& args = cmd.getArgs();
    auto bands = subGhzService.getSupportedBand();
    
    // Band selection
    auto bandIndex = userInputManager.readValidatedChoiceIndex("Select frequency band:", bands, 0);
    subGhzService.setScanBand(bands[bandIndex]);
    std::vector<float> freqs = subGhzService.getSupportedFreq(bands[bandIndex]);

    // RSSI threshold / hold time selection
    int holdMs = userInputManager.readValidatedInt("Enter hold time per frequency (ms):", 4, 1, 5000);
    int rssiThr = userInputManager.readValidatedInt("Enter RSSI threshold detection (dBm):", -67, -127, 0);

    // Prepare the scan
    if (!subGhzService.applyScanProfile(4.8f, 200.0f, 2 /* OOK */, true)) {
        terminalView.println("SUBGHZ: Not configured. Run 'config' first.");
        return;
    }

    terminalView.println("SUBGHZ Scan: Started. Hold=" + std::to_string(holdMs) +
                         " ms, Thr=" + std::to_string(rssiThr) + " dBm.... Pres [ENTER] to stop.\n");

    std::vector<int>  best(freqs.size(), -127);
    std::vector<bool> wasAbove(freqs.size(), false);
    bool stopRequested = false;

    // Scanning
    while (!stopRequested) {
        for (size_t i = 0; i < freqs.size(); ++i) {
            // User enter press
            int c = terminalInput.readChar();
            if (c == '\n' || c == '\r') { stopRequested = true; break; }

            // Set the freq
            float f = freqs[i];
            subGhzService.tune(f);

            // Measure peak on freq for ms
            int peak = subGhzService.measurePeakRssi(holdMs);
            if (peak > best[i]) best[i] = peak;

            // Log spike if any
            if (peak >= rssiThr && !wasAbove[i]) {
                terminalView.println(" [PEAK] f=" + argTransformer.toFixed2(f) + " MHz  RSSI=" + std::to_string(peak) + " dBm");
                wasAbove[i] = true;
            } else if (peak < rssiThr - 2) {
                wasAbove[i] = false;
            }
        }
    }

    // Summary
    std::vector<size_t> idx(freqs.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b){ return best[a] > best[b]; });
    terminalView.println("\n [SCAN] Best peaks:");
    const size_t n = std::min<size_t>(5, idx.size());
    for (size_t k = 0; k < n; ++k) {
        size_t i = idx[k];
        terminalView.println("   " + argTransformer.toFixed2(freqs[i]) + " MHz  RSSI=" + std::to_string(best[i]) + " dBm");
    }

    // Ask to save best frequency if above threshold
    if (!idx.empty() && best[idx[0]] > -120) {
        auto confirm = userInputManager.readYesNo(" Save tuning to best frequency (" + argTransformer.toFixed2(freqs[idx[0]]) + " MHz)?", true);
        if (!confirm) return;
        subGhzService.tune(freqs[idx[0]]);
        terminalView.println(" [FREQ] Saving to config: " + argTransformer.toFixed2(freqs[idx[0]]) + " MHz\n");
        state.setSubGhzFrequency(freqs[idx[0]]);
    } else {
        subGhzService.tune(state.getSubGhzFrequency());
    }
}

/*
Set frequency
*/
void SubGhzController::handleSetFrequency() {
    terminalView.println("");
    terminalView.println("Select SubGHz Frequency:");

    // Band selection
    auto bands = subGhzService.getSupportedBand();
    std::vector<std::string> displayBands = bands;
    if (!displayBands.empty()) displayBands[0] = " Custom";
    int bandIndex = userInputManager.readValidatedChoiceIndex("Frequency band", displayBands, 0);

    // Custom frequency
    if (bandIndex == 0) {
        float mhz = userInputManager.readValidatedFloat("Enter custom frequency (MHz):", state.getSubGhzFrequency(), 0.0f, 1000.0f);
        state.setSubGhzFrequency(mhz);
        subGhzService.tune(mhz);
        terminalView.println("SUBGHZ: Frequency changed to " + argTransformer.toFixed2(mhz) + " MHz\n");
        return;
    }

    // Choose in predefined frequencies
    subGhzService.setScanBand(bands[bandIndex]);
    std::vector<float> freqs = subGhzService.getSupportedFreq(bands[bandIndex]);
    int index = userInputManager.readValidatedChoiceIndex("Frequencies", freqs, 0);
    float selected = freqs[index];

    // Apply
    state.setSubGhzFrequency(selected);
    subGhzService.tune(selected);

    terminalView.println("SUBGHZ: Frequency changed to " + argTransformer.toFixed2(selected) + " MHz\n");
}

/*
Replay
*/
void SubGhzController::handleReplay(const TerminalCommand&) {
    const float f = state.getSubGhzFrequency();

    // Set profile to read frames
    if (!subGhzService.applySniffProfile(f)) {
        terminalView.println("SUBGHZ: Not configured. Run 'config' first.");
        return;
    }

    // Start sniffer
    if (!subGhzService.startRawSniffer(state.getSubGhzGdoPin())) {
        terminalView.println("Failed to start raw sniffer.");
        return;
    }

    terminalView.println("SUBGHZ Replay: recording up to 64 frames @ " +
                         std::to_string(f) + " MHz... Press [ENTER] to stop.\n");

    std::vector<std::vector<rmt_item32_t>> frames;
    frames.reserve(64);

    bool stop = false;
    while (!stop && frames.size() < 64) {
        // Cancel
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') { stop = true; break; }

        // Read frame
        auto items = subGhzService.readRawFrame();
        if (!items.empty()) {
            frames.push_back(std::move(items));
            terminalView.println(" [Frame " + std::to_string(frames.size()) + " captured]");
        }
    }

    subGhzService.stopRawSniffer();
    terminalView.println("\nSUBGHZ Replay: Captured " + std::to_string(frames.size()) + " frame(s).");

    if (frames.empty()) {
        terminalView.println("Nothing to replay.\n");
        return;
    }

    // Profil TX + sender
    if (!subGhzService.applyRawSendProfile(f)) {
        terminalView.println("Failed to apply TX profile.");
        return;
    }

    // Start sending frames
    terminalView.print("SUBGHZ Replay: In progress...");
    bool okAll = true;
    auto gdo = state.getSubGhzGdoPin();
    bool confirm = true;
    while (confirm) {
        for (size_t i = 0; i < frames.size(); ++i) {
            if (!subGhzService.sendRawFrame(gdo, frames[i])) {
                terminalView.println("SUBGHZ Replay: Failed at frame " + std::to_string(i + 1));
                okAll = false;
                break;
            }
            delay(3);
        }
        confirm = userInputManager.readYesNo("SUBGHZ Replay: Done. Run again?", false);
    }

    terminalView.println(okAll ? "SUBGHZ Replay Done without error.\n" : "SUBGHZ Replay: Done with errors.\n");
}

void SubGhzController::handleJam(const TerminalCommand&) {
    // Select band
    auto bands = subGhzService.getSupportedBand();
    int bandIndex = userInputManager.readValidatedChoiceIndex("Select frequency band:", bands, 0);
    subGhzService.setScanBand(bands[bandIndex]);
    std::vector<float> freqs = subGhzService.getSupportedFreq(bands[bandIndex]);

    // Prompt for dwell and gap
    int dwellMs = userInputManager.readValidatedInt("Hold time per frequency (ms):", 5, 1, 10000);
    int gapUs   = userInputManager.readValidatedInt("Gap between bursts (us):",      100, 0, 500000);

    const uint8_t gdo = state.getSubGhzGdoPin();

    terminalView.println("\nSUBGHZ Jam: In progress... Press [ENTER] to stop.");
    terminalView.println("Band: " + bands[bandIndex] + ", FreqCount=" + std::to_string(freqs.size()) +
                         ", Hold=" + std::to_string(dwellMs) + " ms\n");

    bool stop = false;
    while (!stop) {
        for (size_t i = 0; i < freqs.size() && !stop; ++i) {
            // Cancel
            char c = terminalInput.readChar();
            if (c == '\n' || c == '\r') { stop = true; break; }

            float f = freqs[i];
            // Apply TX profile with freq
            if (!subGhzService.applyRawSendProfile(f)) {
                terminalView.println("Failed to apply TX profile at " + argTransformer.toFixed2(f) + " MHz");
                continue;
            }

            unsigned long t0 = millis();
            while (!stop && (millis() - t0 < static_cast<unsigned long>(dwellMs))) {
                char c2 = terminalInput.readChar();
                if (c2 == '\n' || c2 == '\r') { stop = true; break; }

                // Random burst
                if (!subGhzService.sendRandomBurst(gdo)) {
                    terminalView.println("Send failed at " + argTransformer.toFixed2(f) + " MHz");
                    break;
                }

                // Gap
                int remain = gapUs;
                while (remain > 0 && !stop) {
                    char c3 = terminalInput.readChar();
                    if (c3 == '\n' || c3 == '\r') { stop = true; break; }
                    int chunk = std::min(remain, 1000);
                    delayMicroseconds(chunk);
                    remain -= chunk;
                }
            }
        }
    }

    subGhzService.tune(state.getSubGhzFrequency());
    terminalView.println("SUBGHZ Jam: Stopped by user.\n");
}

/*
Config CC1101
*/
void SubGhzController::handleConfig() {
    terminalView.println("\nSubGHz Configuration:");

    const auto& forbidden = state.getProtectedPins();    

    // CC1101 pins
    uint8_t sck  = userInputManager.readValidatedPinNumber("CC1101 SCK pin",  state.getSubGhzSckPin(),  forbidden);
    state.setSubGhzSckPin(sck);

    uint8_t miso = userInputManager.readValidatedPinNumber("CC1101 MISO pin", state.getSubGhzMisoPin(), forbidden);
    state.setSubGhzMisoPin(miso);

    uint8_t mosi = userInputManager.readValidatedPinNumber("CC1101 MOSI pin", state.getSubGhzMosiPin(), forbidden);
    state.setSubGhzMosiPin(mosi);

    uint8_t ss   = userInputManager.readValidatedPinNumber("CC1101 SS/CS pin", state.getSubGhzCsPin(), forbidden);
    state.setSubGhzCsPin(ss);

    uint8_t gdo0 = userInputManager.readValidatedPinNumber("CC1101 GDO0 pin", state.getSubGhzGdoPin(), forbidden);
    state.setSubGhzGdoPin(gdo0);

    float freq = state.getSubGhzFrequency(); 

    // Configure
    auto isConfigured = subGhzService.configure(
        deviceView.getScreenSpiInstance(),
        sck, miso, mosi, ss,
        gdo0, freq
    );

    // CC1101 feedback
    if (!isConfigured) {
        terminalView.println("\nFailed to detect CC1101 module. Check wiring.\n");
    } else {
        terminalView.println("\n[INFO] For SubGHz features, use **USB Serial** connection.");
        terminalView.println("       It offers lower latency and more reliable logging.");
        terminalView.println("       The WiFi Web UI can introduce delays and miss pulses.\n");
        
        // Apply settings
        subGhzService.tune(freq);
        subGhzService.applyScanProfile();
        terminalView.println("CC1101 module detected and configured with default frequency.");
        terminalView.println("Use 'setfrequency' or 'scan' to change the frequency.\n");
        configured = true;
    }
    
}

/*
Ensure SubGHz is configured
*/
void SubGhzController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    };

    uint8_t cs = state.getSubGhzCsPin();
    uint8_t gdo0 = state.getSubGhzGdoPin();
    uint8_t sck = state.getSubGhzSckPin();
    uint8_t miso = state.getSubGhzMisoPin();
    uint8_t mosi = state.getSubGhzMosiPin();
    float freq = state.getSubGhzFrequency();

    subGhzService.configure(deviceView.getScreenSpiInstance(), sck, miso, mosi, cs, gdo0, freq);
}

/*
Help
*/
void SubGhzController::handleHelp() {
    terminalView.println("SubGHz commands:");
    terminalView.println("  scan");
    terminalView.println("  sniff");
    terminalView.println("  setfrequency");
    terminalView.println("  config");
}
