#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720
#define MAX_OBSTACLES 64
#define MAX_PARTICLES 120
#define PLAYER_SIZE   40.0f
#define PLAYER_SPEED  340.0f
#define HIGHSCORE_FILE "highscore.txt"

static const Color COLOR_BG     = { 8, 11, 18, 255 };
static const Color COLOR_CYAN   = { 0, 229, 255, 255 };
static const Color COLOR_PURPLE = { 168, 85, 247, 255 };
static const Color COLOR_RED    = { 255, 59, 92, 255 };
static const Color COLOR_DIM    = { 140, 150, 180, 255 };
static const Color COLOR_WHITE  = { 240, 248, 255, 255 };

typedef enum { STATE_MENU, STATE_HOWTO, STATE_PLAY, STATE_PAUSE, STATE_GAMEOVER } GameState;

typedef struct {
    Vector2 position;
    Vector2 size;
    float speed;
    Color color;
} Player;

typedef struct {
    Vector2 position;
    Vector2 size;
    Vector2 velocity;
    float speed;
    Color color;
    bool active;
} Obstacle;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    Color color;
    bool active;
} Particle;

typedef struct {
    Rectangle bounds;
    const char *text;
} Button;

typedef struct {
    GameState state;
    float transitionAlpha;
    float transitionTimer;
    bool transitioning;

    Player player;
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleCount;
    float spawnTimer;
    float spawnInterval;
    float obstacleSpeed;
    float score;
    int highScore;
    float survivalTime;

    Particle particles[MAX_PARTICLES];

    Button btnPlay;
    Button btnHowto;
    Button btnExit;
    Button btnRetry;
    Button btnMenu;
    Button btnBack;
    Button btnResume;

    bool audioEnabled;
    bool clickLoaded, hitLoaded, pointLoaded, musicLoaded;
    Sound sndClick, sndHit, sndPoint;
    Music music;
} Game;

static int LoadHighScore(void) {
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    int hs = 0;
    if (f) {
        if (fscanf(f, "%d", &hs) != 1) hs = 0;
        fclose(f);
    }
    return hs;
}

static void SaveHighScore(int score) {
    FILE *f = fopen(HIGHSCORE_FILE, "w");
    if (f) {
        fprintf(f, "%d", score);
        fclose(f);
    }
}

static void InitAudioAssets(Game *g) {
    g->audioEnabled = false;
    g->clickLoaded = g->hitLoaded = g->pointLoaded = g->musicLoaded = false;

    if (!IsAudioDeviceReady()) InitAudioDevice();
    if (!IsAudioDeviceReady()) return;
    g->audioEnabled = true;

    if (FileExists("assets/click.wav")) {
        g->sndClick = LoadSound("assets/click.wav");
        g->clickLoaded = true;
    }
    if (FileExists("assets/hit.wav")) {
        g->sndHit = LoadSound("assets/hit.wav");
        g->hitLoaded = true;
    }
    if (FileExists("assets/point.wav")) {
        g->sndPoint = LoadSound("assets/point.wav");
        g->pointLoaded = true;
    }
    if (FileExists("assets/music.ogg")) {
        g->music = LoadMusicStream("assets/music.ogg");
        g->musicLoaded = true;
        SetMusicVolume(g->music, 0.4f);
        PlayMusicStream(g->music);
    }
}

static void PlayClick(Game *g) {
    if (g->audioEnabled && g->clickLoaded) PlaySound(g->sndClick);
}
static void PlayHit(Game *g) {
    if (g->audioEnabled && g->hitLoaded) PlaySound(g->sndHit);
}
static void PlayPoint(Game *g) {
    if (g->audioEnabled && g->pointLoaded) PlaySound(g->sndPoint);
}
static void UpdateMusic(Game *g) {
    if (g->audioEnabled && g->musicLoaded) UpdateMusicStream(g->music);
}
static void StopMusic(Game *g) {
    if (g->audioEnabled && g->musicLoaded) StopMusicStream(g->music);
}
static void CloseAudioAssets(Game *g) {
    if (g->audioEnabled) {
        if (g->clickLoaded) UnloadSound(g->sndClick);
        if (g->hitLoaded) UnloadSound(g->sndHit);
        if (g->pointLoaded) UnloadSound(g->sndPoint);
        if (g->musicLoaded) UnloadMusicStream(g->music);
        CloseAudioDevice();
    }
}

static void InitPlayer(Player *p) {
    p->position = (Vector2){ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f };
    p->size = (Vector2){ PLAYER_SIZE, PLAYER_SIZE };
    p->speed = PLAYER_SPEED;
    p->color = COLOR_CYAN;
}

