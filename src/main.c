#include "game.h"
#include "ui.h"
#include "input.h"
#include "words.h"
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_WIDTH  1000
#define DEFAULT_HEIGHT 700

int main(void) {
    InitWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, "WORDLE");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    /* Load words */
    if (!words_load("data/answer_words.txt", "data/valid_words.txt")) {
        fprintf(stderr, "Warning: using fallback word list\n");
    }

    Game g;
    game_init(&g);
    game_set_sound_cb(ui_audio_play);

    ui_init();
    ui_set_theme(g.settings.theme);
    ui_audio_init();

    bool running = true;
    while (running && !WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        input_update(&g, dt);
        game_update(&g, dt);

        /* Handle hot-button clicks */
        HotButton hots[MAX_HOT];
        int nh = ui_hot_buttons(hots, MAX_HOT);
        Vector2 mouse = GetMousePosition();
        bool clicked = false;
        UIAction act = UI_ACT_NONE;
        for (int i = 0; i < nh && !clicked; i++) {
            if (CheckCollisionPointRec(mouse, hots[i].bounds) &&
                IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                act = hots[i].action;
                clicked = true;
            }
        }

        if (clicked) {
            ui_audio_play(SND_CLICK);
            switch (act) {
                case UI_ACT_PLAY:
                    game_reset(&g);
                    game_change_state(&g, STATE_PLAYING);
                    break;
                case UI_ACT_HOWTO:
                    game_change_state(&g, STATE_HOWTO);
                    break;
                case UI_ACT_STATS:
                    game_change_state(&g, STATE_STATISTICS);
                    break;
                case UI_ACT_SETTINGS:
                    game_change_state(&g, STATE_SETTINGS);
                    break;
                case UI_ACT_EXIT:
                    running = false;
                    break;
                case UI_ACT_BACK:
                    game_change_state(&g, STATE_MENU);
                    break;
                case UI_ACT_RETRY:
                    game_reset(&g);
                    game_change_state(&g, STATE_PLAYING);
                    break;
                case UI_ACT_MENU:
                    game_change_state(&g, STATE_MENU);
                    break;
                case UI_ACT_TOGGLE_THEME:
                    g.settings.theme = (g.settings.theme == 1) ? 0 : 1;
                    ui_set_theme(g.settings.theme);
                    settings_save(&g.settings, NULL);
                    break;
                case UI_ACT_TOGGLE_HARD:
                    g.settings.hard_mode = (g.settings.hard_mode == 1) ? 0 : 1;
                    settings_save(&g.settings, NULL);
                    break;
                case UI_ACT_TOGGLE_AUDIO:
                    g.settings.audio_enabled = (g.settings.audio_enabled == 1) ? 0 : 1;
                    settings_save(&g.settings, NULL);
                    break;
                default: break;
            }
        } else if (g.state == STATE_PLAYING && !g.game_over &&
                   IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            char kch = ui_keyboard_char_at(mouse);
            if (kch != 0) {
                ui_audio_play(SND_CLICK);
                if (kch == '\n') {
                    game_handle_enter(&g);
                } else if (kch == '\b') {
                    game_handle_backspace(&g);
                } else {
                    game_handle_char(&g, kch);
                }
            }
        }

        ui_draw(&g);
    }

    statistics_save(&g.stats, NULL);
    settings_save(&g.settings, NULL);

    ui_close();
    CloseWindow();
    return 0;
}
