'use strict';

const WIDTH = 40;
const HEIGHT = 23;
const SCALE = 6;
const MAX_SNAKE = 240;
const MAX_LINES = 14;
const COLORS = {
  bg: '#000000', grid: '#122434', cyan: '#2debD8', green: '#87f046', white: '#ebfaeb',
  yellow: '#ffcd3c', orange: '#ff6923', blue: '#4691f5', magenta: '#f041a5', purple: '#a541d7', head: '#ffb4f0', red: '#f5414b'
};
const random = (min, max) => Math.floor(Math.random() * (max - min)) + min;

class SnakeGame {
  constructor(canvas) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d', { alpha: false });
    this.ctx.imageSmoothingEnabled = false;
    this.highScore = Number(localStorage.getItem('neonSnakeHighScore') || 0);
    this.started = false;
    this.reset();
  }

  start() {
    this.started = true;
    this.reset();
  }

  reset() {
    this.tick = 0;
    this.score = 0;
    this.foodCount = 0;
    this.lives = 3;
    this.invincibleTimer = 0;
    this.powerTimer = 0;
    this.slowTimer = 0;
    this.blastTimer = 0;
    this.heartTimer = 0;
    this.gameOver = false;
    this.lines = Array.from({ length: MAX_LINES }, () => ({ active: false }));
    this.resetSnake();
    this.spawnFood();
    this.render();
  }

  resetSnake() {
    this.snake = Array.from({ length: 6 }, (_, i) => ({ x: 16 - i, y: 13 }));
    this.direction = 1;
  }

  spawnFood() {
    do {
      this.food = { x: random(2, WIDTH - 2), y: random(3, HEIGHT - 2) };
    } while (this.snake.some(part => Math.abs(part.x - this.food.x) <= 1 && Math.abs(part.y - this.food.y) <= 1));
  }

  spawnPower(type = null) {
    this.powerType = type ?? random(0, 3);
    if (this.powerType === 2 && this.lives >= 5) this.powerType = 1;
    this.power = { x: random(4, WIDTH - 4), y: random(5, HEIGHT - 5) };
    this.powerTimer = 100;
  }

  spawnLine(index) {
    const line = this.lines[index];
    line.vertical = Boolean(random(0, 2));
    line.direction = random(0, 2) ? 1 : -1;
    line.length = random(8, 14);
    line.lane = line.vertical ? random(4, WIDTH - 4) : random(5, HEIGHT - 5);
    const limit = line.vertical ? HEIGHT : WIDTH;
    line.position = line.direction === 1 ? -line.length : limit + line.length;
    line.active = true;
  }

  lineLength(line) {
    return line.length;
  }

  loseLife() {
    if (this.invincibleTimer || this.gameOver) return;
    this.lives--;
    if (!this.lives) {
      this.gameOver = true;
      if (this.score > this.highScore) {
        this.highScore = this.score;
        localStorage.setItem('neonSnakeHighScore', String(this.highScore));
      }
      return;
    }
    this.resetSnake();
    this.lines.forEach(line => { line.active = false; });
    this.invincibleTimer = 40;
    this.slowTimer = 0;
  }

  activatePower(type) {
    this.powerType = type;
    this.power = { ...this.snake[0] };
    this.collectPower();
  }

  collectPower() {
    if (this.powerType === 0) {
      this.blast = { ...this.power };
      this.blastTimer = 35;
      this.lines.forEach(line => { line.active = false; });
    } else if (this.powerType === 1) {
      this.slowTimer = 160;
    } else {
      this.lives = Math.min(5, this.lives + 1);
      this.heartTimer = 35;
    }
    this.powerTimer = 0;
  }

  update(input) {
    this.tick++;
    if (!this.started || this.gameOver) return;
    if (this.invincibleTimer) this.invincibleTimer--;
    if (this.powerTimer) this.powerTimer--;
    if (this.slowTimer) this.slowTimer--;
    if (this.blastTimer) this.blastTimer--;
    if (this.heartTimer) this.heartTimer--;
    if (this.tick % 100 === 0 && this.score <= 99994) this.score += 5;
    if (!this.powerTimer && this.tick > 120 && this.tick % 320 === 0) this.spawnPower();

    const targetLines = Math.min(2 + Math.floor(this.foodCount / 2), MAX_LINES);
    if (this.tick > 50 && this.tick % 45 === 0 && this.lines.filter(line => line.active).length < targetLines) {
      const index = this.lines.findIndex(line => !line.active);
      if (index >= 0) this.spawnLine(index);
    }

    const obstacleInterval = this.slowTimer ? 12 : 7;
    if (this.tick % obstacleInterval === 0) {
      this.lines.forEach(line => {
        if (!line.active) return;
        line.position += line.direction;
        const limit = line.vertical ? HEIGHT : WIDTH;
        if ((line.direction === 1 && line.position > limit + line.length + 1) ||
            (line.direction === -1 && line.position < -line.length - 1)) line.active = false;
      });
    }

    let requested = this.direction;
    if (input.up) requested = 0;
    else if (input.right) requested = 1;
    else if (input.down) requested = 2;
    else if (input.left) requested = 3;
    if ((requested + 2) % 4 !== this.direction) this.direction = requested;

    const snakeInterval = input.action ? 2 : 4;
    if (this.tick % snakeInterval === 0) {
      const head = this.snake[0];
      const next = {
        x: (head.x + [0, 1, 0, -1][this.direction] + WIDTH) % WIDTH,
        y: (head.y + [-1, 0, 1, 0][this.direction] + HEIGHT) % HEIGHT
      };
      if (this.snake.slice(1).some(part => part.x === next.x && part.y === next.y)) {
        this.loseLife();
        return;
      }
      this.snake.unshift(next);
      this.snake.pop();

      if (next.x >= this.food.x && next.x < this.food.x + 2 && next.y >= this.food.y && next.y < this.food.y + 2) {
        this.score = Math.min(99999, this.score + 100);
        this.foodCount++;
        const tail = this.snake[this.snake.length - 1];
        for (let i = 0; i < 3 && this.snake.length < MAX_SNAKE; i++) this.snake.push({ ...tail });
        this.spawnFood();
      }
      if (this.powerTimer && next.x >= this.power.x && next.x < this.power.x + 2 && next.y >= this.power.y && next.y < this.power.y + 2) this.collectPower();
    }

    if (this.invincibleTimer) return;
    for (const line of this.lines) {
      if (!line.active) continue;
      const length = this.lineLength(line);
      for (let i = 0; i < length; i++) {
        const x = line.vertical ? line.lane : line.position - line.direction * i;
        const y = line.vertical ? line.position - line.direction * i : line.lane;
        if (this.snake[0].x === x && this.snake[0].y === y) {
          if (this.slowTimer) {
            line.active = false;
            this.score = Math.min(99999, this.score + 500);
          } else {
            this.loseLife();
          }
          return;
        }
      }
    }
  }

  cell(x, y, color) {
    x = Math.round(x);
    y = Math.round(y);
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    this.ctx.fillStyle = color;
    const width = Math.min(SCALE - 1, this.canvas.width - x * SCALE);
    const height = Math.min(SCALE - 1, this.canvas.height - y * SCALE);
    this.ctx.fillRect(x * SCALE, y * SCALE, width, height);
  }

  rect(x, y, width, height, color) {
    for (let row = y; row < y + height; row++) for (let col = x; col < x + width; col++) this.cell(col, row, color);
  }

  entityCell(x, y, color) {
    x = Math.round(x);
    y = Math.round(y);
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
    this.ctx.fillStyle = color;
    const width = Math.min(SCALE - 1, this.canvas.width - x * SCALE);
    const height = Math.min(SCALE - 1, this.canvas.height - y * SCALE);
    this.ctx.fillRect(x * SCALE, y * SCALE, width, height);
  }

  entityRect(x, y, width, height, color) {
    for (let row = y; row < y + height; row++) for (let col = x; col < x + width; col++) this.entityCell(col, row, color);
  }

  smallCell(x, y, color) {
    if (x < 0 || x >= 60 || y < 0 || y >= 34) return;
    this.ctx.fillStyle = color;
    this.ctx.fillRect(x * 4, y * 4, 3, 3);
  }

  smallRect(x, y, width, height, color) {
    this.ctx.fillStyle = color;
    this.ctx.fillRect(x * 4, y * 4, width * 4 - 1, height * 4 - 1);
  }

  drawHeart(x, y, color) {
    this.cell(x, y, color);
    this.cell(x + 2, y, color);
    this.rect(x, y + 1, 3, 1, color);
    this.entityCell(x + 1, y + 2, color);
  }

  drawClock(x, y) {
    for (let dy = -1; dy <= 1; dy++) {
      for (let dx = -1; dx <= 1; dx++) {
        if (dx || dy) this.entityCell(x + dx, y + dy, COLORS.cyan);
      }
    }
    this.entityCell(x, y - 1, COLORS.white);
    this.entityCell(x, y, COLORS.white);
    this.entityCell(x + 1, y, COLORS.white);
  }

  drawFood() {
    this.entityRect(this.food.x, this.food.y, 2, 2, COLORS.yellow);
    this.entityCell(this.food.x + 1, this.food.y + 1, this.tick % 10 < 5 ? COLORS.white : COLORS.orange);
    this.entityCell(this.food.x + 1, this.food.y - 1, COLORS.green);
  }

  drawPower() {
    if (!this.powerTimer || (this.powerTimer < 20 && this.tick % 4 < 2)) return;
    if (this.powerType === 0) {
      this.entityRect(this.power.x, this.power.y, 2, 2, COLORS.orange);
      this.entityCell(this.power.x + 1, this.power.y - 1, COLORS.yellow);
      this.entityCell(this.power.x + 2, this.power.y - 2, COLORS.white);
    } else if (this.powerType === 1) {
      this.entityCell(this.power.x, this.power.y, COLORS.cyan);
      this.entityCell(this.power.x + 1, this.power.y, COLORS.white);
      this.entityCell(this.power.x, this.power.y + 1, COLORS.white);
      this.entityCell(this.power.x + 1, this.power.y + 1, COLORS.cyan);
    } else {
      this.entityRect(this.power.x, this.power.y, 2, 2, COLORS.magenta);
      this.entityCell(this.power.x + 1, this.power.y, COLORS.head);
    }
  }

  drawDigit(x, y, digit, color, compact = false) {
    const font = [
      [7, 5, 5, 5, 7], [2, 6, 2, 2, 7], [7, 1, 7, 4, 7], [7, 1, 7, 1, 7], [5, 5, 7, 1, 1],
      [7, 4, 7, 1, 7], [7, 4, 7, 5, 7], [7, 1, 1, 2, 2], [7, 5, 7, 5, 7], [7, 5, 7, 1, 7]
    ];
    for (let row = 0; row < 5; row++) for (let col = 0; col < 3; col++) {
      if (font[digit][row] & (1 << (2 - col))) {
        if (compact) this.smallCell(x + col, y + row, color);
        else this.cell(x + col, y + row, color);
      }
    }
  }

  drawHud() {
    for (let i = 0; i < 5; i++) this.drawHeart(i * 4, 0, i < this.lives ? COLORS.magenta : '#462841');
    let value = this.score;
    for (let i = 0; i < 5; i++) {
      this.drawDigit(WIDTH - 3 - i * 4, 0, value % 10, COLORS.yellow);
      value = Math.floor(value / 10);
    }
    if (this.slowTimer) for (let i = 0; i < Math.floor(this.slowTimer / 2); i++) this.cell(i, HEIGHT - 1, COLORS.cyan);
  }

  drawLetter(x, y, letter, color) {
    const glyphs = {
      A: [2, 5, 7, 5, 5], B: [6, 5, 6, 5, 6], E: [7, 4, 6, 4, 7], G: [7, 4, 5, 5, 7],
      K: [5, 5, 6, 5, 5], M: [5, 7, 7, 5, 5], N: [5, 7, 7, 5, 5], O: [7, 5, 5, 5, 7],
      P: [6, 5, 6, 4, 4], R: [6, 5, 6, 5, 5], S: [7, 4, 7, 1, 7],
      T: [7, 2, 2, 2, 2], V: [5, 5, 5, 5, 2]
    };
    const rows = glyphs[letter] || [0, 0, 0, 0, 0];
    for (let row = 0; row < 5; row++) for (let col = 0; col < 3; col++) {
      if (rows[row] & (1 << (2 - col))) this.smallCell(x + col, y + row, color);
    }
  }

  drawText(x, y, text, color) {
    let cursor = x;
    for (const letter of text) {
      if (letter >= '0' && letter <= '9') this.drawDigit(cursor, y, Number(letter), color, true);
      else this.drawLetter(cursor, y, letter, color);
      cursor += 4;
    }
  }

  drawBigLetter(x, y, letter, color) {
    const glyphs = {
      A: [14, 17, 17, 31, 17, 17, 17], E: [31, 16, 16, 30, 16, 16, 31],
      K: [17, 18, 20, 24, 20, 18, 17], N: [17, 25, 25, 21, 19, 19, 17],
      O: [14, 17, 17, 17, 17, 17, 14], S: [15, 16, 16, 14, 1, 1, 30]
    };
    const rows = glyphs[letter] || [0, 0, 0, 0, 0, 0, 0];
    for (let row = 0; row < 7; row++) for (let col = 0; col < 5; col++) {
      if (rows[row] & (1 << (4 - col))) this.smallCell(x + col, y + row, color);
    }
  }

  drawBigText(x, y, text, color) {
    let cursor = x;
    for (const letter of text) {
      this.drawBigLetter(cursor, y, letter, color);
      cursor += 6;
    }
  }

  drawStartScreen() {
    this.ctx.fillStyle = '#000000';
    this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    const stars = [
      [7, 6], [13, 2], [18, 7], [27, 1], [34, 6], [43, 2], [51, 7],
      [5, 13], [55, 14], [9, 20], [18, 22], [42, 21], [51, 19],
      [3, 28], [14, 30], [26, 29], [37, 31], [47, 29], [57, 25]
    ];
    stars.forEach(([x, y], index) => this.smallCell(x, y, index % 3 ? '#181c34' : '#3e1846'));
    for (let i = 0; i < 6; i++) {
      this.smallCell(3 + i, 1, '#46195a');
      this.smallCell(3, 1 + i, '#46195a');
      this.smallCell(56 - i, 1, '#46195a');
      this.smallCell(56, 1 + i, '#46195a');
      this.smallCell(3 + i, 31, '#46195a');
      this.smallCell(3, 31 - i, '#46195a');
      this.smallCell(56 - i, 31, '#46195a');
      this.smallCell(56, 31 - i, '#46195a');
    }
    this.drawBigText(20, 5, 'NEON', '#4b1e55');
    this.drawBigText(17, 14, 'SNAKE', '#4b1e55');
    this.drawBigText(19, 4, 'NEON', COLORS.head);
    this.drawBigText(16, 13, 'SNAKE', COLORS.magenta);
    for (let x = 15; x <= 45; x += 2) this.smallCell(x, 22, x % 4 ? COLORS.purple : COLORS.magenta);
    this.smallCell(30, 23, COLORS.head);

    this.smallCell(5, 26, COLORS.magenta);
    this.smallCell(6, 27, COLORS.magenta);
    this.smallCell(5, 28, COLORS.magenta);
    this.smallCell(54, 26, COLORS.magenta);
    this.smallCell(53, 27, COLORS.magenta);
    this.smallCell(54, 28, COLORS.magenta);
    if (this.tick % 24 < 16) {
      this.drawText(10, 26, 'PRESS ENTER', '#5f4119');
      this.drawText(9, 25, 'PRESS ENTER', COLORS.yellow);
    }
  }

  drawGameOver() {
    this.smallRect(7, 7, 46, 20, '#000000');
    this.smallCell(7, 7, COLORS.purple);
    this.smallCell(52, 7, COLORS.purple);
    this.smallCell(7, 26, COLORS.purple);
    this.smallCell(52, 26, COLORS.purple);
    this.drawText(14, 11, 'GAME OVER', '#50285a');
    this.drawText(13, 10, 'GAME OVER', COLORS.head);
    this.drawText(12, 19, 'BEST ', '#5f4119');
    this.drawText(11, 18, 'BEST ', COLORS.yellow);
    let value = this.highScore;
    for (let i = 0; i < 5; i++) {
      const x = 47 - i * 4;
      this.drawDigit(x + 1, 19, value % 10, '#5f4119', true);
      this.drawDigit(x, 18, value % 10, COLORS.yellow, true);
      value = Math.floor(value / 10);
    }
  }

  render() {
    if (!this.started) {
      this.drawStartScreen();
      return;
    }
    this.ctx.fillStyle = COLORS.bg;
    this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    for (let i = 0; i < 48; i++) this.cell((i * 37 + 7) % WIDTH, (i * 19 + 4) % HEIGHT, COLORS.grid);

    this.lines.forEach(line => {
      if (!line.active) return;
      const color = this.slowTimer ? COLORS.cyan : line.vertical ? COLORS.blue : COLORS.orange;
      const length = this.lineLength(line);
      for (let i = 0; i < length; i++) {
        const x = line.vertical ? line.lane : line.position - line.direction * i;
        const y = line.vertical ? line.position - line.direction * i : line.lane;
        if (this.slowTimer && i === 0) this.drawClock(x, y);
        else this.entityCell(x, y, i === 0 ? COLORS.white : color);
        if (i > 1 && i < length - 1 && i % 3 === 0) {
          if (line.vertical) this.entityCell(x + 1, y, '#46234b');
          else this.entityCell(x, y + 1, '#4b2d23');
        }
      }
    });

    this.drawFood();
    this.drawPower();
    for (let i = this.snake.length - 1; i >= 0; i--) {
      if ((this.invincibleTimer && this.tick % 4 < 2) || i === 0) continue;
      const part = this.snake[i];
      this.entityCell(part.x, part.y, i % 2 ? COLORS.magenta : COLORS.purple);
    }
    if (!this.invincibleTimer || this.tick % 4 >= 2) {
      const hx = this.snake[0].x;
      const hy = this.snake[0].y;
      this.entityRect(hx, hy, 2, 2, COLORS.head);
      if (this.direction === 0) {
        this.entityCell(hx, hy, COLORS.bg);
        this.entityCell(hx + 1, hy, COLORS.bg);
        if (this.tick % 10 < 4) this.entityCell(hx, hy - 1, COLORS.red);
      } else if (this.direction === 1) {
        this.entityCell(hx + 1, hy, COLORS.bg);
        this.entityCell(hx + 1, hy + 1, COLORS.bg);
        if (this.tick % 10 < 4) this.entityCell(hx + 2, hy, COLORS.red);
      } else if (this.direction === 2) {
        this.entityCell(hx, hy + 1, COLORS.bg);
        this.entityCell(hx + 1, hy + 1, COLORS.bg);
        if (this.tick % 10 < 4) this.entityCell(hx, hy + 2, COLORS.red);
      } else {
        this.entityCell(hx, hy, COLORS.bg);
        this.entityCell(hx, hy + 1, COLORS.bg);
        if (this.tick % 10 < 4) this.entityCell(hx - 1, hy, COLORS.red);
      }
    }

    if (this.blastTimer) {
      const radius = (35 - this.blastTimer) * 2;
      for (let i = -radius; i <= radius; i++) {
        this.entityCell(this.blast.x + i, this.blast.y - radius, i % 2 ? COLORS.yellow : COLORS.white);
        this.entityCell(this.blast.x + i, this.blast.y + radius, i % 2 ? COLORS.yellow : COLORS.white);
        this.entityCell(this.blast.x - radius, this.blast.y + i, i % 2 ? COLORS.yellow : COLORS.white);
        this.entityCell(this.blast.x + radius, this.blast.y + i, i % 2 ? COLORS.yellow : COLORS.white);
      }
    }
    if (this.heartTimer) {
      const rise = Math.floor((35 - this.heartTimer) / 5);
      this.drawHeart(this.snake[0].x - 1, this.snake[0].y - 5 - rise, COLORS.magenta);
      this.drawHeart(this.snake[0].x + 4, this.snake[0].y - 3 - rise, COLORS.white);
    }
    this.drawHud();
    if (this.gameOver) this.drawGameOver();
  }
}

