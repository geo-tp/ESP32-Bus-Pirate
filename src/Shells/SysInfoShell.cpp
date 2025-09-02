#include "SysInfoShell.h"
#include <cstdarg>
#include <cstdio>

SysInfoShell::SysInfoShell(ITerminalView& tv,
                           IInput& in,
                           UserInputManager& uim,
                           ArgTransformer& at,
                           SystemService& sys,
                           WifiService& wifi)
    : terminalView(tv)
    , terminalInput(in)
    , userInputManager(uim)
    , argTransformer(at)
    , systemService(sys)
    , wifiService(wifi) {}

void SysInfoShell::run() {
    bool loop = true;
    while (loop) {
        terminalView.println("\n=== System Shell ===");

        int choice = userInputManager.readValidatedChoiceIndex("Select action", actions);

        switch (choice) {
            case 0: cmdSummary(); break;
            case 1: cmdHardwareInfo(); break;
            case 2: cmdMemory(); break;
            case 3: cmdPartitions(); break;
            case 4: cmdFS(); break;
            case 5: cmdNVS(false); break;
            case 6: cmdNVS(true); break;
            case 7: cmdNet(); break;
            case 8: cmdReboot(); break;
            case 9: // Exit
            default:
                loop = false;
                break;
        }
    }

    terminalView.println("Exiting System Shell...\n");
}

void SysInfoShell::cmdSummary() {
    terminalView.println("\n=== System Summary ===");
    terminalView.println("Model         : " + systemService.getChipModel());
    terminalView.println("Uptime        : " + std::to_string(systemService.getUptimeSeconds()) + " s");
    
    const int rr = systemService.getResetReason();
    terminalView.println(std::string("Reset reason  : ") + resetReasonToStr(rr) + " (" + std::to_string(rr) + ")");
    
    terminalView.println("Stack total   : " + std::to_string(systemService.getStackTotal()   / 1024) + " KB");
    terminalView.println("Heap total    : " + std::to_string(systemService.getHeapTotal()   / 1024) + " KB");
    terminalView.println("PSRAM total   : " + std::to_string(systemService.getPsramTotal()  / 1024) + " KB");
    terminalView.println("Flash total   : " + std::to_string(systemService.getFlashSizeBytes() / 1024) + " KB");

    terminalView.println("Firmware      : " + std::string("version " + state.getVersion()));
    terminalView.println("Build date    : " + std::string(__DATE__) + " " + std::string(__TIME__));
    terminalView.println("IDF version   : " + systemService.getIdfVersion());
    terminalView.println("Arduino core  : " + systemService.getArduinoCore());
}

void SysInfoShell::cmdHardwareInfo() {
    terminalView.println("\n=== Hardware Info ===");

    // Chip
    terminalView.println("Model             : " + systemService.getChipModel());
    terminalView.println("CPU cores         : " + std::to_string(systemService.getChipCores()));
    terminalView.println("CPU freq          : " + std::to_string(systemService.getCpuFreqMhz()) + " MHz");

    // Features (WiFi/BT/BLE)
    const uint32_t f = systemService.getChipFeaturesRaw();
    std::string features;
    if (f & CHIP_FEATURE_WIFI_BGN) features += "WiFi ";
    if (f & CHIP_FEATURE_BT)       features += "BT ";
    if (f & CHIP_FEATURE_BLE)      features += "BLE ";
    if (features.empty())          features = "?";
    terminalView.println("Features          : " + features);

    terminalView.println("Revision          : " + std::to_string(systemService.getChipRevision()));
    const int fullrev = systemService.getChipFullRevision();
    if (fullrev >= 0) {
        terminalView.println("Full revision     : " + std::to_string(fullrev));
    }

    // Flash
    terminalView.println("Flash total       : " + std::to_string(systemService.getFlashSizeBytes() / 1024) + " KB");
    terminalView.println("Flash speed       : " + std::to_string(systemService.getFlashSpeedHz() / 1000000U) + " MHz");
    terminalView.println("Flash mode        : " + std::string(flashModeToStr(systemService.getFlashModeRaw())));
    terminalView.println("Flash chip ID     : " + systemService.getFlashJedecIdHex());

    // Sketch
    const size_t sku = systemService.getSketchUsedBytes();
    const size_t skt = systemService.getSketchFreeBytes(); // it returns the total space
    const size_t skl = skt - sku; // space left
    const int    pctInt = skt ? static_cast<int>((sku * 100.0f / skt) + 0.5f) : 0;

    terminalView.println("Sketch total      : " + std::to_string(skt / 1024) + " KB");
    terminalView.println("Sketch free       : " + std::to_string(skl / 1024) + " KB");
    terminalView.println("Sketch usage      : " + std::to_string(pctInt) + " %");
    terminalView.println("Sketch MD5        : " + systemService.getSketchMD5());
}

