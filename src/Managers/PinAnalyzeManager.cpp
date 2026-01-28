#include "PinAnalyzeManager.h"
#include <algorithm>
#include <sstream>
#include <cmath>

PinAnalyzeManager::PinAnalyzeManager(PinService& pinService)
: pinService(pinService) {}

void PinAnalyzeManager::begin(uint8_t pin_) {
    pin = pin_;
    pinService.setInput(pin);

    lastLevel = pinService.read(pin);
    startLevel = lastLevel;

    windowStartMs = millis();
    lastChangeUs = micros();

    edges = 0;
    highUs = 0;
    lowUs  = 0;
    minPulseUs = 0xFFFFFFFF;
    maxPulseUs = 0;
    basePulseUs = 0;

    bursts = 0;
    burstEdges = 0;
    maxGapUs = 0;
    inBurst = false;

    pulseCount = 0;
    pulseHead = 0;

    riseCount = 0;
    riseHead = 0;
    lastRiseUs = 0;
    lastHighPulseUs = 0;
}

bool PinAnalyzeManager::shouldReport(unsigned long nowMs) const {
    return (nowMs - windowStartMs) >= ANALYZE_DURATION_MS;
}

void PinAnalyzeManager::sample() {
    bool v = pinService.read(pin);
    if (v == lastLevel) return;
    onEdge(v, micros());
}

void PinAnalyzeManager::onEdge(bool newLevel, uint32_t nowUs) {
    uint32_t dt = nowUs - lastChangeUs;

    // Accumulate time spent in last level
    if (lastLevel) highUs += dt;
    else           lowUs  += dt;

    // Pulse stats 
    if (dt < minPulseUs) minPulseUs = dt;
    if (dt > maxPulseUs) maxPulseUs = dt;

    pulseRing[pulseHead] = dt;
    pulseHead = (pulseHead + 1) % PULSE_RING;
    if (pulseCount < PULSE_RING) pulseCount++;

    // Burst detection
    const uint32_t gapThresholdUs = 5000; // 5ms
    if (dt >= gapThresholdUs) {
        if (inBurst) inBurst = false;
        bursts++;
        if (dt > maxGapUs) maxGapUs = dt;
    } else {
        inBurst = true;
        burstEdges++;
    }

    // Rising edges for servo period
    if (!lastLevel && newLevel) { // LOW->HIGH
        riseUs[riseHead] = nowUs;
        riseHead = (riseHead + 1) % RISE_RING;
        if (riseCount < RISE_RING) riseCount++;

        if (lastRiseUs != 0) {
            // period candidate is time between rising edges (collected later)
        }
        lastRiseUs = nowUs;
    }

    // Falling edge gives us HIGH pulse width
    if (lastLevel && !newLevel) { // HIGH->LOW
        lastHighPulseUs = dt; // dt was time HIGH
    }

    edges++;
    lastLevel = newLevel;
    lastChangeUs = nowUs;
}

void PinAnalyzeManager::closeTail(uint32_t nowUs) {
    uint32_t dt = nowUs - lastChangeUs;
    if (lastLevel) highUs += dt;
    else           lowUs  += dt;

    // Don’t push tail into pulse ring
    lastChangeUs = nowUs;
}

void PinAnalyzeManager::resetWindow() {
    // Start a fresh window from current state
    windowStartMs = millis();
    lastChangeUs = micros();
    startLevel = lastLevel;

    edges = 0;
    highUs = 0;
    lowUs  = 0;
    minPulseUs = 0xFFFFFFFF;
    maxPulseUs = 0;

    bursts = 0;
    burstEdges = 0;
    maxGapUs = 0;
    inBurst = false;

    pulseCount = 0;
    pulseHead = 0;
    basePulseUs = 0;

    riseCount = 0;
    riseHead = 0;
    lastRiseUs = 0;
    lastHighPulseUs = 0;
}

void PinAnalyzeManager::collectPulses(std::vector<uint32_t>& out) const {
    out.clear();
    out.reserve(pulseCount);

    // Oldest -> newest
    int start = (pulseHead - pulseCount);
    if (start < 0) start += PULSE_RING;

    for (int i = 0; i < (int)pulseCount; i++) {
        int idx = (start + i) % PULSE_RING;
        out.push_back(pulseRing[idx]);
    }
}