static void SpawnObstacle(Game *g) {
    if (g->obstacleCount >= MAX_OBSTACLES) return;

    int side = GetRandomValue(0, 3);
    float x, y, vx, vy;
    float px = g->player.position.x;
    float py = g->player.position.y;

    switch (side) {
        case 0: x = -60.0f;            y = (float)GetRandomValue(0, SCREEN_HEIGHT); break;
        case 1: x = (float)SCREEN_WIDTH + 60.0f; y = (float)GetRandomValue(0, SCREEN_HEIGHT); break;
        case 2: x = (float)GetRandomValue(0, SCREEN_WIDTH); y = -60.0f; break;
        default: x = (float)GetRandomValue(0, SCREEN_WIDTH); y = (float)SCREEN_HEIGHT + 60.0f; break;
    }

    float dx = px - x;
    float dy = py - y;
    float dist = sqrtf(dx * dx + dy * dy);
    if (dist == 0.0f) dist = 1.0f;
    vx = dx / dist;
    vy = dy / dist;

    Obstacle *o = &g->obstacles[g->obstacleCount++];
    o->position = (Vector2){ x, y };
    o->size = (Vector2){ 36.0f, 36.0f };
    o->velocity = (Vector2){ vx, vy };
    o->speed = g->obstacleSpeed + (float)GetRandomValue(-30, 30);
    o->color = COLOR_RED;
    o->active = true;
}

static void ResetGame(Game *g) {
    InitPlayer(&g->player);
    for (int i = 0; i < MAX_OBSTACLES; i++) g->obstacles[i].active = false;
    g->obstacleCount = 0;
    g->score = 0.0f;
    g->survivalTime = 0.0f;
    g->obstacleSpeed = 180.0f;
    g->spawnInterval = 1.1f;
    g->spawnTimer = 0.0f;
    for (int i = 0; i < MAX_PARTICLES; i++) g->particles[i].active = false;
}

static void InitParticles(Game *g) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        g->particles[i].active = false;
        g->particles[i].color = COLOR_CYAN;
        g->particles[i].color.a = 120;
    }
}

static void SpawnParticle(Game *g, float x, float y, Color color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!g->particles[i].active) {
            g->particles[i].active = true;
            g->particles[i].position = (Vector2){ x, y };
            g->particles[i].velocity = (Vector2){
                (float)GetRandomValue(-40, 40) * 0.1f,
                (float)GetRandomValue(-40, 40) * 0.1f
            };
            g->particles[i].life = 0.0f;
            g->particles[i].maxLife = (float)GetRandomValue(30, 90) * 0.05f;
            g->particles[i].color = color;
            return;
        }
    }
}

static void UpdateParticles(Game *g, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &g->particles[i];
        if (!p->active) continue;

        p->position.x += p->velocity.x * dt * 60.0f;
        p->position.y += p->velocity.y * dt * 60.0f;
        p->velocity.x *= 0.94f;
        p->velocity.y *= 0.94f;
        p->life += dt;

        if (p->life >= p->maxLife) p->active = false;
    }

    if (GetRandomValue(0, 4) == 0) {
        SpawnParticle(g, (float)GetRandomValue(0, SCREEN_WIDTH), -12.0f, COLOR_CYAN);
    }
    if (GetRandomValue(0, 6) == 0) {
        SpawnParticle(g, (float)SCREEN_WIDTH + 12.0f, (float)GetRandomValue(0, SCREEN_HEIGHT), COLOR_PURPLE);
    }
}

static void DrawParticle(const Particle *p) {
    float alpha = 1.0f - (p->life / p->maxLife);
    if (alpha < 0.0f) alpha = 0.0f;
    Color c = p->color;
    c.a = (unsigned char)(c.a * alpha);
    float size = 3.0f + 2.0f * alpha;
    DrawRectangle(p->position.x - size * 0.5f, p->position.y - size * 0.5f, size, size, c);
}

static void DrawGlowRect(Rectangle rec, Color color, float glow) {
    Color c = color;
    c.a = (unsigned char)(color.a * glow * 0.35f);
    float pad = rec.width * 0.18f * glow;
    Rectangle outer = {
        rec.x - pad, rec.y - pad,
        rec.width + pad * 2.0f, rec.height + pad * 2.0f
    };
    DrawRectangleRounded(outer, 0.25f, 6, c);
}

