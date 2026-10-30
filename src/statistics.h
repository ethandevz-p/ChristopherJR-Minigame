#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdbool.h>

#define MAX_STREAK 999

typedef struct {
    int games_played;
    int games_won;
    int current_streak;
    int max_streak;
    int distribution[7]; /* index 1..6 = guesses used */
    double win_percentage;
} Statistics;

void statistics_init(Statistics *st);
bool statistics_load(Statistics *st, const char *path);
bool statistics_save(const Statistics *st, const char *path);
void statistics_record_game(Statistics *st, bool won, int guesses_used);
void statistics_recalc(Statistics *st);

#endif /* STATISTICS_H */
