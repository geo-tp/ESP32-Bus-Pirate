#include "SubGhzController.h"

/*
Entry point for commands
*/
void SubGhzController::handleCommand(const TerminalCommand& cmd) {
    const std::string root = cmd.getRoot();

    if (root == "sniff")             handleSniff(cmd);
    else if (root == "scan")         handleScan(cmd);
    else if (root == "sweep")        handleSweep();
    else if (root == "setfrequency") handleSetFrequency();
    else if (root == "setfreq")      handleSetFrequency();
    else if (root == "replay")       handleReplay(cmd);
    else if (root == "jam")          handleJam(cmd);
    else if (root == "bruteforce")   handleBruteforce();
    else if (root == "decode")       handleDecode(cmd);
    else if (root == "trace")        handleTrace();
    else if (root == "listen")       handleListen();
    else if (root == "load")         handleLoad();
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
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
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

    terminalView.println("\nSUBGHZ Sniff: Stopped by user. " + std::to_string(count) + " frames\n");
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
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
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
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
        return;
    }

    // Start sniffer
    if (!subGhzService.startRawSniffer(state.getSubGhzGdoPin())) {
        terminalView.println("Failed to start raw sniffer.");
        return;
    }

    terminalView.println("SUBGHZ Replay: Recording up to 64 frames @ " +
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
        confirm = userInputManager.readYesNo("SUBGHZ Replay: Done. Run again?", true);
    }

    terminalView.println(okAll ? "SUBGHZ Replay Done without error.\n" : "SUBGHZ Replay: Done with errors.\n");
    subGhzService.stopTxBitBang(); // ensure stopped
}

/*
Jam
*/
void SubGhzController::handleJam(const TerminalCommand&) {
    auto confirm = userInputManager.readYesNo("\nSUBGHZ Jam: This will transmit random signals. Continue?", false);
    if (!confirm) return;

    confirm = userInputManager.readYesNo("Jam multiple frequencies?", true);
    if (confirm) {
        handleBandJam();
        return;
    }

    float f = userInputManager.readValidatedFloat("Enter frequency to jam (MHz)", state.getSubGhzFrequency(), 0.0f, 1000.0f);
    
    // Apply TX profile with freq
    if (!subGhzService.applyRawSendProfile(f)) {
        terminalView.println("Failed to apply TX profile at " + argTransformer.toFixed2(f) + " MHz");
        return;
    }

    terminalView.println("SUBGHZ Jam: In progress @ " + argTransformer.toFixed2(f) + " MHz... Press [ENTER] to stop.");
    delay(5); // let display the message
    
    auto gdo = state.getSubGhzGdoPin();
    subGhzService.startTxBitBang();
    while (true) {
        // Stop
        auto c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        // Jam
        pinService.setHigh(gdo);
        delayMicroseconds(30);
        pinService.setLow(gdo);
    }

    subGhzService.stopTxBitBang();
    terminalView.println("SUBGHZ Jam: Stopped by user.\n");
}

