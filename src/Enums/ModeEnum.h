#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

enum class ModeEnum {
    None = -1,
    HIZ,
    OneWire,
    UART,
    HDUART,
    I2C,
    SPI,
    TwoWire,
    ThreeWire,
    DIO,
    LED,
    Infrared,
    USB,
    Bluetooth,
    WiFi,
    JTAG,
    I2S,
    CAN_,
    ETHERNET,
    SUBGHZ,
    RFID,
    RF24_,
    COUNT
};

class ModeEnumMapper {
public:
    static std::string toString(ModeEnum proto) {
        static const std::unordered_map<ModeEnum, std::string> map = {
            {ModeEnum::None,       "None"},
            {ModeEnum::HIZ,        "HIZ"},
            {ModeEnum::OneWire,    "1WIRE"},
            {ModeEnum::UART,       "UART"},
            {ModeEnum::HDUART,     "HDUART"},
            {ModeEnum::I2C,        "I2C"},
            {ModeEnum::SPI,        "SPI"},
            {ModeEnum::TwoWire,    "2WIRE"},
            {ModeEnum::ThreeWire,  "3WIRE"},
            {ModeEnum::DIO,        "DIO"},
            {ModeEnum::LED,        "LED"},
            {ModeEnum::Infrared,   "INFRARED"},
            {ModeEnum::USB,        "USB"},
            {ModeEnum::Bluetooth,  "BLUETOOTH"},
            {ModeEnum::WiFi,       "WIFI"},
            {ModeEnum::JTAG,       "JTAG"},
            {ModeEnum::I2S,        "I2S"},
            {ModeEnum::CAN_,        "CAN"},
            {ModeEnum::ETHERNET,   "ETHERNET"},
            {ModeEnum::SUBGHZ,     "SUBGHZ"},
            {ModeEnum::RFID,       "RFID"},
            {ModeEnum::RF24_,       "RF24"}
        };

        auto it = map.find(proto);
        return it != map.end() ? it->second : "Unknown Protocol";
    }

    static std::vector<std::string> getProtocolNames(const std::vector<ModeEnum>& protocols) {
        std::vector<std::string> names;
        for (const auto& proto : protocols) {
            names.push_back(toString(proto));
        }
        return names;
    }

    static ModeEnum fromString(const std::string& name) {
        static const std::unordered_map<std::string, ModeEnum> reverseMap = {
            {"HIZ",        ModeEnum::HIZ},
            {"1WIRE",      ModeEnum::OneWire},
            {"UART",       ModeEnum::UART},
            {"HDUART",     ModeEnum::HDUART},
            {"I2C",        ModeEnum::I2C},
            {"SPI",        ModeEnum::SPI},
            {"2WIRE",      ModeEnum::TwoWire},
            {"3WIRE",      ModeEnum::ThreeWire},
            {"DIO",        ModeEnum::DIO},
            {"LED",        ModeEnum::LED},
            {"INFRARED",   ModeEnum::Infrared},
            {"USB",        ModeEnum::USB},
            {"BLUETOOTH",  ModeEnum::Bluetooth},
            {"WIFI",       ModeEnum::WiFi},
            {"JTAG",       ModeEnum::JTAG},
            {"I2S",        ModeEnum::I2S},
            {"CAN",        ModeEnum::CAN_},
            {"ETHERNET",   ModeEnum::ETHERNET},
            {"SUBGHZ",     ModeEnum::SUBGHZ},
            {"RFID",       ModeEnum::RFID},
            {"RF24",       ModeEnum::RF24_}
        };

        std::string upper;
        upper.reserve(name.size());
        for (char c : name) {
            upper += std::toupper(static_cast<unsigned char>(c));
        }

        auto it = reverseMap.find(upper);
        return it != reverseMap.end() ? it->second : ModeEnum::None;
    }
};