static void DrawPlayer(const Player *p) {
    Rectangle rec = {
        p->position.x - p->size.x * 0.5f,
        p->position.y - p->size.y * 0.5f,
        p->size.x, p->size.y
    };
    DrawGlowRect(rec, p->color, 1.2f);
    DrawRectangleRounded(rec, 0.25f, 6, p->color);
    Rectangle inner = rec;
    inner.x += rec.width * 0.2f;
    inner.y += rec.height * 0.2f;
    inner.width *= 0.6f;
    inner.height *= 0.6f;
    DrawRectangleRounded(inner, 0.25f, 6, (Color){ 255, 255, 255, 170 });
}

static void DrawObstacle(const Obstacle *o) {
    Rectangle rec = {
        o->position.x - o->size.x * 0.5f,
        o->position.y - o->size.y * 0.5f,
        o->size.x, o->size.y
    };
    DrawGlowRect(rec, o->color, 1.0f);
    DrawRectangleRounded(rec, 0.2f, 4, o->color);
}

static bool ButtonHovered(const Button *b, Vector2 mouse) {
    return CheckCollisionPointRec(mouse, b->bounds);
}

static void DrawButton(const Button *b, bool hovered) {
    Color bg = hovered ? COLOR_PURPLE : (Color){ 30, 38, 60, 230 };
    Color text = hovered ? COLOR_WHITE : COLOR_DIM;

    DrawGlowRect(b->bounds, hovered ? COLOR_PURPLE : COLOR_CYAN, hovered ? 1.0f : 0.5f);
    DrawRectangleRounded(b->bounds, 0.3f, 8, bg);

    int fontSize = 26;
    int textW = MeasureText(b->text, fontSize);
    int textH = fontSize;
    Vector2 pos = {
        b->bounds.x + b->bounds.width * 0.5f - textW * 0.5f,
        b->bounds.y + b->bounds.height * 0.5f - textH * 0.5f
    };
    DrawText(b->text, (int)pos.x, (int)pos.y, fontSize, text);
}

static void NewTransition(Game *g) {
    g->transitioning = true;
    g->transitionTimer = 0.0f;
    g->transitionAlpha = 0.0f;
}

static void UpdateTransition(Game *g, float dt) {
    if (!g->transitioning) return;
    g->transitionTimer += dt;
    if (g->transitionTimer < 0.15f) {
        g->transitionAlpha = g->transitionTimer / 0.15f;
    } else if (g->transitionTimer < 0.30f) {
        g->transitionAlpha = 1.0f - (g->transitionTimer - 0.15f) / 0.15f;
    } else {
        g->transitioning = false;
        g->transitionAlpha = 0.0f;
    }
}

static void DrawTransition(const Game *g) {
    if (!g->transitioning && g->transitionAlpha <= 0.0f) return;
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  (Color){ 8, 11, 18, (unsigned char)(g->transitionAlpha * 220) });
}

static void ChangeState(Game *g, GameState next) {
    bool wasPlaying = (g->state == STATE_PLAY);
    g->state = next;
    NewTransition(g);
    if (next == STATE_PLAY && g->musicLoaded) PlayMusicStream(g->music);
    if (wasPlaying && next != STATE_PLAY && next != STATE_PAUSE) StopMusic(g);
}

static void InitGame(Game *g) {
    memset(g, 0, sizeof(*g));
    g->state = STATE_MENU;
    g->highScore = LoadHighScore();

    float bw = 300.0f, bh = 60.0f;
    float cx = SCREEN_WIDTH * 0.5f;
    float startY = SCREEN_HEIGHT * 0.45f;

    g->btnPlay = (Button){ (Rectangle){ cx - bw * 0.5f, startY, bw, bh }, "PLAY" };
    g->btnHowto = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 80, bw, bh }, "HOW TO PLAY" };
    g->btnExit = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 160, bw, bh }, "EXIT" };

    g->btnRetry = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 60, bw, bh }, "RETRY" };
    g->btnMenu = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 140, bw, bh }, "MAIN MENU" };
    g->btnBack = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 160, bw, bh }, "BACK" };
    g->btnResume = (Button){ (Rectangle){ cx - bw * 0.5f, startY + 20, bw, bh }, "RESUME" };

    InitParticles(g);
    InitAudioAssets(g);
}

