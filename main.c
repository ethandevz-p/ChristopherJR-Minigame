/*
 * WORDLE - Single-file C + Raylib desktop game
 * Light-themed UI matching the classic Wordle look.
 * No external audio. Build with: make
 */

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WORD_LEN 5
#define MAX_TRIES 6
#define MAX_WORDS 2048

/* ---------- colors (light theme from reference) ---------- */
static const Color BG_COLOR      = { 248, 248, 250, 255 };
static const Color PANEL_COLOR   = { 240, 240, 242, 255 };
static const Color TILE_EMPTY    = { 222, 222, 226, 255 };
static const Color TILE_BORDER   = { 180, 180, 185, 255 };
static const Color TILE_GREEN    = { 106, 170, 100, 255 };
static const Color TILE_YELLOW   = { 201, 180, 88, 255 };
static const Color TILE_GRAY     = { 130, 130, 134, 255 };
static const Color TEXT_DARK     = { 30, 30, 33, 255 };
static const Color TEXT_DIM      = { 140, 140, 146, 255 };
static const Color BTN_GREEN     = { 86, 166, 96, 255 };

/* ---------- game state ---------- */
typedef enum {
    STATE_MENU,
    STATE_PLAYING,
    STATE_WIN,
    STATE_LOSE
} GameState;

typedef struct {
    GameState state;
    char target[WORD_LEN + 1];
    char guesses[MAX_TRIES][WORD_LEN + 1];
    int  colors[MAX_TRIES][WORD_LEN];
    int  cur_row;
    int  cur_col;
    bool game_over;
    bool won;
    int  guesses_used;

    /* animations */
    int  flip_row;
    float flip_timer;
    float shake_timer;
    float shake_offset;
    float notify_timer;
    char notify_msg[128];

    /* keyboard key colors (per letter) */
    int key_color[26];
} Game;

