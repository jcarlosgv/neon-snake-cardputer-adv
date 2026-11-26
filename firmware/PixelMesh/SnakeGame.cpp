#include "SnakeGame.h"

namespace {
constexpr uint16_t PM_BG = TFT_BLACK;
constexpr uint16_t PM_GRID = rgb565(18, 36, 52);
constexpr uint16_t PM_CYAN = rgb565(45, 235, 220);
constexpr uint16_t PM_GREEN = rgb565(135, 240, 70);
constexpr uint16_t PM_WHITE = rgb565(235, 250, 235);
constexpr uint16_t PM_YELLOW = rgb565(255, 205, 60);
constexpr uint16_t PM_ORANGE = rgb565(255, 105, 35);
constexpr uint16_t PM_MAGENTA = rgb565(240, 65, 165);
constexpr uint16_t PM_PURPLE = rgb565(165, 65, 215);
constexpr uint16_t PM_HEAD = rgb565(255, 180, 240);
constexpr uint16_t PM_BLUE = rgb565(70, 145, 245);
constexpr uint16_t PM_RED = rgb565(245, 65, 75);
}

void SnakeGame::begin() {
    preferences.begin("neon-snake", false);
    highScore = preferences.getUInt("highscore", 0);
    reset();
}

void SnakeGame::start() {
    started = true;
    reset();
}

void SnakeGame::reset() {
    tick = 0;
    score = 0;
    foodCount = 0;
    lives = 3;
    invincibleTimer = 0;
    powerTimer = 0;
    slowTimer = 0;
    blastTimer = 0;
    heartTimer = 0;
    gameOver = false;
    memset(lines, 0, sizeof(lines));
    resetSnake();
    spawnFood();
}

void SnakeGame::resetSnake() {
    snakeLength = 6;
    direction = 1;
    for (uint8_t i = 0; i < snakeLength; ++i) snake[i] = {static_cast<uint8_t>(16 - i), 13};
}

void SnakeGame::spawnFood() {
    bool occupied;
    do {
        food = {static_cast<uint8_t>(random(2, GRID_W - 2)), static_cast<uint8_t>(random(3, GRID_H - 2))};
        occupied = false;
        for (uint16_t i = 0; i < snakeLength; ++i) {
            if (abs((int)snake[i].x - food.x) <= 1 && abs((int)snake[i].y - food.y) <= 1) occupied = true;
        }
    } while (occupied);
}

void SnakeGame::spawnPower() {
    power = {static_cast<uint8_t>(random(4, GRID_W - 4)), static_cast<uint8_t>(random(5, GRID_H - 5))};
    powerType = random(3);
    if (powerType == POWER_HEART && lives >= 5) powerType = POWER_SLOW;
    powerTimer = 100;
}

void SnakeGame::spawnLine(uint8_t index) {
    Line& line = lines[index];
    line.vertical = random(2);
    line.direction = random(2) ? 1 : -1;
    line.length = 8 + random(6);
    line.lane = line.vertical ? random(4, GRID_W - 4) : random(5, GRID_H - 5);
    int limit = line.vertical ? GRID_H : GRID_W;
    line.position = line.direction == 1 ? -line.length : limit + line.length;
    line.active = true;
}

uint8_t SnakeGame::activeLineCount() const {
    uint8_t count = 0;
    for (const auto& line : lines) if (line.active) ++count;
    return count;
}

uint8_t SnakeGame::lineLength(const Line& line) const {
    return line.length;
}

void SnakeGame::loseLife() {
    if (invincibleTimer || gameOver) return;
    --lives;
    if (!lives) {
        gameOver = true;
        if (score > highScore) {
            highScore = score;
            preferences.putUInt("highscore", highScore);
        }
        return;
    }
    resetSnake();
    memset(lines, 0, sizeof(lines));
    invincibleTimer = 40;
    slowTimer = 0;
}

void SnakeGame::collectPower() {
    if (powerType == POWER_BLAST) {
        blastX = power.x;
        blastY = power.y;
        blastTimer = 35;
        memset(lines, 0, sizeof(lines));
    } else if (powerType == POWER_SLOW) {
        slowTimer = 160;
    } else {
        if (lives < 5) ++lives;
        heartTimer = 35;
    }
    powerTimer = 0;
}

