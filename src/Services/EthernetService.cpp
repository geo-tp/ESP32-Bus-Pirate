#include "EthernetService.h"

#include <cstdio>
#include "esp_netif_defaults.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <Arduino.h>
#include "driver/spi_master.h"
#include "esp_eth_mac_spi.h"
// =================================== LOG ================================

// Erase macro to log on serial
#ifdef ESP_LOGE
  #undef ESP_LOGE
#endif

#define _SERIAL_LOG_BASE(levelChar, tag, format, ...) \
  do { Serial.printf("[%c][%s] " format "\r\n", (levelChar), (tag), ##__VA_ARGS__); } while (0)

#define ESP_LOGE(tag, format, ...) _SERIAL_LOG_BASE('E', (tag), format, ##__VA_ARGS__)
static const char* TAG = "EthernetService";

#define LOG_ERR(fn, err) ESP_LOGE(TAG, "%s -> %s", fn, esp_err_to_name(err))

bool EthernetService::s_stackInited = false;

// helpers for IP cast compatibility
static inline const ip4_addr_t* to_lwip(const esp_ip4_addr_t* a) {
    return reinterpret_cast<const ip4_addr_t*>(a);
}

// ==============================================================

EthernetService::EthernetService()
: _spi(nullptr), _eth(nullptr), _netif(nullptr), _glue(nullptr),
  _pinRST(-1), _pinIRQ(-1), _configured(false), _linkUp(false), _gotIP(false) {
    _ip.addr = _gw.addr = _mask.addr = _dns0.addr = 0;
}

void EthernetService::ensureStacksInited() {
    if (s_stackInited) return;

    esp_err_t e = esp_netif_init();
    e = esp_event_loop_create_default();
    s_stackInited = true;

    esp_log_level_set("esp_eth", ESP_LOG_DEBUG);
    esp_log_level_set("ETH",     ESP_LOG_DEBUG);
    esp_log_level_set("netif",   ESP_LOG_INFO);
}

bool EthernetService::configure(int8_t pinCS, int8_t pinRST, int8_t pinSCK, int8_t pinMISO, int8_t pinMOSI, uint8_t pinIRQ, uint32_t spiHz, const std::array<uint8_t,6>& chosenMac) {
    #ifdef DEVICE_M5STICK
        ESP_LOGE(TAG, "M5Stick not supported");
        return false;
    #else
    
    if (_configured) {
        return true; // already configured
    }

    _pinRST = pinRST;
    _pinIRQ = pinIRQ;

    ensureStacksInited();

    // Service ISR GPIO
    static bool s_isr = false;
    if (!s_isr) {
        esp_err_t e = gpio_install_isr_service(0);
        // Not a problem if already installed
    }

    // Bus SPI
    spi_bus_config_t buscfg{};
    buscfg.mosi_io_num = pinMOSI;
    buscfg.miso_io_num = pinMISO;
    buscfg.sclk_io_num = pinSCK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    #ifdef DEVICE_CARDPUTER
        _spiHost = SPI2_HOST;
    #endif

    esp_err_t e1 = spi_bus_initialize(_spiHost, &buscfg, SPI_DMA_CH_AUTO);
    if (e1 != ESP_OK) { LOG_ERR("failed init spi bus", e1); return false; }

    // Device config
    spi_device_interface_config_t devcfg_final{};
    devcfg_final.mode = 0;
    devcfg_final.clock_speed_hz = spiHz;
    devcfg_final.spics_io_num = pinCS;
    devcfg_final.command_bits = 16;
    devcfg_final.address_bits = 8;
    devcfg_final.flags = SPI_DEVICE_NO_DUMMY;
    devcfg_final.queue_size = 4;

    // Add device
    esp_err_t eDev = spi_bus_add_device(_spiHost, &devcfg_final, &_spi);
    if (eDev != ESP_OK) { LOG_ERR("spi_bus_add_device(final)", eDev); return false; }

    // MAC/PHY W5500
    spi_host_device_t host = SPI2_HOST;              // ou SPI3_HOST selon ton montage
    eth_w5500_config_t mac_cfg = ETH_W5500_DEFAULT_CONFIG(host, &devcfg_final);
    mac_cfg.int_gpio_num = _pinIRQ;

    // Mode polling
    #if defined(W5500_HAS_POLLING)
      if (_pinIRQ < 0) {
          mac_cfg.poll_period_ms = 10; // 10 ms
      }
    #else
      if (_pinIRQ < 0) {
          ESP_LOGE(TAG, "IRQ require");
          return false;
      }
    #endif

    eth_mac_config_t mac_common = ETH_MAC_DEFAULT_CONFIG();
    mac_common.rx_task_stack_size = 4096;
    eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
    phy_cfg.phy_addr       = 0;
    phy_cfg.reset_gpio_num = _pinRST; // let driver do reset if available

    esp_eth_mac_t* mac = esp_eth_mac_new_w5500(&mac_cfg, &mac_common);
    esp_eth_phy_t* phy2 = esp_eth_phy_new_w5500(&phy_cfg);
    if (!mac || !phy2) { ESP_LOGE(TAG, "esp_eth_mac_new_w5500/phy_new_w5500 NULL"); return false; }

    esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy2);
    esp_err_t e3 = esp_eth_driver_install(&eth_cfg, &_eth);
    if (e3 != ESP_OK) { LOG_ERR("esp_eth_driver_install", e3); return false; }

    // Mac
    _mac = chosenMac;
    esp_err_t eMac = esp_eth_ioctl(_eth, ETH_CMD_S_MAC_ADDR, (void*)_mac.data());
    if (eMac != ESP_OK) {
        ESP_LOGE(TAG, "ETH_CMD_S_MAC_ADDR -> %s", esp_err_to_name(eMac));
    }

    // netif + glue
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    _netif = esp_netif_new(&cfg);
    if (!_netif) { ESP_LOGE(TAG, "esp_netif_new failed"); return false; }

    _glue = esp_eth_new_netif_glue(_eth);
    if (!_glue) { ESP_LOGE(TAG, "esp_eth_new_netif_glue failed"); return false; }

    esp_err_t e4 = esp_netif_attach(_netif, _glue);
    if (e4 != ESP_OK) { LOG_ERR("esp_netif_attach", e4); return false; }

    // Hostname
    esp_netif_set_hostname(_netif, "esp32-buspirate-eth");

    // Handlers
    esp_err_t e5 = esp_event_handler_instance_register(ETH_EVENT, ESP_EVENT_ANY_ID, &EthernetService::onEthEvent, this, &_ethHandler);
    if (e5 != ESP_OK) { LOG_ERR("handler ETH_EVENT register", e5); return false; }

    esp_err_t e6 = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &EthernetService::onIpEvent, this, &_ipHandler);
    if (e6 != ESP_OK) { LOG_ERR("handler IP_EVENT register", e6); return false; }

    _configured = true;

    #endif

    return true;
}

