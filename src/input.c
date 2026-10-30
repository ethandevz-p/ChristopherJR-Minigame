#include "input.h"
#include <raylib.h>

void input_update(Game *g, float dt) {
    (void)dt;

    /* Physical keyboard: characters */
    int key = GetCharPressed();
    while (key > 0) {
        if ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {
            game_handle_char(g, key);
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        game_handle_enter(g);
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        game_handle_backspace(g);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (g->state == STATE_PLAYING) {
            game_change_state(g, STATE_SETTINGS);
        } else if (g->state == STATE_SETTINGS || g->state == STATE_STATISTICS ||
                   g->state == STATE_HOWTO || g->state == STATE_WIN ||
                   g->state == STATE_LOSE) {
            game_change_state(g, STATE_MENU);
        }
    }
}