const canvas = document.querySelector('#game-canvas');
const game = new SnakeGame(canvas);
const pressedKeys = new Set();
const pauseButton = document.querySelector('#pause-game');
const stage = document.querySelector('#game-stage');
const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
let paused = reducedMotion;
let stageSkipped = false;
let lastFrame = 0;
let announcedGameOver = false;

function updateStats() {
  document.querySelector('#score-value').textContent = String(game.score).padStart(5, '0');
  document.querySelector('#lives-value').textContent = game.lives;
  const effect = game.blastTimer ? 'Explosión' : game.slowTimer ? 'Reloj' : game.heartTimer ? '+1 vida' : '—';
  document.querySelector('#power-value').textContent = effect;
  if (game.gameOver && !announcedGameOver) {
    document.querySelector('#game-status').textContent = `Fin de la partida. Puntuación ${game.score}. Pulsa Enter para revivir.`;
    announcedGameOver = true;
  }
}

function setPaused(value) {
  paused = value;
  pauseButton.textContent = paused ? 'Continuar' : 'Pausar';
  pauseButton.setAttribute('aria-pressed', String(paused));
}

function restart() {
  game.start();
  announcedGameOver = false;
  document.querySelector('#game-status').textContent = '';
  setPaused(false);
  updateStats();
}