void PinAnalyzeManager::collectRisePeriods(std::vector<uint32_t>& outPeriods) const {
    outPeriods.clear();
    if (riseCount < 2) return;

    // Oldest -> newest
    int start = (riseHead - riseCount);
    if (start < 0) start += RISE_RING;

    uint32_t prev = 0;
    for (int i = 0; i < (int)riseCount; i++) {
        int idx = (start + i) % RISE_RING;
        uint32_t t = riseUs[idx];
        if (prev != 0 && t > prev) outPeriods.push_back(t - prev);
        prev = t;
    }
}

void PinAnalyzeManager::collectHighPulses(std::vector<uint32_t>& outHighPulses) const {
    // We don’t store a ring of highs here to keep it light;
    // Instead, we infer from pulse distribution + duty + min/max.
    // If you want, we can store high pulse widths on each falling edge.
    outHighPulses.clear();
}

uint32_t PinAnalyzeManager::medianOf(std::vector<uint32_t>& v) {
    if (v.empty()) return 0;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n & 1) return v[n/2];
    return (uint32_t)((v[n/2 - 1] + v[n/2]) / 2);
}

uint32_t PinAnalyzeManager::estimateBaseT(const std::vector<uint32_t>& pulses) {
    if (pulses.size() < 10) return 0;

    // Take short pulses as timing base candidate
    std::vector<uint32_t> v = pulses;
    std::sort(v.begin(), v.end());
    size_t m = std::max<size_t>(8, v.size()/4);
    v.resize(m);
    return medianOf(v);
}

float PinAnalyzeManager::jitterScorePct(const std::vector<uint32_t>& pulses, uint32_t ref) {
    if (pulses.size() < 10 || ref == 0) return 100.f;

    // MAD around ref
    std::vector<uint32_t> dev;
    dev.reserve(pulses.size());
    for (auto p : pulses) {
        uint32_t d = (p > ref) ? (p - ref) : (ref - p);
        dev.push_back(d);
    }
    uint32_t mad = medianOf(dev);
    float pct = (100.0f * (float)mad) / (float)ref;
    if (pct < 0.f) pct = 0.f;
    if (pct > 200.f) pct = 200.f;
    return pct;
}

int PinAnalyzeManager::clampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

