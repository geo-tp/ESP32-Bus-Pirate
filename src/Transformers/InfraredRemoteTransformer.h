#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <Models/InfraredFileRemoteCommand.h>
#include <Enums/InfraredProtocolEnum.h>

class InfraredRemoteTransformer {
public:
    static bool isValidInfraredFile(const std::string& fileContent);
    static std::vector<InfraredFileRemoteCommand> transformFromFileFormat(const std::string& fileContent);
    static std::string transformToFileFormat(const std::string fileName, const std::vector<InfraredFileRemoteCommand>& cmds);
    static std::vector<std::string> extractFunctionNames(const std::vector<InfraredFileRemoteCommand>& cmds);
private:
    static uint16_t convertHexToUint16(const std::string& hexString, size_t byteLimit);
    static uint16_t* convertDecToUint16Array(const std::string& decString, size_t& arraySize);
    static std::string hexByte(uint8_t b);
    static std::string toHexBytesLE(uint16_t v, size_t byteCount);
};
