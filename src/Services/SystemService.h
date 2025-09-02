#pragma once
#include <string>
#include <cstdint>

class SystemService {
public:
    // Chip / runtime
    std::string getChipModel() const;
    uint32_t    getUptimeSeconds() const; // millis()/1000
    int         getResetReason() const;
    int         getCpuFreqMhz() const;
    int         getChipCores() const;
    int         getChipRevision() const;
    int         getChipFullRevision() const;
    uint32_t    getChipFeaturesRaw() const;  // bitmask 

    // Versions
    std::string getIdfVersion() const;
    std::string getArduinoCore() const;

    // Stack / Heap / PSRAM
    size_t getStackUsed() const;
    size_t getStackTotal() const;
    size_t getHeapTotal() const;
    size_t getHeapFree() const;
    size_t getHeapMinFree() const;
    size_t getHeapMaxAlloc() const;
    size_t getPsramTotal() const;
    size_t getPsramFree() const;
    size_t getPsramMinFree() const;
    size_t getPsramMaxAlloc() const;

    // Flash / Sketch
    size_t      getFlashSizeBytes() const;
    uint32_t    getFlashSpeedHz() const;
    int         getFlashModeRaw() const; 
    std::string getFlashJedecIdHex() const;
    size_t      getSketchUsedBytes() const;
    size_t      getSketchFreeBytes() const;
    std::string getSketchMD5() const;

    // Network 
    std::string getBaseMac() const;

    // Filesystems
    bool   littlefsBegin(bool autoFormat) const;
    void   littlefsEnd() const;
    size_t littlefsTotalBytes() const;
    size_t littlefsUsedBytes() const;

    // Formatted output
    std::string getPartitions() const;
    std::string getNvsStats() const;
    std::string getNvsEntries() const;

    // Boot
    void reboot(bool hard = false) const;
};
