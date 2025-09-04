#include "SubGhzAnalyzeManager.h"
#include <cmath>
#include <sstream>
#include <unordered_set>

std::string SubGhzAnalyzeManager::analyzeFrame(const std::vector<rmt_item32_t>& items, float tickPerUs) {
    SubGhzDetectResult r;
    if (items.empty()) { r.notes = "Empty frame"; return formatFrame(r); }

    std::vector<uint32_t> highs, lows;
    collectDurations(items, tickPerUs, highs, lows);

    float T = estimateBaseT(highs, lows);
    r.baseT_us = T;

    float ratio = 0.f;
    bool pulse  = looksPulseLength(T, highs, lows, ratio);
    bool manch  = looksManchester(T, highs, lows);
    bool pwm    = looksPWM(T, highs, lows);

    if (manch) r.encoding = RfEncoding::Manchester;
    else if (pulse) r.encoding = RfEncoding::PulseLength;
    else if (pwm) r.encoding = RfEncoding::PWM;
    else r.encoding = RfEncoding::Unknown;

    // Bitrate
    if (r.encoding == RfEncoding::Manchester && T > 0.f) {
        r.bitrate_kbps = 1000.f / (2.f * T);
    } else if (T > 0.f) {
        r.bitrate_kbps = 1000.f / T;
    }

    // Attempt decoding
    if (r.encoding == RfEncoding::PulseLength) {
        std::string hex;
        int bits = 0;
        if (decodePT2262Like(T, items, tickPerUs, hex, bits)) {
            r.payloadHex = hex;
            r.bitCount   = bits;

            // Guess protocol
            if (bits >= 20 && bits <= 36 && ratio > 2.f) {
                r.protocolGuess = "EV1527/PT2262-like";
            } else {
                r.protocolGuess = "Pulse-length (generic)";
            }
            r.confidence = clamp01(0.6f + 0.2f * (ratio > 2.5f));
        } else {
            r.protocolGuess = "Pulse-length (undecoded)";
            r.confidence = 0.35f;
        }
    } else if (r.encoding == RfEncoding::Manchester) {
        r.protocolGuess = "Manchester (generic)";
        r.confidence = 0.5f;
    } else if (r.encoding == RfEncoding::PWM) {
        r.protocolGuess = "PWM (generic)";
        r.confidence = 0.4f;
    } else {
        r.protocolGuess = "Unknown";
        r.confidence = 0.2f;
    }

    return formatFrame(r);
}

std::string SubGhzAnalyzeManager::analyzeFrequencyActivity(
    int dwellMs,
    int windowMs,
    int thresholdDbm,
    const std::function<int(int)>& measure,
    const std::function<bool()>& shouldAbort,
    float neighborLeftConf,
    float neighborRightConf
) {
    if (windowMs < 1) windowMs = 1;
    const int windows = std::max(1, dwellMs / windowMs);

    int hits = 0;
    int peakDbm = -127, minDbm = 127, maxDbm = -127;
    long long sum = 0;

    std::vector<int> samples;
    samples.reserve(windows);

    for (int i = 0; i < windows; ++i) {
        if (shouldAbort && shouldAbort()) break;
        const int v = measure(windowMs);
        samples.push_back(v);
        sum += v;
        if (v > peakDbm) peakDbm = v;
        if (v < minDbm)  minDbm  = v;
        if (v > maxDbm)  maxDbm  = v;
        if (v >= thresholdDbm)   hits++;
    }

    const int used = static_cast<int>(samples.size());
    const float avgDbm = (used > 0) ? static_cast<float>(sum) / used : -127.f;

    // Variance
    double var = 0.0;
    for (int v : samples) {
        const double d = static_cast<double>(v) - static_cast<double>(avgDbm);
        var += d * d;
    }
    var /= std::max(1, used);
    const float stddevDbm = static_cast<float>(std::sqrt(var));

    const float hitRatio = (used > 0) ? static_cast<float>(hits) / used : 0.f;

    // Confidence
    const float margin = static_cast<float>(peakDbm - thresholdDbm);
    float mScore = margin / 25.f;
    if (mScore < 0.f) mScore = 0.f;
    else if (mScore > 1.f) mScore = 1.f;

    float aScore = hitRatio;
    if (aScore < 0.f) aScore = 0.f;
    else if (aScore > 1.f) aScore = 1.f;

    float conf = 0.25f + 0.55f * mScore + 0.30f * aScore;

    if (hits == 0 || peakDbm < thresholdDbm) {
        conf = 0.f;                              // nothing
    } else if (hitRatio < 0.10f && (margin) < 5) {
        conf *= 0.3f;                            // rare
    }

    // Decode modulation
    auto mod = decodeModulationRSSI(samples);
    std::string modStr =
        (mod.first == ModGuess::ASK_OOK)   ? "ASK/OOK" :
        (mod.first == ModGuess::FSK_GFSK) ? "FSK/GFSK" :
        (mod.first == ModGuess::NFM_FM)   ? "NFM/FM"   : "Unknown";

    // Bonus stable
    if (margin >= 15.f && hitRatio >= 0.60f && stddevDbm <= 3.0f) conf += 0.05f;

    // Boost neighboring confidence
    if (neighborLeftConf  > 0.60f) conf += 0.03f;
    if (neighborRightConf > 0.60f) conf += 0.03f;
    if (neighborLeftConf  > 0.80f && neighborRightConf > 0.80f) conf += 0.02f;

    if (conf < 0.f) conf = 0.f;
    else if (conf > 1.f) conf = 1.f;

    auto formatted = formatFrequency(peakDbm, hits, used, avgDbm, stddevDbm, hitRatio, conf, modStr);
    return formatted;
}

