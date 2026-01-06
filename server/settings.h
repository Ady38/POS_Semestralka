#ifndef SETTINGS_H
#define SETTINGS_H

typedef enum {
    MODE_NO_OBSTACLES = 0,
    MODE_OBSTACLES = 1
} GameMode;

typedef enum {
    END_STANDARD = 0, // hra končí po 10s bez hráča
    END_TIMED = 1     // hra končí po uplynutí času
} GameEndMode;

typedef struct {
    int size; // Rozmer sveta (10-30)
    GameMode mode;
    GameEndMode end_mode;
    int game_time_seconds; // len pre END_TIMED, inak 0
} GameSettings;

#endif // SETTINGS_H
