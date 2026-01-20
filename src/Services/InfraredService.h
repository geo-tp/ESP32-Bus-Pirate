#pragma once

#include <vector>
#include <Models/InfraredCommand.h>
#include <Models/InfraredFileRemoteCommand.h>
#include <Vendors/MakeHex.h>
#include "Enums/InfraredProtocolEnum.h"

class InfraredService {
public:
    enum class JamMode : uint8_t { CARRIER, SWEEP, RANDOM };
    void configure(uint8_t tx, uint8_t rx);
    void startReceiver();
    void stopReceiver();
    void sendInfraredCommand(InfraredCommand command);
    void sendInfraredFileCommand(InfraredFileRemoteCommand command);
    InfraredCommand receiveInfraredCommand();
    bool receiveRaw(std::vector<uint16_t>& timings, uint32_t& khz);
    void sendRaw(const std::vector<uint16_t>& timings, uint32_t khz);
    void sendJam(uint8_t modeIndex,
        uint16_t khz,
        uint32_t& sweepIndex,
        uint8_t density);
    std::vector<std::string> getCarrierStrings();
    static std::vector<std::string> getJamModeStrings();
private:        
    inline static constexpr uint16_t carrierKhz[] = {
        30, 33, 36, 38, 40, 42, 56
    };
    uint16_t getKaseikyoVendorIdCode(const std::string& input);
};