std::string SubGhzAnalyzeManager::formatFrequency(
    int peakDbm,
    int hits,
    int windows,
    float avgDbm,
    float stddevDbm,
    float hitRatio,
    float confidence,
    std::string mod
) {
    std::ostringstream oss;
    oss << "peak=" << peakDbm << " dBm"
        << "  activity=" << static_cast<int>(hitRatio * 100.f) << '%'
        << "  conf=" << static_cast<int>(confidence * 100.f) << '%'
        << "  avg=" << static_cast<int>(std::lround(avgDbm)) << " dBm"
        << " (std=" << static_cast<int>(std::lround(stddevDbm)) << ")"
        << "  hits=" << hits << '/' << windows
        << "  mod=" << mod;
    return oss.str();
}


std::string SubGhzAnalyzeManager::formatFrame(const SubGhzDetectResult& r) const {
    auto encToStr = [](RfEncoding e){
        switch (e) {
            case RfEncoding::PulseLength: return "PulseLength";
            case RfEncoding::Manchester:  return "Manchester";
            case RfEncoding::PWM:         return "PWM";
            default:                      return "Unknown";
        }
    };

    std::ostringstream oss;
    oss << " [Frame received]\r\n"
        << " Encoding     : " << encToStr(r.encoding) << "\r\n"
        << " Base T (us)  : " << std::lround(r.baseT_us) << "\r\n"
        << " Bitrate (kb) : " << r.bitrate_kbps << "\r\n"
        << " Bit count    : " << r.bitCount << "\r\n"
        << " Payload (hex): " << (r.payloadHex.empty() ? "-" : r.payloadHex) << "\r\n"
        << " Protocol     : " << r.protocolGuess << "\r\n"
        << " Confidence   : " << r.confidence << "\r\n";
    if (!r.notes.empty()) oss << "Notes        : " << r.notes << "\r\n";
    return oss.str();
}

void SubGhzAnalyzeManager::collectDurations(const std::vector<rmt_item32_t>& items, float tickPerUs,
                                            std::vector<uint32_t>& highs, std::vector<uint32_t>& lows) {
    highs.clear(); lows.clear();
    highs.reserve(items.size());
    lows.reserve(items.size());

    for (const auto& it : items) {
        // Convert RMT ticks
        uint32_t d0 = (uint32_t)std::max(1.f, std::round(it.duration0 / tickPerUs));
        uint32_t d1 = (uint32_t)std::max(1.f, std::round(it.duration1 / tickPerUs));
        highs.push_back(d0);
        lows.push_back(d1);
    }
}

float SubGhzAnalyzeManager::median(std::vector<uint32_t> v) {
    if (v.empty()) return 0.f;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    return (n & 1) ? (float)v[n/2] : 0.5f * (v[n/2 - 1] + v[n/2]);
}

