# WORDLE

A complete, polished desktop Wordle game written in C with Raylib.

## Features

- Classic Wordle gameplay: guess the hidden 5-letter word in 6 tries
- Accurate GREEN / YELLOW / GRAY feedback with correct repeated-letter handling
- Physical keyboard support (A–Z, ENTER, BACKSPACE, ESC)
- Clickable on-screen QWERTY keyboard with color feedback
- Main menu, How to Play, Statistics, Settings screens
- Statistics tracking (games played, won, win rate, streaks, guess distribution)
- Local save of statistics and settings (persists across restarts)
- Dark / Light themes
- Normal / Hard mode (Hard mode enforces revealed hints)
- Optional sound effects (gracefully disabled if assets missing)
- Tile flip animations, invalid-word shake, button hover effects
- Window resize support
- Modular, readable C code separated by responsibility

## Project Structure

```
WORDLE/
├── Makefile
├── README.md
├── src/
│   ├── main.c        # entry point, main loop, window
│   ├── game.h/.c     # game logic, word evaluation, state
│   ├── ui.h/.c       # rendering, themes, buttons, audio
│   ├── input.h/.c    # keyboard + mouse input
│   ├── words.h/.c    # dictionary loading & word selection
│   ├── statistics.h/.c  # stats tracking & save/load
│   └── settings.h/.c    # settings & save/load
├── assets/
│   ├── fonts/        # .ttf font files (optional, falls back to default)
│   └── sounds/       # .wav sound effects (optional)
└── data/
    ├── answer_words.txt   # possible hidden words
    ├── valid_words.txt    # all valid guess words
    ├── statistics.dat     # auto-generated save file
    └── settings.dat       # auto-generated save file
```

## Build & Run

### Prerequisites

- [Raylib](https://www.raylib.com/) development headers and shared library
- A C compiler (gcc/clang) and `make`
- `pkg-config` (recommended for detecting raylib flags)

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
./wordle
```

### Audio & Fonts (optional)

Place files in `assets/` to enable them:

- `assets/fonts/Inter-Regular.ttf` and `Inter-Bold.ttf` (falls back to raylib default font if missing)
- `assets/sounds/click.wav`, `letter.wav`, `submit.wav`, `correct.wav`, `win.wav`, `lose.wav`, `invalid.wav`

The game runs without these assets; missing audio is silently disabled.

## Controls

| Input | Action |
|-------|--------|
| A–Z (physical or on-screen) | Enter a letter |
| ENTER | Submit guess |
| BACKSPACE | Delete last letter |
| ESC | Back to menu / close settings |
| Mouse | Click buttons and keys |

## Word Lists

- `data/answer_words.txt` — words that can be chosen as the hidden target
- `data/valid_words.txt` — all words accepted as valid guesses

Both files are loaded at startup. If missing, the game falls back to a hardcoded word ("APPLE") so it still runs.

## Testing

Compile with warnings enabled (the Makefile already uses `-Wall -Wextra -Wpedantic`):

```bash
make clean && make
```

Run and exercise:
1. Main menu → every button works
2. Start game → type a 5-letter word, submit
3. Submit an incomplete word → notification, no attempt consumed
4. Submit an invalid word → shake animation, no attempt consumed
5. Win on any attempt → win screen shows
6. Lose after 6 attempts → lose screen reveals answer
7. Repeated-letter words (e.g., target "SPEED", guess "SPEEL") → correct GREEN/YELLOW/GRAY
8. Hard mode → revealed hints enforced
9. Toggle theme / audio → persists after restart
10. Close and reopen → statistics and settings preserved

## License

MIT
