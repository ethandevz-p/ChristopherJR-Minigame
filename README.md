# NEON DODGE

A polished 2D arcade survival game written in C with Raylib.

## Overview

NEON DODGE is a neon-themed arcade survival game. Pilot a glowing cyan square,
dodge incoming red neon blocks, survive as long as possible, and chase the
high score. Obstacles accelerate over time and the high score is saved to
`highscore.txt` so it persists between runs.

## Features

- 5 game states: Main Menu, How to Play, Playing, Paused, Game Over
- WASD / Arrow key movement with screen-boundary clamping
- Spawning obstacles that home in on the player
- Obstacle speed increases the longer you survive
- Score and survival timer HUD, plus high score
- Pause (P / ESC), Retry, and Main Menu options
- Neon glow effects, animated background particles, button hover states
- Screen transitions and smooth delta-time movement
- Optional sound effects and background music (gracefully disabled if assets
  or audio are unavailable)
- Window resize safe (uses current screen dimensions)

## Build & Run

### Prerequisites

- [Raylib](https://www.raylib.com/) development headers and shared library
- A C compiler (gcc/clang)
- `make` and `pkg-config` (recommended)

On Ubuntu/Debian:

```bash
sudo apt-get install libraylib-dev libgl1-mesa-dev libx11-dev \
  libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev
```

### Compile

```bash
make
```

### Run

```bash
make run
# or
./neon_dodge
```

### Audio (optional)

Place these files in `assets/` to enable sound:

- `assets/click.wav` — button click
- `assets/hit.wav` — collision
- `assets/point.wav` — obstacle dodged
- `assets/music.ogg` — background music

If Raylib audio or these files are missing, the game runs silently without
errors.

## Controls

| Action | Key |
|--------|-----|
| Move | WASD or Arrow Keys |
| Pause / Resume | P or ESC |
| Menu navigation | Mouse |

## Project Files

- `main.c` — complete game source
- `Makefile` — build instructions
- `README.md` — this file

## License

MIT
