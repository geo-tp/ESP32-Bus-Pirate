#include "I2sController.h"

/*
Constructor
*/
I2sController::I2sController(ITerminalView& terminalView, IInput& terminalInput,
                             I2sService& i2sService, ArgTransformer& argTransformer,
                             UserInputManager& userInputManager, HelpShell& helpShell)
    : terminalView(terminalView), terminalInput(terminalInput),
      i2sService(i2sService), argTransformer(argTransformer),
      userInputManager(userInputManager), helpShell(helpShell) {}

void I2sController::handleCommand(const TerminalCommand& cmd) {
    if (cmd.getRoot() == "config") {
        handleConfig();
    } else if (cmd.getRoot() == "play") {
        handlePlay(cmd);
    } else if (cmd.getRoot() == "record") {
        handleRecord(cmd);
    } else if (cmd.getRoot() == "test") {
        handleTest(cmd);
    } else if (cmd.getRoot() == "reset") {
        handleReset();
    } else {
        handleHelp();
    }
}


/*
Play
*/
void I2sController::handlePlay(const TerminalCommand& cmd) {
    auto args = argTransformer.splitArgs(cmd.getArgs());

    if (!argTransformer.isValidNumber(cmd.getSubcommand())) {
        terminalView.println("Usage: play <frequency> [durationMs]");
        return;
    }

    uint16_t freq = argTransformer.parseHexOrDec32(cmd.getSubcommand());

    // Infinite until ENTER press
    if (args.empty()) {
        terminalView.println("\nI2S Play: Tone @ " + std::to_string(freq) + " Hz (Press [ENTER] to stop)...\n");

        i2sService.playToneInterruptible(state.getI2sSampleRate(), freq, 0xFFFF, [&]() -> bool {
            char ch = terminalInput.readChar();
            return ch == '\n' || ch == '\r';
        });
    
    // Duration or until ENTER press
    } else if (args.size() == 1 && argTransformer.isValidNumber(args[0])) {
        uint16_t duration = argTransformer.parseHexOrDec32(args[0]);

        terminalView.println("\nI2S Play: Tone @ " + std::to_string(freq) + " Hz for " + std::to_string(duration) + " ms (or press [ENTER] to stop early)...\n");

        i2sService.playToneInterruptible(state.getI2sSampleRate(), freq, duration, [&]() -> bool {
            char ch = terminalInput.readChar();
            return ch == '\n' || ch == '\r';
        });

    } else {
        terminalView.println("Usage: play <frequency> [durationMs]");
        return;
    }

    terminalView.println("I2S Play: Done.");
}

/*
Record
*/
void I2sController::handleRecord(const TerminalCommand& cmd) {
    terminalView.println("I2S Record: In progress... Press [Enter] to stop.\n");

    // Configure input
    i2sService.configureInput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    constexpr size_t batchSize = 2048;
    constexpr size_t groupCount = 16;
    std::vector<int16_t> buffer(batchSize);

    int16_t dynamicMax = 5000; // initial value

    while (true) {
        size_t samplesRead = i2sService.recordSamples(buffer.data(), batchSize);
        size_t samplesPerGroup = samplesRead / groupCount;

        // Find peak on the batch
        int16_t batchPeak = 0;
        for (size_t i = 0; i < samplesRead; ++i) {
            int16_t val = abs(buffer[i]);
            if (val > batchPeak) batchPeak = val;
        }

        // Progressive update
        if (batchPeak > dynamicMax) dynamicMax = batchPeak;
        else dynamicMax = (dynamicMax * 9 + batchPeak) / 10;

        std::string line;
        for (size_t g = 0; g < groupCount; ++g) {
            int16_t peak = 0;
            for (size_t i = 0; i < samplesPerGroup; ++i) {
                size_t idx = g * samplesPerGroup + i;
                int16_t val = abs(buffer[idx]);
                if (val > peak) peak = val;
            }

            int level = (peak * 100) / (dynamicMax == 0 ? 1 : dynamicMax);
            if (level > 100) level = 100;
            if (level < 0) level = 0;

            char buf[6];
            sprintf(buf, "%03d ", level);
            line += buf;
        }

        terminalView.println(line);

        char ch = terminalInput.readChar();
        if (ch == '\n' || ch == '\r') break;
    }

    // Reconfigure output
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    terminalView.println("\nI2S Record: Stopped by user.\n");
}

