#ifndef WORLD_H
#define WORLD_H

#include "settings.h"

#define MAX_WORLD_SIZE 30 // Maximálny rozmer sveta
#define MIN_WORLD_SIZE 10 // Minimálny rozmer sveta

// Typy políčok na mape (prázdne, hadík, prekážka, ovocie)
typedef enum {
    CELL_EMPTY,
    CELL_SNAKE,
    CELL_OBSTACLE,
    CELL_FOOD
} CellType;

// Štruktúra sveta (mapa)
typedef struct {
    int size; // Rozmer mapy
    CellType cells[MAX_WORLD_SIZE][MAX_WORLD_SIZE]; // Polia mapy
} World;

// Inicializuje svet podľa nastavení (rozmer, prekážky, ovocie)
void world_init(World* world, const GameSettings* settings);
// Vloží ovocie na náhodnú voľnú pozíciu
void world_spawn_fruit(World* world);
// Získa typ políčka na súradniciach (bezpečný prístup)
CellType world_get_cell(const World* world, int x, int y);
// Nastaví typ políčka na súradniciach (bezpečný prístup)
void world_set_cell(World* world, int x, int y, CellType value);

#endif // WORLD_H