const char* PinAnalyzeManager::kindToStr(SignalKind k) {
    switch (k) {
        case SignalKind::Idle: return "Idle";
        case SignalKind::NoiseOrFloating: return "Noise/Floating";
        case SignalKind::Clock: return "Clock";
        case SignalKind::PWM: return "PWM";
        case SignalKind::Servo: return "Servo";
        case SignalKind::UartLike: return "UART-like";
        case SignalKind::OneWireLike: return "1-Wire-like";
        case SignalKind::BurstData: return "Burst data";
        default: return "Unknown";
    }
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectIdle(float approxHz, float dutyPct, uint32_t edges_, bool startLvl) const {
    Guess g;
    if (edges_ == 0 || approxHz < 0.5f) {
        g.kind = SignalKind::Idle;
        g.confidencePct = 85;
        g.note = std::string("No meaningful activity. Stayed mostly ") + (startLvl ? "HIGH" : "LOW") + ".";
        return g;
    }
    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectNoiseOrFloating(const std::vector<uint32_t>& pulses, float jitterPct, uint32_t minP, uint32_t edges_) const {
    Guess g;
    if (edges_ < 5) return g;

    // Very fast tiny pulses + high jitter => noise
    if (minP <= 3 && jitterPct > 60.f) {
        g.kind = SignalKind::NoiseOrFloating;
        g.confidencePct = 65;
        g.note = "Very short pulses with high jitter.";
        return g;
    }

    // Very broad distribution (max >> median) and jitter high
    if (!pulses.empty()) {
        std::vector<uint32_t> tmp = pulses;
        uint32_t med = medianOf(tmp);
        uint32_t mx = 0;
        for (auto p : pulses) if (p > mx) mx = p;

        if (med > 0 && mx > (med * 20) && jitterPct > 80.f) {
            g.kind = SignalKind::NoiseOrFloating;
            g.confidencePct = 55;
            g.note = "Highly irregular timing.";
        }
    }
    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectClockPwm(float approxHz,
                                                          float dutyPct,
                                                          float /*jitterPct*/,
                                                          uint32_t edges_) const {
    Guess g;
    if (edges_ < 40) return g;
    if (maxPulseUs == 0) return g;
    uint32_t a = basePulseUs ? basePulseUs : minPulseUs;
    uint32_t b = maxPulseUs;

    if (a == 0 || a == 0xFFFFFFFF) return g;

    // Ignore tiny
    if (a < 4) return g;

    if (a > b) { uint32_t t = a; a = b; b = t; }

    // Ratio check
    if (b > a * 16) return g;

    uint32_t periodUs = a + b;
    if (periodUs == 0) return g;

    const float hzFromPulses = 1000000.0f / (float)periodUs;

    bool midDuty = (dutyPct > 45.f && dutyPct < 55.f);
    bool anyDuty = (dutyPct > 5.f  && dutyPct < 95.f);

    int conf = 55;
    if (edges_ > 200)  conf += 10;
    if (edges_ > 2000) conf += 10;
    if (b <= a * 3)    conf += 10;
    if (b <= a * 2)    conf += 5;

    conf = clampInt(conf, 0, 90);

    if (hzFromPulses >= 20.f && anyDuty) {
        if (midDuty) {
            g.kind = SignalKind::Clock;
            g.confidencePct = clampInt(conf + 5, 0, 95);
            g.note = "Regular toggling, near 50% duty.";
        } else {
            g.kind = SignalKind::PWM;
            g.confidencePct = conf;
            g.note = "Regular toggling, duty not 50%.";
        }
        return g;
    }

    return Guess{};
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectServo(const std::vector<uint32_t>& risePeriods, const std::vector<uint32_t>& /*highPulses*/) const {
    Guess g;
    if (risePeriods.size() < 6) return g;

    // Servo: period ~20ms (50Hz). We just detect a strong cluster near 20ms.
    int near20 = 0;
    for (auto p : risePeriods) {
        if (p > 15000 && p < 25000) near20++;
    }
    float ratio = (float)near20 / (float)risePeriods.size();
    if (ratio > 0.70f) {
        g.kind = SignalKind::Servo;
        g.confidencePct = 78;
        g.note = "Strong ~20 ms rhythm.";
        return g;
    }
    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectUartLike(const std::vector<uint32_t>& pulses, uint32_t baseT) const {
    Guess g;
    if (pulses.size() < 30 || baseT < 2 || baseT > 2000) return g;

    // Check how many pulses are close to N*T
    int ok = 0;
    int total = 0;
    float tol = baseT * 0.25f;

    for (auto p : pulses) {
        if (p < baseT / 2) continue;
        total++;

        int bestN = 0;
        float bestErr = 1e9f;
        for (int n = 1; n <= 8; n++) {
            float target = (float)baseT * n;
            float err = fabsf((float)p - target);
            if (err < bestErr) { bestErr = err; bestN = n; }
        }
        if (bestErr <= tol) ok++;
    }

    if (total < 20) return g;

    float score = (float)ok / (float)total;
    if (score > 0.60f) {
        int baud = (baseT > 0) ? (int)(1000000.0f / (float)baseT + 0.5f) : 0;

        // confidence shaped by score
        int conf = (int)(40 + score * 60);
        if (baud < 1200 || baud > 2000000) conf = (int)(conf * 0.7f);

        g.kind = SignalKind::UartLike;
        g.confidencePct = clampInt(conf, 0, 95);
        g.note = "Timings match a bit-time";
        g.extra = "baud~" + std::to_string(baud);
        return g;
    }

    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectOneWireLike(const std::vector<uint32_t>& pulses) const {
    Guess g;
    if (pulses.size() < 20) return g;

    int near60 = 0;
    int near480 = 0;
    int total = 0;

    for (auto p : pulses) {
        total++;
        if (p >= 40 && p <= 90) near60++;
        if (p >= 350 && p <= 650) near480++;
    }

    float s60 = (float)near60 / (float)total;
    float s480 = (float)near480 / (float)total;

    if (s60 > 0.25f && s480 > 0.02f) {
        g.kind = SignalKind::OneWireLike;
        g.confidencePct = 70;
        g.note = "~60 us slots and occasional ~480 us pulses.";
        return g;
    }

    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectBurstData(int bursts_, uint32_t edges_, float approxHz, float jitterPct) const {
    Guess g;
    if (edges_ < 10) return g;

    //many edges but segmented by gaps, and jitter not super low
    if (bursts_ >= 2 && approxHz > 20.f && jitterPct > 15.f) {
        g.kind = SignalKind::BurstData;
        int conf = 55 + (bursts_ > 5 ? 10 : 0);
        g.confidencePct = clampInt(conf, 0, 85);
        g.note = "Feels like data traffic.";
        g.extra = "bursts~" + std::to_string(bursts_);
        return g;
    }

    return g;
}

PinAnalyzeManager::Guess PinAnalyzeManager::detectI2cLike(
    const std::vector<uint32_t>& pulses,
    float approxHz, float dutyPct, float jitterPct,
    int bursts_, uint32_t edges_,
    const Guess& uartGuess
) const {
    Guess g;
    if (edges_ < 30) return g;                 // too little
    if (bursts_ < 2) return g;                 // I2C usually bursty
    if (dutyPct < 55.f) return g;              // idle high / pull-up bias
    if (approxHz < 5000.f || approxHz > 2000000.f) return g; // plausible range

    // Clock stability: I2C can jitter a bit
    int score = 0;

    // burstiness is key
    score += 30;
    if (bursts_ >= 4) score += 10;

    // idle high
    score += 15;
    if (dutyPct >= 75.f) score += 5;

    // jitter: if very stable it's good, if extremely unstable it's suspicious
    if (jitterPct < 15.f) score += 20;
    else if (jitterPct < 35.f) score += 10;
    else if (jitterPct > 80.f) score -= 15;

    // Pulse distribution: SCL tends to have many similar short pulses
    // We check if a large fraction of pulses are close to the median short pulse.
    if (!pulses.empty()) {
        std::vector<uint32_t> tmp = pulses;
        uint32_t med = medianOf(tmp);
        if (med > 0) {
            int close = 0, total = 0;
            float tol = med * 0.25f;
            for (auto p : pulses) {
                if (p < 2) continue;
                // consider only reasonably short pulses (active region)
                if (p > med * 8) continue;
                total++;
                if (fabsf((float)p - (float)med) <= tol) close++;
            }
            if (total > 20) {
                float r = (float)close / (float)total;
                if (r > 0.60f) score += 15;
                else if (r > 0.40f) score += 8;
            }
        }
    }

    // If UART detection is strong, reduce I2C confidence a bit (SDA can look like UART)
    if (uartGuess.kind == SignalKind::UartLike && uartGuess.confidencePct >= 70) {
        score -= 20;
    } else if (uartGuess.kind == SignalKind::UartLike && uartGuess.confidencePct >= 50) {
        score -= 10;
    }

    score = clampInt(score, 0, 90);
    if (score < 45) return g;

    g.kind = SignalKind::I2cLike;
    g.confidencePct = score;
    g.note = "Idle-high bias and clock-like timing.";
    g.extra = "clk~" + std::to_string((int)(approxHz + 0.5f)) + "Hz bursts~" + std::to_string(bursts_);
    return g;
}

std::string PinAnalyzeManager::runPullTest() {
    // Very short tests: 40ms each, purely observational.
    auto measureStability = [&](int ms) -> uint32_t {
        uint32_t e = 0;
        bool last = pinService.read(pin);
        uint32_t t0 = micros();
        unsigned long t0ms = millis();
        while ((millis() - t0ms) < (unsigned long)ms) {
            bool v = pinService.read(pin);
            if (v != last) { e++; last = v; }
        }
        (void)t0;
        return e;
    };

    pinService.setInput(pin);
    uint32_t baseEdges = measureStability(40);
    if (baseEdges == 0xFFFFFFFF) return "";

    // Pull-up
    pinService.setInputPullup(pin);
    uint32_t upEdges = measureStability(40);
    if (upEdges == 0xFFFFFFFF) { pinService.setInput(pin); return ""; }

    // Pull-down
    pinService.setInputPullDown(pin);
    uint32_t dnEdges = measureStability(40);
    if (dnEdges == 0xFFFFFFFF) { pinService.setInput(pin); return ""; }

    // Restore
    pinService.setInput(pin);

    // Interpretation
    if (baseEdges > 20 && upEdges <= (baseEdges / 4)) {
        return "Pull-up stabilizes the line. Floating or open-drain is likely.";
    }
    if (baseEdges > 20 && dnEdges <= (baseEdges / 4)) {
        return "Pull-down  stabilizes the line. Floating input is likely.";
    }
    if (upEdges > baseEdges * 2 && dnEdges > baseEdges * 2) {
        return "Pull resistors make it worse. This line is probably driven.";
    }

    return "";
}

PinAnalyzeManager::Report PinAnalyzeManager::buildReport(bool doPullTest) {
    Report r;

    // Close tail to include the last stable segment time in high/low
    closeTail(micros());

    uint32_t elapsedMs = (uint32_t)(millis() - windowStartMs);
    if (elapsedMs == 0) elapsedMs = 1;
    r.edges = edges; // raw count
    r.edgesPerSec = (uint32_t)((uint64_t)edges * 1000ULL / elapsedMs);
    r.highUs = highUs;
    r.lowUs  = lowUs;
    r.minPulseUs = minPulseUs;
    r.maxPulseUs = maxPulseUs;
    r.bursts = bursts;
    r.burstEdges = burstEdges;
    r.maxGapUs = maxGapUs;

    uint32_t totalUs = highUs + lowUs;
    r.dutyPct = (totalUs ? (100.0f * (float)highUs) / (float)totalUs : 0.f);

    float seconds = elapsedMs / 1000.0f;
    r.approxHz = (seconds > 0.f) ? ((edges / 2.0f) / seconds) : 0.f;


    // Pulse features
    std::vector<uint32_t> pulses;
    collectPulses(pulses);

    if (!pulses.empty()) {
        std::vector<uint32_t> tmp = pulses;
        r.medianPulseUs = medianOf(tmp);
        r.basePulseUs = estimateBaseT(pulses);
        basePulseUs = r.basePulseUs;
        r.jitterPct = jitterScorePct(pulses, r.basePulseUs ? r.basePulseUs : r.medianPulseUs);
    } else {
        r.medianPulseUs = 0;
        r.basePulseUs = 0;
        r.jitterPct = 100.f;
    }

    // Rise periods (servo)
    std::vector<uint32_t> risePeriods;
    collectRisePeriods(risePeriods);

    // Guesses
    std::vector<Guess> guesses;
    guesses.reserve(8);

    auto gIdle  = detectIdle(r.approxHz, r.dutyPct, r.edges, startLevel);
    if (gIdle.confidencePct) guesses.push_back(gIdle);

    auto gNoise = detectNoiseOrFloating(pulses, r.jitterPct, r.minPulseUs, r.edges);
    if (gNoise.confidencePct) guesses.push_back(gNoise);

    auto gCP = detectClockPwm(r.approxHz, r.dutyPct, r.jitterPct, r.edges);
    if (gCP.confidencePct) guesses.push_back(gCP);

    auto gServo = detectServo(risePeriods, {});
    if (gServo.confidencePct) guesses.push_back(gServo);

    auto gUart = detectUartLike(pulses, r.basePulseUs);
    if (gUart.confidencePct) guesses.push_back(gUart);

    auto gI2c = detectI2cLike(pulses, r.approxHz, r.dutyPct, r.jitterPct, r.bursts, r.edges, gUart);
    if (gI2c.confidencePct) guesses.push_back(gI2c);

    auto g1w = detectOneWireLike(pulses);
    if (g1w.confidencePct) guesses.push_back(g1w);

    auto gBurst = detectBurstData(bursts, r.edges, r.approxHz, r.jitterPct);
    if (gBurst.confidencePct) guesses.push_back(gBurst);

    // If nothing matched and there is activity
    if (guesses.empty() && r.edges > 0) {
        Guess g;
        g.kind = SignalKind::Unknown;
        g.confidencePct = 30;
        g.note = "There is activity, but it doesn't match pattern.";
        guesses.push_back(g);
    }

    // Sort top guesses
    std::sort(guesses.begin(), guesses.end(), [](const Guess& a, const Guess& b){
        return a.confidencePct > b.confidencePct;
    });

    r.top1 = guesses.size() > 0 ? guesses[0] : Guess{};
    r.top2 = guesses.size() > 1 ? guesses[1] : Guess{};
    r.top3 = guesses.size() > 2 ? guesses[2] : Guess{};

    // Pull test
    if (doPullTest) {
        r.pullTestDone = true;
        r.pullHint = runPullTest();
    }

    return r;
}

std::string PinAnalyzeManager::formatWizardReport(uint8_t pin, const Report& r) const {
    std::ostringstream oss;

    oss << "[Wizard report on pin " << (int)pin << "]\r\n";

    auto line = [&](const std::string& s) {
        oss << "  " << s << "\r\n";
    };

    if (r.edges == 0) {
        line("I did not see any activity.");
        line(std::string("The line stayed mostly ") + (startLevel ? "HIGH." : "LOW."));
        line("This looks idle. If unexpected, check wiring.");
    } else {
        // Sspeed summary
        if (r.approxHz < 10.f)       line("Activity is slow.");
        else if (r.approxHz < 1000.f) line("Activity is moderate.");
        else                          line("Activity is fast.");

        // Top guesses
        {
            std::string g1 = std::string("Top guess: ") +
                kindToStr(r.top1.kind) + " (" + std::to_string(r.top1.confidencePct) + "%) — " + r.top1.note;
            if (!r.top1.extra.empty()) g1 += " [" + r.top1.extra + "]";
            line(g1);
        }

        if (r.top2.confidencePct > 0) {
            std::string g2 = std::string("Also possible: ") +
                kindToStr(r.top2.kind) + " (" + std::to_string(r.top2.confidencePct) + "%)";
            if (!r.top2.extra.empty()) g2 += " [" + r.top2.extra + "]";
            line(g2);
        }

        // What we see
        int hzInt = (int)(r.approxHz + 0.5f);
        int dutyInt = clampInt((int)(r.dutyPct + 0.5f), 0, 100);
        line("~" + std::to_string(hzInt) + " Hz, " + std::to_string(dutyInt) + "% HIGH.");

        auto isClockish = (r.top1.kind == SignalKind::PWM || r.top1.kind == SignalKind::Clock);

        if (isClockish) {
            // For PWM, jitterPct is often meaningless because the distribution is bimodal (high+low).
            if (r.minPulseUs != 0xFFFFFFFF && r.maxPulseUs > 0) {
                uint32_t a = r.basePulseUs ? r.basePulseUs : r.minPulseUs;
                uint32_t b = r.maxPulseUs;
                if (a > b) { uint32_t t = a; a = b; b = t; }

                float ratio = (a > 0) ? ((float)b / (float)a) : 999.f;

                if (ratio <= 3.0f)      line("Clock-like timing is very consistent.");
                else if (ratio <= 12.0f) line("PWM-like timing (two pulse widths).");
                else                    line("PWM-like but very asymmetric duty.");
            } else {
                line("Clock/PWM-like activity.");
            }
        } else {
            if (r.jitterPct < 10.f)       line("Timing is very stable.");
            else if (r.jitterPct < 35.f)  line("Timing is somewhat stable.");
            else                          line("Timing is quite irregular.");
        }

        if (r.bursts >= 2) {
            line("It comes in bursts (" + std::to_string(r.bursts) +
                 " bursts, max gap ~" + std::to_string(r.maxGapUs) + " us).");
        }

        if (r.minPulseUs != 0xFFFFFFFF) {
            line("Min pulse ~" + std::to_string(r.minPulseUs) +
                 " us, max ~" + std::to_string(r.maxPulseUs) + " us.");
        }
    }

    if (r.pullTestDone && !r.pullHint.empty()) {
        line(r.pullHint);
    }

    // Technical line
    {
        std::string tech = "Report: edges/sec=" + std::to_string(r.edgesPerSec) +
                           " duty=" + std::to_string((int)(r.dutyPct + 0.5f)) + "%" +
                           " jitter~" + std::to_string((int)(r.jitterPct + 0.5f)) + "%";
        if (r.basePulseUs) tech += " baseT~" + std::to_string(r.basePulseUs) + "us";
        line(tech);
    }

    oss << "\r\n";
    return oss.str();
}
