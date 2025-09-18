#include "CanService.h"
#include <SPI.h>
#include <cstdio>
#include <cstring>

void CanService::configure(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi, uint32_t bitrateKbps) {
    // Save to use with reset()
    csPin = cs;
    sckPin = sck;
    misoPin = miso;
    mosiPin = mosi;
    kbps = bitrateKbps;

    reset();
}

void CanService::reset() {
    SPI.end();
    delay(10);
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
    delay(50);
    mcp2515.reset();
    mcp2515.setBitrate(resolveBitrate(kbps));
    mcp2515.setNormalMode();
}

void CanService::end() {
    SPI.end();
}

bool CanService::sendFrame(uint32_t id, const std::vector<uint8_t>& data) {
    struct can_frame frame;
    frame.can_id = id;
    frame.can_dlc = data.size();
    memcpy(frame.data, data.data(), data.size());
    return mcp2515.sendMessage(&frame) == MCP2515::ERROR_OK;
}

bool CanService::readFrame(struct can_frame& outFrame) {
    if (!mcp2515.checkReceive()) return false;
    return mcp2515.readMessage(&outFrame) == MCP2515::ERROR_OK;
}

std::string CanService::readFrameAsString() {
    struct can_frame frame;
    if (!readFrame(frame)) return "";

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "| ID: 0x%03X | DLC: %d | Data:", frame.can_id, frame.can_dlc);

    std::string result = buffer;
    for (int i = 0; i < frame.can_dlc; ++i) {
        snprintf(buffer, sizeof(buffer), " %02X", frame.data[i]);
        result += buffer;
    }
    return result;
}

std::string CanService::getStatus() {
    uint8_t status = mcp2515.getStatus();
    uint8_t interrupts = mcp2515.getInterrupts();
    uint8_t errors = mcp2515.getErrorFlags();
    std::string result;

    // --- Status bits ---
    result += "   Status bits:";
    bool hasStatus = false;
    if (status & MCP2515::CANINTF_RX0IF) { result += " RX0_HAS_MSG"; hasStatus = true; }
    if (status & MCP2515::CANINTF_RX1IF) { result += " RX1_HAS_MSG"; hasStatus = true; }
    if (status & MCP2515::CANINTF_TX0IF) { result += " TX0_REQ"; hasStatus = true; }
    if (status & MCP2515::CANINTF_TX1IF) { result += " TX1_REQ"; hasStatus = true; }
    if (status & MCP2515::CANINTF_TX2IF) { result += " TX2_REQ"; hasStatus = true; }
    if (status & MCP2515::CANINTF_ERRIF) { result += " ERR_INT"; hasStatus = true; }
    if (status & MCP2515::CANINTF_WAKIF) { result += " WAKE_INT"; hasStatus = true; }
    if (status & MCP2515::CANINTF_MERRF) { result += " MERR_INT"; hasStatus = true; }
    if (!hasStatus) result += " NONE";
    result += "\n\r";

    // --- Interrupts ---
    result += "   Interrupts:";
    bool hasInterrupt = false;
    if (interrupts & MCP2515::CANINTF_RX0IF) { result += " RX0"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_RX1IF) { result += " RX1"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_TX0IF) { result += " TX0"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_TX1IF) { result += " TX1"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_TX2IF) { result += " TX2"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_ERRIF) { result += " ERR"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_WAKIF) { result += " WAKE"; hasInterrupt = true; }
    if (interrupts & MCP2515::CANINTF_MERRF) { result += " MERR"; hasInterrupt = true; }
    if (!hasInterrupt) result += " NONE";
    result += "\n\r";

    // --- Error flags ---
    result += "   Error Flags:";
    bool hasError = false;
    if (errors & MCP2515::EFLG_RX0OVR) { result += " RX0_OVR"; hasError = true; }
    if (errors & MCP2515::EFLG_RX1OVR) { result += " RX1_OVR"; hasError = true; }
    if (errors & MCP2515::EFLG_TXBO)   { result += " TX_BUS_OFF"; hasError = true; }
    if (errors & MCP2515::EFLG_TXEP)   { result += " TX_PASSIVE"; hasError = true; }
    if (errors & MCP2515::EFLG_RXEP)   { result += " RX_PASSIVE"; hasError = true; }
    if (errors & MCP2515::EFLG_TXWAR)  { result += " TX_WARNING"; hasError = true; }
    if (errors & MCP2515::EFLG_RXWAR)  { result += " RX_WARNING"; hasError = true; }
    if (errors & MCP2515::EFLG_EWARN)  { result += " ERROR_WARN"; hasError = true; }
    if (!hasError) result += " NO_ERROR";
    result += "\n\r";

    // --- Error counters ---
    char errcounters[64];
    snprintf(errcounters, sizeof(errcounters), "   TX errors: %u \n\r   RX errors: %u\n",
             mcp2515.errorCountTX(), mcp2515.errorCountRX());
    result += errcounters;

    return result;
}

