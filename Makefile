CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -O2
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
    RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRCDIR = src
TARGET = wordle
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run assets

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -o $(TARGET) $(OBJS) $(RAYLIB_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

assets:
	@mkdir -p assets/fonts assets/sounds data
	@echo "Add .ttf fonts to assets/fonts and .wav sounds to assets/sounds"
	@echo "Word lists already in data/"
