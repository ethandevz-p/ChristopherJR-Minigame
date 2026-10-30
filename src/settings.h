#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef struct {
    int theme;          /* 0 = dark, 1 = light */
    int hard_mode;      /* 0 = off, 1 = on */
    int audio_enabled;  /* 0 = off, 1 = on */
} Settings;

void settings_init(Settings *s);
bool settings_load(Settings *s, const char *path);
bool settings_save(const Settings *s, const char *path);

#endif /* SETTINGS_H */
