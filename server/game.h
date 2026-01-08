#ifndef GAME_H
#define GAME_H

#include "world.h"
#include "settings.h"
#include "snake.h"

// Hlavná štruktúra reprezentujúca stav celej hry (svet, hadík, časovač, nastavenia)
typedef struct {
    World world;      // Herný svet (mapa, prekážky, ovocie)
    Snake snake;      // Hadík (pozície segmentov, smer, dĺžka)
    int elapsed;      // Počet uplynutých sekúnd (pre časový režim aj meranie trvania hry)
    int over;         // Príznak, či je hra ukončená (1=koniec)
    int score;        // Skóre hry (napr. dĺžka hada alebo počet zjedených ovocí)
    GameSettings settings; // Nastavenia hry
} Game;

// Inicializuje štruktúru Game podľa nastavení
void game_init(Game* game, const GameSettings* settings);
// Uvoľní zdroje (v tejto verzii nič špeciálne)
void game_cleanup(Game* game);
// Vykoná jeden herný tik (pohyb hadíka, kontrola kolízií, rast, časovač)
// dir: posledný prijatý smer pohybu ('w', 'a', 's', 'd')
void game_tick(Game* game, char dir);
// Zistí, či je hra ukončená (náraz, vypršanie času, ...)
int game_is_over(const Game* game);
// Získa finálne skóre hry
int game_get_score(const Game* game);
// Získa celkový čas trvania hry v sekundách
int game_get_elapsed(const Game* game);
// Legacy: spustí herný cyklus pre server.c (pôvodné rozhranie)
void game_run(const GameSettings* settings, World* world);

#endif // GAME_H
