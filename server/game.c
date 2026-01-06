#include "game.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

extern volatile char snake_dir;
extern pthread_mutex_t dir_mutex;

void game_init(Game* game, const GameSettings* settings) {
    memset(game, 0, sizeof(Game));
    game->settings = *settings;
    world_init(&game->world, settings);
    int dx, dy;
    snake_init(&game->snake, &game->world, &dx, &dy);
    game->elapsed = 0;
    game->over = 0;
}

void game_cleanup(Game* game) {
    // nič špeciálne
}

void game_tick(Game* game, char dir) {
    if (game->over) return;
    if (dir) snake_set_direction(&game->snake, dir);
    int ate_fruit = 0;
    if (!snake_move(&game->snake, &game->world, &ate_fruit)) {
        game->over = 1;
        return;
    }
    if (ate_fruit) world_spawn_fruit(&game->world);
    if (game->settings.end_mode == END_TIMED) {
        game->elapsed++;
        if (game->elapsed >= game->settings.game_time_seconds) game->over = 1;
    }
}

int game_is_over(const Game* game) {
    return game->over;
}

// legacy run pre server.c
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
        sleep(1);
    }
    game_cleanup(&game);
}
