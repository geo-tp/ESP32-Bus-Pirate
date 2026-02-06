#include "WifiOpenScannerService.h"

portMUX_TYPE WifiOpenScannerService::probeMux = portMUX_INITIALIZER_UNLOCKED;
std::vector<std::string> WifiOpenScannerService::probeLog;

bool WifiOpenScannerService::startOpenProbe(uint32_t scanIntervalMs) {
    if (openProbeRunning.load()) return true;
    openProbeRunning = true;

    BaseType_t ok = xTaskCreatePinnedToCore(
        &WifiOpenScannerService::openProbeTaskThunk,
        "wifi_open_probe",
        6144,               // stack
        this,
        1,                  // low prio
        &openProbeHandle,
        0                   // core 0
    );
    if (ok != pdPASS) {
        openProbeRunning = false;
        openProbeHandle = nullptr;
        return false;
    }
    ulTaskNotifyValueClear(openProbeHandle, 0xFFFFFFFF);
    xTaskNotify(openProbeHandle, scanIntervalMs, eSetValueWithOverwrite);
    return true;
}

void WifiOpenScannerService::stopOpenProbe() {
    if (!openProbeRunning.load()) return;
    openProbeRunning = false;
    if (openProbeHandle) {
        // wake up if neded
        xTaskNotifyGive(openProbeHandle);
        for (int i=0; i<40 && openProbeHandle; ++i) vTaskDelay(pdMS_TO_TICKS(25));
    }
}

void WifiOpenScannerService::openProbeTaskThunk(void* arg) {
    auto* self = static_cast<WifiOpenScannerService*>(arg);
    uint32_t interval = 2500, v=0;
    if (xTaskNotifyWait(0, 0xFFFFFFFF, &v, pdMS_TO_TICKS(10)) == pdTRUE && v>0) interval = v;
    self->openProbeTask(interval);
    vTaskDelete(nullptr);
}

bool WifiOpenScannerService::isOpenAuth(int enc) {
    return enc == WIFI_AUTH_OPEN;
}

void WifiOpenScannerService::openProbeTask(uint32_t scanIntervalMs) {
    pushProbeLog("[PROBE] Started, Attempting to connect to open Wi-Fi networks...");
    WiFi.mode(WIFI_STA);

    while (openProbeRunning.load()) {
        unsigned long scanMs = 0;
        int n = doScan(/*showHidden=*/true, scanMs);
        if (n >= 0) {
            pushProbeLog("[SCAN] Found " + std::to_string(n) + " network(s) in " + std::to_string(scanMs) + " ms");
            pushProbeLog("[SCAN] Processing probing connection for each network...");
            processAllNetworks(n);
        } else {
            pushProbeLog("[ERROR] Scan failed");
        }

        pushProbeLog("[DONE] Probe cycle done. Restarting... Press [ENTER] to stop");

        uint32_t slept = 0;
        while (slept < scanIntervalMs && openProbeRunning.load()) {
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) > 0) break;
            slept += 50;
        }
    }

    pushProbeLog("[PROBE] Stopped by user");
    openProbeHandle = nullptr;
}

// ===== Steps (small functions) =====

int WifiOpenScannerService::doScan(bool showHidden, unsigned long& outScanMs) {
    unsigned long t0 = millis();
    int n = WiFi.scanNetworks(/*async=*/false, showHidden);
    outScanMs = millis() - t0;

    // recovery 
    if (n >= 0) maybeRecoverFromFastScan(outScanMs);
    return n;
}

