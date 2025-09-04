// Adapted From Bruce: https://github.com/pr3y/Bruce

#pragma once

#include <map>
#include <stdint.h>
#include <vector>

inline constexpr const char* subghz_protocol_list[] = {
        " Came 12 Bit",
        " Nice 12 Bit",
        " Ansonic 12 Bit",
        " Holtek 12 Bit",
        " Linear 12 Bit",
        " Chamberlain 12 Bit",
    };

class c_rf_protocol {
public:
    std::map<char, std::vector<int>> transposition_table;
    std::vector<int> pilot_period;
    std::vector<int> stop_bit;

    c_rf_protocol() = default;
    virtual ~c_rf_protocol() = default;
};


class protocol_ansonic : public c_rf_protocol {
public:
    protocol_ansonic() {
        transposition_table['0'] = {-1111, 555};
        transposition_table['1'] = {-555, 1111};
        pilot_period = {-19425, 555};
        stop_bit = {};
    }
};

class protocol_came : public c_rf_protocol {
public:
    protocol_came() {
        transposition_table['0'] = {-320, 640};
        transposition_table['1'] = {-640, 320};
        pilot_period = {-11520, 320};
        stop_bit = {};
    }
};

class protocol_chamberlain : public c_rf_protocol {
public:
    protocol_chamberlain() {
        transposition_table['0'] = {-870, 430};
        transposition_table['1'] = {-430, 870};
        pilot_period = {};
        stop_bit = {-3000, 1000};
    }
};

class protocol_holtek : public c_rf_protocol {
public:
    protocol_holtek() {
        transposition_table['0'] = {-870, 430};
        transposition_table['1'] = {-430, 870};
        pilot_period = {-15480, 430};
        stop_bit = {};
    }
};

class protocol_liftmaster : public c_rf_protocol {
public:
    uint16_t timing_high = 800;
    uint16_t timing_low = 400;
};

class protocol_linear : public c_rf_protocol {
public:
    protocol_linear() {
        transposition_table['0'] = {500, -1500};
        transposition_table['1'] = {1500, -500};
        pilot_period = {};
        stop_bit = {1, -21500};
    }
};

class protocol_nice_flo : public c_rf_protocol {
public:
    protocol_nice_flo() {
        transposition_table['0'] = {-700, 1400};
        transposition_table['1'] = {-1400, 700};
        pilot_period = {-25200, 700};
        stop_bit = {};
    }
};
