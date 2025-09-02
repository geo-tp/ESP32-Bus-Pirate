#pragma once

#include <ctype.h>
#include <cstdlib>

enum class SubGhzScanBand : uint8_t {
    Band300_348 = 0,
    Band387_464 = 1,
    Band779_928 = 2,
    BandAll     = 3
};

inline constexpr const char* kSubGhzScanBandNames[] = {
    " All ranges",
    " 300-348 MHz",
    " 387-464 MHz",
    " 779-928 MHz"
};

inline constexpr int kSubGhzRangeLimits[4][2] = {
    {0,  90},
    {0,  25},
    {26, 59},
    {60, 90}
};

inline constexpr float kSubGhzFreqList[] = {
    300.000f, 302.757f, 303.875f, 303.900f, 304.250f, 307.000f, 307.500f, 307.800f,
    309.000f, 310.000f, 312.000f, 312.100f, 312.200f, 313.000f, 313.850f, 314.000f,
    314.350f, 314.960f, 314.980f, 315.000f, 315.020f, 318.000f, 330.000f, 345.000f,
    348.000f, 350.000f,
    387.000f, 389.900f, 390.000f, 390.100f, 418.000f, 418.500f, 430.000f, 430.500f,
    431.000f, 431.500f, 433.050f, 433.075f, 433.220f, 433.420f, 433.657f, 433.800f,
    433.889f, 433.900f, 433.920f, 433.940f, 434.000f, 434.075f, 434.177f, 434.190f,
    434.390f, 434.420f, 434.500f, 434.620f, 434.775f, 434.790f, 438.900f, 440.175f,
    464.000f, 467.750f,
    779.000f,
    868.000f, 868.350f, 868.400f, 868.600f, 868.800f, 868.950f, 869.000f, 869.200f,
    869.400f, 869.500f, 869.650f, 869.700f, 869.850f,
    902.000f, 904.000f, 906.400f, 908.000f, 910.000f, 912.000f, 914.000f, 915.000f,
    916.000f, 918.000f, 920.000f, 922.000f, 924.000f, 925.000f, 926.000f, 927.500f,
    928.000f
};
inline constexpr size_t kSubGhzFreqListSize = sizeof(kSubGhzFreqList) / sizeof(kSubGhzFreqList[0]);
