#pragma once

#include <array>
#include <string>
#include <cstring>

#include "esp_event.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_eth_netif_glue.h"
#include "driver/spi_master.h"
#include "lwip/ip4_addr.h"
#include <Arduino.h>
#include <SPI.h>

class EthernetService {
public:
    EthernetService();

    // Init the W5500 ethernet stack
    void ensureStacksInited();

    // Configure the W5500
    bool configure(int8_t pinCS, int8_t pinRST, int8_t pinSCK, int8_t pinMISO, int8_t pinMOSI, uint8_t pinIRQ, uint32_t spiHz, const std::array<uint8_t,6>& chosenMac);
    
    // Reset the W5500
    void hardReset();

    // Start ETH + DHCP and wait for link then IP.
    bool beginDHCP(unsigned long timeoutMs);

    // Check if the W5500 is connected to a network
    bool isConnected() const;

    // Plug/unplug
    int  linkStatusRaw() const;
    bool linkUp() const;

    // Helpers
    std::string ip4ToString(const ip4_addr_t& a) const;
    std::string getLocalIP() const;
    std::string getSubnetMask() const;
    std::string getGatewayIp() const;
    std::string getDns() const;
    std::string getMac() const;

    // Handlers for the esp network stack
    static void onEthEvent(void* arg, esp_event_base_t, int32_t id, void* data);
    static void onIpEvent (void* arg, esp_event_base_t, int32_t id, void* data);

private:
    static bool s_stackInited;

    spi_device_handle_t _spi;
    spi_host_device_t _spiHost = SPI3_HOST;
    esp_eth_handle_t    _eth;
    esp_netif_t*        _netif;
    esp_eth_netif_glue_handle_t _glue;

    esp_event_handler_instance_t _ethHandler = nullptr;
    esp_event_handler_instance_t _ipHandler  = nullptr;

    std::array<uint8_t,6> _mac;
    ip4_addr_t _ip{}, _gw{}, _mask{}, _dns0{};

    int8_t _pinRST;
    int8_t _pinIRQ;
    bool   _configured;
    volatile bool _linkUp;
    volatile bool _gotIP;
};
