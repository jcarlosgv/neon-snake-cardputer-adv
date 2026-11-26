#pragma once

#include "Display.h"
#include <Preferences.h>

class SnakeGame {
public:
    void begin();
    void start();
    void reset();
    void update(const InputState& input);
    void render(PixelDisplay& display);
    bool isStarted() const { return started; }
    bool isGameOver() const { return gameOver; }

private:
    struct Point { uint8_t x; uint8_t y; };
    struct Line {
        int16_t position;
        uint8_t lane;
        uint8_t length;
        int8_t direction;
        bool vertical;
        bool active;
    };
    enum PowerType : uint8_t { POWER_BLAST = 0, POWER_SLOW = 1, POWER_HEART = 2 };

    static constexpr uint8_t MAX_SNAKE = 240;
    static constexpr uint8_t MAX_LINES = 14;
    Point snake[MAX_SNAKE]{};
    Line lines[MAX_LINES]{};
    Point food{35, 16};
    Point power{0, 0};
    uint16_t snakeLength = 6;
    uint32_t score = 0;
    uint32_t highScore = 0;
    Preferences preferences;
    uint16_t foodCount = 0;
    uint32_t tick = 0;
    uint8_t direction = 1;
    uint8_t lives = 3;
    uint8_t invincibleTimer = 0;
    uint8_t powerType = 0;
    uint8_t powerTimer = 0;
    uint8_t slowTimer = 0;
    uint8_t blastTimer = 0;
    uint8_t heartTimer = 0;
    uint8_t blastX = 24;
    uint8_t blastY = 13;
    bool started = false;
    bool gameOver = false;

    void spawnFood();
    void spawnPower();
    void spawnLine(uint8_t index);
    void resetSnake();
    void loseLife();
    void collectPower();
    uint8_t activeLineCount() const;
    uint8_t lineLength(const Line& line) const;
    void drawHeart(PixelDisplay& display, int x, int y, uint16_t color) const;
    void drawClock(PixelDisplay& display, int x, int y) const;
    void drawFood(PixelDisplay& display) const;
    void drawPower(PixelDisplay& display) const;
    void drawHud(PixelDisplay& display) const;
    void drawDigit(PixelDisplay& display, int x, int y, uint8_t digit, uint16_t color, bool compact = false) const;
    void drawLetter(PixelDisplay& display, int x, int y, char letter, uint16_t color) const;
    void drawText(PixelDisplay& display, int x, int y, const char* text, uint16_t color) const;
    void drawBigLetter(PixelDisplay& display, int x, int y, char letter, uint16_t color) const;
    void drawBigText(PixelDisplay& display, int x, int y, const char* text, uint16_t color) const;
    void drawStartScreen(PixelDisplay& display) const;
    void drawGameOver(PixelDisplay& display) const;
};
