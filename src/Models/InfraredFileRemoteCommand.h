#pragma once

#include <stddef.h>
#include <string>
#include <vector>
#include <Enums/InfraredProtocolEnum.h>

typedef struct {
    std::string functionName;
    InfraredProtocolEnum protocol;
    uint16_t address;
    uint8_t function; // command
    uint16_t* rawData; // unused if protocol != RAW
    size_t rawDataSize; // unused if protocol != RAW
    int frequency; // unused if protocol != RAW
    float dutyCycle; // unused if protocol != RAW
} InfraredFileRemoteCommand;


// Operator == 
inline bool operator==(const InfraredFileRemoteCommand& lhs, const InfraredFileRemoteCommand& rhs) {
    if (lhs.functionName != rhs.functionName) return false;
    if (lhs.protocol != rhs.protocol) return false;
    if (lhs.address != rhs.address) return false;
    if (lhs.function != rhs.function) return false;
    if (lhs.rawDataSize != rhs.rawDataSize) return false;
    if (lhs.frequency != rhs.frequency) return false;
    if (lhs.dutyCycle != rhs.dutyCycle) return false;

    // Compare rawData only if it is used (protocol == RAW)
    if (lhs.protocol == InfraredProtocolEnum::RAW) {
        if (lhs.rawDataSize > 0 && rhs.rawDataSize > 0) {
            for (size_t i = 0; i < 20; ++i) {
                if (lhs.rawData[i] != rhs.rawData[i]) {
                    return false;
                }
            }
        } else if (lhs.rawDataSize != rhs.rawDataSize) {
            return false;
        }
    }

    return true;
}

// Operator !=
inline bool operator!=(const InfraredFileRemoteCommand& lhs, const InfraredFileRemoteCommand& rhs) {
    return !(lhs == rhs);
}