static void UpdateMenu(Game *g, Vector2 mouse) {
    if (ButtonHovered(&g->btnPlay, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        ResetGame(g);
        ChangeState(g, STATE_PLAY);
    } else if (ButtonHovered(&g->btnHowto, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        ChangeState(g, STATE_HOWTO);
    } else if (ButtonHovered(&g->btnExit, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        /* Exit is handled in main loop via WindowShouldClose, but close here directly */
        /* We signal exit by setting a special flag via closing window */
    }
}

static void UpdatePlaying(Game *g, float dt) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        PlayClick(g);
        ChangeState(g, STATE_PAUSE);
        return;
    }

    float dx = 0.0f, dy = 0.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dy = -1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy = 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx = -1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx = 1.0f;

    float len = sqrtf(dx * dx + dy * dy);
    if (len > 0.0f) {
        dx /= len; dy /= len;
        g->player.position.x += dx * g->player.speed * dt;
        g->player.position.y += dy * g->player.speed * dt;
    }

    float hw = g->player.size.x * 0.5f;
    float hh = g->player.size.y * 0.5f;
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();
    if (g->player.position.x < hw) g->player.position.x = hw;
    if (g->player.position.x > sw - hw) g->player.position.x = sw - hw;
    if (g->player.position.y < hh) g->player.position.y = hh;
    if (g->player.position.y > sh - hh) g->player.position.y = sh - hh;

    g->score += dt * 12.0f;
    g->survivalTime += dt;

    g->obstacleSpeed = 180.0f + g->survivalTime * 22.0f;
    if (g->obstacleSpeed > 520.0f) g->obstacleSpeed = 520.0f;
    g->spawnInterval = 1.1f - g->survivalTime * 0.07f;
    if (g->spawnInterval < 0.45f) g->spawnInterval = 0.45f;

    g->spawnTimer += dt;
    while (g->spawnTimer >= g->spawnInterval && g->obstacleCount < MAX_OBSTACLES) {
        g->spawnTimer -= g->spawnInterval;
        SpawnObstacle(g);
    }

    Rectangle playerRec = {
        g->player.position.x - g->player.size.x * 0.5f,
        g->player.position.y - g->player.size.y * 0.5f,
        g->player.size.x, g->player.size.y
    };

    int swInt = GetScreenWidth();
    int shInt = GetScreenHeight();

    for (int i = 0; i < g->obstacleCount; i++) {
        Obstacle *o = &g->obstacles[i];
        if (!o->active) continue;
        o->position.x += o->velocity.x * o->speed * dt;
        o->position.y += o->velocity.y * o->speed * dt;

        Rectangle orec = {
            o->position.x - o->size.x * 0.5f,
            o->position.y - o->size.y * 0.5f,
            o->size.x, o->size.y
        };
        if (CheckCollisionRecs(playerRec, orec)) {
            PlayHit(g);
            if ((int)g->score > g->highScore) {
                g->highScore = (int)g->score;
                SaveHighScore(g->highScore);
            }
            ChangeState(g, STATE_GAMEOVER);
            return;
        }

        if (o->position.x < -100 || o->position.x > swInt + 100 ||
            o->position.y < -100 || o->position.y > shInt + 100) {
            o->active = false;
            g->obstacleCount--;
            for (int j = i; j < g->obstacleCount; j++) {
                g->obstacles[j] = g->obstacles[j + 1];
            }
            i--;
            PlayPoint(g);
            for (int k = 0; k < 6; k++) {
                SpawnParticle(g, o->position.x, o->position.y, COLOR_RED);
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        SpawnParticle(g, g->player.position.x + (float)GetRandomValue(-15, 15),
                      g->player.position.y + (float)GetRandomValue(-15, 15), COLOR_CYAN);
    }
}

static void UpdatePause(Game *g, Vector2 mouse) {
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        PlayClick(g);
        ChangeState(g, STATE_PLAY);
        return;
    }
    if (ButtonHovered(&g->btnResume, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        ChangeState(g, STATE_PLAY);
    }
}

static void UpdateGameOver(Game *g, Vector2 mouse) {
    if (ButtonHovered(&g->btnRetry, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        ResetGame(g);
        ChangeState(g, STATE_PLAY);
    } else if (ButtonHovered(&g->btnMenu, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        StopMusic(g);
        ChangeState(g, STATE_MENU);
    }
}

static void UpdateHowto(Game *g, Vector2 mouse) {
    if (ButtonHovered(&g->btnBack, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlayClick(g);
        ChangeState(g, STATE_MENU);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        PlayClick(g);
        ChangeState(g, STATE_MENU);
    }
}

static void DrawTextCentered(const char *text, int x, int y, int fontSize, Color color) {
    int w = MeasureText(text, fontSize);
    DrawText(text, x - w / 2, y, fontSize, color);
}

static void DrawMenu(const Game *g, Vector2 mouse) {
    DrawTextCentered("NEON DODGE", GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.22f), 54, COLOR_CYAN);
    DrawTextCentered("SURVIVE THE NEON STREAM", GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.29f), 20, COLOR_DIM);

    DrawButton(&g->btnPlay, ButtonHovered(&g->btnPlay, mouse));
    DrawButton(&g->btnHowto, ButtonHovered(&g->btnHowto, mouse));
    DrawButton(&g->btnExit, ButtonHovered(&g->btnExit, mouse));
}

static void DrawHowto(const Game *g, Vector2 mouse) {
    DrawTextCentered("HOW TO PLAY", GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.18f), 40, COLOR_CYAN);

    const char *lines[] = {
        "Move with WASD or Arrow Keys.",
        "Dodge the red neon blocks.",
        "Blocks speed up as you survive.",
        "Survive as long as possible.",
        "Press P or ESC to pause.",
        "Reach the highest score!"
    };
    int count = sizeof(lines) / sizeof(lines[0]);
    int startY = (int)(GetScreenHeight() * 0.32f);
    for (int i = 0; i < count; i++) {
        DrawTextCentered(lines[i], GetScreenWidth() / 2, startY + i * 44, 22, COLOR_DIM);
    }

    DrawButton(&g->btnBack, ButtonHovered(&g->btnBack, mouse));
}

static void DrawPlaying(const Game *g) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (g->particles[i].active) DrawParticle(&g->particles[i]);
    }
    DrawPlayer(&g->player);
    for (int i = 0; i < g->obstacleCount; i++) {
        if (g->obstacles[i].active) DrawObstacle(&g->obstacles[i]);
    }

    char scoreBuf[64];
    snprintf(scoreBuf, sizeof(scoreBuf), "SCORE %d", (int)g->score);
    DrawText(scoreBuf, 30, 30, 28, COLOR_CYAN);

    char highBuf[64];
    snprintf(highBuf, sizeof(highBuf), "BEST %d", g->highScore);
    DrawText(highBuf, 30, 70, 24, COLOR_PURPLE);

    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "TIME %.1fs", g->survivalTime);
    int tw = MeasureText(timeBuf, 24);
    DrawText(timeBuf, GetScreenWidth() - 30 - tw, 30, 24, COLOR_DIM);
}

