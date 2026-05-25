#pragma once
#ifdef DEVICE_CARDPUTER

#include <M5UnitGLASS2.h>

#define GLASS2_SDA 2
#define GLASS2_SCL 1

static M5UnitGLASS2 _g2(GLASS2_SDA, GLASS2_SCL);
static bool _g2_ready = false;

inline void glass2Init() {
    _g2_ready = _g2.init();
    if (_g2_ready) { _g2.fillScreen(TFT_BLACK); _g2.setTextColor(TFT_WHITE, TFT_BLACK); }
}

inline void glass2Show(const char* l1, const char* l2 = nullptr) {
    if (!_g2_ready) return;
    _g2.fillScreen(TFT_BLACK); _g2.setTextSize(1); _g2.setTextColor(TFT_WHITE, TFT_BLACK);
    if (l1) { _g2.setCursor(0,  8); _g2.print(l1); }
    if (l2) { _g2.setCursor(0, 40); _g2.print(l2); }
}

#endif // DEVICE_CARDPUTER
