#include <M5Cardputer.h>
#include "Display.h"
#include "SnakeGame.h"

PixelDisplay pixelDisplay;
SnakeGame snakeGame;
uint32_t lastFrame = 0;

void setup() {
    auto config = M5.config();
    M5Cardputer.begin(config, true);
    pixelDisplay.begin();
    snakeGame.begin();
}

void loop() {
    M5Cardputer.update();
    auto& keys = M5Cardputer.Keyboard.keysState();
    InputState input;
    input.up = M5Cardputer.Keyboard.isKeyPressed(';');
    input.down = M5Cardputer.Keyboard.isKeyPressed('.');
    input.left = M5Cardputer.Keyboard.isKeyPressed(',');
    input.right = M5Cardputer.Keyboard.isKeyPressed('/');
    input.action = keys.space;

    if (!snakeGame.isStarted() && keys.enter) snakeGame.start();
    else if (M5Cardputer.BtnA.wasPressed() || (snakeGame.isGameOver() && keys.enter)) snakeGame.reset();

    uint32_t now = millis();
    if (now - lastFrame >= 50) {
        lastFrame = now;
        snakeGame.update(input);
        snakeGame.render(pixelDisplay);
        pixelDisplay.push();
    }
    delay(2);
}
