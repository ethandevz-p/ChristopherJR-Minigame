#include "game.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

static void (*s_sound_cb)(int) = NULL;

void game_set_sound_cb(void (*cb)(int)) {
    s_sound_cb = cb;
}

#define FLIP_DURATION 0.25f
#define SHAKE_DURATION 0.4f
#define NOTIFY_DURATION 2.0f

static void update_key_color(Game *g, char letter, TileColor color);

void game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->state = STATE_MENU;
    g->previous_state = STATE_MENU;
    statistics_init(&g->stats);
    settings_init(&g->settings);
    statistics_load(&g->stats, NULL);
    settings_load(&g->settings, NULL);

    for (int i = 0; i < 26; i++) g->key_colors[i] = TILE_EMPTY;
    for (int i = 0; i < WORD_LENGTH; i++) g->hard_greens[i] = false;
    for (int i = 0; i < 26; i++) g->hard_yellows[i] = false;

    words_pick_random(g->target);
    game_reset(g);}

void game_reset(Game *g) {
    g->current_row = 0;
    g->current_col = 0;
    g->game_over = false;
    g->won = false;
    g->guesses_used = 0;
    g->shake_offset = 0.0f;
    g->shake_time = 0.0f;
    g->shaking = false;
    g->notification_time = 0.0f;
    g->notification_msg[0] = '\0';

    memset(g->guesses, 0, sizeof(g->guesses));
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < WORD_LENGTH; c++) {
            g->guesses[r][c] = '\0';
            g->guess_colors[r][c] = TILE_EMPTY;
        }
    }
    for (int i = 0; i < 26; i++) g->key_colors[i] = TILE_EMPTY;
    for (int i = 0; i < WORD_LENGTH; i++) g->hard_greens[i] = false;
    for (int i = 0; i < 26; i++) g->hard_yellows[i] = false;
    for (int i = 0; i < 26; i++) g->hard_yellow_count[i] = 0;

    g->flip_row = -1;
    g->flip_timer = 0.0f;
    g->pending_state = STATE_MENU;
    g->state_pending = false;

    words_pick_random(g->target);
}

void game_change_state(Game *g, GameState next) {
    g->previous_state = g->state;
    g->state = next;
}

static void update_key_color(Game *g, char letter, TileColor color) {
    if (letter < 'A' || letter > 'Z') return;
    int idx = letter - 'A';
    TileColor cur = g->key_colors[idx];
    /* Priority: GREEN > YELLOW > GRAY > EMPTY */
    int rank_new = (color == TILE_GREEN) ? 3 : (color == TILE_YELLOW) ? 2 : (color == TILE_GRAY) ? 1 : 0;
    int rank_cur = (cur == TILE_GREEN) ? 3 : (cur == TILE_YELLOW) ? 2 : (cur == TILE_GRAY) ? 1 : 0;
    if (rank_new > rank_cur) g->key_colors[idx] = color;
}

void game_evaluate_guess(Game *g, int row) {
    const char *target = g->target;
    const char *guess = g->guesses[row];
    TileColor *colors = g->guess_colors[row];

    /* Initialize all to gray */
    for (int i = 0; i < WORD_LENGTH; i++) colors[i] = TILE_GRAY;

    /* Count remaining target letters (excluding exact matches) */
    int target_remaining[26] = {0};
    bool matched[WORD_LENGTH] = {false};

    /* Pass 1: greens */
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == target[i]) {
            colors[i] = TILE_GREEN;
            matched[i] = true;
        } else {
            target_remaining[target[i] - 'A']++;
        }
    }

    /* Pass 2: yellows */
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (colors[i] == TILE_GREEN) continue;
        int idx = guess[i] - 'A';
        if (target_remaining[idx] > 0) {
            colors[i] = TILE_YELLOW;
            target_remaining[idx]--;
        }
    }

    /* Update keyboard colors */
    for (int i = 0; i < WORD_LENGTH; i++) {
        update_key_color(g, guess[i], colors[i]);
    }

    /* Hard mode constraints for next guess */
    if (g->settings.hard_mode) {
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (colors[i] == TILE_GREEN) {
                g->hard_greens[i] = true;
            }
        }
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (colors[i] == TILE_YELLOW) {
                int idx = guess[i] - 'A';
                g->hard_yellows[idx] = true;
                g->hard_yellow_count[idx]++;
            }
        }
    }
}

