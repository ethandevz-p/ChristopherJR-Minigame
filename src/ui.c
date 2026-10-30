#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <raylib.h>

#define SND_COUNT 7

#define MAX_HOT 32
static HotButton s_hot[MAX_HOT];
static int s_hot_count = 0;

static struct { Rectangle rec; char ch; } s_key_rects[64];
static int s_key_count = 0;

char ui_keyboard_char_at(Vector2 mouse) {
    for (int i = 0; i < s_key_count; i++) {
        if (CheckCollisionPointRec(mouse, s_key_rects[i].rec))
            return s_key_rects[i].ch;
    }
    return 0;
}

Theme theme_dark = {
    .bg        = { 18, 18, 19, 255 },
    .panel     = { 28, 28, 30, 255 },
    .text      = { 255, 255, 255, 255 },
    .text_dim  = { 160, 160, 165, 255 },
    .tile_border = { 58, 58, 60, 255 },
    .tile_empty  = { 18, 18, 19, 255 },
    .tile_green  = { 83, 141, 78, 255 },
    .tile_yellow = { 181, 159, 59, 255 },
    .tile_gray   = { 58, 58, 60, 255 },
    .key_bg      = { 58, 58, 60, 255 },
    .key_text    = { 255, 255, 255, 255 },
    .accent      = { 0, 229, 255, 255 },
    .danger      = { 255, 80, 80, 255 },
};

Theme theme_light = {
    .bg        = { 245, 245, 247, 255 },
    .panel     = { 230, 230, 232, 255 },
    .text      = { 20, 20, 22, 255 },
    .text_dim  = { 100, 100, 105, 255 },
    .tile_border = { 58, 58, 60, 255 },
    .tile_empty  = { 245, 245, 247, 255 },
    .tile_green  = { 83, 141, 78, 255 },
    .tile_yellow = { 181, 159, 59, 255 },
    .tile_gray   = { 58, 58, 60, 255 },
    .key_bg      = { 58, 58, 60, 255 },
    .key_text    = { 255, 255, 255, 255 },
    .accent      = { 0, 150, 200, 255 },
    .danger      = { 220, 60, 60, 255 },
};

static Theme s_current;
static Font s_font;
static Font s_font_bold;
static Sound s_sounds[SND_COUNT];
static bool s_audio_ready = false;
static bool s_audio_enabled = true;

static const char *s_sound_files[SND_COUNT] = {
    "assets/sounds/click.wav",
    "assets/sounds/letter.wav",
    "assets/sounds/submit.wav",
    "assets/sounds/correct.wav",
    "assets/sounds/win.wav",
    "assets/sounds/lose.wav",
    "assets/sounds/invalid.wav",
};

void ui_init(void) {
    s_current = theme_dark;
    s_font = LoadFontEx("assets/fonts/Inter-Regular.ttf", 32, 0, 0);
    if (s_font.texture.id == 0) s_font = GetFontDefault();
    s_font_bold = LoadFontEx("assets/fonts/Inter-Bold.ttf", 32, 0, 0);
    if (s_font_bold.texture.id == 0) s_font_bold = GetFontDefault();
    SetTextLineSpacing(24);
}

void ui_close(void) {
    ui_audio_close();
    if (s_font.texture.id != 0 && s_font.texture.id != GetFontDefault().texture.id) UnloadFont(s_font);
    if (s_font_bold.texture.id != 0 && s_font_bold.texture.id != GetFontDefault().texture.id) UnloadFont(s_font_bold);
}

void ui_audio_init(void) {
    if (!IsAudioDeviceReady()) InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        s_audio_ready = false;
        return;
    }
    s_audio_ready = true;
    s_audio_enabled = true;
    for (int i = 0; i < SND_COUNT; i++) {
        if (FileExists(s_sound_files[i])) {
            s_sounds[i] = LoadSound(s_sound_files[i]);
        } else {
            s_sounds[i] = (Sound){0};
        }
    }
}

void ui_audio_play(int sound_id) {
    if (!s_audio_ready || !s_audio_enabled) return;
    if (sound_id < 0 || sound_id >= SND_COUNT) return;
    if (s_sounds[sound_id].frameCount > 0) PlaySound(s_sounds[sound_id]);
}

void ui_audio_close(void) {
    if (!s_audio_ready) return;
    for (int i = 0; i < SND_COUNT; i++) {
        if (s_sounds[i].frameCount > 0) UnloadSound(s_sounds[i]);
    }
    CloseAudioDevice();
    s_audio_ready = false;
}

