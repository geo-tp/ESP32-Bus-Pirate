#include "ArgTransformer.h"


uint8_t ArgTransformer::parseByte(const std::string& str, int index) const {
    std::istringstream ss(str);
    int value = 0;

    if (str.rfind("0x", 0) == 0 || str.rfind("0X", 0) == 0)
        ss >> std::hex >> value;
    else
        ss >> std::dec >> value;

    if (ss.fail() || value < 0 || value > 255) {
        return 0xFF;
    }

    return static_cast<uint8_t>(value);
}

std::vector<uint8_t> ArgTransformer::parseByteList(const std::string& input) const {
    std::istringstream iss(input);
    std::string token;
    std::vector<uint8_t> bytes;

    int index = 0;
    while (iss >> token) {
        uint8_t parsed = parseByte(token, index);
        if (parsed == 0xFF && token != "0xFF" && token != "255") {
            continue; // ignore
        }
        bytes.push_back(parsed);
        index++;
    }

    return bytes;
}

std::vector<uint8_t> ArgTransformer::parseHexList(const std::string& input) const {
    std::vector<uint8_t> result;
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        try {
            // Convertir chaque token en uint8_t
            uint8_t value = static_cast<uint8_t>(std::stoul(token, nullptr, 16));
            result.push_back(value);
        } catch (...) {
        }
    }

    return result;
}

std::vector<uint16_t> ArgTransformer::parseHexList16(const std::string& input) const {
    std::vector<uint16_t> result;

    std::string normalized = input;
    for (char& c : normalized) {
        if (c == ',' || c == ';') c = ' ';
    }

    std::istringstream iss(normalized);
    std::string token;

    while (iss >> token) {
        try {
            size_t pos = 0;
            unsigned long v = std::stoul(token, &pos, 16);
            if (pos != token.size()) continue;
            if (v <= 0xFFFF) result.push_back(static_cast<uint16_t>(v));
        } catch (...) {
            // token invalide
        }
    }

    return result;
}


uint8_t ArgTransformer::parseHexOrDec(const std::string& str) const {
    if (str.empty()) return 0;

    int base = 10;
    const char* cstr = str.c_str();

    // Check Hex format
    if (str.size() > 2 && (str[0] == '0') && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;
        cstr += 2; // prefix 0x
    }

    // Check chars
    for (size_t i = 0; i < strlen(cstr); ++i) {
        char c = cstr[i];
        if (base == 10 && !isdigit(c)) return 0;
        if (base == 16 && !isxdigit(c)) return 0;
    }

    long value = strtol(str.c_str(), nullptr, base);
    if (value < 0 || value > 255) return 0;

    return static_cast<uint8_t>(value);
}

uint16_t ArgTransformer::parseHexOrDec16(const std::string& str) const {
    uint32_t value = parseHexOrDec32(str);
    return static_cast<uint16_t>(value & 0xFFFF);
}

uint32_t ArgTransformer::parseHexOrDec32(const std::string& str) const {
    if (str.empty()) return 0;

    int base = 10;
    const char* cstr = str.c_str();

    // Check Hex format
    if (str.size() > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        base = 16;
        cstr += 2;
    }

    // Check chars
    for (size_t i = 0; i < strlen(cstr); ++i) {
        char c = cstr[i];
        if (base == 10 && !isdigit(c)) return 0;
        if (base == 16 && !isxdigit(c)) return 0;
    }

    unsigned long value = strtoul(str.c_str(), nullptr, base);
    return static_cast<uint32_t>(value);
}



std::vector<std::string> ArgTransformer::splitArgs(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string token;
    while (iss >> token) {
        result.push_back(token);
    }
    return result;
}

bool ArgTransformer::parseInt(const std::string& input, int& output) {
    const char* str = input.c_str();
    char* end = nullptr;
    int base = 10;

    if (input.rfind("0x", 0) == 0 || input.rfind("0X", 0) == 0) {
        base = 16;
        str += 2;
    } else if (!input.empty() && (input.back() == 'h' || input.back() == 'H')) {
        base = 16;
        std::string trimmed = input.substr(0, input.size() - 1);
        str = trimmed.c_str();
    }

    long result = strtol(str, &end, base);
    if (*end != '\0') return false; // invalid characters found

    output = static_cast<int>(result);
    return true;
}