/*
Test
*/
void I2sController::handleTest(const TerminalCommand& cmd) {
    std::string mode = cmd.getSubcommand();

    if (mode.empty()) {
        terminalView.println("Usage: test <speaker|mic>");
        return;
    }

    if (mode[0] == 's') {
        handleTestSpeaker();
    }
    else if (mode[0] == 'm') {
        handleTestMic();
    }
    else {
        terminalView.println("Usage: test <speaker|mic>");
    }
}

/*
Test Speaker
*/
void I2sController::handleTestSpeaker() {
    terminalView.println("I2S Speaker Test: Running full tests...\n");

    // Show pin config
    terminalView.println("Using pins:");
    terminalView.println("  BCLK : " + std::to_string(state.getI2sBclkPin()));
    terminalView.println("  LRCK : " + std::to_string(state.getI2sLrckPin()));
    terminalView.println("  DATA : " + std::to_string(state.getI2sDataPin()));
    terminalView.println("");

    auto rate = state.getI2sSampleRate();

    // Melody
    terminalView.println("  Melody...");
    const uint16_t melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C major
    for (uint16_t f : melody) {
        i2sService.playTone(rate, f, 200);
        delay(50);
    }
    delay(1000);

    // Frequency Sweep
    terminalView.println("  Frequency Sweep...");
    for (uint16_t f = 100; f <= 3000; f += 300) {
        i2sService.playTone(rate, f, 100);
    }
    delay(800);

    // Low Freq
    terminalView.println("  Low Frequency...");
    for (uint16_t f : {50, 100, 150, 200, 250, 300, 350, 400, 450, 500}) {
        i2sService.playTone(rate, f, 400);
        delay(100);
    }
    delay(800);

    // High Freq
    terminalView.println("  High Frequency...");
    for (uint16_t f = 10000; f <= 16000; f += 1000) {
        i2sService.playTone(rate, f, 300);
        delay(100);
    }
    delay(800);

    // Beep Pattern
    terminalView.println("  Beep Pattern (short/long)...");
    i2sService.playTone(rate, 800, 100);
    delay(100);
    i2sService.playTone(rate, 800, 100);
    delay(100);
    i2sService.playTone(rate, 800, 100);
    delay(100);
    i2sService.playTone(rate, 800, 400);
    delay(100);
    i2sService.playTone(rate, 800, 400);
    delay(100);
    i2sService.playTone(rate, 800, 400);
    delay(800);

    // Binary pattern (square wave)
    terminalView.println("  Binary tone pattern...");
    for (int i = 0; i < 15; ++i) {
        i2sService.playTone(rate, 1000, 50);
        delay(50);
    }
    delay(800);

    // Config for PCM playback
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        12000,
        16
    );
    
    // PCM Playback test
    terminalView.println("  PCM playback...");
    i2sService.playPcm(PcmSoundtestComplete, sizeof(PcmSoundtestComplete));
    
    // Restaure config
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );
    
    terminalView.println("\nI2S Speaker Test: Done.");
}

