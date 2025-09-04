#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "Interfaces/ITerminalView.h"
#include "Interfaces/IInput.h"
#include "driver/rmt.h"

enum class RfEncoding { Unknown, PulseLength, Manchester, PWM };

struct SubGhzDetectResult {
    RfEncoding   encoding = RfEncoding::Unknown;
    float        baseT_us = 0.f;        // estimate period (µs)
    float        bitrate_kbps = 0.f;
    int          bitCount = 0;          // useful bits if decoded
    std::string  payloadHex;            // payload in hex if decoded
    std::string  protocolGuess;
    float        confidence = 0.f;      // 0..1
    std::string  notes;                 // free text
};

struct FrequencyActivity {
    // Inputs
    int   thresholdDbm   = -80;
    int   windows        = 0;
    int   hits           = 0;     // number of windows with peak >= threshold
    float hitRatio       = 0.f;   // hits / windows

    // Stats RSSI
    int   peakDbm        = -127;
    int   minDbm         =  127;
    int   maxDbm         = -127;
    float avgDbm         = -127.f;
    float stddevDbm      = 0.f;

    // Score
    float confidence     = 0.f;

    // Optional
    std::string reason;
};

class SubGhzAnalyzeManager {
public:
    enum class ModGuess { ASK_OOK, FSK_GFSK, NFM_FM, Unknown };

    // Analyse frame
    std::string analyzeFrame(const std::vector<rmt_item32_t>& items,
                                    float tickPerUs = 1.0f);

    // Analyze activity
    std::string analyzeFrequencyActivity(int dwellMs,
                                         int windowMs,
                                         int thresholdDbm,
                                         const std::function<int(int)>& measure,
                                         const std::function<bool()>& shouldAbort = nullptr,
                                         float neighborLeftConf = 0.f,
                                         float neighborRightConf = 0.f);


private:
    std::string formatFrame(const SubGhzDetectResult& r) const;
    std::string formatFrequency(int peakDbm,
                                int hits,
                                int windows,
                                float avgDbm,
                                float stddevDbm,
                                float hitRatio,
                                float confidence,
                                std::string mod);

    void   collectDurations(const std::vector<rmt_item32_t>& items, float tickPerUs,
                                   std::vector<uint32_t>& highs, std::vector<uint32_t>& lows);
    float  median(std::vector<uint32_t> v);
    float  estimateBaseT(const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows);

    // Protocols prediction
    bool   looksManchester(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows);
    bool   looksPulseLength(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows, float& ratioOut);
    bool   looksPWM(float T, const std::vector<uint32_t>& highs, const std::vector<uint32_t>& lows);

    // Decode
    bool   decodePT2262Like(float T, const std::vector<rmt_item32_t>& items, float tickPerUs,
                                   std::string& hexOut, int& bitCountOut);
    std::pair<ModGuess, float>  decodeModulationRSSI(const std::vector<int>& samples);

    // Utils
    std::string bitsToHex(const std::string& bits);
    float       clamp01(float v);
    bool        nearf(float a, float b, float tol);
};
