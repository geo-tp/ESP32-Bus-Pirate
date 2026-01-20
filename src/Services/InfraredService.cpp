#include <IRremote.hpp> // defined here to avoid conflicts
#include "InfraredService.h"

void InfraredService::configure(uint8_t tx, uint8_t rx) {
    IrSender.begin(tx);
    IrReceiver.begin(rx, ENABLE_LED_FEEDBACK);
}

void InfraredService::startReceiver() {
    IrReceiver.start();
}

void InfraredService::stopReceiver() {
    IrReceiver.stop();
}

void InfraredService::sendInfraredCommand(InfraredCommand command) {
    uint16_t vendorCode;
    std::string protocolString = InfraredProtocolMapper::toString(command.getProtocol());
    uint8_t device = command.getDevice();
    uint8_t subdevice = command.getSubdevice() == -1 ? 0 : command.getSubdevice();

    // Combine device and subdevice into a 16-bit integer
    uint16_t address = (static_cast<uint16_t>(subdevice) << 8) | device;

    switch (command.getProtocol()) {
        case InfraredProtocolEnum::_SAMSUNG:
        case InfraredProtocolEnum::SAMSUNG20: {
            IrSender.sendSamsung(address, command.getFunction(), 0);
            break;
        }

        case InfraredProtocolEnum::_PANASONIC:
        case InfraredProtocolEnum::PANASONIC2: {
            // Panasonic can be used by many manufacturers in the IRDB format, we check for vendor name
            vendorCode = getKaseikyoVendorIdCode(protocolString);
            
            IrSender.sendKaseikyo(address, command.getFunction(), 0, vendorCode);
            break;
        }

        // Same for Sony20
        case InfraredProtocolEnum::SONY20: {
            IrSender.sendSony(address, command.getFunction(), 0, SIRCS_20_PROTOCOL);  
            break;
        }

        // LEGO
        case InfraredProtocolEnum::LEGO: {
            IrSender.sendLegoPowerFunctions(address, command.getFunction(), 0);  
            break;
        }
        
        // Handle by MakeHex
        default: {
            int frequency = 38; // Default frequency, passed by reference to encodeRemoteCommand
            std::vector<float> sequence = encodeRemoteCommand(command, protocolString.c_str(), frequency);

            // Convert for sendRaw
            uint16_t* raw = new uint16_t[sequence.size()];
            for (size_t i = 0; i < sequence.size(); ++i) {
                raw[i] = static_cast<uint16_t>(sequence[i]);
            }
            
            // Send the raw generated sequence with the correct frequency
            IrSender.sendRaw(raw, sequence.size(), frequency);
            delete[] raw;
        }
    }
}

void InfraredService::sendInfraredFileCommand(InfraredFileRemoteCommand command) {

    if (command.protocol == InfraredProtocolEnum::RAW) {
        IrSender.sendRaw(command.rawData, command.rawDataSize, command.frequency);
        return;
    }

    uint8_t device = static_cast<uint8_t>(command.address & 0xFF);
    uint8_t sub = static_cast<uint8_t>((command.address >> 8) & 0xFF);
    auto finalCmd = InfraredCommand(
        command.protocol,
        device,
            sub,
            command.function
        );

    sendInfraredCommand(finalCmd);
}

InfraredCommand InfraredService::receiveInfraredCommand() {
    InfraredCommand command;

    if (!IrReceiver.decode()) {
        return command;
    }

    const IRData& data = IrReceiver.decodedIRData;
    IrReceiver.resume();

    // Ignore invalid
    if (data.numberOfBits == 0 || data.protocol == UNKNOWN) {
        return command;
    }

    // Mapping protocols
    switch (data.protocol) {
        case NEC:
            command.setProtocol(_NEC);
            break;
        case NEC2:
        case ONKYO:
            command.setProtocol(_NEC2);
            break;

        case DENON:
            command.setProtocol(_DENON);
            break;

        case APPLE:
            command.setProtocol(_APPLE);
            break;

        case SONY:
            command.setProtocol(SONY20);
            break;

        case JVC:
            command.setProtocol(_JVC);
            break;

        case RC5:
            command.setProtocol(_RC5);
            break;

        case BOSEWAVE:
        case BANG_OLUFSEN:
            command.setProtocol(BOSE);

        case RC6:
            command.setProtocol(_RC6);
            break;
        case LG:
        case LG2:
        case SAMSUNG:
            command.setProtocol(SAMSUNG20);
            break;

        case SAMSUNGLG:
        case SAMSUNG48:
            command.setProtocol(_SAMSUNG);
            break;

        case PANASONIC:
            command.setProtocol(_PANASONIC);
            break;

        case KASEIKYO_DENON:
            command.setProtocol(DENON_K);
            break;

        case KASEIKYO:
        case KASEIKYO_SHARP:
        case KASEIKYO_JVC:
        case KASEIKYO_MITSUBISHI:
            command.setProtocol(_KASEIKYO);
            break;

        case LEGO_PF:
            command.setProtocol(LEGO);
            break;

        default:
            return InfraredCommand();
    }

    // Panasonic and derived
    if (data.protocol == PANASONIC || data.protocol == KASEIKYO || data.protocol == KASEIKYO_JVC ||
        data.protocol == KASEIKYO_SHARP || data.protocol == KASEIKYO_MITSUBISHI || data.protocol == KASEIKYO_DENON) {
        // PANASONIC: 16 bits = 8b subdevice + 8b device
        command.setDevice((data.address >> 8) & 0xFF);
        command.setSubdevice(data.address & 0xFF);
        
    // Samsung
    } else if (data.protocol == SAMSUNG || data.protocol == SAMSUNGLG) {
        command.setDevice(data.address & 0xFF);
        command.setSubdevice((data.address >> 8) & 0xFF);

    } else {
        // que l'adresse
        int device = data.address & 0xFF;
        int subdevice = (data.address >> 8) & 0xFF;

        // Si subdevice est 0, ignore
        command.setDevice(device);
        command.setSubdevice(subdevice == 0 ? -1 : subdevice);
    }

    // Commande principale
    command.setFunction(data.command);

    return command;
}

