#include <Transformers/SubGhzTransformer.h>
#include <sstream>
#include <algorithm>
#include <cctype>

bool SubGhzTransformer::isValidSubGhzFile(const std::string& content) {
    if (content.empty()) return false;

    const char* p   = content.data();
    const char* end = p + content.size();

    // Skip UTF-8 BOM
    if (end - p >= 3 && (unsigned char)p[0]==0xEF && (unsigned char)p[1]==0xBB && (unsigned char)p[2]==0xBF) {
        p += 3;
    }

    // Find first line
    const char* q = p;
    while (q < end && *q != '\n' && *q != '\r') ++q;

    // Substring to find
    static constexpr char needle[] = "Filetype: Flipper SubGhz";
    constexpr size_t nlen = sizeof(needle) - 1;

    if ((size_t)(q - p) < nlen) return false;

    // Search in first line
    const char* last = q - nlen + 1;
    for (const char* s = p; s < last; ++s) {
        size_t i = 0;
        for (; i < nlen && s[i] == needle[i]; ++i) {}
        if (i == nlen) return true;
    }
    
    return false;
}

std::vector<SubGhzFileCommand> SubGhzTransformer::transformFromFileFormat(const std::string& fileContent,
                                           const std::string& sourcePath) {
    std::vector<SubGhzFileCommand> out;
    if (fileContent.empty()) return out;

    std::string protocolStr, presetStr;
    uint32_t frequency = 0;
    uint16_t te = 0;

    // Accumulateurs
    std::vector<int> bitList, bitRawList;
    std::vector<uint64_t> keyList;
    std::vector<std::string> rawDataLines;   // pour Protocol: RAW uniquement (timings signés)
    std::vector<uint8_t>     binRawBytes;    // pour Protocol: BinRAW (octets)

    // Buffer de lecture
    const char* p   = fileContent.c_str();
    const char* end = p + fileContent.size();

    while (p < end) {
        const char* lineStart = p;
        while (p < end && *p != '\n') ++p;
        const char* lineEnd = p;
        if (p < end && *p == '\n') ++p;
        if (lineEnd > lineStart && *(lineEnd - 1) == '\r') --lineEnd;

        const char* colon = lineStart;
        while (colon < lineEnd && *colon != ':') ++colon;
        if (colon == lineEnd) continue;

        std::string key(lineStart, colon - lineStart);
        std::string val(colon + 1, lineEnd - (colon + 1));
        trim(key);
        trim(val);
        if (key.empty()) continue;

        if (iequals(key, "Protocol")) {
            protocolStr = val;
        } else if (iequals(key, "Preset")) {
            presetStr = val;
        } else if (iequals(key, "Frequency")) {
            uint32_t hz = 0; if (parseUint32(val, hz)) frequency = hz;
        } else if (iequals(key, "TE")) {
            uint16_t tev = 0; if (parseUint16(val, tev)) te = tev;
        } else if (iequals(key, "Bit")) {
            int v = 0; if (parseInt(val, v)) bitList.push_back(v);
        } else if (iequals(key, "Bit_RAW")) {
            int v = 0; if (parseInt(val, v)) bitRawList.push_back(v);
        } else if (iequals(key, "Key")) {
            uint64_t k = 0; if (parseHexToU64(val, k)) keyList.push_back(k);
        } else if (iequals(key, "BinRAW")) {
            auto bytes = parseHexByteStream(val);
            if (!bytes.empty()) {
                binRawBytes.insert(binRawBytes.end(), bytes.begin(), bytes.end());
            }
        } else if (iequals(key, "RAW_Data") || iequals(key, "Data_RAW")) {
            // IMPORTANT :
            // - Bytes if Protocol: BinRAW
            // - Timings if Protocol: RAW
            if (iequals(protocolStr, "BinRAW")) {
                auto bytes = parseHexByteStream(val);
                if (!bytes.empty()) {
                    binRawBytes.insert(binRawBytes.end(), bytes.begin(), bytes.end());
                }
            } else {
                rawDataLines.push_back(val);
            }
        }
    }

    // Finalisation (ne crée plus de "compat RAW" quand Protocol != RAW)
    flushAccumulated(protocolStr, presetStr, frequency, te,
                     bitList, bitRawList, keyList,
                     rawDataLines,             // timings pour RAW
                     binRawBytes,              // octets pour BinRAW
                     out, sourcePath);

    return out;
}

