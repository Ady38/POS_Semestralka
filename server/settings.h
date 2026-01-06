#ifndef SETTINGS_H
#define SETTINGS_H

// Herné režimy: bez prekážok alebo s prekážkami
// MODE_NO_OBSTACLES: hadík môže prechádzať cez steny
// MODE_OBSTACLES: hadík nemôže prejsť cez stenu, na mape sú prekážky
typedef enum {
    MODE_NO_OBSTACLES = 0,
    MODE_OBSTACLES = 1
} GameMode;

// Spôsoby ukončenia hry
// END_STANDARD: hra končí po 10 sekundách bez hráča
// END_TIMED: hra končí po uplynutí nastaveného času
typedef enum {
    END_STANDARD = 0, // hra končí po 10s bez hráča
    END_TIMED = 1     // hra končí po uplynutí času
} GameEndMode;

// Nastavenia hry (veľkosť sveta, režim, spôsob ukončenia, čas hry)
typedef struct {
    int size; // Rozmer sveta (10-30)
    GameMode mode; // Herný režim
    GameEndMode end_mode; // Spôsob ukončenia hry
    int game_time_seconds; // len pre END_TIMED, inak 0
} GameSettings;

#endif // SETTINGS_H