void ui_set_theme(int theme_index) {
    s_current = (theme_index == 1) ? theme_light : theme_dark;
}

const Theme *ui_current_theme(void) {
    return &s_current;
}

int ui_hot_buttons(HotButton *out, int max) {
    int n = s_hot_count < max ? s_hot_count : max;
    memcpy(out, s_hot, n * sizeof(HotButton));
    return n;
}

static void hot_add(Rectangle bounds, UIAction action) {
    if (s_hot_count < MAX_HOT) {
        s_hot[s_hot_count].bounds = bounds;
        s_hot[s_hot_count].action = action;
        s_hot_count++;
    }
}

/* ---------------- helpers ---------------- */

static Color tile_color_for(TileColor tc, const Theme *t) {
    switch (tc) {
        case TILE_GREEN: return t->tile_green;
        case TILE_YELLOW: return t->tile_yellow;
        case TILE_GRAY: return t->tile_gray;
        default: return t->tile_empty;
    }
}

static void draw_text_centered(const char *text, int x, int y, int fontSize, Color color) {
    Vector2 ts = MeasureTextEx(s_font_bold, text, fontSize, 1);
    DrawTextEx(s_font_bold, text, (Vector2){ x - ts.x * 0.5f, y - fontSize * 0.5f }, fontSize, 1, color);
}

static void draw_text(const char *text, int x, int y, int fontSize, Color color) {
    DrawTextEx(s_font, text, (Vector2){ (float)x, (float)y }, fontSize, 1, color);
}