bool InfraredService::receiveRaw(std::vector<uint16_t>& timings, uint32_t& khz) {
    if (!IrReceiver.decode()) {
        return false;
    }

    const IRData& data = IrReceiver.decodedIRData;
    const auto* raw = data.rawDataPtr;

    // Ensure we resume after processing
    struct ResumeGuard {
        ~ResumeGuard(){ IrReceiver.resume(); }
    } guard;

    if (raw == nullptr || raw->rawlen <= 1) {
        return false;
    }

    khz = 38; // TODO: default frequency, handle that ?

    timings.clear();
    timings.reserve(raw->rawlen - 1);

    for (uint16_t i = 1; i < raw->rawlen; i++) {
        // Convert ticks to microseconds
        uint32_t us = static_cast<uint32_t>(raw->rawbuf[i]) * MICROS_PER_TICK;
        timings.push_back(static_cast<uint16_t>(us));
    }

    return true;
}

void InfraredService::sendRaw(const std::vector<uint16_t>& timings, uint32_t khz) {
    if (timings.empty()) return;

    uint16_t* buf = new uint16_t[timings.size()];
    for (size_t i = 0; i < timings.size(); ++i) buf[i] = timings[i];

    IrSender.sendRaw(buf, timings.size(), khz);
    delete[] buf;
}

void InfraredService::sendJam(uint8_t modeIndex,
                                 uint16_t khz,
                                 uint32_t& sweepIndex,
                                 uint8_t density) {

    // Burst pattern
    static std::vector<uint16_t> pat;
    uint16_t halfPeriodUs = 500 / khz;
    if (halfPeriodUs < 1) halfPeriodUs = 1;

    if (pat.empty()) {
        pat.reserve(80);
        for (int i = 0; i < 40; ++i) {
            pat.push_back(halfPeriodUs);
            pat.push_back(halfPeriodUs);
        }
    }

    auto mode = static_cast<JamMode>(modeIndex);

    // Delay shrinks as density grows
    const uint16_t interBurstDelayUs = 100 / density + 2;
    const uint8_t sweepCount = sizeof(carrierKhz) / sizeof(carrierKhz[0]);

    for (uint8_t b = 0; b < density; ++b) {

        uint16_t f = khz;

        if (mode == JamMode::SWEEP) {
            f = carrierKhz[sweepIndex++ % sweepCount];
            sendRaw(pat, f);
        }
        else if (mode == JamMode::CARRIER) {
            sendRaw(pat, khz);
        }
        else { // RANDOM
            std::vector<uint16_t> rnd;
            rnd.reserve(30);
            for (int i = 0; i < 30; ++i) {
                rnd.push_back((uint16_t)random(5, 1001));
            }

            // Occasionally vary frequency
            if (random(10) < 3) {
                f = carrierKhz[random(0, sweepCount)];
            }

            sendRaw(rnd, f);
        }

        delayMicroseconds(interBurstDelayUs);
    }
}

uint16_t InfraredService::getKaseikyoVendorIdCode(const std::string& input) {
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);

    if (lowerInput.find("panasonic") != std::string::npos) {
        return PANASONIC_VENDOR_ID_CODE;
    } else if (lowerInput.find("denon") != std::string::npos) {
        return DENON_VENDOR_ID_CODE;
    } else if (lowerInput.find("mitsubishi") != std::string::npos) {
        return MITSUBISHI_VENDOR_ID_CODE;
    } else if (lowerInput.find("sharp") != std::string::npos) {
        return SHARP_VENDOR_ID_CODE;
    } else if (lowerInput.find("jvc") != std::string::npos) {
        return JVC_VENDOR_ID_CODE;
    } else {
        return PANASONIC_VENDOR_ID_CODE; // default
    }
}

std::vector<std::string> InfraredService::getCarrierStrings() {
    std::vector<std::string> out;
    out.reserve(sizeof(carrierKhz) / sizeof(carrierKhz[0]));

    for (uint16_t f : carrierKhz) {
        out.push_back(std::to_string(f));
    }
    return out;
}

std::vector<std::string> InfraredService::getJamModeStrings() {
    return {"carrier", "sweep", "random"};
}