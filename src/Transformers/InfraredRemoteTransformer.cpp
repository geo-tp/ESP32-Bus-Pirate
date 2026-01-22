#include "InfraredRemoteTransformer.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdint>

bool InfraredRemoteTransformer::isValidInfraredFile(const std::string& fileContent) {
    if (fileContent.empty()) return false;

    const char* p   = fileContent.c_str();
    const char* end = p + fileContent.size();

    // Find first line
    const char* line1_start = p;
    while (p < end && *p != '\n' && *p != '\r') ++p;
    const char* line1_end = p;
    // skip
    if (p < end && (*p == '\r')) ++p;
    if (p < end && (*p == '\n')) ++p;

    // Substring to find
    const char k1[] = "Filetype: IR";
    const size_t k1len = sizeof(k1) - 1;

    if ((size_t)(line1_end - line1_start) < k1len) return false;
    if (std::memcmp(line1_start, k1, k1len) != 0)  return false;

    return true;
}

std::vector<InfraredFileRemoteCommand> InfraredRemoteTransformer::transformFromFileFormat(const std::string& fileContent) {
    std::vector<InfraredFileRemoteCommand> commands;
    if (fileContent.empty()) return commands;

    InfraredFileRemoteCommand command; 

    const auto trim = [](const std::string& s) -> std::string {
        size_t i = 0, j = s.size();
        while (i < j && (s[i]==' '||s[i]=='\t'||s[i]=='\r'||s[i]=='\n')) ++i;
        while (j > i && (s[j-1]==' '||s[j-1]=='\t'||s[j-1]=='\r'||s[j-1]=='\n')) --j;
        return (i<j) ? s.substr(i, j-i) : std::string();
    };

    const char* p   = fileContent.c_str();
    const char* end = p + fileContent.size();

    while (p < end) {
        // Line boundaries [lineStart, lineEnd)
        const char* lineStart = p;
        while (p < end && *p != '\n') ++p;
        const char* lineEnd = p;
        if (p < end && *p == '\n') ++p;
        if (lineEnd > lineStart && *(lineEnd-1) == '\r') --lineEnd;

        // key:value ?
        const char* colon = lineStart;
        while (colon < lineEnd && *colon != ':') ++colon;
        if (colon == lineEnd) continue;

        // create small key/value strings (only this line)
        std::string key(lineStart, colon - lineStart);
        std::string val(colon + 1, lineEnd - (colon + 1));
        key = trim(key);
        val = trim(val);

        // prefix tests (equivalent to line.find("xxx:")==0)
        if (key == "name") {
            // push previous if complete
            if (!command.functionName.empty()) {
                commands.push_back(command);
            }
            command = InfraredFileRemoteCommand(); // reset
            command.functionName = val;
        }
        else if (key == "type" && val == "raw") {
            command.protocol = InfraredProtocolEnum::RAW;
        }
        else if (key == "protocol") {
            command.protocol = InfraredProtocolMapper::toEnum(val);
        }
        else if (key == "address") {
            command.address = convertHexToUint16(val, 2);
        }
        else if (key == "frequency") {
            // IRemote attend des kHz
            command.frequency = std::stoi(val) / 1000;
        }
        else if (key == "command") {
            command.function = convertHexToUint16(val, 1);
        }
        else if (key == "duty_cycle") {
            command.dutyCycle = static_cast<float>(std::atof(val.c_str()));
        }
        else if (key == "data") {
            size_t size = 0;
            uint16_t* rawData = convertDecToUint16Array(val, size);
            command.rawData = rawData;
            command.rawDataSize = size;
        }
    }

    // push last
    if (!command.functionName.empty()) {
        commands.push_back(command);
    }

    return commands;
}

std::string InfraredRemoteTransformer::transformToFileFormat(const std::string fileName, const std::vector<InfraredFileRemoteCommand>& cmds) {
    std::ostringstream out;

    out << "Filetype: IR\n";
    out << "Version: 1\n\n";

    out << "#\n";
    out << "# ";
    out << fileName;
    out << "\n#\n\n";

    for (size_t i = 0; i < cmds.size(); ++i) {
        const auto& c = cmds[i];

        out << "name: " << c.functionName << "\n";

        if (c.protocol == InfraredProtocolEnum::RAW) {
            out << "type: raw\n";

            out << "frequency: " << (static_cast<int>(c.frequency) * 1000) << "\n";

            out << "duty_cycle: " << c.dutyCycle << "\n";

            out << "data: ";
            for (size_t k = 0; k < c.rawDataSize; ++k) {
                if (k) out << ' ';
                out << static_cast<unsigned>(c.rawData[k]);
            }
            out << "\n";
        } else {
            out << "type: " << "parsed\n";
            out << "protocol: " << InfraredProtocolMapper::toString(c.protocol) << "\n";
            out << "address: " << toHexBytesLE(c.address, 2) << "\n";
            out << "command: " << toHexBytesLE(c.function, 1) << "\n";
        }

        if (i + 1 < cmds.size()) out << "\n";
    }

    return out.str();
}

std::vector<std::string> InfraredRemoteTransformer::extractFunctionNames(
    const std::vector<InfraredFileRemoteCommand>& cmds) 
{
    std::vector<std::string> names;
    names.reserve(cmds.size());

    for (const auto& cmd : cmds) {
        names.push_back(cmd.functionName);
    }
    return names;
}

uint16_t InfraredRemoteTransformer::convertHexToUint16(const std::string& hexString, size_t byteLimit) {
    std::istringstream iss(hexString);
    std::string tok;
    uint16_t result = 0;
    unsigned shift = 0;
    size_t count = 0;

    while (iss >> tok && count < byteLimit) {
        unsigned v = std::stoul(tok, nullptr, 16) & 0xFFu;
        result |= static_cast<uint16_t>(v) << shift;
        shift += 8;
        ++count;
    }
    return result;
}

uint16_t* InfraredRemoteTransformer::convertDecToUint16Array(const std::string& dataString, size_t& arraySize) {
    std::istringstream iss(dataString);
    std::string valueStr;
    std::vector<uint16_t> values;

    // Read each value from the string and convert it to uint16_t
    while (iss >> valueStr) {
        uint16_t value = static_cast<uint16_t>(std::stoul(valueStr, nullptr, 10));
        values.push_back(value);
    }

    arraySize = values.size(); // Store the size of the array
    uint16_t* resultArray = new uint16_t[arraySize];

    // Copy the values from the vector to the array
    for (size_t i = 0; i < arraySize; ++i) {
        resultArray[i] = values[i];
    }

    return resultArray;
}

std::string InfraredRemoteTransformer::hexByte(uint8_t b) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<unsigned>(b);
    return oss.str();
}

std::string InfraredRemoteTransformer::toHexBytesLE(uint16_t v, size_t byteCount) {
    uint8_t lo = static_cast<uint8_t>(v & 0xFFu);
    uint8_t hi = static_cast<uint8_t>((v >> 8) & 0xFFu);

    std::ostringstream oss;
    oss << InfraredRemoteTransformer::hexByte(lo) << " " << InfraredRemoteTransformer::hexByte(hi);
    return oss.str();
}