# Neon Snake for Cardputer ADV

<img width="1320" height="742" alt="cover" src="https://github.com/user-attachments/assets/b7c3b3c1-49d2-4f8f-bcb1-cabd1fef8511" />


<img width="1320" height="742" alt="gameplay" src="https://github.com/user-attachments/assets/3cd771a7-1090-4aa7-941e-863663fcb378" />


A colorful single-player retro snake game designed for M5Stack Cardputer ADV.

## Download

[Download Neon Snake v1.0.0](https://github.com/jcarlosgv/neon-snake-cardputer-adv/releases/download/v1.0.0/PixelMesh.merged.bin)

## Features

- Full-screen separated-pixel graphics.
- Pure black background with subtle background pixels.
- Wrap-around movement across all four screen edges.
- Horizontal and vertical moving obstacles.
- Clock power-ups that turn obstacles into food worth 500 points.
- Blast and heart power-ups.
- Three starting lives and up to five lives.
- Persistent high score using non-volatile storage.
- English start screen and retro Game Over panel.
- No LoRa or additional accessories required.

## Controls

| Action | Key |
|---|---|
| Up | `;` |
| Down | `.` |
| Left | `,` |
| Right | `/` |
| Speed up | Hold `Space` |
| Restart | Top button `G0` |
| Start or restart after Game Over | `Enter` |

Directional controls do not require the Fn key.

## Install with esptool

```bash
python3 -m esptool \
  --chip esp32s3 \
  --port SERIAL_PORT \
  --baud 921600 \
  write_flash 0x0 \
  PixelMesh.merged.bin