float SubGhzAnalyzeManager::estimateBaseT(const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows) {
    // Take short durations as approximation
    std::vector<uint32_t> pool;
    pool.reserve(highs.size() + lows.size());
    for (auto d : highs) if (d >= 2) pool.push_back(d);
    for (auto d : lows)  if (d >= 2) pool.push_back(d);
    if (pool.size() < 8) return 0.f;

    std::sort(pool.begin(), pool.end());
    size_t m = std::max<size_t>(4, pool.size()/4);
    std::vector<uint32_t> q(pool.begin(), pool.begin() + m);
    return median(q);
}

bool SubGhzAnalyzeManager::looksManchester(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows) {
    if (T <= 0.f || highs.size() < 8) return false;
    int ok = 0, total = 0;
    float tol = T * 0.40f; // tolerance
    for (size_t i = 0; i < highs.size(); ++i) {
        total++;
        if (nearf(highs[i], T, tol) && nearf(lows[i], T, tol)) ok++;
    }
    return total >= 8 && (float)ok / total > 0.6f;
}

bool SubGhzAnalyzeManager::looksPulseLength(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows, float& ratioOut) {
    if (T <= 0.f || highs.size() < 8) { ratioOut = 0.f; return false; }

    std::vector<float> r1, r2;
    r1.reserve(highs.size()); r2.reserve(highs.size());
    for (size_t i = 0; i < highs.size(); ++i) {
        float h = (float)highs[i];
        float l = (float)lows[i];
        if (h < 1.f || l < 1.f) continue;
        r1.push_back(h / l);
        r2.push_back(l / h);
    }
    if (r1.empty() || r2.empty()) { ratioOut = 0.f; return false; }
    std::sort(r1.begin(), r1.end());
    std::sort(r2.begin(), r2.end());
    float m1 = r1[r1.size()/2];
    float m2 = r2[r2.size()/2];
    ratioOut = (m1 > m2) ? m1 : m2;

    return ratioOut > 1.8f && ratioOut < 4.2f;
}

bool SubGhzAnalyzeManager::looksPWM(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows) {
    if (T <= 0.f || highs.size() < 8) return false;

    auto medH = median(highs);
    auto medL = median(lows);
    float varH = 0.f, varL = 0.f;
    for (auto d : highs) varH += std::fabs((float)d - medH);
    for (auto d : lows)  varL += std::fabs((float)d - medL);
    varH /= std::max<size_t>(1, highs.size());
    varL /= std::max<size_t>(1, lows.size());

    return (varH < 0.25f * varL) || (varL < 0.25f * varH);
}

static inline bool isShortT(float d, float T)  { return std::fabs(d - T) <= (T * 0.6f); }
static inline bool isLong3T(float d, float T)  { return std::fabs(d - 3*T) <= (T * 0.8f); }

bool SubGhzAnalyzeManager::decodePT2262Like(float T, const std::vector<rmt_item32_t>& items, float tickPerUs,
                                            std::string& hexOut, int& bitCountOut) {
    if (T <= 0.f || items.size() < 8) return false;

    std::string bits;
    bits.reserve(items.size());

    for (const auto& it : items) {
        float h = it.duration0 / tickPerUs;
        float l = it.duration1 / tickPerUs;

        // Ignore
        if (h < 2 || l < 2) continue;

        bool hS = isShortT(h, T), hL = isLong3T(h, T);
        bool lS = isShortT(l, T), lL = isLong3T(l, T);

        if (hS && lL)      bits.push_back('0');
        else if (hL && lS) bits.push_back('1');
        else {
            float ehS = std::fabs(h - T),   ehL = std::fabs(h - 3*T);
            float elS = std::fabs(l - T),   elL = std::fabs(l - 3*T);
            if (ehS+elL < ehL+elS) bits.push_back('0');
            else                   bits.push_back('1');
        }
    }

    // Too short
    if ((int)bits.size() < 16) return false;

    // Truncate to multiple of 4
    bitCountOut = (int)(bits.size() & ~0x3);
    if (bitCountOut == 0) return false;

    bits.resize((size_t)bitCountOut);
    hexOut = bitsToHex(bits);
    return true;
}

bool SubGhzAnalyzeManager::nearf(float a, float b, float tol) { return std::fabs(a - b) <= tol; }

float SubGhzAnalyzeManager::clamp01(float v) {
    if (v < 0.f) return 0.f;
    if (v > 1.f) return 1.f;
    return v;
}

