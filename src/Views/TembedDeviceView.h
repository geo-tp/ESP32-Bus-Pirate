#pragma once

#if defined(DEVICE_TEMBEDS3) || defined(DEVICE_TEMBEDS3CC1101)

#include "Interfaces/IDeviceView.h"
#include "States/GlobalState.h"

#include <Arduino.h>
#include <LovyanGFX.hpp>

#ifdef DEVICE_TEMBEDS3CC1101
  // ===== User_Setup 214 (CC1101) =====
  #define PIN_LCD_BL    21
  #define PIN_LCD_MISO  10
  #define PIN_LCD_MOSI  9
  #define PIN_LCD_SCLK  11
  #define PIN_LCD_CS    41
  #define PIN_LCD_DC    16
  #define PIN_LCD_RST   -1
#else
  // ===== User_Setup 210 (Normal S3) =====
  #define PIN_LCD_BL    15
  #define PIN_LCD_MISO  -1
  #define PIN_LCD_MOSI  11
  #define PIN_LCD_SCLK  12
  #define PIN_LCD_CS    10
  #define PIN_LCD_DC    13
  #define PIN_LCD_RST   9
#endif

#define PIN_POWER_ON 46
#define DARK_GREY_RECT 0x4208

// Lovyan driver
class LGFX_Tembed : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI      _bus;
  lgfx::Light_PWM    _light;

public:
  LGFX_Tembed() {
    {
    auto cfg = _bus.config();

    cfg.spi_host   = SPI2_HOST;
    cfg.spi_mode   = 0;

    cfg.freq_write = 40000000; 
    cfg.freq_read  = 20000000;

    cfg.pin_sclk = PIN_LCD_SCLK;
    cfg.pin_mosi = PIN_LCD_MOSI;
    cfg.pin_miso = PIN_LCD_MISO;
    cfg.pin_dc   = PIN_LCD_DC;

    cfg.spi_3wire = false;
    cfg.dma_channel = SPI_DMA_CH_AUTO;

    _bus.config(cfg);
    _panel.setBus(&_bus);
    }

    // --- PANEL
    {
    auto cfg = _panel.config();


    cfg.pin_cs   = PIN_LCD_CS;
    cfg.pin_rst  = PIN_LCD_RST;
    cfg.pin_busy = -1;

    cfg.panel_width  = 170;
    cfg.panel_height = 320;

    // VRAM interne 240x320
    cfg.memory_width  = 240;
    cfg.memory_height = 320;
    cfg.offset_x = 35;
    cfg.offset_y = 0;
    
    cfg.invert = true;
    cfg.rgb_order = false;  // RGB

    cfg.dlen_16bit = false;
    _panel.config(cfg);
    }

    setPanel(&_panel);
  }
};

class TembedDeviceView : public IDeviceView {
public:
  TembedDeviceView();

  void initialize() override;
  SPIClass& getScreenSpiInstance() override;
  void* getScreen() override;
  void logo() override;
  void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
  void show(PinoutConfig& config) override;
  void loading() override;
  void clear() override;
  void drawLogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
  void drawAnalogicTrace(uint8_t pin, const std::vector<uint8_t>& buffer, uint8_t step) override;
  void setRotation(uint8_t rotation) override;
  void setBrightness(uint8_t brightness) override;
  uint8_t getBrightness() override;
  void topBar(const std::string& title, bool submenu, bool searchBar) override;
  void horizontalSelection(
    const std::vector<std::string>& options,
    uint16_t selectedIndex,
    const std::string& description1,
    const std::string& description2
  ) override;

  void shutDown();

  
private:
  LGFX_Tembed tft;
  //   lgfx::LGFX_Sprite canvas; // Not used currently, memory consumption is too high
  uint8_t brightnessPct = 100;
  SPIClass screenSpi{HSPI}; // or FSPI

  void drawCenterText(const std::string& text, int y, int fontSize);
  void welcomeWeb(const std::string& ip);
  void welcomeSerial(const std::string& baud);
};

#endif