/* ---------- word list (embedded fallback) ---------- */
static const char *fallback_words[] = {
    "APPLE","BEACH","BRICK","CHAIR","CLOUD","CRANE","DANCE","EAGLE",
    "EARTH","FLAME","GHOST","HEART","INPUT","JUICE","LIGHT","MUSIC",
    "NIGHT","OCEAN","PIANO","QUEEN","RIVER","SHARK","TIGER","UNION",
    "VOICE","WATER","YACHT","ZEBRA","ABOUT","ABOVE","ADULT","ANGLE",
    "ARROW","BEGIN","BLACK","BLAME","BLANK","BLIND","BLOCK","BLOOD",
    "BOARD","BRAIN","BRAND","BRAVE","BREAD","BREAK","BRING","BUILD",
    "CABLE","CHAIN","CHAOS","CHARM","CHART","CHASE","CHEAP","CHECK",
    "CHEST","CHIEF","CHILD","CLAIM","CLASS","CLEAN","CLEAR","CLOCK",
    "CLOSE","CLOUD","COACH","COAST","COLOR","COVER","CRASH","CREAM",
    "CRIME","CROSS","CROWD","CROWN","DEATH","DEPTH","DREAM","DRINK",
    "DRIVE","EAGER","EARLY","EIGHT","EMPTY","ENEMY","ENTER","ENTRY",
    "ERROR","EVENT","EVERY","EXACT","EXTRA","FAITH","FALSE","FAULT",
    "FIELD","FIFTY","FIGHT","FINAL","FIRST","FLOAT","FLOOR","FOCUS",
    "FORCE","FORUM","FRAME","FRAUD","FRESH","FRONT","FRUIT","GHOST",
    "GIANT","GLASS","GLOBE","GRACE","GRADE","GRAIN","GRAND","GRANT",
    "GRAPE","GRASS","GREAT","GREEN","GRIEF","GROUP","GUARD","GUESS",
    "GUEST","HAPPY","HARSH","HEART","HORSE","HOTEL","HOUSE","HUMAN",
    "IDEAL","IMAGE",    "ISSUE","JUDGE","LABEL","LARGE","LATER",
    "LAYER","LEARN","LEAST","LEAVE","LEVEL","LIGHT","LIMIT","LINKS",
    "MUSIC","NEVER","NIGHT","NOISE","NORTH","NOTED","NOVEL","NURSE",
    "OCEAN","OFFER","OFTEN","ORDER","OTHER","PAINT","PANEL","PAPER",
    "PARTY","PHASE","PIANO","PIECE","PILOT","PLACE","PLANE","PLANT",
    "PLATE","PLACE","POINT","POLAR","POUND","POWER","PRESS","PRICE",
    "PRIME","PRIOR","PROUD","PROVE","QUEEN","QUICK","QUIET","RADIO",
    "RAISE","RANGE","REACH","READY","REFER","RELAX","RIGHT","RISKY",
    "RIVER","ROUND","ROUTE","ROYAL","SALAD","SALES","SAUCE","SCARE",
    "SCENE","SCORE","SENSE","SERVE","SEVEN","SHARK","SHARP","SHEEP",
    "SHEET","SHINE","SHIRT","SHOCK","SHOOT","SHORE","SHORT","SHOUT",
    "SIGHT","SILLY","SINCE","SIXTH","SKILL","SMALL","SMART","SMILE",
    "SMOKE","SNAKE","SOBER","SOUND","SOUTH","SPACE","SPARE","SPEAK",
    "SPEED","SPEND","SPORT","STAGE","STAIR","STAKE","STAMP","START",
    "STATE","STEAM","STEEL","STICK","STILL","STOCK","STONE","STOOD",
    "STORE","STORM","STORY","STRIP","STUCK","STUDY","STUFF","STYLE",
    "SUGAR","SUPER","TABLE","TASTE","TEACH","TEETH","THEME","THICK",
    "THING","THINK","THIRD","THOSE","THREE","THROW","TIGHT","TIMES",
    "TITLE","TODAY","TOUCH","TOUGH","TOXIC","TRACE","TRACK","TRADE",
    "TRAIL","TRAIN","TRAIT","TRASH","TREAT","TREND","TRIAL","TRIBE",
    "TRICK","TRUCK","TRULY","TRUST","TRUTH","TWICE","TWIST","UNION",
    "UNTIL","UPPER","URBAN","USUAL","VALID","VALUE","VERSE","VIGOR",
    "VIRUS","VISIT","VITAL","VOCAL","VOICE","WATCH","WATER","WEIRD",
    "WHEAT","WHEEL","WHERE","WHILE","WHITE","WHOLE","WOMEN","WORLD",
    "WORRY","WORSE","WORST","WORTH","WOULD","WRITE","WRONG","YIELD",
    "YOUNG","YOUTH","ZEBRA"
};
static const int fallback_count = sizeof(fallback_words) / sizeof(fallback_words[0]);

static char s_words[MAX_WORDS][WORD_LEN + 1];
static int s_word_count = 0;
static char s_target[WORD_LEN + 1];

