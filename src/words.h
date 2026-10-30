#ifndef WORDS_H
#define WORDS_H

#include <stdbool.h>

#define WORD_LENGTH 5
#define MAX_ANSWER_WORDS 1024
#define MAX_VALID_WORDS 4096

/* Load word lists from disk. Returns false on failure. */
bool words_load(const char *answers_path, const char *valid_path);

/* Pick a random answer word into `out` (must be at least 6 bytes). */
void words_pick_random(char *out);

/* Check if a 5-letter word exists in the valid word list. */
bool words_is_valid(const char *word);

/* Get the current target word (null-terminated, 5 chars). */
const char *words_get_target(void);

/* Get count of loaded words for diagnostics. */
int words_answer_count(void);
int words_valid_count(void);

#endif /* WORDS_H */