void SnakeGame::update(const InputState& input) {
    ++tick;
    if (!started || gameOver) return;
    if (invincibleTimer) --invincibleTimer;
    if (powerTimer) --powerTimer;
    if (slowTimer) --slowTimer;
    if (blastTimer) --blastTimer;
    if (heartTimer) --heartTimer;
    if (tick % 100 == 0 && score <= 99994) score += 5;
    if (!powerTimer && tick > 120 && tick % 320 == 0) spawnPower();

    uint8_t targetLines = 2 + foodCount / 2;
    if (targetLines > MAX_LINES) targetLines = MAX_LINES;
    if (tick > 50 && tick % 45 == 0 && activeLineCount() < targetLines) {
        for (uint8_t i = 0; i < MAX_LINES; ++i) {
            if (!lines[i].active) {
                spawnLine(i);
                break;
            }
        }
    }

    uint8_t obstacleInterval = slowTimer ? 12 : 7;
    if (tick % obstacleInterval == 0) {
        for (auto& line : lines) {
            if (!line.active) continue;
            line.position += line.direction;
            int limit = line.vertical ? GRID_H : GRID_W;
            if ((line.direction == 1 && line.position > limit + line.length + 1) ||
                (line.direction == -1 && line.position < -line.length - 1)) line.active = false;
        }
    }

    uint8_t requested = direction;
    if (input.up) requested = 0;
    else if (input.right) requested = 1;
    else if (input.down) requested = 2;
    else if (input.left) requested = 3;
    if ((requested + 2) % 4 != direction) direction = requested;

    uint8_t snakeInterval = input.action ? 2 : 4;
    if (tick % snakeInterval == 0) {
        for (uint16_t i = snakeLength - 1; i > 0; --i) snake[i] = snake[i - 1];
        const int8_t dx[4] = {0, 1, 0, -1};
        const int8_t dy[4] = {-1, 0, 1, 0};
        snake[0].x = (snake[0].x + dx[direction] + GRID_W) % GRID_W;
        snake[0].y = (snake[0].y + dy[direction] + GRID_H) % GRID_H;

        for (uint16_t i = 1; i < snakeLength; ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                loseLife();
                return;
            }
        }
        if (snake[0].x >= food.x && snake[0].x < food.x + 2 && snake[0].y >= food.y && snake[0].y < food.y + 2) {
            score = score > 99899 ? 99999 : score + 100;
            ++foodCount;
            uint16_t oldLength = snakeLength;
            for (uint8_t i = 0; i < 3 && snakeLength < MAX_SNAKE; ++i) {
                snake[snakeLength] = snake[oldLength - 1];
                ++snakeLength;
            }
            spawnFood();
        }
        if (powerTimer && snake[0].x >= power.x && snake[0].x < power.x + 2 && snake[0].y >= power.y && snake[0].y < power.y + 2) collectPower();
    }

    if (invincibleTimer) return;
    for (auto& line : lines) {
        if (!line.active) continue;
        uint8_t length = lineLength(line);
        for (uint8_t i = 0; i < length; ++i) {
            int x = line.vertical ? line.lane : line.position - line.direction * i;
            int y = line.vertical ? line.position - line.direction * i : line.lane;
            if (snake[0].x == x && snake[0].y == y) {
                if (slowTimer) {
                    line.active = false;
                    score = score > 99499 ? 99999 : score + 500;
                } else {
                    loseLife();
                }
                return;
            }
        }
    }
}

void SnakeGame::drawHeart(PixelDisplay& display, int x, int y, uint16_t color) const {
    display.cell(x, y, color);
    display.cell(x + 2, y, color);
    display.rect(x, y + 1, 3, 1, color);
    display.entityCell(x + 1, y + 2, color);
}

void SnakeGame::drawClock(PixelDisplay& display, int x, int y) const {
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx || dy) display.entityCell(x + dx, y + dy, PM_CYAN);
        }
    }
    display.entityCell(x, y - 1, PM_WHITE);
    display.entityCell(x, y, PM_WHITE);
    display.entityCell(x + 1, y, PM_WHITE);
}

void SnakeGame::drawFood(PixelDisplay& display) const {
    display.entityRect(food.x, food.y, 2, 2, PM_YELLOW);
    display.entityCell(food.x + 1, food.y + 1, tick % 10 < 5 ? PM_WHITE : PM_ORANGE);
    display.entityCell(food.x + 1, food.y - 1, PM_GREEN);
}