static bool load_words(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    s_word_count = 0;
    char line[64];
    while (fgets(line, sizeof line, f) && s_word_count < MAX_WORDS) {
        size_t len = strlen(line);
        while (len > 0 && (line[len-1]=='\n'||line[len-1]=='\r'||
                           line[len-1]==' '||line[len-1]=='\t')) line[--len]='\0';
        if (len == 0) continue;
        for (size_t i = 0; i < len; i++) {
            char c = line[i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            if (i < WORD_LEN) s_words[s_word_count][i] = c;
        }
        s_words[s_word_count][WORD_LEN] = '\0';
        if (strlen(s_words[s_word_count]) == WORD_LEN) s_word_count++;
    }
    fclose(f);
    return s_word_count > 0;
}

static void pick_target(char *out) {
    int idx = rand() % (s_word_count > 0 ? s_word_count : fallback_count);
    if (s_word_count > 0) strcpy(out, s_words[idx]);
    else strcpy(out, fallback_words[idx % fallback_count]);
}

static bool word_is_valid(const char *w) {
    for (int i = 0; i < s_word_count; i++)
        if (strcmp(w, s_words[i]) == 0) return true;
    /* also accept fallback words */
    for (int i = 0; i < fallback_count; i++)
        if (strcmp(w, fallback_words[i]) == 0) return true;
    return false;
}

/* ---------- game logic ---------- */
static void game_init(Game *g) {
    memset(g, 0, sizeof(*g));
    g->state = STATE_PLAYING;
    g->cur_row = 0;
    g->cur_col = 0;
    for (int i = 0; i < 26; i++) g->key_color[i] = 0;
    pick_target(g->target);
}

static void game_reset(Game *g) {
    g->state = STATE_PLAYING;
    g->cur_row = 0;
    g->cur_col = 0;
    g->game_over = false;
    g->won = false;
    g->guesses_used = 0;
    g->flip_row = -1;
    g->flip_timer = 0.0f;
    g->shake_timer = 0.0f;
    g->shake_offset = 0.0f;
    g->notify_timer = 0.0f;
    g->notify_msg[0] = '\0';
    for (int r = 0; r < MAX_TRIES; r++) {
        for (int c = 0; c < WORD_LEN; c++) {
            g->guesses[r][c] = '\0';
            g->colors[r][c] = 0;
        }
    }
    for (int i = 0; i < 26; i++) g->key_color[i] = 0;
    pick_target(g->target);
}

static void evaluate_guess(Game *g, int row) {
    const char *target = g->target;
    const char *guess = g->guesses[row];
    int *colors = g->colors[row];
    for (int i = 0; i < WORD_LEN; i++) colors[i] = 3; /* gray default */

    int target_rem[26] = {0};
    bool matched[WORD_LEN] = {false};

    /* pass 1: greens */
    for (int i = 0; i < WORD_LEN; i++) {
        if (guess[i] == target[i]) {
            colors[i] = 1;
            matched[i] = true;
        } else {
            target_rem[target[i] - 'A']++;
        }
    }
    /* pass 2: yellows */
    for (int i = 0; i < WORD_LEN; i++) {
        if (colors[i] == 1) continue;
        int idx = guess[i] - 'A';
        if (target_rem[idx] > 0) {
            colors[i] = 2;
            target_rem[idx]--;
        }
    }
    /* keyboard colors */
    for (int i = 0; i < WORD_LEN; i++) {
        int idx = guess[i] - 'A';
        int rank_new = (colors[i]==1)?3:(colors[i]==2)?2:(colors[i]==3)?1:0;
        int rank_cur = (g->key_color[idx]==1)?3:(g->key_color[idx]==2)?2:(g->key_color[idx]==3)?1:0;
        if (rank_new > rank_cur) g->key_color[idx] = colors[i];
    }
}

static void handle_char(Game *g, int ch) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->cur_row >= MAX_TRIES) return;
    if (g->cur_col >= WORD_LEN) return;
    char c = (char)ch;
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    if (c < 'A' || c > 'Z') return;
    g->guesses[g->cur_row][g->cur_col] = c;
    g->guesses[g->cur_row][g->cur_col + 1] = '\0';
    g->cur_col++;
}

static void handle_backspace(Game *g) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->cur_row >= MAX_TRIES) return;
    if (g->cur_col == 0) return;
    g->cur_col--;
    g->guesses[g->cur_row][g->cur_col] = '\0';
}

static void handle_enter(Game *g) {
    if (g->state != STATE_PLAYING || g->game_over) return;
    if (g->cur_row >= MAX_TRIES) return;
    if (g->cur_col != WORD_LEN) {
        snprintf(g->notify_msg, sizeof g->notify_msg, "Not enough letters");
        g->notify_timer = 2.0f;
        g->shake_timer = 0.4f;
        g->shake_offset = 0.0f;
        return;
    }
    char word[WORD_LEN + 1];
    memcpy(word, g->guesses[g->cur_row], WORD_LEN);
    word[WORD_LEN] = '\0';
    if (!word_is_valid(word)) {
        snprintf(g->notify_msg, sizeof g->notify_msg, "Not in word list");
        g->notify_timer = 2.0f;
        g->shake_timer = 0.4f;
        g->shake_offset = 0.0f;
        return;
    }
    g->flip_row = g->cur_row;
    g->flip_timer = 0.0f;
    evaluate_guess(g, g->cur_row);

    if (strcmp(word, g->target) == 0) {
        g->won = true;
        g->game_over = true;
        g->guesses_used = g->cur_row + 1;
        g->state = STATE_WIN;
    } else {
        g->cur_row++;
        g->cur_col = 0;
        if (g->cur_row >= MAX_TRIES) {
            g->game_over = true;
            g->won = false;
            g->guesses_used = MAX_TRIES;
            g->state = STATE_LOSE;
        }
    }
}