void SubGhzTransformer::flushAccumulated(
    const std::string& protocolStr,
    const std::string& presetStr,
    uint32_t frequency,
    uint16_t te,
    const std::vector<int>& bitList,
    const std::vector<int>& bitRawList,
    const std::vector<uint64_t>& keyList,
    const std::vector<std::string>& rawDataLines,
    const std::vector<uint8_t>&     binRawBytes,
    std::vector<SubGhzFileCommand>& out,
    const std::string& sourcePath
) {
    const auto protocol = detectProtocol(protocolStr);
    const auto preset   = mapPreset(presetStr);

    // BINRAW
    if (protocol == SubGhzProtocolEnum::BinRAW) {
        if (!binRawBytes.empty()) {
            SubGhzFileCommand cmd;
            cmd.protocol        = SubGhzProtocolEnum::BinRAW;
            cmd.preset          = preset;
            cmd.frequency_hz    = frequency;
            cmd.te_us           = te;
            cmd.bitstream_bytes = binRawBytes;

            // Number of bits to send
            if (!bitList.empty())       cmd.bits = (uint16_t)std::max(0, bitList.back());
            else if (!bitRawList.empty()) cmd.bits = (uint16_t)std::max(0, bitRawList.back());
            else                          cmd.bits = (uint16_t)(binRawBytes.size() * 8);

            cmd.source_file     = sourcePath;
            out.emplace_back(std::move(cmd));
        }
        return;
    }

    // RAW
    if (protocol == SubGhzProtocolEnum::RAW) {
        if (!rawDataLines.empty()) {
            SubGhzFileCommand cmd;
            cmd.protocol      = SubGhzProtocolEnum::RAW;
            cmd.preset        = preset;
            cmd.frequency_hz  = frequency;
            cmd.te_us         = te;
            cmd.raw_timings   = parseRawTimingsJoined(rawDataLines);
            cmd.source_file   = sourcePath;
            if (!cmd.raw_timings.empty()) {
                out.emplace_back(std::move(cmd));
            }
        }
        return;
    }

    //  RcSwitch
    const bool protoKeyed =
        (protocol == SubGhzProtocolEnum::RcSwitch) ||
        (protocol == SubGhzProtocolEnum::Princeton) ||
        (protocol == SubGhzProtocolEnum::Unknown);

    if (!protoKeyed) {
        return;
    }

    // If we have keys, create one command per key
    if (!keyList.empty()) {
        uint16_t inferredBits = 0;
        if (!bitList.empty())      inferredBits = (uint16_t)std::max(0, bitList.back());
        else if (!bitRawList.empty()) inferredBits = (uint16_t)std::max(0, bitRawList.back());
        else {
            for (uint64_t k : keyList) {
                uint16_t b = 0;
                while ((k >> b) && b < 64) ++b;
                inferredBits = std::max(inferredBits, (uint16_t)std::max(1, (int)b));
            }
        }

        for (uint64_t k : keyList) {
            SubGhzFileCommand cmd;
            cmd.protocol      = (protocol == SubGhzProtocolEnum::Unknown)
                                ? SubGhzProtocolEnum::RcSwitch
                                : protocol;
            cmd.preset        = preset;
            cmd.frequency_hz  = frequency;
            cmd.te_us         = te;
            cmd.key           = k;
            cmd.bits          = inferredBits ? inferredBits : 24;
            cmd.source_file   = sourcePath;
            out.emplace_back(std::move(cmd));
        }
        return;
    }

    // No key, but bits
    if (!bitList.empty() || !bitRawList.empty()) {
        const int b = !bitList.empty() ? bitList.back() : bitRawList.back();
        SubGhzFileCommand cmd;
        cmd.protocol      = (protocol == SubGhzProtocolEnum::Unknown)
                            ? SubGhzProtocolEnum::RcSwitch
                            : protocol;
        cmd.preset        = preset;
        cmd.frequency_hz  = frequency;
        cmd.te_us         = te;
        cmd.bits          = (uint16_t)std::max(0, b);
        cmd.key           = 0;
        cmd.source_file   = sourcePath;
        out.emplace_back(std::move(cmd));
    }
}

std::vector<std::string>
SubGhzTransformer::extractSummaries(const std::vector<SubGhzFileCommand>& cmds) {
    std::vector<std::string> out;
    out.reserve(cmds.size());
    for (const auto& c : cmds) {
        std::ostringstream os;
        os << "[";
        switch (c.protocol) {
            case SubGhzProtocolEnum::RAW:     os << "RAW"; break;
            case SubGhzProtocolEnum::BinRAW:  os << "BinRAW"; break;
            case SubGhzProtocolEnum::RcSwitch:os << "RcSwitch"; break;
            case SubGhzProtocolEnum::Princeton:os << "Princeton"; break;
            default: os << "Unknown"; break;
        }
        os << "] " << (c.preset.empty() ? "<no preset>" : c.preset)
           << " @ " << c.frequency_hz << "Hz";

        if (c.protocol == SubGhzProtocolEnum::RAW) {
            os << " timings=" << c.raw_timings.size();
        } else if (c.protocol == SubGhzProtocolEnum::BinRAW) {
            os << " bytes=" << c.bitstream_bytes.size();
        } else {
            if (c.bits) os << " bits=" << c.bits;
            if (c.key)  os << " key=0x" << std::hex << c.key << std::dec;
            if (c.te_us) os << " te=" << c.te_us << "us";
        }
        out.push_back(os.str());
    }
    return out;
}

