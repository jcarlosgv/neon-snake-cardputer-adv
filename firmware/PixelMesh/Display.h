#pragma once

#include <M5Cardputer.h>
#include <M5GFX.h>

constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 135;
constexpr int CELL_SIZE = 6;
constexpr int GRID_W = 40;
constexpr int GRID_H = 23;

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

struct InputState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool action = false;
};

class PixelDisplay {
public:
    bool begin();
    void clear(uint16_t color = TFT_BLACK);
    void cell(int x, int y, uint16_t color);
    void rect(int x, int y, int w, int h, uint16_t color);
    void entityCell(int x, int y, uint16_t color);
    void entityRect(int x, int y, int w, int h, uint16_t color);
    void smallCell(int x, int y, uint16_t color);
    void smallRect(int x, int y, int w, int h, uint16_t color);
    void push();

private:
    M5Canvas canvas{&M5Cardputer.Display};
};