void SnakeGame::drawPower(PixelDisplay& display) const {
    if (!powerTimer || (powerTimer < 20 && tick % 4 < 2)) return;
    if (powerType == POWER_BLAST) {
        display.entityRect(power.x, power.y, 2, 2, PM_ORANGE);
        display.entityCell(power.x + 1, power.y - 1, PM_YELLOW);
        display.entityCell(power.x + 2, power.y - 2, PM_WHITE);
    } else if (powerType == POWER_SLOW) {
        display.entityCell(power.x, power.y, PM_CYAN);
        display.entityCell(power.x + 1, power.y, PM_WHITE);
        display.entityCell(power.x, power.y + 1, PM_WHITE);
        display.entityCell(power.x + 1, power.y + 1, PM_CYAN);
    } else {
        display.entityRect(power.x, power.y, 2, 2, PM_MAGENTA);
        display.entityCell(power.x + 1, power.y, PM_HEAD);
    }
}

void SnakeGame::drawDigit(PixelDisplay& display, int x, int y, uint8_t digit, uint16_t color, bool compact) const {
    static const uint8_t FONT[10][5] = {
        {0x7, 0x5, 0x5, 0x5, 0x7}, {0x2, 0x6, 0x2, 0x2, 0x7},
        {0x7, 0x1, 0x7, 0x4, 0x7}, {0x7, 0x1, 0x7, 0x1, 0x7},
        {0x5, 0x5, 0x7, 0x1, 0x1}, {0x7, 0x4, 0x7, 0x1, 0x7},
        {0x7, 0x4, 0x7, 0x5, 0x7}, {0x7, 0x1, 0x1, 0x2, 0x2},
        {0x7, 0x5, 0x7, 0x5, 0x7}, {0x7, 0x5, 0x7, 0x1, 0x7}
    };
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (FONT[digit][row] & (1 << (2 - col))) {
                if (compact) display.smallCell(x + col, y + row, color);
                else display.cell(x + col, y + row, color);
            }
        }
    }
}

void SnakeGame::drawHud(PixelDisplay& display) const {
    for (uint8_t i = 0; i < 5; ++i) {
        uint16_t color = i < lives ? PM_MAGENTA : rgb565(70, 40, 65);
        drawHeart(display, i * 4, 0, color);
    }
    uint32_t value = score;
    for (int i = 0; i < 5; ++i) {
        drawDigit(display, GRID_W - 3 - i * 4, 0, value % 10, PM_YELLOW);
        value /= 10;
    }
    if (slowTimer) {
        uint8_t width = slowTimer / 2;
        for (uint8_t i = 0; i < width; ++i) display.cell(i, GRID_H - 1, PM_CYAN);
    }
}

void SnakeGame::drawLetter(PixelDisplay& display, int x, int y, char letter, uint16_t color) const {
    uint8_t rows[5] = {0, 0, 0, 0, 0};
    switch (letter) {
        case 'A': memcpy(rows, (const uint8_t[]){2, 5, 7, 5, 5}, 5); break;
        case 'B': memcpy(rows, (const uint8_t[]){6, 5, 6, 5, 6}, 5); break;
        case 'E': memcpy(rows, (const uint8_t[]){7, 4, 6, 4, 7}, 5); break;
        case 'G': memcpy(rows, (const uint8_t[]){7, 4, 5, 5, 7}, 5); break;
        case 'K': memcpy(rows, (const uint8_t[]){5, 5, 6, 5, 5}, 5); break;
        case 'M': memcpy(rows, (const uint8_t[]){5, 7, 7, 5, 5}, 5); break;
        case 'N': memcpy(rows, (const uint8_t[]){5, 7, 7, 5, 5}, 5); break;
        case 'O': memcpy(rows, (const uint8_t[]){7, 5, 5, 5, 7}, 5); break;
        case 'P': memcpy(rows, (const uint8_t[]){6, 5, 6, 4, 4}, 5); break;
        case 'R': memcpy(rows, (const uint8_t[]){6, 5, 6, 5, 5}, 5); break;
        case 'S': memcpy(rows, (const uint8_t[]){7, 4, 7, 1, 7}, 5); break;
        case 'T': memcpy(rows, (const uint8_t[]){7, 2, 2, 2, 2}, 5); break;
        case 'V': memcpy(rows, (const uint8_t[]){5, 5, 5, 5, 2}, 5); break;
        default: break;
    }
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 3; ++col) {
            if (rows[row] & (1 << (2 - col))) display.smallCell(x + col, y + row, color);
        }
    }
}