std::string SubGhzTransformer::mapPreset(const std::string& presetStr) {
    std::string p; p = presetStr;
    return p;
}

std::vector<int32_t> SubGhzTransformer::parseRawTimingsJoined(const std::vector<std::string>& lines) {
    std::vector<int32_t> out;
    out.reserve(1024);
    for (const auto& lnRaw : lines) {
        std::string ln = lnRaw;
        // Replace tabs with spaces for simplification
        std::replace(ln.begin(), ln.end(), '\t', ' ');
        std::istringstream iss(ln);
        std::string tok;
        while (iss >> tok) {
            int v=0;
            if (parseInt(tok, v)) {
                if (v != 0) { // ignore 0 terminator-style if present
                    out.push_back(v);
                }
            }
        }
    }
    return out;
}

std::vector<uint8_t> SubGhzTransformer::parseHexByteStream(const std::string& hexLine) {
    // Accept "A1 b2 0C" etc
    std::vector<uint8_t> out;
    std::istringstream iss(hexLine);
    std::string tok;
    while (iss >> tok) {
        uint64_t v64=0;
        if (!parseHexToU64(tok, v64) || v64 > 0xFFull) {
            return {};
        }
        out.push_back(static_cast<uint8_t>(v64));
    }
    return out;
}

SubGhzProtocolEnum SubGhzTransformer::detectProtocol(const std::string& protocolStr) {
    if (iequals(protocolStr, "RAW")) return SubGhzProtocolEnum::RAW;
    if (iequals(protocolStr, "BinRAW")) return SubGhzProtocolEnum::BinRAW;
    if (iequals(protocolStr, "RcSwitch") || iequals(protocolStr, "RCSwitch")) return SubGhzProtocolEnum::RcSwitch;
    if (startsWith(protocolStr, "Princeton")) return SubGhzProtocolEnum::Princeton;
    return SubGhzProtocolEnum::Unknown;
}

void SubGhzTransformer::trim(std::string& s) {
    size_t i = 0; while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r')) ++i;
    size_t j = s.size(); while (j > i && (s[j-1] == ' ' || s[j-1] == '\t' || s[j-1] == '\r')) --j;
    s.assign(s.begin() + i, s.begin() + j);
}

std::string SubGhzTransformer::trim_copy(const std::string& s) {
    std::string t = s; trim(t); return t;
}

bool SubGhzTransformer::startsWith(const std::string& s, const char* prefix) {
    const size_t n = std::char_traits<char>::length(prefix);
    return s.size() >= n && std::equal(prefix, prefix + n, s.begin());
}

bool SubGhzTransformer::iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i=0;i<a.size();++i) if (std::tolower(a[i]) != std::tolower(b[i])) return false;
    return true;
}

bool SubGhzTransformer::parseKeyValueLine(const std::string& line, std::string& keyOut, std::string& valOut) {
    auto pos = line.find(':');
    if (pos == std::string::npos) return false;
    keyOut = trim_copy(line.substr(0, pos));
    valOut = trim_copy(line.substr(pos + 1));
    return true;
}

bool SubGhzTransformer::parseUint32(const std::string& s, uint32_t& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    unsigned long v = std::strtoul(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0') return false;
    out = static_cast<uint32_t>(v);
    return true;
}

bool SubGhzTransformer::parseUint16(const std::string& s, uint16_t& out) {
    uint32_t tmp; if (!parseUint32(s, tmp) || tmp > 0xFFFFu) return false;
    out = static_cast<uint16_t>(tmp); return true;
}

bool SubGhzTransformer::parseInt(const std::string& s, int& out) {
    if (s.empty()) return false;
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    if (end == s.c_str() || *end != '\0') return false;
    out = static_cast<int>(v); return true;
}

bool SubGhzTransformer::parseHexToU64(const std::string& hexIn, uint64_t& out) {
    // Allow "0x...", "A1B2..." et espaces
    std::string s; s.reserve(hexIn.size());
    for (char c: hexIn) {
        if (c==' ' || c=='\t') continue;
        s.push_back(c);
    }
    if (s.size() >= 2 && s[0]=='0' && (s[1]=='x' || s[1]=='X')) s = s.substr(2);
    if (s.empty()) return false;
    // Valide chars
    for (char c: s) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
    }
    char* end = nullptr;
    // strtoull base 16
    unsigned long long v = std::strtoull(s.c_str(), &end, 16);
    if (end == s.c_str() || *end != '\0') return false;
    out = static_cast<uint64_t>(v);
    return true;
}