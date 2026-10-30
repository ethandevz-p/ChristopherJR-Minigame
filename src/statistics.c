#include "statistics.h"
#include <stdio.h>
#include <string.h>

static const char *DEFAULT_PATH = "data/statistics.dat";

void statistics_init(Statistics *st) {
    memset(st, 0, sizeof(*st));
    for (int i = 0; i < 7; i++) st->distribution[i] = 0;
    st->win_percentage = 0.0;
}

bool statistics_load(Statistics *st, const char *path) {
    const char *p = path ? path : DEFAULT_PATH;
    FILE *f = fopen(p, "r");
    if (!f) return false;

    int n = fscanf(f,
        "%d %d %d %d %d %d %d %d %d %d %lf",
        &st->games_played, &st->games_won, &st->current_streak,
        &st->max_streak,
        &st->distribution[1], &st->distribution[2], &st->distribution[3],
        &st->distribution[4], &st->distribution[5], &st->distribution[6],
        &st->win_percentage);
    fclose(f);
    return n == 11;
}

bool statistics_save(const Statistics *st, const char *path) {
    const char *p = path ? path : DEFAULT_PATH;
    FILE *f = fopen(p, "w");
    if (!f) {
        fprintf(stderr, "Warning: could not save statistics to '%s'\n", p);
        return false;
    }
    fprintf(f,
        "%d %d %d %d %d %d %d %d %d %d %.6f\n",
        st->games_played, st->games_won, st->current_streak,
        st->max_streak,
        st->distribution[1], st->distribution[2], st->distribution[3],
        st->distribution[4], st->distribution[5], st->distribution[6],
        st->win_percentage);
    fclose(f);
    return true;
}

void statistics_recalc(Statistics *st) {
    if (st->games_played > 0) {
        st->win_percentage = (double)st->games_won / (double)st->games_played * 100.0;
    } else {
        st->win_percentage = 0.0;
    }
}

void statistics_record_game(Statistics *st, bool won, int guesses_used) {
    st->games_played++;
    if (won) {
        st->games_won++;
        st->current_streak++;
        if (st->current_streak > st->max_streak) st->max_streak = st->current_streak;
        if (guesses_used >= 1 && guesses_used <= 6) {
            st->distribution[guesses_used]++;
        }
    } else {
        st->current_streak = 0;
    }
    statistics_recalc(st);
}