pauseButton.addEventListener('click', () => setPaused(!paused));
document.querySelector('#restart-game').addEventListener('click', () => {
  restart();
  canvas.focus();
});
setPaused(paused);

const keyMap = {
  arrowup: 'up', arrowdown: 'down', arrowleft: 'left', arrowright: 'right', ' ': 'action'
};
canvas.addEventListener('pointerdown', () => canvas.focus());
canvas.addEventListener('keydown', event => {
  const key = event.key.toLowerCase();
  if (keyMap[key]) {
    event.preventDefault();
    pressedKeys.add(keyMap[key]);
  } else if (key === 'r') {
    event.preventDefault();
    restart();
  } else if (key === 'enter' && (!game.started || game.gameOver)) {
    event.preventDefault();
    restart();
  }
});
canvas.addEventListener('keyup', event => {
  const key = keyMap[event.key.toLowerCase()];
  if (key) {
    event.preventDefault();
    pressedKeys.delete(key);
  }
});
canvas.addEventListener('blur', () => pressedKeys.clear());

function bindHold(button, direction) {
  button.addEventListener('pointerdown', event => {
    event.preventDefault();
    button.setPointerCapture(event.pointerId);
    pressedKeys.add(direction);
  });
  const release = () => pressedKeys.delete(direction);
  button.addEventListener('pointerup', release);
  button.addEventListener('pointercancel', release);
  button.addEventListener('lostpointercapture', release);
}
document.querySelectorAll('[data-direction]').forEach(button => bindHold(button, button.dataset.direction));
bindHold(document.querySelector('#action-button'), 'action');

document.querySelectorAll('[data-power]').forEach(button => button.addEventListener('click', () => {
  game.activatePower(Number(button.dataset.power));
  setPaused(false);
  canvas.focus();
}));

stage.addEventListener('contentvisibilityautostatechange', event => {
  stageSkipped = event.skipped;
}, { capture: true });
if (!('contentVisibility' in document.documentElement.style)) {
  const observer = new IntersectionObserver(entries => {
    entries.forEach(entry => { stageSkipped = !entry.isIntersecting; });
  }, { rootMargin: '200px' });
  observer.observe(stage);
}

function currentInput() {
  return {
    up: pressedKeys.has('up'), down: pressedKeys.has('down'),
    left: pressedKeys.has('left'), right: pressedKeys.has('right'), action: pressedKeys.has('action')
  };
}

function animate(timestamp) {
  if (!paused && !stageSkipped && document.visibilityState === 'visible' && timestamp - lastFrame >= 50) {
    lastFrame = timestamp;
    game.update(currentInput());
    game.render();
    updateStats();
  }
  requestAnimationFrame(animate);
}
updateStats();
requestAnimationFrame(animate);
