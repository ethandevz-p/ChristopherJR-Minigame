CC ?= gcc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -O2
RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)
ifeq ($(RAYLIB_LIBS),)
    RAYLIB_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC = main.c
TARGET = wordle

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -o $(TARGET) $(SRC) $(RAYLIB_LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