void SnakeGame::drawText(PixelDisplay& display, int x, int y, const char* text, uint16_t color) const {
    int cursor = x;
    while (*text) {
        if (*text >= '0' && *text <= '9') drawDigit(display, cursor, y, *text - '0', color, true);
        else drawLetter(display, cursor, y, *text, color);
        cursor += 4;
        ++text;
    }
}

void SnakeGame::drawBigLetter(PixelDisplay& display, int x, int y, char letter, uint16_t color) const {
    uint8_t rows[7] = {0, 0, 0, 0, 0, 0, 0};
    switch (letter) {
        case 'A': memcpy(rows, (const uint8_t[]){14, 17, 17, 31, 17, 17, 17}, 7); break;
        case 'E': memcpy(rows, (const uint8_t[]){31, 16, 16, 30, 16, 16, 31}, 7); break;
        case 'K': memcpy(rows, (const uint8_t[]){17, 18, 20, 24, 20, 18, 17}, 7); break;
        case 'N': memcpy(rows, (const uint8_t[]){17, 25, 25, 21, 19, 19, 17}, 7); break;
        case 'O': memcpy(rows, (const uint8_t[]){14, 17, 17, 17, 17, 17, 14}, 7); break;
        case 'S': memcpy(rows, (const uint8_t[]){15, 16, 16, 14, 1, 1, 30}, 7); break;
        default: break;
    }
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (rows[row] & (1 << (4 - col))) display.smallCell(x + col, y + row, color);
        }
    }
}

void SnakeGame::drawBigText(PixelDisplay& display, int x, int y, const char* text, uint16_t color) const {
    int cursor = x;
    while (*text) {
        drawBigLetter(display, cursor, y, *text, color);
        cursor += 6;
        ++text;
    }
}

void SnakeGame::drawStartScreen(PixelDisplay& display) const {
    display.clear(TFT_BLACK);
    static const uint8_t STARS[][2] = {
        {7, 6}, {13, 2}, {18, 7}, {27, 1}, {34, 6}, {43, 2}, {51, 7},
        {5, 13}, {55, 14}, {9, 20}, {18, 22}, {42, 21}, {51, 19},
        {3, 28}, {14, 30}, {26, 29}, {37, 31}, {47, 29}, {57, 25}
    };
    for (uint8_t i = 0; i < sizeof(STARS) / sizeof(STARS[0]); ++i) {
        display.smallCell(STARS[i][0], STARS[i][1], i % 3 ? rgb565(24, 28, 52) : rgb565(62, 24, 70));
    }
    for (uint8_t i = 0; i < 6; ++i) {
        uint16_t color = rgb565(70, 25, 90);
        display.smallCell(3 + i, 1, color);
        display.smallCell(3, 1 + i, color);
        display.smallCell(56 - i, 1, color);
        display.smallCell(56, 1 + i, color);
        display.smallCell(3 + i, 31, color);
        display.smallCell(3, 31 - i, color);
        display.smallCell(56 - i, 31, color);
        display.smallCell(56, 31 - i, color);
    }
    drawBigText(display, 20, 5, "NEON", rgb565(75, 30, 85));
    drawBigText(display, 17, 14, "SNAKE", rgb565(75, 30, 85));
    drawBigText(display, 19, 4, "NEON", PM_HEAD);
    drawBigText(display, 16, 13, "SNAKE", PM_MAGENTA);
    for (uint8_t x = 15; x <= 45; x += 2) {
        display.smallCell(x, 22, x % 4 ? PM_PURPLE : PM_MAGENTA);
    }
    display.smallCell(30, 23, PM_HEAD);

    display.smallCell(5, 26, PM_MAGENTA);
    display.smallCell(6, 27, PM_MAGENTA);
    display.smallCell(5, 28, PM_MAGENTA);
    display.smallCell(54, 26, PM_MAGENTA);
    display.smallCell(53, 27, PM_MAGENTA);
    display.smallCell(54, 28, PM_MAGENTA);
    if (tick % 24 < 16) {
        drawText(display, 10, 26, "PRESS ENTER", rgb565(95, 65, 25));
        drawText(display, 9, 25, "PRESS ENTER", PM_YELLOW);
    }
}

