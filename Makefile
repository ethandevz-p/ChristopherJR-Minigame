CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2 -std=c11

# Detect raylib build flags via pkg-config with a static fallback.
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
    # Common Linux link order for raylib.
    RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC = main.c
TARGET = neon_dodge

.PHONY: all clean run assets

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -o $(TARGET) $(SRC) $(RAYLIB_LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

# Optional: place audio assets here to enable sound/music.
assets:
	@mkdir -p assets
	@echo "Place click.wav, hit.wav, point.wav and music.ogg in assets/"
