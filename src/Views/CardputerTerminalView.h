#ifdef DEVICE_CARDPUTER

#pragma once

#include <string>
#include <vector>
#include <deque>
#include <stdint.h>

#include <M5Cardputer.h>
#include <Enums/TerminalTypeEnum.h>
#include "Interfaces/ITerminalView.h"
#include "Inputs/InputKeys.h"
#include "States/GlobalState.h"

#define BACKGROUND_COLOR TFT_BLACK
#define PRIMARY_COLOR 0xfa03
#define RECT_COLOR_DARK 0x0841
#define RECT_COLOR_LIGHT 0xd69a
#define TEXT_COLOR TFT_GREEN
#define TEXT_COLOR_ALT TFT_DARKGRAY

#define DEFAULT_MARGIN 5
#define DEFAULT_ROUND_RECT 5

class CardputerTerminalView : public ITerminalView {
public:
    ~CardputerTerminalView() override = default;

    void initialize() override;
    void welcome(TerminalTypeEnum& terminalType, std::string& terminalInfos) override;
    void print(const std::string& text) override;
    void print(const uint8_t data) override;
    void println(const std::string& text) override;
    void printPrompt(const std::string& mode = "HIZ") override;
    void waitPress() override;
    void clear() override;

private:
    // Terminal emulation
    void termReset();
    void termPutChar(char c);
    void termPutText(const char* s, size_t n);
    void termNewLine();
    void termCarriageReturn();
    void termBackspace();
    void termScrollUp();
    void termEraseInLine(int mode);     // 0: cursor→EOL, 1: SOL→cursor, 2: whole line
    void termEraseInDisplay(int mode);  // 0: cursor→end, 1: start→cursor, 2: all
    void termMoveCursorRel(int dx, int dy);
    void termMoveCursorAbs(int row1, int col1); // 1-based

    // ANSI parser
    void ansiFeed(char c);
    void ansiReset();
    void ansiFinalizeCSI(char final);

    // Rendering
    void drawLine(const std::string& s, int16_t y, bool keepIfContent);
    void renderAll();
    void maybeRender();
    void recomputeMetrics();
    std::string htmlDecodeBasic(const std::string& s) const;
    std::string mapCodepointToASCII(uint32_t cp) const;
    void emitCodepoint(uint32_t cp);
    void feedFilteredByte(uint8_t data);
    void feedFilteredBytes(const uint8_t* data, size_t n);

private:
    // Screen buffer
    std::vector<std::string> lines;
    int rows = 10;
    int cols = 34;

    // Cursor 
    int curRow = 0;
    int curCol = 0;

    // Parser state
    bool inEsc = false;   // got ESC
    bool inCSI = false;   // got ESC [
    std::vector<int> csiParams; // collected numeric params
    int csiParamAcc = -1;

    // Layout (pixels)
    int16_t originX = 0, originY = 0; // top left text origin
    int16_t charW = 6, charH = 12;    // measured/assumed cell size
    int16_t scrW = 0, scrH = 0;

    // Options
    bool useMonospace = true;   // try a monospace font for stable grid

    // Double buffer monochrome
    LGFX_Sprite termSprite{ &M5Cardputer.Display };
    bool spriteReady = false;

    // UTF-8 decode state
    uint32_t u8_cp = 0;
    int      u8_rem = 0;

    int16_t padX = 6;   // marge horizontale
    int16_t padY = 4;   // marge verticale

    uint32_t lastRenderMs = 0;
    uint32_t frameIntervalMs = 16; // 60 FPS
    bool     dirty = false;
    bool instantRender = false;

    // Scrollback
    std::deque<std::string> history;
    size_t historyMax = 256;
    int scrollOffset = 0;
    bool padBeforeErase = false;

};

#endif