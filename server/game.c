#include "game.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "shared_state.h"

// Inicializuje štruktúru Game podľa nastavení
void game_init(Game* game, const GameSettings* settings) {
    memset(game, 0, sizeof(Game));
    game->settings = *settings;
    world_init(&game->world, settings); // inicializuje mapu, prekážky, ovocie
    int dx, dy;
    snake_init(&game->snake, &game->world, &dx, &dy); // inicializuje hadíka
    game->elapsed = 0;
    game->over = 0;
    game->score = 0;
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
    if (ate_fruit) {
        world_spawn_fruit(&game->world); // ak zjedol ovocie, spawn nové
        game->score++; // pripočítaj skóre za ovocie
    }
    // Meraj čas trvania hry vždy (pre všetky režimy)
    game->elapsed++;
    if (game->settings.end_mode == END_TIMED) {
        if (game->elapsed >= game->settings.game_time_seconds) game->over = 1; // koniec po čase
    }
}

// Zistí, či je hra ukončená (náraz, vypršanie času, ...)
int game_is_over(const Game* game) {
    return game->over;
}

int game_get_score(const Game* game) {
    return game->score;
}

int game_get_elapsed(const Game* game) {
    return game->elapsed;
}

// Legacy: spustí herný cyklus pre server.c (pôvodné rozhranie)
void game_run(const GameSettings* settings, World* world, SharedState* shared) {
    Game game;
    game_init(&game, settings);
    // Skopíruj pointer na world pre spätnú kompatibilitu
    *world = game.world;
    while (!game_is_over(&game)) {
        pthread_mutex_lock(&shared->dir_mutex);
        char dir = shared->snake_dir;
        pthread_mutex_unlock(&shared->dir_mutex);
        pthread_mutex_lock(&shared->pause_mutex);
        int paused = shared->snake_paused;
        int resume_tick = shared->snake_resume_tick;
        pthread_mutex_unlock(&shared->pause_mutex);
        if (paused) {
            // Ak je pozastavené, hadík sa nehýbe
            sleep(1);
            continue;
        } else if (resume_tick > 0) {
            // Po obnovení pohybu čakaj 3 sekundy
            pthread_mutex_lock(&shared->pause_mutex);
            shared->snake_resume_tick--;
            pthread_mutex_unlock(&shared->pause_mutex);
            sleep(1);
            continue;
        }
        game_tick(&game, dir);
        *world = game.world;
        sleep(1); // sekundový tik
    }
}
