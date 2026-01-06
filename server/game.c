#include "game.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// Globálne zdieľané premenné pre smer hadíka (ovládané z komunikačného vlákna)
extern volatile char snake_dir;
extern pthread_mutex_t dir_mutex;

// Inicializuje štruktúru Game podľa nastavení
void game_init(Game* game, const GameSettings* settings) {
    memset(game, 0, sizeof(Game));
    game->settings = *settings;
    world_init(&game->world, settings); // inicializuje mapu, prekážky, ovocie
    int dx, dy;
    snake_init(&game->snake, &game->world, &dx, &dy); // inicializuje hadíka
    game->elapsed = 0;
    game->over = 0;
}

// Uvoľní zdroje (v tejto verzii nič špeciálne)
void game_cleanup(Game* game) {
    // nič špeciálne
}

// Vykoná jeden herný tik (pohyb hadíka, kontrola kolízií, rast, časovač)
// dir: posledný prijatý smer pohybu ('w', 'a', 's', 'd')
void game_tick(Game* game, char dir) {
    if (game->over) return;
    if (dir) snake_set_direction(&game->snake, dir); // nastav smer podľa vstupu
    int ate_fruit = 0;
    if (!snake_move(&game->snake, &game->world, &ate_fruit)) {
        game->over = 1; // kolízia, koniec hry
        return;
    }
    if (ate_fruit) world_spawn_fruit(&game->world); // ak zjedol ovocie, spawn nové
    if (game->settings.end_mode == END_TIMED) {
        game->elapsed++;
        if (game->elapsed >= game->settings.game_time_seconds) game->over = 1; // koniec po čase
    }
}

// Zistí, či je hra ukončená (náraz, vypršanie času, ...)
int game_is_over(const Game* game) {
    return game->over;
}

// Legacy: spustí herný cyklus pre server.c (pôvodné rozhranie)
void game_run(const GameSettings* settings, World* world) {
    Game game;
    game_init(&game, settings);
    // Skopíruj pointer na world pre spätnú kompatibilitu
    *world = game.world;
    while (!game_is_over(&game)) {
        pthread_mutex_lock(&dir_mutex);
        char dir = snake_dir;
        pthread_mutex_unlock(&dir_mutex);
        game_tick(&game, dir);
        *world = game.world;
        sleep(1); // sekundový tik
    }
    game_cleanup(&game);
}