void CanService::setFilter(uint32_t id) {
    // Switch MCP2515 to configuration mode
    mcp2515.setConfigMode();

    // Apply full mask on both
    mcp2515.setFilterMask(MCP2515::MASK0, false, 0x7FF);
    mcp2515.setFilterMask(MCP2515::MASK1, false, 0x7FF);

    // Apply same filter ID to all filters
    mcp2515.setFilter(MCP2515::RXF0, false, id);
    mcp2515.setFilter(MCP2515::RXF1, false, id);
    mcp2515.setFilter(MCP2515::RXF2, false, id);
    mcp2515.setFilter(MCP2515::RXF3, false, id);
    mcp2515.setFilter(MCP2515::RXF4, false, id);
    mcp2515.setFilter(MCP2515::RXF5, false, id);

    // Return to normal mode
    mcp2515.setNormalMode();
}

void CanService::setMask(uint32_t mask) {
    mcp2515.setFilterMask(MCP2515::MASK0, false, mask);
}

void CanService::setBitrate(uint32_t bitrateKbps) {
    mcp2515.setBitrate(resolveBitrate(bitrateKbps));
    mcp2515.setNormalMode();
}

void CanService::flush() {
    unsigned long start = millis();
    struct can_frame tmp;

    while (millis() - start < 10) {
        if (!readFrame(tmp)) break;
    }
}

CAN_SPEED CanService::resolveBitrate(uint32_t kbps) {
    switch (kbps) {
        case 5:    return CAN_5KBPS;
        case 10:   return CAN_10KBPS;
        case 20:   return CAN_20KBPS;
        case 31:   return CAN_31K25BPS;
        case 33:   return CAN_33KBPS;
        case 40:   return CAN_40KBPS;
        case 50:   return CAN_50KBPS;
        case 80:   return CAN_80KBPS;
        case 100:  return CAN_100KBPS;
        case 125:  return CAN_125KBPS;
        case 200:  return CAN_200KBPS;
        case 250:  return CAN_250KBPS;
        case 500:  return CAN_500KBPS;
        case 1000: return CAN_1000KBPS;
        default:   return CAN_125KBPS;
    }
}

uint32_t CanService::closestSupportedBitrate(uint32_t kbps) {
    const uint32_t supported[] = {
        5, 10, 20, 31, 33, 40, 50, 80, 100, 125, 200, 250, 500, 1000
    };

    uint32_t best = supported[0];
    uint32_t minDiff = abs((int32_t)(kbps - best));

    for (size_t i = 1; i < sizeof(supported)/sizeof(supported[0]); ++i) {
        uint32_t diff = abs((int32_t)(kbps - supported[i]));
        if (diff < minDiff) {
            best = supported[i];
            minDiff = diff;
        }
    }

    return best;
}

bool CanService::probe() {
    // Loopback test
    if (mcp2515.setLoopbackMode() != MCP2515::ERROR_OK) return false;

    // Prepare a test frame
    struct can_frame f{};
    f.can_id  = 0x123;
    f.can_dlc = 2;
    f.data[0] = 0xAA;
    f.data[1] = 0xAA;

    // Send a test frame
    if (mcp2515.sendMessage(&f) != MCP2515::ERROR_OK) {
        mcp2515.setNormalMode();
        return false;
    }

    // Receive it back
    unsigned long t0 = millis();
    while (millis() - t0 < 20) {
        if (mcp2515.checkReceive()) {
            struct can_frame rx{};
            if (mcp2515.readMessage(&rx) == MCP2515::ERROR_OK &&
                rx.can_id == f.can_id && rx.can_dlc == f.can_dlc &&
                rx.data[0] == f.data[0] && rx.data[1] == f.data[1]) {
                mcp2515.setNormalMode();
                return true; // CAN bus OK, we received what we sent
            }
        }
        delay(1);
    }
    mcp2515.setNormalMode();
    return false;
}
