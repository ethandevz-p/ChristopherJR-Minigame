#ifndef UI_H
#define UI_H

#include "game.h"

#include <raylib.h>

#define UI_MAX_KEYS 30
#define MAX_HOT 32

typedef struct {
    Color bg;
    Color panel;
    Color text;
    Color text_dim;
    Color tile_border;
    Color tile_empty;
    Color tile_green;
    Color tile_yellow;
    Color tile_gray;
    Color key_bg;
    Color key_text;
    Color accent;
    Color danger;
} Theme;

extern Theme theme_dark;
extern Theme theme_light;

typedef enum {
    UI_ACT_NONE,
    UI_ACT_PLAY,
    UI_ACT_HOWTO,
    UI_ACT_STATS,
    UI_ACT_SETTINGS,
    UI_ACT_EXIT,
    UI_ACT_BACK,
    UI_ACT_RETRY,
    UI_ACT_MENU,
    UI_ACT_TOGGLE_THEME,
    UI_ACT_TOGGLE_HARD,
    UI_ACT_TOGGLE_AUDIO
} UIAction;

typedef struct {
    Rectangle bounds;
    UIAction action;
} HotButton;

void ui_init(void);
void ui_close(void);
void ui_audio_init(void);
void ui_audio_play(int sound_id);
void ui_audio_close(void);
void ui_set_theme(int theme_index);
const Theme *ui_current_theme(void);
void ui_draw(const Game *g);
int ui_hot_buttons(HotButton *out, int max);
char ui_keyboard_char_at(Vector2 mouse);

#endif /* UI_H */
