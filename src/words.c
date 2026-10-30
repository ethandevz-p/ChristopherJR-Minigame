#include "words.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char s_answers[MAX_ANSWER_WORDS][WORD_LENGTH + 1];
static int s_answer_count = 0;

static char s_valid[MAX_VALID_WORDS][WORD_LENGTH + 1];
static int s_valid_count = 0;

static char s_target[WORD_LENGTH + 1] = {0};

static bool load_words_file(const char *path, char words[][WORD_LENGTH + 1], int *count, int max) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Warning: could not open word list '%s'\n", path);
        return false;
    }

    char line[64];
    *count = 0;
    while (fgets(line, sizeof(line), f) && *count < max) {
        /* Strip whitespace/newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' ||
                           line[len - 1] == ' ' || line[len - 1] == '\t')) {
            line[--len] = '\0';
        }
        if (len == 0) continue;

        /* Uppercase and trim to WORD_LENGTH */
        for (size_t i = 0; i < len; i++) {
            char c = line[i];
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
            if (i < WORD_LENGTH) words[*count][i] = c;
        }
        words[*count][WORD_LENGTH] = '\0';

        /* Only keep exactly 5-letter words */
        if (strlen(words[*count]) == WORD_LENGTH) {
            (*count)++;
        }
    }
    fclose(f);
    return true;
}

bool words_load(const char *answers_path, const char *valid_path) {
    bool ok = true;
    if (!load_words_file(answers_path, s_answers, &s_answer_count, MAX_ANSWER_WORDS)) ok = false;
    if (!load_words_file(valid_path, s_valid, &s_valid_count, MAX_VALID_WORDS)) ok = false;

    /* Fallback: if answer list failed, build from valid list */
    if (s_answer_count == 0 && s_valid_count > 0) {
        for (int i = 0; i < s_valid_count && i < MAX_ANSWER_WORDS; i++) {
            strcpy(s_answers[i], s_valid[i]);
        }
        s_answer_count = s_valid_count;
    }

    /* Pick a random target */
    srand((unsigned int)time(NULL));
    if (s_answer_count > 0) {
        int idx = rand() % s_answer_count;
        strcpy(s_target, s_answers[idx]);
    } else {
        /* Hardcoded fallback */
        strcpy(s_target, "APPLE");
    }
    return ok;
}

void words_pick_random(char *out) {
    if (s_answer_count > 0) {
        int idx = rand() % s_answer_count;
        strcpy(out, s_answers[idx]);
    } else {
        strcpy(out, "APPLE");
    }
}

bool words_is_valid(const char *word) {
    for (int i = 0; i < s_valid_count; i++) {
        if (strcmp(word, s_valid[i]) == 0) return true;
    }
    return false;
}

const char *words_get_target(void) {
    return s_target;
}

int words_answer_count(void) { return s_answer_count; }
int words_valid_count(void) { return s_valid_count; }