std::string SubGhzAnalyzeManager::bitsToHex(const std::string& bits) {
    static const char* HEX = "0123456789ABCDEF";
    std::string out;
    out.reserve(bits.size()/4);
    for (size_t i = 0; i + 3 < bits.size(); i += 4) {
        int v = ((bits[i]-'0')<<3) | ((bits[i+1]-'0')<<2) | ((bits[i+2]-'0')<<1) | (bits[i+3]-'0');
        out.push_back(HEX[v & 0xF]);
    }
    return out;
}

std::pair<SubGhzAnalyzeManager::ModGuess, float>
SubGhzAnalyzeManager::decodeModulationRSSI(const std::vector<int>& samples) {
    if (samples.size() < 8) return {ModGuess::Unknown, 0.f};

    // k means
    int vmin = 127, vmax = -127;
    for (size_t i = 0; i < samples.size(); ++i) {
        int v = samples[i];
        if (v < vmin) vmin = v;
        if (v > vmax) vmax = v;
    }

    double c1 = vmin, c2 = vmax;
    for (int it = 0; it < 6; ++it) {
        double s1 = 0.0, s2 = 0.0; int n1 = 0, n2 = 0;
        for (size_t i = 0; i < samples.size(); ++i) {
            int v = samples[i];
            if (std::fabs(v - c1) <= std::fabs(v - c2)) { s1 += v; ++n1; }
            else                                         { s2 += v; ++n2; }
        }
        if (n1 > 0) c1 = s1 / n1;
        if (n2 > 0) c2 = s2 / n2;
    }

    // Variances in clusters
    double s1 = 0.0, s2 = 0.0; int n1 = 0, n2 = 0;
    for (size_t i = 0; i < samples.size(); ++i) {
        int v = samples[i];
        if (std::fabs(v - c1) <= std::fabs(v - c2)) { s1 += (v - c1) * (v - c1); ++n1; }
        else                                         { s2 += (v - c2) * (v - c2); ++n2; }
    }
    double v1 = (n1 > 1) ? (s1 / (n1 - 1)) : 1.0;
    double v2 = (n2 > 1) ? (s2 / (n2 - 1)) : 1.0;

    // Normalize
    double sep = std::fabs(c1 - c2);
    double sp  = std::sqrt(v1 + v2 + 1e-6);
    double d   = (sp > 0.0) ? (sep / sp) : 0.0;

    double p1 = (samples.empty() ? 0.0 : double(n1) / double(samples.size()));
    double p2 = (samples.empty() ? 0.0 : double(n2) / double(samples.size()));
    double pmin = (p1 < p2) ? p1 : p2;

    // Global stats
    double mean = 0.0;
    for (size_t i = 0; i < samples.size(); ++i) mean += samples[i];
    mean /= samples.size();

    double var = 0.0;
    for (size_t i = 0; i < samples.size(); ++i) {
        double e = samples[i] - mean;
        var += e * e;
    }
    var /= samples.size();
    double sd = std::sqrt(var);

    // ASK/OOK : bimodal
    if (d >= 1.6 && pmin >= 0.15) {
        double t = d / 3.0;
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;
        double score = t * (pmin / 0.5);
        if (score < 0.0) score = 0.0;
        if (score > 1.0) score = 1.0;
        return {ModGuess::ASK_OOK, float(score)};
    }

    // FSK/GFSK : weak amplitude variation
    if (sd <= 2.5 && d < 0.9) {
        double t = (2.5 - sd) / 2.5;
        if (t < 0.0) t = 0.0;
        if (t > 1.0) t = 1.0;
        return {ModGuess::FSK_GFSK, float(t)};
    }

    // NFM Moderate amplitude variation
    if (sd >= 3.0 && sd <= 10.0 && d < 1.2) {
        float sSd = float((sd - 3.0) / 7.0);   // 0..1
        if (sSd < 0.0f) sSd = 0.0f; if (sSd > 1.0f) sSd = 1.0f;
        float sD  = float((1.2 - d) / 1.2);    // 0..1
        if (sD  < 0.0f) sD  = 0.0f; if (sD  > 1.0f) sD  = 1.0f;

        float score = 0.85f * sSd + 0.15f * sD;
        if (score < 0.0f) score = 0.0f;
        if (score > 1.0f) score = 1.0f;
        return {ModGuess::NFM_FM, score};
    }

    return {ModGuess::Unknown, 0.f};
}