static bool draw_button(Rectangle bounds, const char *label, UIAction action, const Theme *t, Vector2 mouse) {
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    Color bg = hovered ? t->accent : t->panel;
    Color text = hovered ? t->bg : t->text;
    float round = 0.25f;

    DrawRectangleRounded(bounds, round, 8, bg);
    DrawRectangleRoundedLines(bounds, round, 8, 2.0f, t->text_dim);

    int fs = 24;
    Vector2 ts = MeasureTextEx(s_font_bold, label, fs, 1);
    Vector2 pos = { bounds.x + bounds.width * 0.5f - ts.x * 0.5f,
                    bounds.y + bounds.height * 0.5f - ts.y * 0.5f };
    DrawTextEx(s_font_bold, label, pos, fs, 1, text);

    hot_add(bounds, action);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

/* ---------------- board ---------------- */

static float tile_size_for_screen(int screen_w) {
    float board_w = screen_w * 0.85f;
    float tile = board_w / 5.0f;
    if (tile > 90.0f) tile = 90.0f;
    if (tile < 50.0f) tile = 50.0f;
    return tile;
}

static void draw_tile_simple(const Theme *t, float x, float y, float size, char letter, TileColor color, float flip_progress) {
    /* flip_progress: 0..1 (0 = letter face, 1 = color face) */
    Color back = tile_color_for(color, t);

    /* Tile body: empty face while flipping to color, color face after */
    Color body = (flip_progress < 0.5f) ? t->tile_empty : back;
    DrawRectangleRounded((Rectangle){ x, y, size, size }, 0.15f, 6, body);

    /* Border: always visible, matches color face once revealed */
    Color border = (flip_progress < 0.5f) ? t->tile_border : back;
    DrawRectangleRoundedLines((Rectangle){ x, y, size, size }, 0.15f, 6, 3.0f, border);

    if (letter == '\0') return;

    int fs = (int)(size * 0.6f);
    Vector2 ts = MeasureTextEx(s_font_bold, TextFormat("%c", letter), fs, 1);
    float cx = x + size * 0.5f;
    float cy = y + size * 0.5f - ts.y * 0.5f;

    if (flip_progress < 0.5f) {
        /* Letter face: fade out as we approach the middle */
        float alpha = cosf(flip_progress * 3.14159f); /* 1 -> 0 */
        if (alpha > 0.02f) {
            Color c = t->text;
            c.a = (unsigned char)(255 * alpha);
            float sx = ts.x * alpha;
            Vector2 pos = { cx - sx * 0.5f, cy };
            DrawTextEx(s_font_bold, TextFormat("%c", letter), pos, fs, 1, c);
        }
    } else {
        /* Color face: fade in after the middle */
        float alpha = cosf((flip_progress - 0.5f) * 3.14159f); /* 0 -> 1 */
        if (alpha > 0.02f) {
            Color c = WHITE;
            c.a = (unsigned char)(255 * alpha);
            Vector2 pos = { cx - ts.x * 0.5f, cy };
            DrawTextEx(s_font_bold, TextFormat("%c", letter), pos, fs, 1, c);
        }
    }
}

/* ---------------- keyboard ---------------- */

static const char *KEY_ROWS[3] = {
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"
};

static void draw_keyboard(const Theme *t, const Game *g, float base_x, float base_y, float key_w, float key_h) {
    for (int r = 0; r < 3; r++) {
        const char *row = KEY_ROWS[r];
        int len = (int)strlen(row);
        float row_w = key_w * len + 8.0f * (len - 1);
        float start_x = base_x - row_w * 0.5f;
        if (r == 1) start_x += key_w * 0.25f;
        if (r == 2) start_x += key_w * 0.5f;

        float y = base_y + r * (key_h + 8.0f);
        for (int i = 0; i < len; i++) {
            char ch = row[i];
            float x = start_x + i * (key_w + 8.0f);
            TileColor kc = g->key_colors[ch - 'A'];
            Color kbg = (kc == TILE_EMPTY) ? t->key_bg : tile_color_for(kc, t);

            Rectangle rec = { x, y, key_w, key_h };
            DrawRectangleRounded(rec, 0.22f, 6, kbg);
            DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, t->tile_border);

            Vector2 ts = MeasureTextEx(s_font_bold, TextFormat("%c", ch), key_h * 0.5f, 1);
            Vector2 pos = { x + key_w * 0.5f - ts.x * 0.5f, y + key_h * 0.5f - ts.y * 0.5f };
            DrawTextEx(s_font_bold, TextFormat("%c", ch), pos, key_h * 0.5f, 1, t->key_text);

            if (s_key_count < 64) {
                s_key_rects[s_key_count].rec = rec;
                s_key_rects[s_key_count].ch = ch;
                s_key_count++;
            }
        }
    }

    float y = base_y + 3 * (key_h + 8.0f);
    float bw = key_w * 2.2f;
    float bx_enter = base_x - bw * 0.5f - key_w * 0.5f - 4.0f;
    float bx_back  = base_x + bw * 0.5f + key_w * 0.5f + 4.0f;

    /* ENTER */
    {
        Rectangle rec = { bx_enter, y, bw, key_h };
        DrawRectangleRounded(rec, 0.22f, 6, t->accent);
        DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, t->text);
        const char *lbl = "ENTER";
        Vector2 ts = MeasureTextEx(s_font_bold, lbl, key_h * 0.4f, 1);
        Vector2 pos = { rec.x + rec.width * 0.5f - ts.x * 0.5f, rec.y + rec.height * 0.5f - ts.y * 0.5f };
        DrawTextEx(s_font_bold, lbl, pos, key_h * 0.4f, 1, t->bg);
        if (s_key_count < 64) {
            s_key_rects[s_key_count].rec = rec;
            s_key_rects[s_key_count].ch = '\n';
            s_key_count++;
        }
    }
    /* BACKSPACE */
    {
        Rectangle rec = { bx_back, y, bw, key_h };
        DrawRectangleRounded(rec, 0.22f, 6, t->key_bg);
        DrawRectangleRoundedLines(rec, 0.22f, 6, 2.0f, t->tile_border);
        const char *lbl = "⌫";
        Vector2 ts = MeasureTextEx(s_font_bold, lbl, key_h * 0.5f, 1);
        Vector2 pos = { rec.x + rec.width * 0.5f - ts.x * 0.5f, rec.y + rec.height * 0.5f - ts.y * 0.5f };
        DrawTextEx(s_font_bold, lbl, pos, key_h * 0.5f, 1, t->key_text);
        if (s_key_count < 64) {
            s_key_rects[s_key_count].rec = rec;
            s_key_rects[s_key_count].ch = '\b';
            s_key_count++;
        }
    }
}

/* ---------------- screens ---------------- */