void SnakeGame::drawGameOver(PixelDisplay& display) const {
    display.smallRect(7, 7, 46, 20, TFT_BLACK);
    display.smallCell(7, 7, PM_PURPLE);
    display.smallCell(52, 7, PM_PURPLE);
    display.smallCell(7, 26, PM_PURPLE);
    display.smallCell(52, 26, PM_PURPLE);
    drawText(display, 14, 11, "GAME OVER", rgb565(80, 40, 90));
    drawText(display, 13, 10, "GAME OVER", PM_HEAD);
    drawText(display, 12, 19, "BEST ", rgb565(95, 65, 25));
    drawText(display, 11, 18, "BEST ", PM_YELLOW);
    uint32_t value = highScore;
    for (int i = 0; i < 5; ++i) {
        int x = 47 - i * 4;
        drawDigit(display, x + 1, 19, value % 10, rgb565(95, 65, 25), true);
        drawDigit(display, x, 18, value % 10, PM_YELLOW, true);
        value /= 10;
    }
}

void SnakeGame::render(PixelDisplay& display) {
    if (!started) {
        drawStartScreen(display);
        return;
    }
    display.clear(PM_BG);
    for (int i = 0; i < 48; ++i) display.cell((i * 37 + 7) % GRID_W, (i * 19 + 4) % GRID_H, PM_GRID);

    for (uint8_t index = 0; index < MAX_LINES; ++index) {
        const auto& line = lines[index];
        if (!line.active) continue;
        uint16_t color = slowTimer ? PM_CYAN : (line.vertical ? PM_BLUE : PM_ORANGE);
        uint8_t length = lineLength(line);
        for (uint8_t i = 0; i < length; ++i) {
            int x = line.vertical ? line.lane : line.position - line.direction * i;
            int y = line.vertical ? line.position - line.direction * i : line.lane;
            if (slowTimer && i == 0) drawClock(display, x, y);
            else display.entityCell(x, y, i == 0 ? PM_WHITE : color);
            if (i > 1 && i < length - 1 && i % 3 == 0) {
                if (line.vertical) display.entityCell(x + 1, y, rgb565(70, 35, 75));
                else display.entityCell(x, y + 1, rgb565(75, 45, 35));
            }
        }
    }

    drawFood(display);
    drawPower(display);
    for (int i = (int)snakeLength - 1; i >= 0; --i) {
        if ((invincibleTimer && tick % 4 < 2) || i == 0) continue;
        display.entityCell(snake[i].x, snake[i].y, i % 2 ? PM_MAGENTA : PM_PURPLE);
    }
    if (!invincibleTimer || tick % 4 >= 2) {
        int hx = snake[0].x;
        int hy = snake[0].y;
        display.entityRect(hx, hy, 2, 2, PM_HEAD);
        if (direction == 0) {
            display.entityCell(hx, hy, PM_BG);
            display.entityCell(hx + 1, hy, PM_BG);
            if (tick % 10 < 4) display.entityCell(hx, hy - 1, PM_RED);
        } else if (direction == 1) {
            display.entityCell(hx + 1, hy, PM_BG);
            display.entityCell(hx + 1, hy + 1, PM_BG);
            if (tick % 10 < 4) display.entityCell(hx + 2, hy, PM_RED);
        } else if (direction == 2) {
            display.entityCell(hx, hy + 1, PM_BG);
            display.entityCell(hx + 1, hy + 1, PM_BG);
            if (tick % 10 < 4) display.entityCell(hx, hy + 2, PM_RED);
        } else {
            display.entityCell(hx, hy, PM_BG);
            display.entityCell(hx, hy + 1, PM_BG);
            if (tick % 10 < 4) display.entityCell(hx - 1, hy, PM_RED);
        }
    }

    if (blastTimer) {
        int radius = (35 - blastTimer) * 2;
        for (int i = -radius; i <= radius; ++i) {
            display.entityCell(blastX + i, blastY - radius, i % 2 ? PM_YELLOW : PM_WHITE);
            display.entityCell(blastX + i, blastY + radius, i % 2 ? PM_YELLOW : PM_WHITE);
            display.entityCell(blastX - radius, blastY + i, i % 2 ? PM_YELLOW : PM_WHITE);
            display.entityCell(blastX + radius, blastY + i, i % 2 ? PM_YELLOW : PM_WHITE);
        }
    }
    if (heartTimer) {
        int rise = (35 - heartTimer) / 5;
        drawHeart(display, snake[0].x - 1, snake[0].y - 5 - rise, PM_MAGENTA);
        drawHeart(display, snake[0].x + 4, snake[0].y - 3 - rise, PM_WHITE);
    }
    drawHud(display);
    if (gameOver) drawGameOver(display);
}