static void game_update(Game *g, float dt) {
    if (g->flip_row >= 0) {
        g->flip_timer += dt;
        if (g->flip_timer >= 0.25f) {
            g->flip_timer = 0.25f;
            g->flip_row = -1;
        }
    }
    if (g->shake_timer > 0.0f) {
        g->shake_timer -= dt;
        if (g->shake_timer <= 0.0f) {
            g->shake_timer = 0.0f;
            g->shake_offset = 0.0f;
        } else {
            g->shake_offset = sinf(g->shake_timer * 60.0f) * 6.0f;
        }
    }
    if (g->notify_timer > 0.0f) {
        g->notify_timer -= dt;
        if (g->notify_timer < 0.0f) g->notify_timer = 0.0f;
    }
}

/* ---------- drawing ---------- */
static Color tile_color_for(int c) {
    switch (c) {
        case 1: return TILE_GREEN;
        case 2: return TILE_YELLOW;
        case 3: return TILE_GRAY;
        default: return TILE_EMPTY;
    }
}

static void draw_tile(float x, float y, float size, char letter, int color, float flip) {
    Color back = (flip < 0.5f) ? TILE_EMPTY : tile_color_for(color);
    Color border = (flip < 0.5f) ? TILE_BORDER : tile_color_for(color);
    DrawRectangleRounded((Rectangle){x, y, size, size}, 0.12f, 6, back);
    DrawRectangleRoundedLines((Rectangle){x, y, size, size}, 0.12f, 6, 3.0f, border);
    if (letter == '\0') return;
    int fs = (int)(size * 0.6f);
    Vector2 ts = MeasureTextEx(GetFontDefault(), TextFormat("%c", letter), fs, 1);
    float cx = x + size * 0.5f;
    float cy = y + size * 0.5f - ts.y * 0.5f;
    if (flip < 0.5f) {
        float a = cosf(flip * 3.14159f);
        if (a > 0.02f) {
            Color col = TEXT_DARK;
            col.a = (unsigned char)(255 * a);
            float sx = ts.x * a;
            DrawTextEx(GetFontDefault(), TextFormat("%c", letter),
                       (Vector2){cx - sx * 0.5f, cy}, fs, 1, col);
        }
    } else {
        float a = cosf((flip - 0.5f) * 3.14159f);
        if (a > 0.02f) {
            Color col = WHITE;
            col.a = (unsigned char)(255 * a);
            DrawTextEx(GetFontDefault(), TextFormat("%c", letter),
                       (Vector2){cx - ts.x * 0.5f, cy}, fs, 1, col);
        }
    }
}

static const char *KEY_ROWS[3] = {
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"
};

static float key_w, key_h, key_base_x, key_base_y;
static struct { Rectangle rec; char ch; } s_keys[64];
static int s_key_count = 0;

