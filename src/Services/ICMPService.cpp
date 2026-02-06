#include "ICMPService.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <vector>
#include <algorithm>
#include <ESP32Ping.h>

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "ping/ping_sock.h"
extern "C"
{
#include "esp_ping.h"
}

portMUX_TYPE ICMPService::icmpMux = portMUX_INITIALIZER_UNLOCKED;
std::vector<std::string> ICMPService::icmpLog;
bool ICMPService::stopICMPFlag = false;

// Task params
struct ICMPTaskParams
{
    std::string targetIP;
    int count;
    int timeout_ms;
    int interval_ms;
    ICMPService *service;
};

struct DiscoveryTaskParams
{
    std::string deviceIP;
    ICMPService *service;
};

ICMPService::ICMPService() {}

ICMPService::~ICMPService()
{
    cleanupICMPService();
}

void ICMPService::cleanupICMPService()
{
    // Reset last results
    pingReady = false;
    pingRC = ping_rc_t::ping_error;
    pingMedianMs = -1;
    pingTX = 0;
    pingRX = 0;
    report.clear();
}

std::string ICMPService::getPingHelp() const{
    std::string helpMenu = std::string("Usage: ping <host> [-c <count>] [-t <timeout>] [-i <interval>]\r\nOptions:\r\n ") + 
        "\t-c <count>    Number of pings (default: 5)\r\n " +
        "\t-t <timeout>  Timeout in milliseconds (default: 1000)\r\n" +
        "\t-i <interval> Interval between pings in milliseconds (default: 200)";
    return helpMenu;
}

