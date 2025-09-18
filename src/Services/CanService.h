#pragma once

#include <string>
#include <vector>
#include <mcp2515.h>
#include <Arduino.h>

// Must be global to work, cs pin needs to be set at compile time
#ifdef DEVICE_TEMBEDS3CC1101
    MCP2515 mcp2515 = MCP2515(CAN_CS_PIN, 10000000, &SPI); // avoid screen conflicts
#else 
    MCP2515 mcp2515 = MCP2515(CAN_CS_PIN);
#endif

class CanService {
public:
    void configure(uint8_t csPin, uint8_t sck, uint8_t miso, uint8_t mosi, uint32_t bitrateKbps = 125);
    void end();
    void reset();
    void flush();

    bool sendFrame(uint32_t id, const std::vector<uint8_t>& data);
    bool readFrame(struct can_frame& outFrame);
    std::string readFrameAsString();  // pour affichage

    void setBitrate(uint32_t bitrateKbps);
    uint32_t closestSupportedBitrate(uint32_t kbps);
    void setFilter(uint32_t id);
    void setMask(uint32_t mask);

    std::string getStatus();
    bool probe();
    
private:
    CAN_SPEED resolveBitrate(uint32_t kbps);
    uint8_t csPin, sckPin, misoPin, mosiPin;
    uint32_t kbps;

};