static void draw_menu(const Game *g, const Theme *t, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float bw = 300.0f, bh = 60.0f;
    float cx = sw * 0.5f;
    float start_y = sh * 0.38f;

    ClearBackground(t->bg);

    draw_text_centered("WORDLE", cx, (int)(sh * 0.20f), 72, t->accent);
    draw_text_centered("GUESS THE WORD IN 6 TRIES", cx, (int)(sh * 0.29f), 20, t->text_dim);

    float y = start_y;
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "PLAY", UI_ACT_PLAY, t, mouse); y += 75;
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "HOW TO PLAY", UI_ACT_HOWTO, t, mouse); y += 75;
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "STATISTICS", UI_ACT_STATS, t, mouse); y += 75;
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "SETTINGS", UI_ACT_SETTINGS, t, mouse); y += 75;
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "EXIT", UI_ACT_EXIT, t, mouse);
    (void)g;
}

static void draw_howto(const Game *g, const Theme *t, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    ClearBackground(t->bg);
    draw_text_centered("HOW TO PLAY", sw / 2, (int)(sh * 0.12f), 56, t->accent);

    const char *lines[] = {
        "Guess the hidden 5-letter word in 6 tries.",
        "Each guess must be a valid 5-letter word.",
        "GREEN: letter is in the correct spot.",
        "YELLOW: letter is in the word, wrong spot.",
        "GRAY: letter is not in the word.",
        "Type with your keyboard or click the keys.",
        "Press ENTER to submit, BACKSPACE to delete.",
    };
    int n = sizeof(lines) / sizeof(lines[0]);
    float start_y = sh * 0.26f;
    for (int i = 0; i < n; i++) {
        draw_text_centered(lines[i], sw / 2, (int)(start_y + i * 34), 22, t->text_dim);
    }

    float bw = 260.0f, bh = 55.0f;
    draw_button((Rectangle){ sw/2 - bw*0.5f, sh - 120, bw, bh }, "BACK", UI_ACT_BACK, t, mouse);
    (void)g;
}

static void draw_playing(const Game *g, const Theme *t, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    ClearBackground(t->bg);

    draw_text_centered("WORDLE", sw / 2, 40, 48, t->accent);

    float tile = tile_size_for_screen(sw);
    float gap = 8.0f;
    float board_w = tile * 5 + gap * 4;
    float board_x = sw * 0.5f - board_w * 0.5f;
    float board_y = sh * 0.30f;

    for (int r = 0; r < 6; r++) {
        bool active_row = (r == g->current_row && !g->game_over);
        float shake = (g->shaking && r == g->current_row) ? g->shake_offset : 0.0f;
        for (int c = 0; c < WORD_LENGTH; c++) {
            float x = board_x + c * (tile + gap) + shake;
            float y = board_y + r * (tile + gap);
            char letter = g->guesses[r][c];
            TileColor color = g->guess_colors[r][c];
            float flip = 0.0f;
            if (g->flip_row == r) {
                flip = g->flip_timer / FLIP_DURATION;
                if (flip > 1.0f) flip = 1.0f;
                if (!active_row && c >= g->current_col) flip = 1.0f;
            }
            draw_tile_simple(t, x, y, tile, letter, color, flip);
        }
    }

    if (g->notification_time > 0.0f) {
        float a = g->notification_time / NOTIFY_DURATION;
        Color c = t->danger;
        c.a = (unsigned char)(255 * a);
        draw_text_centered(g->notification_msg, sw / 2, (int)(board_y - 40), 22, c);
    }

    float key_w = 50.0f, key_h = 62.0f;
    if (sw < 700) { key_w = 38.0f; key_h = 48.0f; }
    draw_keyboard(t, g, sw * 0.5f, sh * 0.62f, key_w, key_h);

    (void)mouse;
}

static void draw_result(const Game *g, const Theme *t, const char *title, const char *answer, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    ClearBackground(t->bg);

    draw_text_centered(title, sw / 2, (int)(sh * 0.30f), 56, t->accent);
    draw_text_centered(TextFormat("The word was %s", answer), sw / 2, (int)(sh * 0.39f), 26, t->text_dim);

    if (g->won) {
        draw_text_centered(TextFormat("Solved in %d %s!", g->guesses_used,
                       g->guesses_used == 1 ? "try" : "tries"),
                       sw / 2, (int)(sh * 0.46f), 24, t->tile_green);
    } else {
        draw_text_centered("Better luck next time!", sw / 2, (int)(sh * 0.46f), 24, t->text_dim);
    }

    float bw = 260.0f, bh = 55.0f;
    float start_y = sh * 0.58f;
    draw_button((Rectangle){ sw/2 - bw*0.5f - bw - 20, start_y, bw, bh }, "PLAY AGAIN", UI_ACT_RETRY, t, mouse);
    draw_button((Rectangle){ sw/2 - bw*0.5f + bw + 20, start_y, bw, bh }, "MAIN MENU", UI_ACT_MENU, t, mouse);
    draw_button((Rectangle){ sw/2 - bw*0.5f, start_y + 75, bw, bh }, "STATISTICS", UI_ACT_STATS, t, mouse);
    (void)g;
}

