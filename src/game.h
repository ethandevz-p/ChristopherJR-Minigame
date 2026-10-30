#ifndef GAME_H
#define GAME_H

#include "words.h"
#include "statistics.h"
#include "settings.h"

#define SND_CLICK    0
#define SND_LETTER   1
#define SND_SUBMIT   2
#define SND_CORRECT  3
#define SND_WIN      4
#define SND_LOSE     5
#define SND_INVALID  6

#define FLIP_DURATION 0.25f

typedef enum {
    STATE_MENU,
    STATE_HOWTO,
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE,
    STATE_SETTINGS,
    STATE_STATISTICS
} GameState;

typedef enum {
    TILE_EMPTY,
    TILE_GREEN,
    TILE_YELLOW,
    TILE_GRAY
} TileColor;

typedef struct {
    char letter;        /* A-Z or '\0' */
    TileColor color;    /* current feedback color */
    float flip_angle;   /* 0 = front face, 180 = fully flipped back */
    bool flipping;      /* animation in progress */
    float flip_start;   /* time when flip started */
} Tile;

typedef struct {
    GameState state;
    GameState previous_state;

    char target[WORD_LENGTH + 1];
    char guesses[6][WORD_LENGTH + 1];
    TileColor guess_colors[6][WORD_LENGTH];
    int current_row;    /* 0..5 */
    int current_col;    /* 0..4 */
    bool game_over;
    bool won;

    /* Keyboard key colors (one per letter A-Z) */
    TileColor key_colors[26];

    /* Animations */
    float shake_offset;
    float shake_time;
    bool shaking;
    float notification_time;
    char notification_msg[128];

    /* Hard mode helpers */
    bool hard_greens[WORD_LENGTH];   /* position must match */
    bool hard_yellows[26];           /* letter must appear somewhere */
    int hard_yellow_count[26];       /* required count of yellow letter */

    /* Flip animation state */
    int flip_row;                    /* row currently flipping, -1 = none */
    float flip_timer;                /* 0 .. FLIP_DURATION */

    /* Pending state change applied after flip animation completes */
    GameState pending_state;
    bool state_pending;

    Statistics stats;
    Settings settings;

    int guesses_used; /* set when game ends */
} Game;

void game_init(Game *g);
void game_reset(Game *g);
void game_change_state(Game *g, GameState next);
void game_handle_char(Game *g, int c);
void game_handle_backspace(Game *g);
void game_handle_enter(Game *g);
void game_update(Game *g, float dt);
void game_evaluate_guess(Game *g, int row);
bool game_hard_mode_valid(const Game *g, const char *word);
void game_play_sound(Game *g, int sound_id);
void game_set_sound_cb(void (*cb)(int));

#endif /* GAME_H */
