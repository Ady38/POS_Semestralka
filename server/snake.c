#include "game.h"
#include "snake.h"
#include <stdio.h>
#include <string.h>

Game* g_game_ptr = NULL;

// Inicializuje hadíka na začiatku hry.
// Vyberie bezpečnú štartovaciu pozíciu a smer podľa sveta.
// Výstupné parametre out_dx, out_dy nastaví na počiatočný smer.
void snake_init(Snake* snake, const World* world, int* out_dx, int* out_dy) {
    int size = world->size;
    int start_x = size / 2, start_y = size / 2;
    int dx = 0, dy = 1; // default smer (dole)
    // Ak je stred obsadený, hľadá prvé voľné miesto a bezpečný smer
    if (out_dx && out_dy) { *out_dx = dx; *out_dy = dy; }
    if (world->cells[start_x][start_y] != CELL_EMPTY) {
        // fallback: nájdi prvé voľné miesto a smer
        int found = 0;
        for (int x = 0; x < size && !found; ++x) {
            for (int y = 0; y < size && !found; ++y) {
                if (world->cells[x][y] == CELL_EMPTY) {
                    int dirs[4][2] = { {0,1}, {1,0}, {0,-1}, {-1,0} };
                    for (int d = 0; d < 4; ++d) {
                        int nx = x + dirs[d][0];
                        int ny = y + dirs[d][1];
                        // Ak je v danom smere voľné políčko, použijeme túto pozíciu a smer
                        if (nx >= 0 && nx < size && ny >= 0 && ny < size && world->cells[nx][ny] == CELL_EMPTY) {
                            start_x = x;
                            start_y = y;
                            dx = dirs[d][0];
                            dy = dirs[d][1];
                            if (out_dx && out_dy) { *out_dx = dx; *out_dy = dy; }
                            found = 1;
                            break;
                        }
                    }
                }
            }
        }
    }
    // Nastav počiatočnú pozíciu a smer hadíka
    snake->segments[0].x = start_x;
    snake->segments[0].y = start_y;
    snake->length = 1;
    snake->dx = dx;
    snake->dy = dy;
}

// Nastaví smer pohybu hadíka podľa znaku ('w', 'a', 's', 'd')
// Zabezpečí, že ďalší pohyb bude v tomto smere
void snake_set_direction(Snake* snake, char dir) {
    if (dir == 'w') { snake->dx = -1; snake->dy = 0; } // hore
    else if (dir == 's') { snake->dx = 1; snake->dy = 0; } // dole
    else if (dir == 'a') { snake->dx = 0; snake->dy = -1; } // vľavo
    else if (dir == 'd') { snake->dx = 0; snake->dy = 1; } // vpravo
}

// Skontroluje, či by hadík narazil na daných súradniciach (kolízia so stenou, prekážkou alebo so sebou)
// Vracia 1 ak by narazil, inak 0
int snake_check_collision(const Snake* snake, const World* world, int new_x, int new_y) {
    // Kontrola hraníc sveta
    if (new_x < 0 || new_x >= world->size || new_y < 0 || new_y >= world->size)
        return 1;
    // Kolízia s prekážkou
    if (world->cells[new_x][new_y] == CELL_OBSTACLE)
        return 1;
    // Kolízia so sebou samým
    for (int i = 0; i < snake->length; ++i) {
        if (snake->segments[i].x == new_x && snake->segments[i].y == new_y)
            return 1;
    }
    return 0;
}

// Posunie hadíka o jedno políčko v aktuálnom smere.
// Ak zje ovocie, predĺži sa. Ak narazí, vráti 0.
// ate_fruit: výstupný parameter, nastaví sa na 1 ak hadík zjedol ovocie.
// Vracia 1 ak pohyb prebehol, 0 ak došlo ku kolízii.
int snake_move(Snake* snake, World* world, int* ate_fruit) {
    // Vypočítaj nové súradnice hlavy podľa smeru
    int new_x = snake->segments[0].x + snake->dx;
    int new_y = snake->segments[0].y + snake->dy;
    // Ošetrenie prechodu cez stenu (len ak nie sú prekážky)
    int cez_stenu = 1;
    if (world->size > 0 && world->cells) {
        // Ak je na mape aspoň jedna prekážka, neumožni prechod cez stenu
        cez_stenu = 1;
        for (int i = 0; i < world->size && cez_stenu; ++i)
            for (int j = 0; j < world->size && cez_stenu; ++j)
                if (world->cells[i][j] == CELL_OBSTACLE)
                    cez_stenu = 0;
        if (cez_stenu) {
            if (new_x < 0) new_x = world->size - 1;
            if (new_x >= world->size) new_x = 0;
            if (new_y < 0) new_y = world->size - 1;
            if (new_y >= world->size) new_y = 0;
        } else {
            // Ak by mal hadík naraziť do steny, kolízia
            if (new_x < 0 || new_x >= world->size || new_y < 0 || new_y >= world->size)
                return 0;
        }
    }
    // Kolízia so stenou, prekážkou alebo so sebou
    if (snake_check_collision(snake, world, new_x, new_y))
        return 0;
    // Zisti, či hadík zje ovocie
    *ate_fruit = (world->cells[new_x][new_y] == CELL_FOOD);
    // Zapamätaj si pozíciu chvosta (pre rast po zjedení ovocia)
    int old_tail_x = snake->segments[snake->length - 1].x;
    int old_tail_y = snake->segments[snake->length - 1].y;
    // Posuň segmenty tela (každý segment sa posunie na miesto predchádzajúceho)
    for (int i = snake->length - 1; i > 0; --i) {
        snake->segments[i] = snake->segments[i - 1];
    }
    // Nastav novú hlavu
    snake->segments[0].x = new_x;
    snake->segments[0].y = new_y;
    // Vyčisti všetky CELL_SNAKE v mape (pred vykreslením nových pozícií)
    for (int i = 0; i < world->size; ++i)
        for (int j = 0; j < world->size; ++j)
            if (world->cells[i][j] == CELL_SNAKE)
                world->cells[i][j] = CELL_EMPTY;
    // Nastav nové pozície tela hadíka v mape
    for (int i = 0; i < snake->length; ++i)
        world->cells[snake->segments[i].x][snake->segments[i].y] = CELL_SNAKE;
    // Ak hadík zjedol ovocie, pridaj segment na miesto pôvodného chvosta
    if (*ate_fruit) {
        if (snake->length < SNAKE_MAX_LENGTH) {
            // Nový segment sa objaví tam, kde bol chvost pred pohybom
            snake->segments[snake->length].x = old_tail_x;
            snake->segments[snake->length].y = old_tail_y;
            world->cells[old_tail_x][old_tail_y] = CELL_SNAKE;
            snake->length++;
        }
    }
    return 1;
}