bool game_hard_mode_valid(const Game *g, const char *word) {
    if (!g->settings.hard_mode) return true;
    /* Green positions must match */
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (g->hard_greens[i] && word[i] != g->target[i]) return false;
    }
    /* Yellow letters must appear at least required count */
    int counts[26] = {0};
    for (int i = 0; i < WORD_LENGTH; i++) counts[word[i] - 'A']++;
    for (int i = 0; i < 26; i++) {
        if (g->hard_yellows[i] && counts[i] < g->hard_yellow_count[i]) return false;
    }
    return true;
}

void game_handle_char(Game *g, int c) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->current_row >= 6) return;
    if (g->current_col >= WORD_LENGTH) return;

    char ch = (char)c;
    if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
    if (ch < 'A' || ch > 'Z') return;

    g->guesses[g->current_row][g->current_col] = ch;
    g->guesses[g->current_row][g->current_col + 1] = '\0';
    g->current_col++;
    game_play_sound(g, SND_LETTER);
}

void game_handle_backspace(Game *g) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->current_row >= 6) return;
    if (g->current_col == 0) return;
    g->current_col--;
    g->guesses[g->current_row][g->current_col] = '\0';
    game_play_sound(g, SND_LETTER);
}

void game_handle_enter(Game *g) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->current_row >= 6) return;
    if (g->current_col != WORD_LENGTH) {
        snprintf(g->notification_msg, sizeof(g->notification_msg), "Not enough letters");
        g->notification_time = NOTIFY_DURATION;
        game_play_sound(g, SND_INVALID);
        return;
    }

    char word[WORD_LENGTH + 1];
    memcpy(word, g->guesses[g->current_row], WORD_LENGTH);
    word[WORD_LENGTH] = '\0';

    if (!words_is_valid(word)) {
        snprintf(g->notification_msg, sizeof(g->notification_msg), "Not in word list");
        g->notification_time = NOTIFY_DURATION;
        g->shaking = true;
        g->shake_time = SHAKE_DURATION;
        g->shake_offset = 0.0f;
        game_play_sound(g, SND_INVALID);
        return;
    }

    if (!game_hard_mode_valid(g, word)) {
        snprintf(g->notification_msg, sizeof(g->notification_msg), "Hard mode: use revealed hints");
        g->notification_time = NOTIFY_DURATION;
        g->shaking = true;
        g->shake_time = SHAKE_DURATION;
        g->shake_offset = 0.0f;
        game_play_sound(g, SND_INVALID);
        return;
    }

    game_play_sound(g, SND_SUBMIT);
    g->flip_row = g->current_row;
    g->flip_timer = 0.0f;
    game_evaluate_guess(g, g->current_row);

    if (strcmp(word, g->target) == 0) {
        g->won = true;
        g->game_over = true;
        g->guesses_used = g->current_row + 1;
        statistics_record_game(&g->stats, true, g->guesses_used);
        statistics_save(&g->stats, NULL);
        g->pending_state = STATE_WIN;
        g->state_pending = true;
        game_play_sound(g, SND_WIN);
    } else {
        g->current_row++;
        g->current_col = 0;
        if (g->current_row >= 6) {
            g->game_over = true;
            g->won = false;
            g->guesses_used = 6;
            statistics_record_game(&g->stats, false, 6);
            statistics_save(&g->stats, NULL);
            g->pending_state = STATE_LOSE;
            g->state_pending = true;
            game_play_sound(g, SND_LOSE);
        }
    }
}

void game_update(Game *g, float dt) {
    /* Flip animation timer */
    if (g->flip_row >= 0) {
        g->flip_timer += dt;
        if (g->flip_timer >= FLIP_DURATION) {
            g->flip_timer = FLIP_DURATION;
            g->flip_row = -1;
            if (g->state_pending) {
                g->state_pending = false;
                game_change_state(g, g->pending_state);
            }
        }
    }

    /* Shake animation */
    if (g->shaking) {
        g->shake_time -= dt;
        if (g->shake_time <= 0.0f) {
            g->shaking = false;
            g->shake_offset = 0.0f;
        } else {
            g->shake_offset = sinf(g->shake_time * 60.0f) * 6.0f;
        }
    }

    /* Notification fade */
    if (g->notification_time > 0.0f) {
        g->notification_time -= dt;
        if (g->notification_time < 0.0f) g->notification_time = 0.0f;
    }
}

void game_play_sound(Game *g, int sound_id) {
    (void)g;
    if (s_sound_cb) s_sound_cb(sound_id);
}