void WifiOpenScannerService::maybeRecoverFromFastScan(unsigned long scanMs) {
    if (scanMs < 20) {
        pushProbeLog("[WARN] Fast scan (<20ms), resetting WiFi STA...");
        WiFi.disconnect(true);
        vTaskDelay(pdMS_TO_TICKS(300));
        WiFi.mode(WIFI_STA);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void WifiOpenScannerService::processAllNetworks(int count) {
    for (int i = 0; i < count && openProbeRunning.load(); ++i) {
        processOneNetwork(i);
    }
}

void WifiOpenScannerService::processOneNetwork(int idx) {
    const int enc = WiFi.encryptionType(idx);
    const bool open = isOpenAuth(enc);
    const std::string ssid = getSsid(idx);

    // only open networks
    if (!open) {
        char skip[256];
        snprintf(skip, sizeof(skip),
                 "[SKIP] SSID=\"%s\" ENC=%s (non-open)",
                 ssid.c_str(), encToStr(enc));
        pushProbeLog(skip);
        return;
    }

    std::string ip;
    unsigned long connectMs = 0;
    const unsigned long timeoutMs = 12000;

    const bool ok = connectToNetwork(ssid, /*isOpen=*/true, timeoutMs, ip, connectMs);
    if (!ok) {
        char line[320];
        snprintf(line, sizeof(line),
                "[TRY]  SSID=\"%s\" ENC=%s -> connect FAILED (%lums)",
                ssid.c_str(), encToStr(enc), connectMs);
        pushProbeLog(line);
        safeDisconnect();

        // Test HTTP/Internet
        int httpCode = -1; unsigned long httpMs = 0;
        const bool internet = performHttpCheck(httpCode, httpMs);

        snprintf(line, sizeof(line),
                "[TRY]  SSID=\"%s\" ENC=%s -> CONNECTED ip=%s (connect %lums) HTTP=%d (%s, %lums)",
                ssid.c_str(), encToStr(enc), ip.c_str(), connectMs, httpCode,
                internet ? "Internet OK" : "No Internet", httpMs);
        pushProbeLog(line);

        safeDisconnect(50);
    }
}

bool WifiOpenScannerService::connectToNetwork(const std::string& ssid,
                                          bool isOpen,
                                          unsigned long timeoutMs,
                                          std::string& outIp,
                                          unsigned long& outElapsedMs) {
    unsigned long t0 = millis();

    WiFi.begin(ssid.c_str(), isOpen ? nullptr : "");

    while (WiFi.status() != WL_CONNECTED && (millis() - t0) < timeoutMs) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    const bool ok = (WiFi.status() == WL_CONNECTED);
    outElapsedMs = millis() - t0;
    outIp = ok ? std::string(WiFi.localIP().toString().c_str()) : std::string();

    // Log result
    if (ok) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "[TRY]  SSID=\"%s\" -> CONNECTED ip=%s (connect %lums)",
                 ssid.c_str(), outIp.c_str(), outElapsedMs);
        pushProbeLog(msg);
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "[TRY]  SSID=\"%s\" -> CONNECT FAILED (%lums)",
                 ssid.c_str(), outElapsedMs);
        pushProbeLog(msg);
    }

    return ok;
}

bool WifiOpenScannerService::performHttpCheck(int& outHttpCode, unsigned long& outHttpMs) {
    unsigned long t0 = millis();
    HTTPClient http;

    if (http.begin("http://connectivitycheck.gstatic.com/generate_204")) {
        http.setTimeout(4000);
        outHttpCode = http.GET();
        http.end();
        outHttpMs = millis() - t0;
        if (outHttpCode == 204) return true;
        if (outHttpCode > 0)   return true;
    } else {
        if (http.begin("http://example.com")) {
            http.setTimeout(4000);
            outHttpCode = http.GET();
            http.end();
            outHttpMs = millis() - t0;
            return outHttpCode > 0;
        }
    }
    outHttpMs = millis() - t0;
    return false;
}

void WifiOpenScannerService::safeDisconnect(unsigned delayMs) {
    WiFi.disconnect(true);
    if (delayMs) vTaskDelay(pdMS_TO_TICKS(delayMs));
}

const char* WifiOpenScannerService::encToStr(int enc) const {
    switch (enc) {
        case WIFI_AUTH_OPEN:           return "OPEN";
        case WIFI_AUTH_WEP:            return "WEP";
        case WIFI_AUTH_WPA_PSK:        return "WPA";
        case WIFI_AUTH_WPA2_PSK:       return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:   return "WPA+WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE:return "WPA2-E";
        case WIFI_AUTH_WPA3_PSK:       return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK:  return "WPA2+WPA3";
        case WIFI_AUTH_WAPI_PSK:       return "WAPI";
        default:                       return "UNKNOWN";
    }
}

std::string WifiOpenScannerService::getSsid(int idx) const {
    String s = WiFi.SSID(idx);
    return s.length() ? std::string(s.c_str()) : std::string("Hidden SSID");
}

void WifiOpenScannerService::pushProbeLog(const std::string& line) {
    portENTER_CRITICAL(&probeMux);
    probeLog.push_back(line);
    if (probeLog.size() > PROBE_LOG_MAX) {
        size_t excess = probeLog.size() - PROBE_LOG_MAX;
        probeLog.erase(probeLog.begin(), probeLog.begin() + excess);
    }
    portEXIT_CRITICAL(&probeMux);
}

std::vector<std::string> WifiOpenScannerService::fetchProbeLog() {
    std::vector<std::string> batch;
    portENTER_CRITICAL(&probeMux);
    batch.swap(probeLog);
    portEXIT_CRITICAL(&probeMux);
    return batch;
}

void WifiOpenScannerService::clearProbeLog() {
    portENTER_CRITICAL(&probeMux);
    probeLog.clear();
    portEXIT_CRITICAL(&probeMux);
}