void EthernetService::hardReset() {
    pinMode(_pinRST, OUTPUT);
    digitalWrite(_pinRST, LOW);  delay(5);
    digitalWrite(_pinRST, HIGH); delay(200);
}

bool EthernetService::beginDHCP(unsigned long timeoutMs) {
    if (!_configured) { ESP_LOGE(TAG, "service non configure"); return false; }

    // Launch Ethernet
    esp_err_t eStart = esp_eth_start(_eth);
    if (eStart != ESP_OK && eStart != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_eth_start -> %s", esp_err_to_name(eStart));
        return false;
    }
    if (eStart == ESP_ERR_INVALID_STATE) ESP_LOGE(TAG, "esp_eth_start: Eth already started");

    // Launch DHCP
    esp_err_t eDhcp = esp_netif_dhcpc_start(_netif);
    
    // Wait LINK
    const unsigned long t0 = millis();
    while (millis() - t0 < timeoutMs) {
        if (_linkUp) break;
        delay(25);
    }

    // Wait IP
    const unsigned long t1 = millis();
    while (millis() - t1 < timeoutMs) {
        if (_gotIP) {
            return true;
        }
        delay(25);
    }

    return false;
}

static std::string ip4_to_string_(const ip4_addr_t& a) {
    char buf[16];
    ip4addr_ntoa_r(&a, buf, sizeof(buf));
    return std::string(buf);
}

std::string EthernetService::getMac() const {
    uint8_t m[6] = {0};
    if (_eth) {
        esp_err_t e = esp_eth_ioctl(_eth, ETH_CMD_G_MAC_ADDR, m);
        // if (e != ESP_OK) LOG_ERR("ETH_CMD_G_MAC_ADDR", e);
    } else {
        memcpy(m, _mac.data(), 6);
    }
    char buf[18];
    sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", m[0],m[1],m[2],m[3],m[4],m[5]);
    return std::string(buf);
}

bool EthernetService::isConnected() const { return _linkUp && _gotIP; }
int  EthernetService::linkStatusRaw() const { return _linkUp ? 1 : 0; }
bool EthernetService::linkUp() const { return _linkUp; }
std::string EthernetService::ip4ToString(const ip4_addr_t& a) const { return ip4_to_string_(a); }
std::string EthernetService::getLocalIP() const   { return ip4_to_string_(_ip); }
std::string EthernetService::getSubnetMask() const{ return ip4_to_string_(_mask); }
std::string EthernetService::getGatewayIp() const { return ip4_to_string_(_gw); }
std::string EthernetService::getDns() const       { return ip4_to_string_(_dns0); }

//================== Events =============================

void EthernetService::onEthEvent(void* arg, esp_event_base_t, int32_t id, void*) {
    auto* self = static_cast<EthernetService*>(arg);
    if (!self) { ESP_LOGE(TAG, "onEthEvent self=null"); return; }

    switch (id) {
        case ETHERNET_EVENT_CONNECTED:
            self->_linkUp = true;
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            self->_linkUp = false;
            self->_gotIP  = false;
            break;
        case ETHERNET_EVENT_START:
            break;
        case ETHERNET_EVENT_STOP:
            break;
        default:
            break;
    }
}

