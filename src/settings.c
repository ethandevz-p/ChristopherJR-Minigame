#include "settings.h"
#include <stdio.h>

static const char *DEFAULT_PATH = "data/settings.dat";

void settings_init(Settings *s) {
    s->theme = 0;
    s->hard_mode = 0;
    s->audio_enabled = 1;
}

bool settings_load(Settings *s, const char *path) {
    const char *p = path ? path : DEFAULT_PATH;
    FILE *f = fopen(p, "r");
    if (!f) return false;
    int n = fscanf(f, "%d %d %d", &s->theme, &s->hard_mode, &s->audio_enabled);
    fclose(f);
    return n == 3;
}

bool settings_save(const Settings *s, const char *path) {
    const char *p = path ? path : DEFAULT_PATH;
    FILE *f = fopen(p, "w");
    if (!f) {
        fprintf(stderr, "Warning: could not save settings to '%s'\n", p);
        return false;
    }
    fprintf(f, "%d %d %d\n", s->theme, s->hard_mode, s->audio_enabled);
    fclose(f);
    return true;
}
