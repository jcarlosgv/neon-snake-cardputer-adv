#include "Display.h"

bool PixelDisplay::begin() {
    M5Cardputer.Display.setRotation(0);
    canvas.setColorDepth(16);
    if (canvas.createSprite(SCREEN_W, SCREEN_H) == nullptr) return false;
    canvas.fillScreen(TFT_BLACK);
    canvas.pushSprite(0, 0);
    return true;
}

void PixelDisplay::clear(uint16_t color) {
    canvas.fillScreen(color);
}

void PixelDisplay::cell(int x, int y, uint16_t color) {
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    int width = min(CELL_SIZE - 1, SCREEN_W - x * CELL_SIZE);
    int height = min(CELL_SIZE - 1, SCREEN_H - y * CELL_SIZE);
    canvas.fillRect(x * CELL_SIZE, y * CELL_SIZE, width, height, color);
}

void PixelDisplay::rect(int x, int y, int w, int h, uint16_t color) {
    for (int row = y; row < y + h; ++row) {
        for (int col = x; col < x + w; ++col) cell(col, row, color);
    }
}

void PixelDisplay::entityCell(int x, int y, uint16_t color) {
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return;
    int width = min(CELL_SIZE - 1, SCREEN_W - x * CELL_SIZE);
    int height = min(CELL_SIZE - 1, SCREEN_H - y * CELL_SIZE);
    canvas.fillRect(x * CELL_SIZE, y * CELL_SIZE, width, height, color);
}

void PixelDisplay::entityRect(int x, int y, int w, int h, uint16_t color) {
    for (int row = y; row < y + h; ++row) {
        for (int col = x; col < x + w; ++col) entityCell(col, row, color);
    }
}

void PixelDisplay::smallCell(int x, int y, uint16_t color) {
    if (x < 0 || x >= 60 || y < 0 || y >= 34) return;
    canvas.fillRect(x * 4, y * 4, 3, 3, color);
}

void PixelDisplay::smallRect(int x, int y, int w, int h, uint16_t color) {
    canvas.fillRect(x * 4, y * 4, w * 4 - 1, h * 4 - 1, color);
}

void PixelDisplay::push() {
    canvas.pushSprite(0, 0);
}