static bool resolve_ipv4_to_ip_addr(const std::string &targetIP, ip_addr_t &out)
{
    // literal first
    ip4_addr_t a4{};
    if (ip4addr_aton(targetIP.c_str(), &a4))
    {
        out.type = IPADDR_TYPE_V4;
        out.u_addr.ip4 = a4;
        return true;
    }
    // DNS fallback
    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    struct addrinfo *res = nullptr;
    if (getaddrinfo(targetIP.c_str(), nullptr, &hints, &res) != 0 || !res)
        return false;
    out.type = IPADDR_TYPE_V4;
    out.u_addr.ip4.addr = ((sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
    freeaddrinfo(res);
    return true;
}

static int median_ms(std::vector<uint32_t> &v)
{
    if (v.empty())
        return -1;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n & 1)
        return (int)v[n / 2];
    return (int)((v[n / 2 - 1] + v[n / 2] + 1) / 2);
}

void ICMPService::discoveryTask(void* params){
    auto* taskParams = static_cast<DiscoveryTaskParams*>(params);
    std::string deviceIP = taskParams->deviceIP;
    ICMPService* service = taskParams->service;
    ip4_addr_t targetIP;
    uint32_t targetsResponded = 0;

    pushICMPLog("Discovery: Scanning network for devices... Press [ENTER] to stop.\r\n");

    if (!ip4addr_aton(deviceIP.c_str(), &targetIP))
    {
        pushICMPLog("Discovery: failed to parse IP address " + deviceIP);
        delete taskParams;
        vTaskDelete(nullptr);
        return;
    }

    // Octets of an IPv4 addr
    uint8_t o1 = ip4_addr1(&targetIP);
    uint8_t o2 = ip4_addr2(&targetIP);
    uint8_t o3 = ip4_addr3(&targetIP);
    uint8_t deviceIndex = ip4_addr4(&targetIP);

    for (uint8_t targetIndex = 1; targetIndex < 255; targetIndex++)
    {
        if (ICMPService::getICMPServiceStatus() == true)
        {
            pushICMPLog("Discovery: Stopped by user\r\n");
            delete taskParams;
            vTaskDelete(nullptr);
            return;
        }

        if (targetIndex == deviceIndex)
            continue;

        // Rebuild the target IP from octets
        IP4_ADDR(&targetIP, o1, o2, o3, targetIndex);

        // Convert to dotted string
        char targetIPCStr[16];
        ip4addr_ntoa_r(&targetIP, targetIPCStr, sizeof(targetIPCStr));
        std::string targetIPStr(targetIPCStr);

        service->cleanupICMPService();
        auto *params = new ICMPTaskParams{targetIPStr, 2, 150, 100, service};

        xTaskCreatePinnedToCore(pingAPI, "ICMPPing", 4096, params, 1, nullptr, 1);

        while (!service->pingReady)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        if (service->pingRC == ping_rc_t::ping_ok){
            pushICMPLog("Device found: " + targetIPStr);
            targetsResponded++;
        }
    }

    pushICMPLog(std::to_string(targetsResponded) + " devices up, pinged " + 
        std::to_string(254 - targetsResponded) + " devices down");

    service->discoveryReady = true;

    delete taskParams;
    vTaskDelete(nullptr);
}

void ICMPService::startDiscoveryTask(const std::string deviceIP)
{
    report.clear();
    discoveryReady = false;
    stopICMPFlag = false;

    // Start job
    auto* p = new DiscoveryTaskParams{deviceIP, this};
    xTaskCreatePinnedToCore(discoveryTask, "ICMPDiscover", 8192, p, 1, nullptr, 0);
}

void ICMPService::startPingTask(const std::string &targetIP, int count, int timeout_ms, int interval_ms)
{
    // Cleanup first
    this->cleanupICMPService();

    auto *params = new ICMPTaskParams{targetIP,
                                      count > 0 ? count : 5,
                                      timeout_ms > 0 ? timeout_ms : 1000,
                                      interval_ms > 0 ? interval_ms : 200,
                                      this};
    xTaskCreatePinnedToCore(pingAPI, "ICMPPing", 4096, params, 1, nullptr, 1);

    while(!this->pingReady)
        vTaskDelay(pdMS_TO_TICKS(10));

    // Prepare the report
    this->report.clear();
    if (this->pingRC == ping_rc_t::ping_ok || this->pingRC == ping_rc_t::ping_timeout) {
        this->report = "--- " + targetIP + " ping statistics ---\r\n";
        this->report += std::to_string(this->pingTX) + " packets transmitted, ";
        this->report += std::to_string(this->pingRX) + " received, ";
        this->report += std::to_string(100 - this->pingRX * 100 / this->pingTX) + "\% packet loss,";
        this->report += " time " + std::to_string(this->pingMedianMs) + " ms\r\n";
    } else if (this->pingRC == ping_rc_t::ping_resolve_fail) {
        this->report = "Failed to resolve \"" + targetIP + "\"\r\n";
    } else if (this->pingRC == ping_rc_t::ping_session_fail) {
        this->report = "Failed to create session\r\n";
    } else {
        report = "Unknown error\r\n";
    }
        
}

void ICMPService::pingAPI(void *pvParams)
{
    auto *params = static_cast<ICMPTaskParams *>(pvParams);
    ICMPService *service = params->service;

    ip_addr_t target{};
    if (!resolve_ipv4_to_ip_addr(params->targetIP, target))
    {
        service->pingRC = ping_rc_t::ping_resolve_fail;
        service->pingReady = true;
        delete params;
        vTaskDelete(nullptr);
        return;
    }

    // esp_ping setup
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr = target;  // destination
    config.count = params->count; // total probes
    config.interval_ms = params->interval_ms;
    config.timeout_ms = params->timeout_ms;

    struct Ctx
    {
        std::vector<uint32_t> rtts;
        volatile uint32_t rx = 0;
        volatile bool done = false;
    } ctx;

    esp_ping_callbacks_t cbs{};
    cbs.on_ping_success = [](esp_ping_handle_t h, void *arg)
    {
        uint32_t time_ms = 0;
        esp_ping_get_profile(h, ESP_PING_PROF_TIMEGAP, &time_ms, sizeof(time_ms)); // RTT
        auto *c = static_cast<Ctx *>(arg);
        c->rtts.push_back(time_ms);
        c->rx++;
    };
    cbs.on_ping_timeout = [](esp_ping_handle_t, void *)
    {
        // Count successes only for median
    };
    cbs.on_ping_end = [](esp_ping_handle_t, void *arg)
    {
        static_cast<Ctx *>(arg)->done = true;
    };
    cbs.cb_args = &ctx;

    esp_ping_handle_t h = nullptr;
    if (esp_ping_new_session(&config, &cbs, &h) != ESP_OK)
    {
        service->pingRC = ping_rc_t::ping_session_fail;
        service->pingReady = true;
        delete params;
        vTaskDelete(nullptr);
        return;
    }

    esp_ping_start(h);

    // Wait up to count * (timeout + interval) + a small delay
    uint32_t wait_ms = (uint32_t)config.count * (config.timeout_ms + config.interval_ms) + 100;
    uint32_t t0 = millis();
    while (!ctx.done && (millis() - t0) < wait_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    esp_ping_stop(h);
    esp_ping_delete_session(h);

    // Results
    service->pingTX = (int)config.count;
    service->pingRX = (int)ctx.rx;
    service->pingRC = (service->pingRX > 0) ? ping_rc_t::ping_ok : ping_rc_t::ping_timeout;
    service->pingMedianMs = median_ms(ctx.rtts);

    if (service->pingRX > 0)
        service->pingRC = ping_rc_t::ping_ok;
    else if (service->pingRX == 0)
        service->pingRC = ping_rc_t::ping_timeout;

    service->pingReady = true;

    delete params;
    vTaskDelete(nullptr);
}

void ICMPService::clearICMPLogging() {
    portENTER_CRITICAL(&icmpMux);
    icmpLog.clear();
    stopICMPFlag = false;
    portEXIT_CRITICAL(&icmpMux);
}

void ICMPService::pushICMPLog(const std::string& line) {
    portENTER_CRITICAL(&icmpMux);
    icmpLog.push_back(line);
    if (icmpLog.size() > ICMP_LOG_MAX) {
        size_t excess = icmpLog.size() - ICMP_LOG_MAX;
        icmpLog.erase(icmpLog.begin(), icmpLog.begin() + excess);
    }
    portEXIT_CRITICAL(&icmpMux);
}

bool ICMPService::getICMPServiceStatus() {
    bool v;
    portENTER_CRITICAL(&ICMPService::icmpMux);
    v = ICMPService::stopICMPFlag;
    portEXIT_CRITICAL(&ICMPService::icmpMux);
    return v;
}

std::vector<std::string> ICMPService::fetchICMPLog() {
    std::vector<std::string> batch;
    portENTER_CRITICAL(&ICMPService::icmpMux);
    batch.swap(ICMPService::icmpLog);
    portEXIT_CRITICAL(&ICMPService::icmpMux);
    return batch;
}

void ICMPService::stopICMPService(){
    portENTER_CRITICAL(&ICMPService::icmpMux);
    ICMPService::stopICMPFlag = true;
    portEXIT_CRITICAL(&ICMPService::icmpMux);
}