#pragma once
#include <cstdint>
#include <ctype.h>

struct FlashChipInfo {
    uint8_t manufacturerId;
    uint8_t memoryType;
    uint8_t capacityCode;
    const char* manufacturerName;
    const char* modelName;
    uint32_t capacityBytes;
};

static constexpr FlashChipInfo flashDatabase[] = {
    // Winbond
    {0xEF, 0x40, 0x14, "Winbond",  "W25X80",    1UL << 20},
    {0xEF, 0x40, 0x15, "Winbond",  "W25X16",    2UL << 20},
    {0xEF, 0x40, 0x16, "Winbond",  "W25Q32",    4UL << 20},
    {0xEF, 0x40, 0x17, "Winbond",  "W25Q64",    8UL << 20},
    {0xEF, 0x40, 0x18, "Winbond",  "W25Q128",  16UL << 20},
    {0xEF, 0x40, 0x19, "Winbond",  "W25Q256",  32UL << 20},
    {0xEF, 0x40, 0x13, "Winbond",  "W25X40",     512UL << 10},
    {0xEF, 0x40, 0x12, "Winbond",  "W25X20",     256UL << 10},
    {0xEF, 0x40, 0x11, "Winbond",  "W25X10",     128UL << 10},

    // Macronix
    {0xC2, 0x20, 0x15, "Macronix", "MX25L1606E", 2UL << 20},
    {0xC2, 0x20, 0x16, "Macronix", "MX25L3206E", 4UL << 20},
    {0xC2, 0x20, 0x17, "Macronix", "MX25L6406E", 8UL << 20},
    {0xC2, 0x20, 0x18, "Macronix", "MX25L12835F",16UL << 20},
    {0xC2, 0x20, 0x18, "Macronix", "MX25L12805D", 16UL << 20},
    {0xC2, 0x20, 0x14, "Macronix", "MX25L8005",   1UL << 20},

    // Spansion / Cypress
    {0x01, 0x02, 0x17, "Spansion", "S25FL064L",  8UL << 20},
    {0x01, 0x02, 0x18, "Spansion",  "S25FL128L",  16UL << 20},
    {0x01, 0x20, 0x18, "Spansion",  "S25FL127S",  16UL << 20},

    // SST
    {0xBF, 0x25, 0x16, "SST",      "SST25VF032B",4UL << 20},

    // GigaDevice
    {0xC8, 0x40, 0x17, "GigaDevice","GD25Q64",    8UL << 20},
    {0xC8, 0x40, 0x16, "GigaDevice","GD25Q32",    4UL << 20},
    {0xC8, 0x40, 0x18, "GigaDevice","GD25Q128",  16UL << 20},

    // Atmel / Adesto
    {0x1F, 0x45, 0x17, "Atmel",    "AT25DF641",  8UL << 20},
    {0x1F, 0x45, 0x16, "Adesto",    "AT25DF321",   4UL << 20},
    {0x1F, 0x45, 0x15, "Adesto",    "AT25DF161",   2UL << 20},

    // ISSI
    {0x9D, 0x60, 0x17, "ISSI",     "IS25LP064",  8UL << 20},
    {0x9D, 0x60, 0x18, "ISSI",      "IS25LP128",  16UL << 20},
    {0x9D, 0x60, 0x19, "ISSI",      "IS25LP256",  32UL << 20},

    // STMicro
    {0x20, 0x20, 0x15, "STMicro", "M25P16", 2UL << 20},
    {0x20, 0x20, 0x17, "STMicro", "M25P64", 8UL << 20},

    // Micron / Numonyx
    {0x20, 0xBA, 0x17, "Micron", "N25Q064A", 8UL << 20},
    {0x20, 0xBA, 0x18, "Micron", "N25Q128A", 16UL << 20},

    // Zetta
    {0x1C, 0x30, 0x17, "Zetta",  "ZB25Q64", 8UL << 20},
    
    // Boya microelectronics
    {0x68, 0x40, 0x17, "Boya", "BY25Q64AS", 8UL << 20},
};

static constexpr size_t flashDatabaseSize = sizeof(flashDatabase)/sizeof(flashDatabase[0]);

inline const FlashChipInfo* findFlashInfo(uint8_t m, uint8_t t, uint8_t c) {
    for (size_t i = 0; i < flashDatabaseSize; ++i) {
        auto& e = flashDatabase[i];
        if (e.manufacturerId == m && e.memoryType == t && e.capacityCode == c)
            return &e;
    }
    return nullptr;
}

inline const char* findManufacturerName(uint8_t manufacturerId) {
    for (size_t i = 0; i < flashDatabaseSize; ++i) {
        if (flashDatabase[i].manufacturerId == manufacturerId) {
            return flashDatabase[i].manufacturerName;
        }
    }
    return "Unknown";
}
