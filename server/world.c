#include "world.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Pomocná funkcia: zistí, či je možné umiestniť prekážku na dané miesto (v okolí nesmie byť iná prekážka)
static int is_valid_obstacle(const World* world, int x, int y) {
    int dx[] = {-1, 0, 1};
    int dy[] = {-1, 0, 1};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            int nx = x + dx[i];
            int ny = y + dy[j];
            // Kontrola hraníc
            if (nx < 0 || ny < 0 || nx >= world->size || ny >= world->size) continue;
            if (world->cells[nx][ny] == CELL_OBSTACLE) return 0;
        }
    }
    return 1;
}

// (Nepoužíva sa priamo, hadík sa spawnuje v game/snake)
void world_spawn_snake(World* world) {
    int mid = world->size / 2;
    world->cells[mid][mid] = CELL_SNAKE;
}

// Vloží ovocie na náhodnú voľnú pozíciu
void world_spawn_fruit(World* world) {
    srand((unsigned)time(NULL));
    int max_attempts = 1000;
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        int x = rand() % world->size;
        int y = rand() % world->size;
        if (world->cells[x][y] == CELL_EMPTY) {
            world->cells[x][y] = CELL_FOOD;
            return;
        }
    }
    // Ak sa nenašlo miesto, ovocie sa nespawnuje
}

// Inicializuje svet podľa nastavení (rozmer, prekážky, ovocie)
void world_init(World* world, const GameSettings* settings) {
    world->size = settings->size;
    memset(world->cells, 0, sizeof(world->cells));
    // Ak je režim s prekážkami, náhodne rozmiestni prekážky
    if (settings->mode == MODE_OBSTACLES) {
        srand((unsigned)time(NULL));
        int num_obstacles = settings->size / 2;
        int placed = 0;
        while (placed < num_obstacles) {
            int x = rand() % settings->size;
            int y = rand() % settings->size;
            if (world->cells[x][y] == CELL_EMPTY && is_valid_obstacle(world, x, y)) {
                world->cells[x][y] = CELL_OBSTACLE;
                placed++;
            }
        }
    }
    // world_spawn_snake(world); // Odstránené, hadík sa spawnuje v game_run/snake_init
    world_spawn_fruit(world);
}

// Bezpečný getter pre typ políčka
CellType world_get_cell(const World* world, int x, int y) {
    if (x < 0 || y < 0 || x >= world->size || y >= world->size) return CELL_OBSTACLE;
    return world->cells[x][y];
}

// Bezpečný setter pre typ políčka
void world_set_cell(World* world, int x, int y, CellType value) {
    if (x < 0 || y < 0 || x >= world->size || y >= world->size) return;
    world->cells[x][y] = value;
}
