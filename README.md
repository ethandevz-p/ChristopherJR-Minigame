# WORDLE

A single-file, light-themed Wordle desktop game in C using Raylib.

## Build & Run

Requires [Raylib](https://www.raylib.com/) development headers/library.

```bash
# Ubuntu/Debian:
sudo apt-get install libraylib-dev libgl1-mesa-dev libx11-dev \
  libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev

make
./wordle
```

## Controls

- **A–Z** (physical or on-screen keys): enter a letter
- **ENTER** / **Enter** key: submit guess
- **BACKSPACE** / **⌫** key: delete last letter
- **ESC**: reset after win/loss
- **Mouse**: click on-screen keys and the "NEW GAME" button

## Features

- 6 attempts, 5-letter words
- GREEN / YELLOW / GRAY feedback with correct repeated-letter handling
- Light-themed UI matching the classic Wordle look
- Tile flip animation, invalid-word shake, "You Win!" / "Game Over" screens
- Optional external word list: `data/valid_words.txt` (falls back to embedded list)
- No audio dependencies

## Files

- `main.c` — complete game (single file)
- `Makefile` — build
- `data/valid_words.txt` — optional guess word list
