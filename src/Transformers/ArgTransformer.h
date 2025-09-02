#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>

class ArgTransformer {
public:
    std::vector<uint8_t> parseByteList(const std::string& input) const;
    std::vector<uint8_t> parseHexList(const std::string& input) const;
    std::vector<uint16_t> parseHexList16(const std::string& input) const;
    uint8_t parseByte(const std::string& str, int index = -1) const;
    uint8_t parseHexOrDec(const std::string& s) const ;
    uint16_t parseHexOrDec16(const std::string& str) const;
    uint32_t parseHexOrDec32(const std::string& s) const ;
    bool parseInt(const std::string& input, int& output);
    std::vector<std::string> splitArgs(const std::string& input);
    bool isValidNumber(const std::string& input);
    uint8_t toUint8(const std::string& input);
    uint32_t toUint32(const std::string& input);
    std::string toLower(const std::string& input);
    std::string filterPrintable(const std::string& input);
    std::string decodeEscapes(const std::string& input);
    int8_t toClampedInt8(const std::string& input);
    bool isValidSignedNumber(const std::string& input);
    std::string toHex(uint32_t value, int width = 2);
    std::string formatFloat(double value, int decimals);
    std::string toAsciiLine(uint32_t address, const std::vector<uint8_t>& line);
    std::string toAsciiLine(uint32_t startAddr, const std::vector<uint16_t>& words);
    bool parseMac(const std::string& s, std::array<uint8_t,6>& out);
    std::string ensureHttpScheme(std::string u);
    std::string normalizeLines(const std::string& in);
    std::string extractHostFromUrl(const std::string& url);
    std::vector<uint8_t> parse01List(const std::string& line);
    std::vector<uint8_t> packLsbFirst(const std::vector<uint8_t>& bits);
    bool unpackLsbFirst(const std::vector<uint8_t>& bytes,
                        size_t qty,
                        std::vector<uint8_t>& outBits);
    std::string toFixed2(float f);
};
