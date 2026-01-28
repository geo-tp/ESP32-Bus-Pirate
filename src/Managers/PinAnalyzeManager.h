#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>
#include "Services/PinService.h"

class PinAnalyzeManager {
public:
    enum class SignalKind {
        Idle,
        NoiseOrFloating,
        Clock,
        PWM,
        Servo,
        UartLike,
        OneWireLike,
        BurstData,
        I2cLike,
        Unknown
    };

    struct Guess {
        SignalKind kind = SignalKind::Unknown;
        int confidencePct = 0;           // 0..100
        std::string note;                // short explanation
        std::string extra;               // e.g. "baud=115200", "freq=1kHz"
    };

    struct Report {
        uint32_t edges = 0;
        uint32_t edgesPerSec = 0;
        uint32_t highUs = 0;
        uint32_t lowUs = 0;
        uint32_t minPulseUs = 0xFFFFFFFF;
        uint32_t maxPulseUs = 0;
        uint32_t medianPulseUs = 0;
        uint32_t basePulseUs = 0; 
        float dutyPct = 0.f;  
        float approxHz = 0.f;
        float jitterPct = 0.f;   

        // Activity structure
        int bursts = 0;
        int burstEdges = 0;
        uint32_t maxGapUs = 0;

        // Derived guesses
        Guess top1, top2, top3;

        // Pull test hints
        bool pullTestDone = false;
        std::string pullHint;
    };

public:
    explicit PinAnalyzeManager(PinService& pinService);

    void begin(uint8_t pin);
    void end();
    void sample(); 
    bool shouldReport(unsigned long nowMs) const; 
    Report buildReport(bool doPullTest);
    std::string formatWizardReport(uint8_t pin, const Report& r) const;
    void resetWindow();

private:
    PinService& pinService;
    uint8_t pin = 0;

    // Window timing
    unsigned long windowStartMs = 0;
    unsigned long nextReportMs  = 0;
    uint32_t lastChangeUs = 0;

    // State
    bool lastLevel = false;
    bool startLevel = false;

    // Accumulators
    uint32_t edges = 0;
    uint32_t highUs = 0;
    uint32_t lowUs = 0;
    uint32_t minPulseUs = 0xFFFFFFFF;
    uint32_t maxPulseUs = 0;

    // Burst detection
    int bursts = 0;
    int burstEdges = 0;
    uint32_t maxGapUs = 0;
    bool inBurst = false;

    // Pulse ring buffer
    static constexpr int PULSE_RING = 256;
    uint32_t *pulseRing = nullptr;
    uint16_t pulseCount = 0; 
    uint16_t pulseHead = 0;    // next index to write
    uint32_t basePulseUs = 0;

    // Rising-edge based
    static constexpr int RISE_RING = 64;
    uint32_t *riseUs = nullptr;
    uint16_t riseCount = 0;
    uint16_t riseHead = 0;
    uint32_t lastRiseUs = 0;
    uint32_t lastHighPulseUs = 0;

    static const uint32_t ANALYZE_DURATION_MS = 8000;

private:
    void onEdge(bool newLevel, uint32_t nowUs);
    void closeTail(uint32_t nowUs);

    // Stats helpers
    static uint32_t medianOf(std::vector<uint32_t>& v);
    static uint32_t estimateBaseT(const std::vector<uint32_t>& pulses);
    static float jitterScorePct(const std::vector<uint32_t>& pulses, uint32_t ref);
    static int clampInt(int v, int lo, int hi);

    // Pattern detectors
    Guess detectIdle(float approxHz, float dutyPct, uint32_t edges, bool startLevel) const;
    Guess detectNoiseOrFloating(const std::vector<uint32_t>& pulses, float jitterPct, uint32_t minPulseUs, uint32_t edges) const;
    Guess detectClockPwm(float approxHz, float dutyPct, float jitterPct, uint32_t edges) const;
    Guess detectServo(const std::vector<uint32_t>& risePeriods, const std::vector<uint32_t>& highPulses) const;
    Guess detectUartLike(const std::vector<uint32_t>& pulses, uint32_t baseT) const;
    Guess detectOneWireLike(const std::vector<uint32_t>& pulses) const;
    Guess detectBurstData(int bursts, uint32_t edges, float approxHz, float jitterPct) const;
    Guess detectI2cLike(const std::vector<uint32_t>& pulses,
                        float approxHz, float dutyPct, float jitterPct,
                        int bursts, uint32_t edges, const Guess& uartGuess) const;

    // Pull tests
    std::string runPullTest();

    // Collectors
    void collectPulses(std::vector<uint32_t>& out) const;
    void collectRisePeriods(std::vector<uint32_t>& outPeriods) const;
    void collectHighPulses(std::vector<uint32_t>& outHighPulses) const;

    static const char* kindToStr(SignalKind k);
};
