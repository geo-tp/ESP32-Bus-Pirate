#include "SystemService.h"

#include <Arduino.h>
#include <LittleFS.h>

#include <esp_system.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <nvs.h>

namespace {
    inline void appendLine(std::string& s, const std::string& line) {
        s += line;
        s += "\r\n";
    }

    inline std::string padRight(const char* s, size_t w) {
        std::string r = s ? s : "";
        if (r.size() < w) r.append(w - r.size(), ' ');
        return r;
    }

    auto nvsTypeToStr = [](uint8_t t) -> const char* {
        switch (t) {
            case NVS_TYPE_U8:   return "U8";
            case NVS_TYPE_I8:   return "I8";
            case NVS_TYPE_U16:  return "U16";
            case NVS_TYPE_I16:  return "I16";
            case NVS_TYPE_U32:  return "U32";
            case NVS_TYPE_I32:  return "I32";
            case NVS_TYPE_U64:  return "U64";
            case NVS_TYPE_I64:  return "I64";
            case NVS_TYPE_STR:  return "STR";
            case NVS_TYPE_BLOB: return "BLOB";
            default:            return "?";
        }
    };
}

// -----------------------------
// Chip / runtime
// -----------------------------

std::string SystemService::getChipModel() const
{
    return std::string(ESP.getChipModel());
}

uint32_t SystemService::getUptimeSeconds() const
{
    return millis() / 1000u;
}

int SystemService::getResetReason() const
{
    return static_cast<int>(esp_reset_reason());
}

int SystemService::getCpuFreqMhz() const
{
    return getCpuFrequencyMhz();
}

// -----------------------------
// Chip details
// -----------------------------

int SystemService::getChipCores() const
{
    esp_chip_info_t ci{};
    esp_chip_info(&ci);
    return ci.cores;
}

int SystemService::getChipRevision() const
{
    esp_chip_info_t ci{};
    esp_chip_info(&ci);
    return ci.revision;
}

int SystemService::getChipFullRevision() const
{
    esp_chip_info_t ci{};
    esp_chip_info(&ci);
    return ci.full_revision;
}

uint32_t SystemService::getChipFeaturesRaw() const
{
    esp_chip_info_t ci{};
    esp_chip_info(&ci);
    return ci.features;
}

// -----------------------------
// Versions
// -----------------------------

std::string SystemService::getIdfVersion() const
{
    return std::string(esp_get_idf_version());
}

std::string SystemService::getArduinoCore() const
{
#ifdef ARDUINO_BOARD
    return std::string(ARDUINO_BOARD);
#else
    return "Arduino default";
#endif
}

// -----------------------------
// Stack / Heap / PSRAM
// -----------------------------

size_t SystemService::getStackUsed() const {
    TaskHandle_t h = xTaskGetCurrentTaskHandle();
    size_t usedNow = 0;

    // Get snapshot of tasks
    TaskSnapshot_t snaps[16];
    UBaseType_t count = 0;
    UBaseType_t got = uxTaskGetSnapshotAll(snaps, 16, &count);
    if (got == 0) {
        return -1;
    }

    // Find the snapshot of the current task
    const TaskSnapshot_t* self = nullptr;
    for (UBaseType_t i = 0; i < got; ++i) {
        if (snaps[i].pxTCB == h) {
            self = &snaps[i];
            break;
        }
    }
    if (!self) {
        return -1;
    }

    uintptr_t top = reinterpret_cast<uintptr_t>(self->pxTopOfStack);
    uintptr_t end = reinterpret_cast<uintptr_t>(self->pxEndOfStack);
    if (top == 0 || end == 0 || end <= top) {
        return -1;
    }

    usedNow = end - top;
    return usedNow;
}

size_t SystemService::getStackTotal() const {
    return CONFIG_ARDUINO_LOOP_STACK_SIZE;
}

size_t SystemService::getHeapTotal() const
{
    return ESP.getHeapSize();
}

size_t SystemService::getHeapFree() const
{
    return ESP.getFreeHeap();
}

size_t SystemService::getHeapMinFree() const
{
    return ESP.getMinFreeHeap();
}

size_t SystemService::getHeapMaxAlloc() const
{
    return ESP.getMaxAllocHeap();
}

size_t SystemService::getPsramTotal() const
{
    return ESP.getPsramSize();
}

size_t SystemService::getPsramFree() const
{
    return ESP.getFreePsram();
}

size_t SystemService::getPsramMinFree() const
{
    return ESP.getMinFreePsram();
}

size_t SystemService::getPsramMaxAlloc() const
{
    return ESP.getMaxAllocPsram();
}

// -----------------------------
// Flash / Sketch
// -----------------------------

size_t SystemService::getFlashSizeBytes() const
{
    return ESP.getFlashChipSize();
}

uint32_t SystemService::getFlashSpeedHz() const
{
    return ESP.getFlashChipSpeed();
}

int SystemService::getFlashModeRaw() const
{
    return ESP.getFlashChipMode();
}

std::string SystemService::getFlashJedecIdHex() const
{
    uint32_t jedec = 0;
    if (esp_flash_read_id(nullptr, &jedec) != ESP_OK) {
        return "read failed";
    }

    char buf[12];
    std::snprintf(buf, sizeof(buf), "0x%06X",
                  static_cast<unsigned>(jedec & 0xFFFFFFu));
    return std::string(buf);
}

