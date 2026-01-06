#ifndef SNAKE_H
#define SNAKE_H

#include "world.h"

#define SNAKE_MAX_LENGTH 100 // Maximálna dĺžka hadíka

// Jeden segment tela hadíka (súradnice na mape)
typedef struct {
    int x, y;
} SnakeSegment;

// Štruktúra reprezentujúca hadíka (pole segmentov, dĺžka, smer)
typedef struct {
    SnakeSegment segments[SNAKE_MAX_LENGTH]; // segmenty tela
    int length; // aktuálna dĺžka
    int dx, dy; // posledný smer pohybu
} Snake;

// Inicializuje hadíka na začiatku hry podľa sveta (nájde bezpečnú pozíciu a smer)
void snake_init(Snake* snake, const World* world, int* out_dx, int* out_dy);
// Nastaví smer pohybu podľa znaku ('w', 'a', 's', 'd')
void snake_set_direction(Snake* snake, char dir);
// Posunie hadíka o jedno políčko, vráti 1 ak pohyb prebehol, 0 ak narazil
// ate_fruit: výstupný parameter, nastaví sa na 1 ak hadík zjedol ovocie
int snake_move(Snake* snake, World* world, int* ate_fruit);
// Skontroluje, či by hadík narazil na daných súradniciach (1=kolízia, 0=bezpečné)
int snake_check_collision(const Snake* snake, const World* world, int new_x, int new_y);

#endif // SNAKE_H