bool ArgTransformer::isValidNumber(const std::string& input) {
    std::string s = input;
    int base = 10;
    if (s.empty()) return false;

    if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) {
        base = 16;
        s = s.substr(2);
    }

    for (char c : s) {
        if ((base == 10 && !isdigit(c)) ||
            (base == 16 && !isxdigit(c))) {
            return false;
        }
    }

    return true;
}

bool ArgTransformer::isValidSignedNumber(const std::string& input) {
    if (input.empty()) return false;

    std::string s = input;
    int base = 10;
    size_t start = 0;

    // Optional sign
    if (s[0] == '-' || s[0] == '+') {
        if (s.size() == 1) return false;  // - or + alone is not valid
        start = 1;
    }

    // Check for hex prefix
    if (s.find("0x", start) == start || s.find("0X", start) == start) {
        base = 16;
        start += 2;
    }

    if (start >= s.size()) return false;

    for (size_t i = start; i < s.size(); ++i) {
        char c = s[i];
        if ((base == 10 && !isdigit(c)) || (base == 16 && !isxdigit(c))) {
            return false;
        }
    }

    return true;
}

uint8_t ArgTransformer::toUint8(const std::string& input) {
    return static_cast<uint8_t>(std::stoi(input));
}

int8_t ArgTransformer::toClampedInt8(const std::string& input) {
    int value = std::stoi(input);
    if (value < -127) value = -127;
    if (value > 127) value = 127;
    return static_cast<int8_t>(value);
}

uint32_t ArgTransformer::toUint32(const std::string& input) {
    return static_cast<uint32_t>(std::stoul(input));
}

std::string ArgTransformer::toLower(const std::string& input) {
    std::string result = input;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
    return result;
}

std::string ArgTransformer::filterPrintable(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isprint(static_cast<unsigned char>(c)) || c == '\n' || c == '\r' || c == '\t') {
            result += c;
        }
    }
    return result;
}

std::string ArgTransformer::decodeEscapes(const std::string& input) {
    std::ostringstream out;
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\\' && i + 1 < input.length()) {
            char next = input[++i];
            switch (next) {
                case 'n': out << '\n'; break;
                case 'r': out << '\r'; break;
                case 't': out << '\t'; break;
                case '0': out << '\0'; break;
                case '\\': out << '\\'; break;
                case 'x':
                    if (i + 2 < input.length()) {
                        std::string hex = input.substr(i + 1, 2);
                        try {
                            out << static_cast<char>(std::stoi(hex, nullptr, 16));
                        } catch (...) {
                            out << "\\x" << hex; // Invalid
                        }
                        i += 2;
                    } else {
                        out << "\\x"; // Incomplete
                    }
                    break;
                default:
                    out << '\\' << next; // Unrecognized,
                    break;
            }
        } else {
            out << input[i];
        }
    }
    return out.str();
}

std::string ArgTransformer::toHex(uint32_t value, int width) {
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(width) << value;
    return ss.str();
}

std::string ArgTransformer::formatFloat(double value, int decimals) {
    char buf[32];
    snprintf(buf, sizeof(buf), ("%." + std::to_string(decimals) + "f").c_str(), value);
    return std::string(buf);
}

std::string ArgTransformer::toAsciiLine(uint32_t address, const std::vector<uint8_t>& line) {
    std::stringstream ss;

    // Address
    ss << std::hex << std::uppercase << std::setfill('0')
       << std::setw(6) << address << ": ";

    // Hex
    for (size_t i = 0; i < 16; ++i) {
        if (i < line.size()) {
            ss << std::setw(2) << (int)line[i] << " ";
        } else {
            ss << "   ";
        }
    }

    // ASCII
    ss << " ";
    for (size_t i = 0; i < line.size(); ++i) {
        char c = static_cast<char>(line[i]);
        ss << (isprint(c) ? c : '.');
    }

    return ss.str();
}

