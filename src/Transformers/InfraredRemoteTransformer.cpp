#include "InfraredRemoteTransformer.h"


bool InfraredRemoteTransformer::isValidInfraredFile(const std::string& fileContent) {
    std::string line;

    if (fileContent.empty()) {
        return false;
    }

    // Split the fileContent into lines
    std::istringstream stream(fileContent);
    size_t i = 1;
    while (std::getline(stream, line) && i < 3) {
        if (line.find("Filetype:") != 0 && i == 1) {
            return false;
        }
        if (line.find("Version:") != 0 && i == 2) {
            return false;
        }
        i++;
    }

    return true;
}

std::vector<InfraredFileRemoteCommand> InfraredRemoteTransformer::transformFromFileFormat(const std::string& fileContent) {
    std::vector<InfraredFileRemoteCommand> commands;
    InfraredFileRemoteCommand command;
    std::vector<std::string> lines;
    std::string value;
    std::string line;
    int count = 0;

    // Split the fileContent into lines
    std::istringstream stream(fileContent);
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    // Iterate over each line
    // Helper lambda to trim whitespace from both ends of a string
    auto trim = [](const std::string& s) -> std::string {
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    };

    for (const auto& line : lines) {
        value = line.substr(line.find(":") + 1);
        value = trim(value);


        if (line.find("name:") == 0) {

            // We push the existing command if any
            if (!command.functionName.empty()) {
                commands.push_back(command);
            }

            // We reset command for the new one
            command = InfraredFileRemoteCommand();
            command.functionName = value;
        }
        if (line.find("type:") == 0 && value == "raw") {
            command.protocol = InfraredProtocolEnum::RAW;
        }
        if (line.find("protocol:") == 0) {
            command.protocol = InfraredProtocolMapper::toEnum(value);
        }
        if (line.find("address:") == 0) {
            command.address = convertHexToUint16(value, 2);
        }
        if (line.find("frequency:") == 0) {
            command.frequency = std::stoi(value) / 1000; // IRemote needs frequency in KHZ
        }
        if (line.find("command:") == 0) {
            command.function = convertHexToUint16(value, 1);
        }
        if (line.find("duty_cycle:") == 0) {
            command.dutyCycle = std::stof(value);
        }
        if (line.find("data:") == 0) {
            size_t size;
            uint16_t* rawData = convertDecToUint16Array(value, size);
            command.rawData = rawData;
            command.rawDataSize = size;
        }

    }

    // Don't forget to push the last command to the vector
    if (!command.functionName.empty()) {
        commands.push_back(command);
    }

    return commands;
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
    std::string byteStr;
    uint16_t result = 0;
    int byteCount = 0;

    // Vector to hold the bytes in reverse order
    std::vector<uint8_t> bytes;

    // Read each byte from the string and store in the vector
    while (iss >> byteStr && byteCount < byteLimit) {
        // Convert the byte string to an integer and store it in the vector
        uint8_t byte = std::stoul(byteStr, nullptr, 16);
        bytes.push_back(byte);
        byteCount++;
    }

    // Combine the bytes in reverse order
    for (int i = byteCount - 1; i >= 0; --i) {
        result = (result << 8) | bytes[i];
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