/*
Test Mic
*/
void I2sController::handleTestMic() {
    terminalView.println("\nI2S Micro: Analyzing input signal...\n");

    i2sService.configureInput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    // Show pin config
    terminalView.println("Using pins:");
    terminalView.println("  BCLK : " + std::to_string(state.getI2sBclkPin()));
    terminalView.println("  LRCK : " + std::to_string(state.getI2sLrckPin()));
    terminalView.println("  DATA : " + std::to_string(state.getI2sDataPin()));
    terminalView.println("");

    constexpr size_t sampleCount = 4096;
    std::vector<int16_t> buffer(sampleCount);
    size_t read = i2sService.recordSamples(buffer.data(), sampleCount);

    if (read == 0) {
        terminalView.println("\nI2S Micro: Failed to read from I2S microphone.");
        return;
    }

    int32_t sum = 0;
    int16_t minVal = INT16_MAX;
    int16_t maxVal = INT16_MIN;

    for (size_t i = 0; i < read; ++i) {
        int16_t val = buffer[i];
        sum += abs(val);
        if (val < minVal) minVal = val;
        if (val > maxVal) maxVal = val;
    }

    int avgAmplitude = sum / read;
    int peakToPeak = maxVal - minVal;

    // Score
    std::string verdict;
    if (avgAmplitude < 30 || peakToPeak < 60) {
        verdict = "Weak or no signal detected";
    } else if (avgAmplitude > 200 && peakToPeak > 400) {
        verdict = "Strong and valid signal";
    } else {
        verdict = "Low signal, maybe too quiet?";
    }

    // Display summay
    terminalView.println("Summary:");
    terminalView.println("  Avg amplitude : " + std::to_string(avgAmplitude));
    terminalView.println("  Min value     : " + std::to_string(minVal));
    terminalView.println("  Max value     : " + std::to_string(maxVal));
    terminalView.println("  Peak-to-peak  : " + std::to_string(peakToPeak));
    terminalView.println("  Verdict       : " + verdict);

    // Reconfig output
    i2sService.configureOutput(
        state.getI2sBclkPin(),
        state.getI2sLrckPin(),
        state.getI2sDataPin(),
        state.getI2sSampleRate(),
        state.getI2sBitsPerSample()
    );

    terminalView.println("\nI2S Micro: Done.");
}

/*
Config
*/
void I2sController::handleConfig() {
    terminalView.println("I2S Configuration:");

    const auto& forbidden = state.getProtectedPins();

    uint8_t bclk = userInputManager.readValidatedPinNumber("BCLK pin", state.getI2sBclkPin(), forbidden);
    state.setI2sBclkPin(bclk);

    uint8_t lrck = userInputManager.readValidatedPinNumber("LRCK/WS pin", state.getI2sLrckPin(), forbidden);
    state.setI2sLrckPin(lrck);

    uint8_t data = userInputManager.readValidatedPinNumber("DATA pin", state.getI2sDataPin(), forbidden);
    state.setI2sDataPin(data);

    uint32_t freq = userInputManager.readValidatedUint32("Sample rate (e.g. 44100)", state.getI2sSampleRate());
    state.setI2sSampleRate(freq);

    uint8_t bits = userInputManager.readValidatedUint8("Bits per sample (e.g. 16)", state.getI2sBitsPerSample());
    state.setI2sBitsPerSample(bits);

    #if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)
        terminalView.println("\n[WARNING] I2S may not work properly on T-Embed devices due to internal pin conflicts.");
        terminalView.println("          This includes shared SPI pins used for the display. Use with caution.");
        terminalView.println("          Freeze can happen on your device just after this message.\n");
    #endif

    i2sService.configureOutput(bclk, lrck, data, freq, bits);

    terminalView.println("I2S configured.\n");
}


/*
Help
*/
void I2sController::handleHelp() {
    terminalView.println("\nUnknown command. Available I2S commands:");
    helpShell.run(state.getCurrentMode(), false);
}


/*
Reset
*/
void I2sController::handleReset() {
    i2sService.end();

    // Config output
    i2sService.configureOutput(state.getI2sBclkPin(),
                         state.getI2sLrckPin(),
                         state.getI2sDataPin(),
                         state.getI2sSampleRate(),
                         state.getI2sBitsPerSample());

    terminalView.println("I2S Reset: TX (output) mode.");
}


/*
Ensure configuration
*/
void I2sController::ensureConfigured() {
    if (!configured) {
        handleConfig();
        configured = true;
    } else {
        // Reapply
        i2sService.end();
        i2sService.configureOutput(state.getI2sBclkPin(), state.getI2sLrckPin(),
                             state.getI2sDataPin(), state.getI2sSampleRate(),
                             state.getI2sBitsPerSample());
    }
}