size_t SystemService::getSketchUsedBytes() const
{
    return ESP.getSketchSize();
}

size_t SystemService::getSketchFreeBytes() const
{
    return ESP.getFreeSketchSpace();
}

std::string SystemService::getSketchMD5() const
{
    return std::string(ESP.getSketchMD5().c_str());
}

// -----------------------------
// Network (raw values)
// -----------------------------

std::string SystemService::getBaseMac() const
{
    uint8_t m[6]{};
    esp_efuse_mac_get_default(m);

    char macStr[18];
    std::snprintf(macStr, sizeof(macStr),
                  "%02X:%02X:%02X:%02X:%02X:%02X",
                  m[0], m[1], m[2], m[3], m[4], m[5]);

    return std::string(macStr);
}

// -----------------------------
// Filesystems (LittleFS / SPIFFS)
// -----------------------------

bool SystemService::littlefsBegin(bool autoFormat) const
{
    return LittleFS.begin(autoFormat);
}

void SystemService::littlefsEnd() const
{
    LittleFS.end();
}

size_t SystemService::littlefsTotalBytes() const
{
    return LittleFS.totalBytes();
}

size_t SystemService::littlefsUsedBytes() const
{
    return LittleFS.usedBytes();
}

// -----------------------------
// Partitions (formatted output)
// -----------------------------

std::string SystemService::getPartitions() const
{
    std::string out;
    const esp_partition_t* run  = esp_ota_get_running_partition();
    const esp_partition_t* boot = esp_ota_get_boot_partition();
    const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);

    auto partLine = [](const esp_partition_t* p) -> std::string {
        if (!p) {
            return "(none)";
        }

        const char* type = (p->type == ESP_PARTITION_TYPE_APP)  ? "APP" :
                           (p->type == ESP_PARTITION_TYPE_DATA) ? "DATA" :
                                                                  "?";

        char b[128];
        std::snprintf(b, sizeof(b), "%-4s %-8s  @0x%06X  %uB",
                      type, p->label, static_cast<unsigned>(p->address), static_cast<unsigned>(p->size));
        return std::string(b);
    };

    appendLine(out, std::string("Running  : ") + partLine(run));
    appendLine(out, std::string("Boot     : ") + partLine(boot));
    appendLine(out, std::string("Next OTA : ") + partLine(next));

    appendLine(out, "");
    appendLine(out, "TYPE LABEL    ADDRESS   SIZE(B)");

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY,
                                                     ESP_PARTITION_SUBTYPE_ANY,
                                                     nullptr);
    if (!it) {
        appendLine(out, "(no partitions)");
        return out;
    }

    for (auto iter = it; iter; iter = esp_partition_next(iter)) {
        const esp_partition_t* p = esp_partition_get(iter);
        if (!p) {
            continue;
        }

        const char* type = (p->type == ESP_PARTITION_TYPE_APP)  ? "APP" :
                           (p->type == ESP_PARTITION_TYPE_DATA) ? "DATA" :
                                                                  "?";

        char line[128];
        std::snprintf(line, sizeof(line), "%-4s %-8s 0x%06X  %u",
                      type, p->label, static_cast<unsigned>(p->address), static_cast<unsigned>(p->size));
        appendLine(out, line);
    }

    #ifdef esp_partition_iterator_release
        esp_partition_iterator_release(it);
    #endif

    return out;
}

// -----------------------------
// NVS (formatted output)
// -----------------------------

std::string SystemService::getNvsStats() const {
    std::string out;

    nvs_stats_t st{};
    if (nvs_get_stats(nullptr, &st) == ESP_OK) {
        appendLine(out, "Used entries    : " + std::to_string(st.used_entries));
        appendLine(out, "Free entries    : " + std::to_string(st.free_entries));
        appendLine(out, "Total entries   : " + std::to_string(st.total_entries));
        appendLine(out, "Namespace count : " + std::to_string(st.namespace_count));
    } else {
        appendLine(out, "NVS stats not available on this build.");
    }
    return out;
}

std::string SystemService::getNvsEntries() const {
    std::string out;

    nvs_iterator_t it = nvs_entry_find("nvs", nullptr, NVS_TYPE_ANY);
    if (!it) {
        appendLine(out, "(no entries)");
        return out;
    }

    // Largeurs fixes (monospace-friendly)
    constexpr size_t W_NS  = 16;
    constexpr size_t W_KEY = 20;

    appendLine(out, padRight("NS", W_NS) + " " + padRight("KEY", W_KEY) + " TYPE");

    for (nvs_iterator_t iter = it; iter; iter = nvs_entry_next(iter)) {
        nvs_entry_info_t info{};
        nvs_entry_info(iter, &info);

        std::string line = padRight(info.namespace_name, W_NS)
                         + " "
                         + padRight(info.key, W_KEY)
                         + " "
                         + nvsTypeToStr(info.type);
        appendLine(out, line);
    }

    #if defined(nvs_release_iterator)
        nvs_release_iterator(it);
    #endif

    return out;
}

void SystemService::reboot(bool hard) const {
    if (hard) {
        ESP.restart();    // Arduino-style reset
    } else {
        esp_restart();    // IDF reset
    }
}