static void draw_keyboard(Game *g) {
    int sw = GetScreenWidth();
    key_w = 52.0f; key_h = 64.0f;
    if (sw < 700) { key_w = 40.0f; key_h = 50.0f; }
    key_base_x = sw * 0.5f;
    key_base_y = (float)GetScreenHeight() * 0.66f;
    s_key_count = 0;

    for (int r = 0; r < 3; r++) {
        const char *row = KEY_ROWS[r];
        int len = (int)strlen(row);
        float row_w = key_w * len + 8.0f * (len - 1);
        float start_x = key_base_x - row_w * 0.5f;
        if (r == 1) start_x += key_w * 0.25f;
        if (r == 2) start_x += key_w * 0.5f;
        float y = key_base_y + r * (key_h + 8.0f);
        for (int i = 0; i < len; i++) {
            char ch = row[i];
            float x = start_x + i * (key_w + 8.0f);
            int kc = g->key_color[ch - 'A'];
            Color kbg = (kc == 0) ? TILE_EMPTY : tile_color_for(kc);
            Rectangle rec = {x, y, key_w, key_h};
            DrawRectangleRounded(rec, 0.22f, 6, kbg);
            DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, TILE_BORDER);
            int fs = (int)(key_h * 0.5f);
            Vector2 ts = MeasureTextEx(GetFontDefault(), TextFormat("%c", ch), fs, 1);
            DrawTextEx(GetFontDefault(), TextFormat("%c", ch),
                       (Vector2){x + key_w*0.5f - ts.x*0.5f, y + key_h*0.5f - ts.y*0.5f},
                       fs, 1, TEXT_DARK);
            if (s_key_count < 64) {
                s_keys[s_key_count].rec = rec;
                s_keys[s_key_count].ch = ch;
                s_key_count++;
            }
        }
    }

    float y = key_base_y + 3 * (key_h + 8.0f);
    float bw = key_w * 2.4f;
    float bx_enter = key_base_x - bw*0.5f - key_w*0.5f - 4.0f;
    float bx_back  = key_base_x + bw*0.5f + key_w*0.5f + 4.0f;

    /* ENTER */
    {
        Rectangle rec = {bx_enter, y, bw, key_h};
        DrawRectangleRounded(rec, 0.22f, 6, PANEL_COLOR);
        DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, TEXT_DIM);
        const char *lbl = "Enter";
        int fs = (int)(key_h * 0.4f);
        Vector2 ts = MeasureTextEx(GetFontDefault(), lbl, fs, 1);
        DrawTextEx(GetFontDefault(), lbl,
                   (Vector2){rec.x + rec.width*0.5f - ts.x*0.5f, rec.y + rec.height*0.5f - ts.y*0.5f},
                   fs, 1, TEXT_DARK);
        if (s_key_count < 64) { s_keys[s_key_count].rec = rec; s_keys[s_key_count].ch = '\n'; s_key_count++; }
    }
    /* BACKSPACE */
    {
        Rectangle rec = {bx_back, y, bw, key_h};
        DrawRectangleRounded(rec, 0.22f, 6, TILE_EMPTY);
        DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, TILE_BORDER);
        const char *lbl = "⌫";
        int fs = (int)(key_h * 0.55f);
        Vector2 ts = MeasureTextEx(GetFontDefault(), lbl, fs, 1);
        DrawTextEx(GetFontDefault(), lbl,
                   (Vector2){rec.x + rec.width*0.5f - ts.x*0.5f, rec.y + rec.height*0.5f - ts.y*0.5f},
                   fs, 1, TEXT_DARK);
        if (s_key_count < 64) { s_keys[s_key_count].rec = rec; s_keys[s_key_count].ch = '\b'; s_key_count++; }
    }
}

static char keyboard_char_at(Vector2 mouse) {
    for (int i = 0; i < s_key_count; i++)
        if (CheckCollisionPointRec(mouse, s_keys[i].rec)) return s_keys[i].ch;
    return 0;
}

static void draw_text_centered(const char *text, int x, int y, int fs, Color col) {
    Vector2 ts = MeasureTextEx(GetFontDefault(), text, fs, 1);
    DrawTextEx(GetFontDefault(), text, (Vector2){x - ts.x*0.5f, y - fs*0.5f}, fs, 1, col);
}

static void draw_button(Rectangle bounds, const char *label, bool hovered, Color bgcol, Color textcol) {
    Color bg = hovered ? (Color){bgcol.r*0.85f,bgcol.g*0.85f,bgcol.b*0.85f,bgcol.a} : bgcol;
    DrawRectangleRounded(bounds, 0.28f, 8, bg);
    int fs = 24;
    Vector2 ts = MeasureTextEx(GetFontDefault(), label, fs, 1);
    DrawTextEx(GetFontDefault(), label,
               (Vector2){bounds.x + bounds.width*0.5f - ts.x*0.5f, bounds.y + bounds.height*0.5f - ts.y*0.5f},
                fs, 1, textcol);
}