void SysInfoShell::cmdMemory() {
    terminalView.println("\n=== Memory ===");
    
    // Stack
    char line[96];
    size_t stTot  = systemService.getStackTotal();
    float totKB  = stTot  / 1024.0f;
    snprintf(line, sizeof(line), "Stack total       : %.2f KB", totKB);
    terminalView.println(line);

    #ifndef DEVICE_M5STICK // M5StickC has no more IRAM left to handle this

    size_t stUsed = systemService.getStackUsed();
    float usedKB = stUsed / 1024.0f;
    float freeKB = (stTot > stUsed ? (stTot - stUsed) : 0) / 1024.0f;
    float pct    = (stUsed * 100.0f) / (float)stTot;
    int   stPct  = (stUsed * 100.0 / stTot) + 0.5;
    snprintf(line, sizeof(line), "Stack free        : %.2f KB", freeKB);
    terminalView.println(line);
    snprintf(line, sizeof(line), "Stack used        : %.2f KB (%.0f%%)", usedKB, pct);
    terminalView.println(line);

    #endif

    // Heap
    const size_t heapTotal = systemService.getHeapTotal();
    const size_t heapFree  = systemService.getHeapFree();
    const int    heapPct   = heapTotal ? static_cast<int>(((heapTotal - heapFree) * 100.0f / heapTotal) + 0.5f) : 0;
    terminalView.println("Heap total        : " + std::to_string(heapTotal / 1024) + " KB");
    terminalView.println("Heap free         : " + std::to_string(heapFree / 1024) + " KB");
    terminalView.println("Heap used         : " + std::to_string((heapTotal - heapFree) / 1024) + " KB (" + std::to_string(heapPct) + "%)");
    terminalView.println("Min free heap     : " + std::to_string(systemService.getHeapMinFree()  / 1024) + " KB");
    terminalView.println("Max alloc heap    : " + std::to_string(systemService.getHeapMaxAlloc() / 1024) + " KB");

    // PSRAM
    const size_t psramTotal = systemService.getPsramTotal();
    const size_t psramFree  = systemService.getPsramFree();
    const int    psramPct   = psramTotal ? static_cast<int>(((psramTotal - psramFree) * 100.0f / psramTotal) + 0.5f) : 0;
    terminalView.println("PSRAM total       : " + std::to_string(psramTotal / 1024) + " KB");
    terminalView.println("PSRAM free        : " + std::to_string(psramFree / 1024) + " KB");
    terminalView.println("PSRAM used        : " + std::to_string((psramTotal - psramFree) / 1024) + " KB (" + std::to_string(psramPct) + "%)");
    terminalView.println("Min free PSRAM    : " + std::to_string(systemService.getPsramMinFree()  / 1024) + " KB");
    terminalView.println("Max alloc PSRAM   : " + std::to_string(systemService.getPsramMaxAlloc() / 1024) + " KB");
}

void SysInfoShell::cmdPartitions() {
    terminalView.println("\n=== Partitions ===");
    terminalView.println(systemService.getPartitions());
}

void SysInfoShell::cmdFS() {
    terminalView.println("\n=== LittleFS ===");
    if (systemService.littlefsBegin(true)) { // autoformat if failed mounted
        const size_t total = systemService.littlefsTotalBytes();
        const size_t used  = systemService.littlefsUsedBytes();
        const size_t freeB = (total >= used) ? (total - used) : 0;

        terminalView.println("Total  : " + std::to_string(total / 1024) + " KB");
        terminalView.println("Used   : " + std::to_string(used  / 1024) + " KB");
        terminalView.println("Free   : " + std::to_string(freeB / 1024) + " KB");

        systemService.littlefsEnd();
    } else {
        terminalView.println("LittleFS not mounted.");
    }
}

void SysInfoShell::cmdNVS(bool listEntries) {
    terminalView.println("\n=== NVS ===");
    if (listEntries) {
        terminalView.println(systemService.getNvsEntries());
        return;
    }
    
    terminalView.println(systemService.getNvsStats());
}

void SysInfoShell::cmdNet() {
    terminalView.println("\n=== Network Info ===");

    auto ssid     = wifiService.getSsid();     if (ssid.empty()) ssid = "N/A";
    auto bssid    = wifiService.getBssid();    if (bssid.empty()) bssid = "N/A";
    auto hostname = wifiService.getHostname(); if (hostname.empty()) hostname = "N/A";

    terminalView.println("Base MAC     : " + systemService.getBaseMac());
    terminalView.println("AP MAC       : " + wifiService.getMacAddressAp());
    terminalView.println("STA MAC      : " + wifiService.getMacAddressSta());
    terminalView.println("IP           : " + wifiService.getLocalIp());
    terminalView.println("Subnet       : " + wifiService.getSubnetMask());
    terminalView.println("Gateway      : " + wifiService.getGatewayIp());
    terminalView.println("DNS1         : " + wifiService.getDns1());
    terminalView.println("DNS2         : " + wifiService.getDns2());
    terminalView.println("Hostname     : " + hostname);

    terminalView.println("SSID         : " + ssid);
    terminalView.println("BSSID        : " + bssid);

    const int status = wifiService.getWifiStatusRaw();
    if (status == 3 /* WL_CONNECTED */) {
        terminalView.println("RSSI         : " + std::to_string(wifiService.getRssi()) + " dBm");
        terminalView.println("Channel      : " + std::to_string(wifiService.getChannel()));
    } else {
        terminalView.println("RSSI         : N/A");
        terminalView.println("Channel      : N/A");
    }

    terminalView.println("Mode         : " + std::string(wifiService.wifiModeToStr(wifiService.getWifiModeRaw())));
    terminalView.println("Status       : " + std::string(wifiService.wlStatusToStr(status)));
    terminalView.println("Prov enabled : " + std::string(wifiService.isProvisioningEnabled() ? "Yes" : "No"));
}

void SysInfoShell::cmdReboot(bool hard) {
    auto confirmation = userInputManager.readYesNo("Reboot the device? (y/n)", false);
    if (confirmation) {
        terminalView.println("\nRebooting, your session will be lost...");
        systemService.reboot(hard);
    }
}