static void DrawPauseOverlay(const Game *g, Vector2 mouse) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 120 });
    DrawTextCentered("PAUSED", GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.42f), 44, COLOR_CYAN);
    DrawButton(&g->btnResume, ButtonHovered(&g->btnResume, mouse));
}

static void DrawGameOver(const Game *g, Vector2 mouse) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 150 });

    DrawTextCentered("GAME OVER", GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.30f), 52, COLOR_RED);

    char finalBuf[64];
    snprintf(finalBuf, sizeof(finalBuf), "FINAL SCORE %d", (int)g->score);
    DrawTextCentered(finalBuf, GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.40f), 28, COLOR_WHITE);

    char bestBuf[64];
    snprintf(bestBuf, sizeof(bestBuf), "BEST SCORE %d", g->highScore);
    DrawTextCentered(bestBuf, GetScreenWidth() / 2, (int)(GetScreenHeight() * 0.46f), 24, COLOR_PURPLE);

    DrawButton(&g->btnRetry, ButtonHovered(&g->btnRetry, mouse));
    DrawButton(&g->btnMenu, ButtonHovered(&g->btnMenu, mouse));
}

static void DrawGame(const Game *g, Vector2 mouse) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    switch (g->state) {
        case STATE_MENU:   DrawMenu(g, mouse); break;
        case STATE_HOWTO:  DrawHowto(g, mouse); break;
        case STATE_PLAY:   DrawPlaying(g); break;
        case STATE_PAUSE:  DrawPlaying(g); DrawPauseOverlay(g, mouse); break;
        case STATE_GAMEOVER: DrawGameOver(g, mouse); break;
    }

    DrawTransition(g);
    EndDrawing();
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NEON DODGE");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    Game g;
    InitGame(&g);

    bool requestExit = false;

    while (!WindowShouldClose() && !requestExit) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        Vector2 mouse = GetMousePosition();

        if (g.state == STATE_MENU) {
            UpdateMenu(&g, mouse);
            if (ButtonHovered(&g.btnExit, mouse) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                PlayClick(&g);
                requestExit = true;
            }
        } else if (g.state == STATE_PLAY) {
            UpdatePlaying(&g, dt);
        } else if (g.state == STATE_PAUSE) {
            UpdatePause(&g, mouse);
        } else if (g.state == STATE_GAMEOVER) {
            UpdateGameOver(&g, mouse);
        } else if (g.state == STATE_HOWTO) {
            UpdateHowto(&g, mouse);
        }

        UpdateMusic(&g);
        UpdateParticles(&g, dt);
        UpdateTransition(&g, dt);
        DrawGame(&g, mouse);
    }

    StopMusic(&g);
    CloseAudioAssets(&g);
    CloseWindow();
    return 0;
}