std::string ArgTransformer::toAsciiLine(uint32_t startAddr, const std::vector<uint16_t>& words) {
    std::stringstream line;

    // Adress
    line << std::hex << std::uppercase << std::setfill('0')
         << std::setw(6) << startAddr << ": ";

    // HEX : 8 x 16-bit (4 hex digits)
    for (size_t i = 0; i < words.size(); ++i) {
        line << std::setw(4) << words[i] << " ";
    }

    // Padding
    for (size_t i = words.size(); i < 8; ++i) {
        line << "     ";
    }

    // ASCII
    line << " ";
    for (uint16_t word : words) {
        char high = (word >> 8) & 0xFF;
        char low  = word & 0xFF;

        line << (isprint(high) ? high : '.');
        line << (isprint(low)  ? low  : '.');
    }

    return line.str();
}

bool ArgTransformer::parseMac(const std::string& s, std::array<uint8_t,6>& out) {
    // Accept "AA:BB:CC:DD:EE:FF" or "AABBCCDDEEFF"
    std::string hex;
    hex.reserve(12);
    for (char c : s) {
        if (c == ':' || c == '-') continue;
        if (!isxdigit((unsigned char)c)) return false;
        hex.push_back(c);
    }
    if (hex.size() != 12) return false;

    auto hexVal = [](char c)->uint8_t {
        if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
        if (c >= 'a' && c <= 'f') return (uint8_t)(10 + c - 'a');
        if (c >= 'A' && c <= 'F') return (uint8_t)(10 + c - 'A');
        return 0xFF;
    };

    for (int i=0; i<6; ++i) {
        uint8_t hi = hexVal(hex[2*i]);
        uint8_t lo = hexVal(hex[2*i+1]);
        if (hi == 0xFF || lo == 0xFF) return false;
        out[i] = (uint8_t)((hi<<4) | lo);
    }
    return true;
}

std::string ArgTransformer::ensureHttpScheme(std::string u) {
    if (u.rfind("http://", 0) != 0 && u.rfind("https://", 0) != 0) {
        u = "https://" + u;   // defaut https
    }
    return u;
}

std::string ArgTransformer::normalizeLines(const std::string& in) {
    std::string out;
    out.reserve(in.size() * 2);
    for (size_t i = 0; i < in.size(); i++) {
        if (in[i] == '\n') {
            out += "\r\n";
        } else {
            out += in[i];
        }
    }
    return out;
}

std::string ArgTransformer::extractHostFromUrl(const std::string& url) {
    size_t s = url.find("://");
    std::string host = (s == std::string::npos) ? url : url.substr(s + 3);
    size_t slash = host.find('/');
    if (slash != std::string::npos) host.erase(slash);
    return host;
}

std::vector<uint8_t> ArgTransformer::parse01List(const std::string& line) {
  std::vector<uint8_t> bits;
  auto toks = splitArgs(line);
  if (toks.size() > 1) {
    for (auto& t : toks) {
      if (t == "0" || t == "1") bits.push_back(t == "1" ? 1 : 0);
      else return {};
    }
  } else {
    for (char c : line) {
      if (c=='0' || c=='1') bits.push_back(c=='1' ? 1 : 0);
    }
  }
  return bits;
}

std::vector<uint8_t> ArgTransformer::packLsbFirst(const std::vector<uint8_t>& bits) {
    std::vector<uint8_t> out((bits.size() + 7) / 8, 0);
    for (size_t i = 0; i < bits.size(); ++i) {
        if (bits[i] & 1) out[i / 8] |= (1u << (i % 8));
    }
    return out;
}

bool ArgTransformer::unpackLsbFirst(const std::vector<uint8_t>& bytes,
                                    size_t qty,
                                    std::vector<uint8_t>& outBits) {
    const size_t need = (qty + 7) / 8;
    if (bytes.size() < need) return false;    // too short
    outBits.resize(qty);
    for (size_t i = 0; i < qty; ++i) {
        outBits[i] = (bytes[i / 8] >> (i % 8)) & 0x1;
    }
    return true;
}

std::string ArgTransformer::toFixed2(float f) {
    std::ostringstream oss; oss.setf(std::ios::fixed); oss.precision(2); oss << f;
    return oss.str();
}