/* ---------- main ---------- */
int main(void) {
    InitWindow(900, 720, "WORDLE");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    srand((unsigned int)time(NULL));
    load_words("data/valid_words.txt");

    Game g;
    game_init(&g);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        Vector2 mouse = GetMousePosition();

        /* physical keyboard input */
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z'))
                handle_char(&g, key);
            key = GetCharPressed();
        }

        bool enter_pressed = IsKeyPressed(KEY_ENTER);
        bool backspace_pressed = IsKeyPressed(KEY_BACKSPACE);

        /* mouse clicks on keyboard */
        if (g.state == STATE_PLAYING && !g.game_over && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            char kch = keyboard_char_at(mouse);
            if (kch == '\n') enter_pressed = true;
            else if (kch == '\b') backspace_pressed = true;
            else handle_char(&g, kch);
        }

        if (enter_pressed) handle_enter(&g);
        if (backspace_pressed) handle_backspace(&g);

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (g.state == STATE_WIN || g.state == STATE_LOSE) game_reset(&g);
        }

        game_update(&g, dt);

        BeginDrawing();
        ClearBackground(BG_COLOR);

        int sw = GetScreenWidth();
        int sh = GetScreenHeight();

        if (g.state == STATE_PLAYING || g.state == STATE_WIN || g.state == STATE_LOSE) {
            /* header */
            draw_text_centered("WORDLE", sw / 2, 52, 56, TEXT_DARK);

            /* board */
            float tile = 68.0f;
            float gap = 10.0f;
            float board_w = tile * 5 + gap * 4;
            float board_x = sw * 0.5f - board_w * 0.5f;
            float board_y = sh * 0.20f;

            for (int r = 0; r < MAX_TRIES; r++) {
                bool active = (r == g.cur_row && !g.game_over);
                float shake = (g.shake_timer > 0.0f && r == g.cur_row) ? g.shake_offset : 0.0f;
                for (int c = 0; c < WORD_LEN; c++) {
                    float x = board_x + c * (tile + gap) + shake;
                    float y = board_y + r * (tile + gap);
                    char letter = g.guesses[r][c];
                    int color = g.colors[r][c];
                    float flip = 0.0f;
                    if (g.flip_row == r) {
                        flip = g.flip_timer / 0.25f;
                        if (flip > 1.0f) flip = 1.0f;
                        if (!active && c >= g.cur_col) flip = 1.0f;
                    }
                    draw_tile(x, y, tile, letter, color, flip);
                }
            }

            if (g.notify_timer > 0.0f) {
                float a = g.notify_timer / 2.0f;
                Color col = TILE_GRAY;
                col.a = (unsigned char)(255 * a);
                draw_text_centered(g.notify_msg, sw / 2, (int)(board_y - 36), 20, col);
            }

            draw_keyboard(&g);
        }

        if (g.state == STATE_WIN) {
            DrawRectangle(0, 0, sw, sh, (Color){248, 248, 250, 230});
            draw_text_centered("You Win! 🎉", sw / 2, sh * 0.34f, 40, TEXT_DARK);
            draw_text_centered(TextFormat("The word was %s", g.target), sw / 2, sh * 0.41f, 22, TEXT_DIM);
            Rectangle btn = {sw/2 - 130, sh * 0.52f, 260, 56};
            bool hovered = CheckCollisionPointRec(mouse, btn);
            draw_button(btn, "NEW GAME", hovered, BTN_GREEN, WHITE);
            if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) game_reset(&g);
        }

        if (g.state == STATE_LOSE) {
            DrawRectangle(0, 0, sw, sh, (Color){248, 248, 250, 230});
            draw_text_centered("Game Over", sw / 2, sh * 0.34f, 40, TEXT_DARK);
            draw_text_centered(TextFormat("The word was %s", g.target), sw / 2, sh * 0.41f, 22, TEXT_DIM);
            Rectangle btn = {sw/2 - 130, sh * 0.52f, 260, 56};
            bool hovered = CheckCollisionPointRec(mouse, btn);
            draw_button(btn, "NEW GAME", hovered, BTN_GREEN, WHITE);
            if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) game_reset(&g);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
