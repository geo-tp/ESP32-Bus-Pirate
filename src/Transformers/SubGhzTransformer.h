#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <cctype>
#include "Enums/SubGhzProtocolEnum.h"

struct SubGhzFileCommand {
    uint32_t            frequency_hz {0};   // ex: 433920000
    uint16_t            te_us {0};          // pulse length 
    std::string         preset {};
    SubGhzProtocolEnum  protocol {SubGhzProtocolEnum::Unknown};

    // RcSwitch / Princeton
    uint16_t            bits {0};           // number of bits
    uint64_t            key {0};            // value already decoded

    // RAW
    std::vector<int32_t> raw_timings;       // us

    // BinRAW
    std::vector<uint8_t> bitstream_bytes;   // byte sequence

    std::string         source_file;
};

class SubGhzTransformer {
public:

    // Validate sub file
    bool isValidSubGhzFile(const std::string& fileContent);

    // Transform from .sub file content to commands
    std::vector<SubGhzFileCommand> transformFromFileFormat(const std::string& fileContent, const std::string& sourcePath = {});

    // Extract readable summaries of commands
    std::vector<std::string> extractSummaries(const std::vector<SubGhzFileCommand>& cmds);

private:
    // Helpers
    static void trim(std::string& s);
    static std::string trim_copy(const std::string& s);
    static bool startsWith(const std::string& s, const char* prefix);
    static bool iequals(const std::string& a, const std::string& b);

    static bool parseKeyValueLine(const std::string& line, std::string& keyOut, std::string& valOut);
    static bool parseUint32(const std::string& s, uint32_t& out);
    static bool parseUint16(const std::string& s, uint16_t& out);
    static bool parseInt(const std::string& s, int& out);
    static bool parseHexToU64(const std::string& hex, uint64_t& out);
    std::string mapPreset(const std::string& presetStr);

    // RAW: “123 456 789 …”
    std::vector<int32_t> parseRawTimingsJoined(const std::vector<std::string>& rawLines);

    // BinRAW: "00 01 02 FF" → octets
    std::vector<uint8_t> parseHexByteStream(const std::string& hexLine);

    // Finalizer
    void flushAccumulated(
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
    );

    SubGhzProtocolEnum detectProtocol(const std::string& protocolStr);
};