/*
Band Jam
*/
void SubGhzController::handleBandJam() {
    // Select band
    auto bands = subGhzService.getSupportedBand();
    std::vector<std::string> bandsWithCustom = bands;

    int bandIndex = userInputManager.readValidatedChoiceIndex(
        "Select frequency band:", bandsWithCustom, 0
    );

    // Set band and get freqs
    subGhzService.setScanBand(bands[bandIndex]);
    auto freqs = subGhzService.getSupportedFreq(bands[bandIndex]);

    // Prompt for dwell and gap
    int dwellMs = userInputManager.readValidatedInt("Hold time per frequency (ms):", 5, 1, 10000);
    int gapUs   = userInputManager.readValidatedInt("Gap between bursts (us):",      1, 0, 500000);

    const uint8_t gdo = state.getSubGhzGdoPin();

    terminalView.println("\nSUBGHZ Jam: In progress... Press [ENTER] to stop.");
    terminalView.println("Band: " + bands[bandIndex] + ", FreqCount=" + std::to_string(freqs.size()) +
                         ", Hold=" + std::to_string(dwellMs) + " ms\n");

    bool stop = false;
    subGhzService.startTxBitBang();

    while (!stop) {
        for (size_t i = 0; i < freqs.size() && !stop; ++i) {
            // Cancel
            char c = terminalInput.readChar();
            if (c == '\n' || c == '\r') { stop = true; break; }

            float f = freqs[i];
            // Apply TX profile with freq
            if (!subGhzService.applyRawSendProfile(f)) {
                terminalView.println("Failed to apply TX profile at " + argTransformer.toFixed2(f) + " MHz");
                subGhzService.stopTxBitBang();
                return;
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

                // Jam
                for (int i = 0; i < 64; i++) {
                    pinService.setHigh(gdo);
                    delayMicroseconds(30);
                    pinService.setLow(gdo);
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

    subGhzService.stopTxBitBang();
    subGhzService.tune(state.getSubGhzFrequency());
    terminalView.println("SUBGHZ Jam: Stopped by user.\n");
}

/*
Decode
*/
void SubGhzController::handleDecode(const TerminalCommand&) {
    float f = state.getSubGhzFrequency();

    // Sniff profil
    if (!subGhzService.applySniffProfile(f)) {
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
        return;
    }

    // Start sniffing
    if (!subGhzService.startRawSniffer(state.getSubGhzGdoPin())) {
        terminalView.println("\nFailed to start raw sniffer.\n");
        return;
    }

    terminalView.println("SUBGHZ Decode: Listening @ " +
                         std::to_string(f) + " MHz... Press [ENTER] to stop.\n");

    std::vector<rmt_item32_t> frame;
    bool stop = false;
    while (!stop) {
        // Cancel
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') { stop = true; break; }

        // Read and analyze frame
        frame = subGhzService.readRawFrame();
        if (!frame.empty()) {
            auto result = subGhzAnalyzeManager.analyzeFrame(frame);
            terminalView.println(result);
        }
    }

    subGhzService.stopRawSniffer();
    terminalView.println("SUBGHZ Decode: Stopped by user.\n");
}

/*
Trace
*/
void SubGhzController::handleTrace() {
    const float f = state.getSubGhzFrequency();

    // Sniff profile
    if (!subGhzService.applySniffProfile(f)) {
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
        return;
    }

    terminalView.println("\nSUBGHZ Trace: Displaying signals @ " + argTransformer.toFixed2(f) + " MHz on the ESP32 screen... Press [ENTER] to stop.\n");

    const uint8_t gdo = state.getSubGhzGdoPin();
    uint32_t sampleUs = 900;

    // Update device view
    deviceView.clear();
    deviceView.topBar("SubGHz Trace", false, false);

    std::vector<uint8_t> buffer;
    buffer.reserve(240); // screen width default

    unsigned long lastPoll = millis();

    // Samples loop
    while (true) {
        // Cancel
        if (millis() - lastPoll >= 10) {
            lastPoll = millis();
            const char c = terminalInput.readChar();
            if (c == '\n' || c == '\r') {
                terminalView.println("SUBGHZ Trace: Stopped by user.\n");
                break;
            }
        }

        // Samples
        buffer.push_back(pinService.read(gdo));

        // Render
        if (buffer.size() >= 240) {
            buffer.resize(240);
            deviceView.drawLogicTrace(gdo, buffer);
            buffer.clear();
        }

        delayMicroseconds(sampleUs);
    }
}

/*
Sweep
*/
void SubGhzController::handleSweep() {
    // Select band
    auto bands = subGhzService.getSupportedBand();
    int bandIndex = userInputManager.readValidatedChoiceIndex("Select frequency band:", bands, 0);
    subGhzService.setScanBand(bands[bandIndex]);
    std::vector<float> freqs = subGhzService.getSupportedFreq(bands[bandIndex]);
    if (freqs.empty()) {
        terminalView.println("SUBGHZ Sweep: No frequencies in selected band.");
        return;
    }

    // Params
    int dwellMs  = userInputManager.readValidatedInt("Hold time per frequency (ms)", 300, 20, 5000);
    int windowMs = userInputManager.readValidatedInt("Window granularity (ms)",      20,  5,  200);
    int thrDbm   = userInputManager.readValidatedInt("RSSI threshold (dBm)",        -67, -120, 0);

    // Scan profile
    if (!subGhzService.applyScanProfile(4.8f, 200.0f, 2 /* OOK */, true)) {
        terminalView.println("SUBGHZ: Not configured. Run 'config' first.");
        return;
    }

    terminalView.println("\nSUBGHZ Sweep:" + bands[bandIndex] +
                         " | hold=" + std::to_string(dwellMs) + " ms" +
                         " | window=" + std::to_string(windowMs) + " ms" +
                         " | thr=" + std::to_string(thrDbm) + " dBm... Press [ENTER] to stop.\n");
    
    // Sweep and analyze each frequency
    bool run = true;
    while (run) {
        for (size_t i = 0; i < freqs.size() && run; ++i) {
            // Cancel
            char c = terminalInput.readChar();
            if (c == '\n' || c == '\r') { run = false; break; }

            // Tune
            float f = freqs[i];
            subGhzService.tune(f);

            // Analyze
            auto line = subGhzAnalyzeManager.analyzeFrequencyActivity(dwellMs, windowMs, thrDbm,
                // measure(windowMs)
                [&](int winMs){ return subGhzService.measurePeakRssi(winMs); },
                // shouldAbort()
                [&](){
                    char cc = terminalInput.readChar();
                    if (cc == '\n' || cc == '\r') { run = false; return true; }
                    return false;
                },
                /*neighborLeftConf=*/0.f,
                /*neighborRightConf=*/0.f
            );

            // Log result
            terminalView.println("  " + argTransformer.toFixed2(f) + " MHz  " + line);
        }
    }

    terminalView.println("\nSUBGHZ Sweep: Stopped by user.\n");
}

/*
Load
*/
void SubGhzController::handleLoad() {
    if (!littleFsService.mounted()) {
        littleFsService.begin();
    }

    // List .sub files
    auto files = littleFsService.listFiles(/*root*/ "/", ".sub");
    if (files.empty()) {
        terminalView.println("SUBGHZ: No .sub files found in LittleFS root ('/').\n");
        return;
    }

    // Select file
    terminalView.println("\n=== '.sub' files in LittleFS ===");
    int fileIndex = userInputManager.readValidatedChoiceIndex("File number", files, 0);
    std::string filename = files[fileIndex];

    // Check size
    int MAX_FILE_SIZE = 32 * 1024; // 32 KB
    auto fileSize = littleFsService.getFileSize("/" + filename);
    if (fileSize == 0 || fileSize > MAX_FILE_SIZE) {
        terminalView.println("\nSUBGHZ: File size invalid (>32KB): " + filename + " (" + std::to_string(fileSize) + " bytes)\n");
        return;
    }
    
    terminalView.println("\nSUBGHZ: Loading file '" + filename + "' (" + std::to_string(fileSize) + " bytes)...");

    // Load file
    std::string fileContent;
    fileContent.reserve(fileSize + 1);
    if (!littleFsService.readAll("/" + filename, fileContent)) {
        terminalView.println("\nSUBGHZ: Failed to read " + filename + "\n");
        return;
    }

    // Validate
    if (!subGhzTransformer.isValidSubGhzFile(fileContent)) {
        terminalView.println("\nSUBGHZ: Invalid .sub file: " + filename + "\n");
        return;
    }

    // Parse
    auto frames = subGhzTransformer.transformFromFileFormat(fileContent);
    if (frames.empty()) {
        terminalView.println("\nSUBGHZ: Failed to parse .sub file: " + filename + "\n");
        return; 
    }

    // Get frame names
    auto summaries = subGhzTransformer.extractSummaries(frames);
    summaries.push_back("Exit"); // for exit option

    while (true) {
        // Select frame to send
        terminalView.println("\n=== Commands in file '" + filename + "' ===");
        uint16_t idx = userInputManager.readValidatedChoiceIndex("Frame number", summaries, 0);

        // Exit
        if (idx == summaries.size() - 1) {
            terminalView.println("Exiting command send...\n");
            break;
        }

        // Send
        terminalView.println("\n Sending frame #" + std::to_string(idx + 1) + "...");
        const auto& cmd = frames[idx];
        if (subGhzService.send(cmd)) {
            terminalView.println(" ✅ " + summaries[idx]);
        } else {
            terminalView.println(" ❌ Send failed for frame #" + std::to_string(idx + 1));
        }
    }
}

/*
Listen
*/
void SubGhzController::handleListen() {
    // Params
    float mhz = userInputManager.readValidatedFloat("Enter frequency (MHz):", state.getSubGhzFrequency(), 0.0f, 1000.0f);
    int   rssiGate = userInputManager.readValidatedInt("RSSI gate (dBm):", -65, -127, 0);
    state.setSubGhzFrequency(mhz);

    // Radio init
    if (!subGhzService.applySniffProfile(mhz)) {
        terminalView.println("SUBGHZ: Not detected. Run 'config' first.");
        return;
    }
    subGhzService.tune(mhz);

    // I2S init with configured pins
    i2sService.configureOutput(
        state.getI2sBclkPin(), state.getI2sLrckPin(), state.getI2sDataPin(),
        state.getI2sSampleRate(), state.getI2sBitsPerSample()
    );

    terminalView.println("\nSUBGHZ: RSSI to Audio mapping @ " + argTransformer.toFixed2(mhz) +
                         " MHz... Press [ENTER] to stop.\n");

    terminalView.println("[INFO] Using I2S configured pins for audio output.\n");

    // Mapping params
    const uint16_t fMin = 800;      // Hz for weak signals
    const uint16_t fMax = 12000;    // Hz for strong signals
    const uint16_t toneMs = 1;      // short sound
    const uint16_t refreshUs = 200; // 5 kHz update rate

    while (true) {
        // Stop on ENTER
        char c = terminalInput.readChar();
        if (c == '\n' || c == '\r') break;

        // Get RSSI
        int rssi = subGhzService.measurePeakRssi(1);
        if (rssi >= rssiGate) {
            float norm = (rssi + 120.0f) / 120.0f; // normalize dbm
            if (norm < 0) norm = 0;
            if (norm > 1) norm = 1;
            uint16_t freqHz = fMin + (uint16_t)(norm * (fMax - fMin));

            // Play the tone on I2S configured pins
            i2sService.playTone(state.getI2sSampleRate(), freqHz, toneMs);
        }

        delayMicroseconds(refreshUs);
    }
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
        terminalView.println("\n ❌ Failed to detect CC1101 module. Check wiring.\n");
    } else {

        if (state.getTerminalMode() != TerminalTypeEnum::Standalone) {
            terminalView.println("\n[INFO] For SubGHz features, use **USB Serial** connection.");
            terminalView.println("       It offers lower latency and more reliable logging.");
            terminalView.println("       The WiFi Web UI can introduce delays and miss pulses.\n");
        }
        
        // Apply settings
        subGhzService.tune(freq);
        subGhzService.applyScanProfile();
        terminalView.println(" ✅ CC1101 module detected and configured with default frequency.");
        terminalView.println(" Use 'setfrequency' or 'scan' to change the frequency.\n");
        configured = true;
    }
}

void SubGhzController::handleBruteforce() {
    // Adapted From Bruce: https://github.com/pr3y/Bruce
    // Bruteforce attack for 12 Bit SubGHz protocols

    auto gdo0 = state.getSubGhzGdoPin();
    std::vector<std::string> protocolNames(
        std::begin(subghz_protocol_list),
        std::end(subghz_protocol_list)
    );

    // Prompt for protocol index
    auto protocolIndex = userInputManager.readValidatedChoiceIndex("\nSelect protocol to brute force:", protocolNames, 0);
    auto bruteProtocol = protocolNames[protocolIndex];

    // Freq
    float mhz = userInputManager.readValidatedFloat("Enter frequency (MHz):", 433.92f, 0.0f, 1000.0f);
    state.setSubGhzFrequency(mhz);

    // Profil TX + sender
    if (!subGhzService.applyRawSendProfile(mhz)) {
        terminalView.println("Failed to apply TX profile.");
        return;
    }

    c_rf_protocol protocol;
    int bits = 0;

    // Protocol selection
    if (bruteProtocol == " Nice 12 Bit") {
        protocol = protocol_nice_flo();
        bits = 12;
    } else if (bruteProtocol == " Came 12 Bit") {
        protocol = protocol_came();
        bits = 12;
    } else if (bruteProtocol == " Ansonic 12 Bit") {
        protocol = protocol_ansonic();
        bits = 12;
    } else if (bruteProtocol == " Holtek 12 Bit") {
        protocol = protocol_holtek();
        bits = 12;
    } else if (bruteProtocol == " Linear 12 Bit") {
        protocol = protocol_linear();
        bits = 12;
    } else if (bruteProtocol == " Chamberlain 12 Bit") {
        protocol = protocol_chamberlain();
        bits = 12;
    } else {
        terminalView.println("SUBGHZ BruteForce: Protocol not implemented yet.");
        return;
    }

    // Repeat
    auto bruteRepeats = userInputManager.readValidatedUint8("Enter number of repeats per code:", 1);
    subGhzService.startTxBitBang();

    // Send all codes
    terminalView.println("SUBGHZ BruteForce: Sending all codes for" + bruteProtocol + "... Press [ENTER] to stop.\n");
    auto count = 0;
    for (int i = 0; i < (1 << bits); ++i) {
        for (int r = 0; r < bruteRepeats; ++r) {
            for (const auto &pulse : protocol.pilot_period) { 
                subGhzService.sendRawPulse(gdo0, pulse); 
            }

            for (int j = bits - 1; j >= 0; --j) {
                bool bit = (i >> j) & 1;
                const std::vector<int> &timings = protocol.transposition_table[bit ? '1' : '0'];
                for (auto duration : timings) { 
                    subGhzService.sendRawPulse(gdo0, duration); 
                }
            }

            for (const auto &pulse : protocol.stop_bit) { subGhzService.sendRawPulse(gdo0, pulse); }
            
        }

        // Display progress
        count++;
        if (count % 100 == 0) {
            terminalView.println(" " + bruteProtocol + " @ " + argTransformer.toFixed2(mhz) + " MHz sent " + std::to_string(count) + " codes.");
        }

        // Cancel
        char cc = terminalInput.readChar();
        if (cc == '\n' || cc == '\r') {
            terminalView.println("\nSUBGHZ BruteForce: Stopped by user.\n");
            subGhzService.stopTxBitBang();
            return;
        }
    }

    subGhzService.stopTxBitBang();
    terminalView.println("\nSUBGHZ Bruteforce: Done.\n");
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
    terminalView.println("  sweep");
    terminalView.println("  sniff");
    terminalView.println("  decode");
    terminalView.println("  replay");
    terminalView.println("  jam");
    terminalView.println("  bruteforce");
    terminalView.println("  trace");
    terminalView.println("  load");
    terminalView.println("  listen");
    terminalView.println("  setfrequency");
    terminalView.println("  config");
}
