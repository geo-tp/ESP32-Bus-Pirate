#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

enum class SubGhzProtocolEnum : uint8_t {
    Unknown = 0,
    RAW,
    BinRAW,
    RcSwitch,
    Princeton,
};

class SubGhzProtocolEnumMapper {
public:
    static std::string toString(SubGhzProtocolEnum proto) {
        static const std::unordered_map<SubGhzProtocolEnum, std::string> map = {
            {SubGhzProtocolEnum::Unknown,  "Unknown"},
            {SubGhzProtocolEnum::RAW,      "RAW"},
            {SubGhzProtocolEnum::BinRAW,   "BinRAW"},
            {SubGhzProtocolEnum::RcSwitch, "RcSwitch"},
            {SubGhzProtocolEnum::Princeton,"Princeton"},
        };

        auto it = map.find(proto);
        return it != map.end() ? it->second : "Unknown Protocol";
    }

    static std::vector<std::string> getProtocolNames(const std::vector<SubGhzProtocolEnum>& protocols) {
        std::vector<std::string> names;
        names.reserve(protocols.size());
        for (auto p : protocols) names.push_back(toString(p));
        return names;
    }

    static SubGhzProtocolEnum fromString(const std::string& name) {
        static const std::unordered_map<std::string, SubGhzProtocolEnum> reverseMap = {
            // RAW
            {"RAW",        SubGhzProtocolEnum::RAW},

            // BinRAW
            {"BINRAW",     SubGhzProtocolEnum::BinRAW},
            {"BIN_RAW",    SubGhzProtocolEnum::BinRAW},
            {"BIN-RAW",    SubGhzProtocolEnum::BinRAW},

            // RcSwitch
            {"RCSWITCH",   SubGhzProtocolEnum::RcSwitch},
            {"RC_SWITCH",  SubGhzProtocolEnum::RcSwitch},
            {"RC-SWITCH",  SubGhzProtocolEnum::RcSwitch},

            // Princeton
            {"PRINCETON",  SubGhzProtocolEnum::Princeton},
            {"PT2262",     SubGhzProtocolEnum::Princeton},
        };

        std::string upper;
        upper.reserve(name.size());
        for (char c : name) upper += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

        auto it = reverseMap.find(upper);
        return it != reverseMap.end() ? it->second : SubGhzProtocolEnum::Unknown;
    }
};