static void draw_settings(const Game *g, const Theme *t, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    ClearBackground(t->bg);
    draw_text_centered("SETTINGS", sw / 2, (int)(sh * 0.14f), 56, t->accent);

    float bw = 300.0f, bh = 55.0f;
    float cx = sw * 0.5f;
    float y = sh * 0.30f;

    draw_text_centered("THEME", cx, (int)(y - 30), 22, t->text_dim);
    const char *theme_label = (g->settings.theme == 1) ? "LIGHT" : "DARK";
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh },
                TextFormat("THEME: %s", theme_label), UI_ACT_TOGGLE_THEME, t, mouse); y += 80;

    draw_text_centered("HARD MODE", cx, (int)(y - 30), 22, t->text_dim);
    const char *hm_label = g->settings.hard_mode ? "ON" : "OFF";
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh },
                TextFormat("HARD MODE: %s", hm_label), UI_ACT_TOGGLE_HARD, t, mouse); y += 80;

    draw_text_centered("AUDIO", cx, (int)(y - 30), 22, t->text_dim);
    const char *au_label = g->settings.audio_enabled ? "ON" : "OFF";
    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh },
                TextFormat("SOUND: %s", au_label), UI_ACT_TOGGLE_AUDIO, t, mouse); y += 110;

    draw_button((Rectangle){ cx - bw * 0.5f, y, bw, bh }, "BACK", UI_ACT_BACK, t, mouse);
    (void)g;
}

static void draw_statistics(const Game *g, const Theme *t, Vector2 mouse) {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    ClearBackground(t->bg);
    draw_text_centered("STATISTICS", sw / 2, (int)(sh * 0.12f), 56, t->accent);

    const Statistics *s = &g->stats;
    float y = sh * 0.26f;
    draw_text_centered(TextFormat("GAMES PLAYED: %d", s->games_played), sw / 2, (int)y, 24, t->text); y += 34;
    draw_text_centered(TextFormat("GAMES WON: %d", s->games_won), sw / 2, (int)y, 24, t->text); y += 34;
    draw_text_centered(TextFormat("WIN RATE: %.1f%%", s->win_percentage), sw / 2, (int)y, 24, t->text); y += 34;
    draw_text_centered(TextFormat("CURRENT STREAK: %d", s->current_streak), sw / 2, (int)y, 24, t->text); y += 34;
    draw_text_centered(TextFormat("MAX STREAK: %d", s->max_streak), sw / 2, (int)y, 24, t->text); y += 50;

    draw_text_centered("GUESS DISTRIBUTION", sw / 2, (int)y, 22, t->text_dim); y += 30;
    for (int i = 1; i <= 6; i++) {
        draw_text_centered(TextFormat("%d: %d", i, s->distribution[i]), sw / 2, (int)y, 22, t->text); y += 26;
    }

    float bw = 220.0f, bh = 50.0f;
    draw_button((Rectangle){ sw/2 - bw*0.5f, sh - 110, bw, bh }, "BACK", UI_ACT_BACK, t, mouse);
    (void)g;
}

/* ---------------- main draw ---------------- */

void ui_draw(const Game *g) {
    const Theme *t = ui_current_theme();
    Vector2 mouse = GetMousePosition();
    s_hot_count = 0;
    s_key_count = 0;

    BeginDrawing();
    switch (g->state) {
        case STATE_MENU:     draw_menu(g, t, mouse); break;
        case STATE_HOWTO:    draw_howto(g, t, mouse); break;
        case STATE_PLAYING:  draw_playing(g, t, mouse); break;
        case STATE_WIN:
        case STATE_LOSE:     draw_result(g, t,
                    g->state == STATE_WIN ? "GREAT JOB!" : "GAME OVER", g->target, mouse); break;
        case STATE_SETTINGS: draw_settings(g, t, mouse); break;
        case STATE_STATISTICS: draw_statistics(g, t, mouse); break;
    }
    EndDrawing();
}