void EthernetService::onIpEvent(void* arg, esp_event_base_t, int32_t id, void* data) {
    auto* self = static_cast<EthernetService*>(arg);
    if (!self) { ESP_LOGE(TAG, "onIpEvent self=null"); return; }

    if (id == IP_EVENT_ETH_GOT_IP) {
        auto* e = reinterpret_cast<ip_event_got_ip_t*>(data);
        self->_ip.addr   = e->ip_info.ip.addr;
        self->_gw.addr   = e->ip_info.gw.addr;
        self->_mask.addr = e->ip_info.netmask.addr;

        esp_netif_dns_info_t dns{};
        if (esp_netif_get_dns_info(self->_netif, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK) {
            self->_dns0.addr = dns.ip.u_addr.ip4.addr;
        } else {
            self->_dns0.addr = 0;
        }
        self->_gotIP = true;

        char ip[16], gw[16], mask[16];
        ip4addr_ntoa_r(to_lwip(&e->ip_info.ip), ip, sizeof(ip));
        ip4addr_ntoa_r(to_lwip(&e->ip_info.gw), gw, sizeof(gw));
        ip4addr_ntoa_r(to_lwip(&e->ip_info.netmask), mask, sizeof(mask));
    }
}

// ==================== W5500 Test Helpers ====================

static bool _w5500_spi_read(spi_device_handle_t dev, uint16_t addr, uint8_t bsb, uint8_t& out) {
    if (!dev) return false;
    uint8_t tx[4] = { (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF), (uint8_t)((bsb << 3) | 0x01), 0x00 };
    uint8_t rx[4] = {0};
    spi_transaction_t t{};
    t.length = 8 * sizeof(tx);
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    esp_err_t e = spi_device_transmit(dev, &t);
    if (e != ESP_OK) return false;
    out = rx[3];
    return true;
}

static bool _w5500_probe(spi_device_handle_t dev, uint8_t& ver) {
    // VERSIONR @ common regs 0x0039 should be 0x04 on W5500
    if (!_w5500_spi_read(dev, 0x0039, 0x00, ver)) return false;
    return (ver != 0x00 && ver != 0xFF);
}

// W5500 1-byte FDM helpers (OM=01)
static bool _w5500_spi_write1(spi_device_handle_t dev, uint16_t addr, uint8_t bsb, uint8_t val) {
    if (!dev) return false;
    uint8_t tx[4] = { uint8_t(addr >> 8), uint8_t(addr), uint8_t((bsb << 3) | 0x04 | 0x01), val }; // RWB=1, OM=01
    spi_transaction_t t{}; t.length = 32; t.tx_buffer = tx;
    return spi_device_transmit(dev, &t) == ESP_OK;
}

static bool _w5500_spi_read1(spi_device_handle_t dev, uint16_t addr, uint8_t bsb, uint8_t& out) {
    if (!dev) return false;
    uint8_t tx[4] = { uint8_t(addr >> 8), uint8_t(addr), uint8_t((bsb << 3) | 0x00 | 0x01), 0x00 }; // RWB=0, OM=01
    uint8_t rx[4] = {0};
    spi_transaction_t t{}; t.length = 32; t.tx_buffer = tx; t.rx_buffer = rx;
    if (spi_device_transmit(dev, &t) != ESP_OK) return false;
    out = rx[3]; return true;
}

static bool _w5500_soft_reset(spi_device_handle_t dev) {
    if (!_w5500_spi_write1(dev, 0x0000, 0x00, 0x80)) return false; // MR bit7=RST
    delay(5);
    for (int i=0;i<50;i++) { uint8_t mr;
        if (!_w5500_spi_read1(dev, 0x0000, 0x00, mr)) return false;
        if ((mr & 0x80)==0) return true;
        delay(2);
    }
    return false;
}

// write+read sur SUBR (0x001A..0x001D) for validation
static bool _w5500_rw_selftest(spi_device_handle_t dev) {
    const uint16_t SUBR=0x001A; const uint8_t BSB=0x00; const uint8_t v[4]={255,255,255,0};
    for (int i=0;i<4;i++) if (!_w5500_spi_write1(dev, SUBR+i, BSB, v[i])) return false;
    for (int i=0;i<4;i++) { uint8_t r=0; if (!_w5500_spi_read1(dev, SUBR+i, BSB, r) || r!=v[i]) return false; }
    return true;
}
static bool _w5500_read_phycfgr(spi_device_handle_t dev, uint8_t& v) { // bit0=LNK, bit1=100M, bit2=FULL
    return _w5500_spi_read1(dev, 0x002E, 0x00, v);
}

static bool _w5500_spi_write(spi_device_handle_t dev, uint16_t addr, uint8_t bsb, uint8_t val) {
    if (!dev) return false;
    uint8_t tx[4] = { (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF), (uint8_t)((bsb << 3) | 0x04), val }; // RWB=0, OM=00
    spi_transaction_t t{};
    t.length = 8 * sizeof(tx);
    t.tx_buffer = tx;
    return (spi_device_transmit(dev, &t) == ESP_OK);
}