#ifndef GAME_H
#define GAME_H

#include "world.h"
#include "settings.h"
#include "snake.h"

typedef struct {
    World world;
    Snake snake;
    int elapsed;
    int over;
    GameSettings settings;
} Game;

void game_init(Game* game, const GameSettings* settings);
void game_cleanup(Game* game);
void game_tick(Game* game, char dir);
int game_is_over(const Game* game);
void game_run(const GameSettings* settings, World* world); // legacy, pre server.c

#endif // GAME_H
