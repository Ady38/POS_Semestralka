// Hlavný server pre hru Hadík. Starostlivo komentované pre obhajobu.
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "settings.h"
#include "world.h"
#include "game.h"
#include <pthread.h>
#include <stdatomic.h>
#include <fcntl.h>
#define BUFFER_SIZE 1024

// Zdieľaný smer hadíka (w/a/s/d), inicializovaný na 's' (dole)
volatile char snake_dir = 's';
pthread_mutex_t dir_mutex = PTHREAD_MUTEX_INITIALIZER;

// Príznak pozastavenia hry a časovač pre obnovenie pohybu
volatile int snake_paused = 0;
volatile int snake_resume_tick = 0;
pthread_mutex_t pause_mutex = PTHREAD_MUTEX_INITIALIZER;

// Zdieľané informácie o priebehu a konci hry medzi vláknami
typedef struct {
    int game_over_flag; // 1 ak hra skončila
    int final_score;            // finálne skóre
    int final_elapsed;          // celkový čas trvania hry
    pthread_mutex_t mutex;      // ochrana pre čítanie/zápis výsledkov
} GameSharedState;

// Argumenty pre herné vlákno
// Uchováva nastavenia, svet a socket klienta
typedef struct {
    const GameSettings* settings;
    World* world;
    int client_fd;
    GameSharedState* shared;
} GameThreadArgs;

// Herné vlákno: spúšťa hernú logiku v samostatnom vlákne
void* game_thread_func(void* arg) {
    GameThreadArgs* a = (GameThreadArgs*)arg;

    Game game;
    game_init(&game, a->settings);
    // Synchronizuj svet pre komunikačné vlákno
    *(a->world) = game.world;

    while (!game_is_over(&game)) {
        // Získaj aktuálny smer pred tickom
        char tick_dir;
        pthread_mutex_lock(&dir_mutex);
        tick_dir = snake_dir;
        pthread_mutex_unlock(&dir_mutex);

        pthread_mutex_lock(&pause_mutex);
        int paused = snake_paused;
        int resume_tick = snake_resume_tick;
        pthread_mutex_unlock(&pause_mutex);

        if (paused) {
            sleep(1);
            continue;
        }
        if (resume_tick > 0) {
            printf("[DEBUG] Waiting %d more seconds before snake moves after resume\n", resume_tick);
            pthread_mutex_lock(&pause_mutex);
            snake_resume_tick--;
            pthread_mutex_unlock(&pause_mutex);
            sleep(1);
            continue;
        }
        // Only update score and time when not paused and not resuming
        pthread_mutex_lock(&a->shared->mutex);
        a->shared->final_score = game_get_score(&game);
        a->shared->final_elapsed = game_get_elapsed(&game);
        pthread_mutex_unlock(&a->shared->mutex);
        game_tick(&game, tick_dir);
        *(a->world) = game.world;
        sleep(1);
    }

    // Po skončení hry ulož výsledky a nastav príznak
    pthread_mutex_lock(&a->shared->mutex);
    a->shared->final_score = game_get_score(&game);
    a->shared->final_elapsed = game_get_elapsed(&game);
    a->shared->game_over_flag = 1;
    pthread_mutex_unlock(&a->shared->mutex);

    game_cleanup(&game);
    return NULL;
}

// Argumenty pre komunikačné vlákno
// Uchováva socket klienta a pointer na svet
typedef struct {
    int client_fd;
    World* world;
    GameSharedState* shared;
} CommunicationThreadArgs;

// Serializuje stav sveta do textovej podoby (pre klienta)
void serialize_world(const World* world, char* buffer, size_t bufsize, int score, int elapsed) {
    int idx = 0;
    // Prvý riadok: skóre a čas
    idx += snprintf(buffer + idx, bufsize - idx, "SCORE: %d  TIME: %d\n", score, elapsed);
    for (int i = 0; i < world->size; ++i) {
        for (int j = 0; j < world->size; ++j) {
            char c = '.';
            if (world->cells[i][j] == CELL_SNAKE) c = 'O';
            else if (world->cells[i][j] == CELL_OBSTACLE) c = '#';
            else if (world->cells[i][j] == CELL_FOOD) c = '*';
            buffer[idx++] = c;
        }
        buffer[idx++] = '\n';
    }
    buffer[idx] = '\0';
}

// Komunikačné vlákno: prijíma príkazy od klienta a posiela stav sveta
void* communication_thread_func(void* arg) {
    CommunicationThreadArgs* comm = (CommunicationThreadArgs*)arg;
    char buffer[BUFFER_SIZE * 2];
    int flags = fcntl(comm->client_fd, F_GETFL, 0);
    fcntl(comm->client_fd, F_SETFL, flags | O_NONBLOCK);
    struct timeval last_send, now;
    gettimeofday(&last_send, NULL);
    while (1) {
        // Ak hra skončila, odošli informácie o konci hry a ukonči komunikáciu
        pthread_mutex_lock(&comm->shared->mutex);
        int game_over = comm->shared->game_over_flag;
        int score = comm->shared->final_score;
        int elapsed = comm->shared->final_elapsed;
        pthread_mutex_unlock(&comm->shared->mutex);
        if (game_over) {
            // Formát správy: GAMEOVER:score=<N>;time=<S>\n
            char gameover_msg[BUFFER_SIZE];
            snprintf(gameover_msg, sizeof(gameover_msg), "GAMEOVER:score=%d;time=%d\n", score, elapsed);
            send(comm->client_fd, gameover_msg, strlen(gameover_msg), 0);
            // Ukonči server po skončení hry
            exit(0);
        }

        // Prijímanie správ od klienta (polling každých 10 ms)
        char recvbuf[BUFFER_SIZE];
        int bytes = recv(comm->client_fd, recvbuf, BUFFER_SIZE - 1, 0);
        static int client_disconnected = 0;
        static time_t disconnect_time = 0;
        if (bytes > 0) {
            recvbuf[bytes] = '\0';
            char c = recvbuf[0];
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                pthread_mutex_lock(&dir_mutex);
                snake_dir = c;
                pthread_mutex_unlock(&dir_mutex);
            } else if (c == 'p' || c == 'P') {
                pthread_mutex_lock(&pause_mutex);
                if (!snake_paused) {
                    snake_paused = 1;
                } else {
                    snake_resume_tick = 3;
                    snake_paused = 0;
                }
                pthread_mutex_unlock(&pause_mutex);
            }
            client_disconnected = 0;
            disconnect_time = 0;
        } else if (bytes == 0) {
            if (!client_disconnected) {
                printf("[COMM] Klient sa odpojil\n");
                client_disconnected = 1;
                disconnect_time = time(NULL);
            } else {
                if (difftime(time(NULL), disconnect_time) >= 10) {
                    printf("[COMM] Server sa ukoncuje po 10 sekundach od odpojenia klienta.\n");
                    exit(0);
                }
            }
            // Počas čakania na timeout stále pokračuj v cykle
        }
        // Každú sekundu pošli stav sveta
        gettimeofday(&now, NULL);
        long elapsed_ms = (now.tv_sec - last_send.tv_sec) * 1000 + (now.tv_usec - last_send.tv_usec) / 1000;
        if (elapsed_ms >= 1000) {
            // Získaj aktuálne skóre a čas priamo zo sveta
            serialize_world(comm->world, buffer, sizeof(buffer), score, elapsed);
            send(comm->client_fd, buffer, strlen(buffer), 0);
            last_send = now;
        }
        // Krátka pauza, aby vlákno zbytočne nezaťažovalo CPU
        sleep(1);
    }
    return NULL;
}

// Hlavná funkcia servera: inicializuje server, prijíma klienta, spúšťa vlákna
int main(int argc, char* argv[])
{
  // Spracovanie argumentov príkazového riadku
  if (argc < 5) {
    printf("Pouzitie: %s <port> <size> <mode> <end_mode> [game_time]\n", argv[0]);
    printf("  port: port na ktorom server pocuva\n  size: 10-30\n  mode: 0=bez prekazok, 1=s prekazkami\n  end_mode: 0=standard, 1=casovy\n  game_time: sekundy (len pre end_mode=1)\n");
    return 1;
  }
  char* endptr;
  int port = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0' || port <= 0) { printf("Neplatny port!\n"); return 1; }
  long size = strtol(argv[2], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny size!\n"); return 1; }
  long mode = strtol(argv[3], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny mode!\n"); return 1; }
  long end_mode = strtol(argv[4], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny end_mode!\n"); return 1; }
  long game_time = 0;
  if (end_mode == 1) {
    if (argc < 6) {
      printf("Chyba: Pri casovom rezime musite zadat aj game_time v sekundach!\n");
      return 1;
    }
    game_time = strtol(argv[5], &endptr, 10);
    if (*endptr != '\0' || game_time <= 0) {
      printf("Chyba: game_time musi byt kladne cislo!\n");
      return 1;
    }
  }
  if(size < MIN_WORLD_SIZE || size > MAX_WORLD_SIZE || (mode != 0 && mode != 1) || (end_mode != 0 && end_mode != 1)) {
    printf("Neplatne nastavenia hry: %ld %ld %ld\n", size, mode, end_mode);
    return 2;
  }
  // Nastavenie parametrov hry
  GameSettings settings = { .size = (int)size, .mode = (GameMode)mode, .end_mode = (GameEndMode)end_mode, .game_time_seconds = (int)game_time };
  World world;
  world_init(&world, &settings);
  int server_fd;
  // Vytvorenie socketu
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) {
    perror("Chyba pri vytvarani socketu");
    return -1;
  }
  printf("Server Socket bol vytvoreny\n");
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  // Nastavenie ip adresy servera
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);
  printf("Socket naviazany na port: %d\n", port);
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Chyba pri priradeni adresy");
    close(server_fd);
    return -2;
  }
  printf("Socket naviazany na port: %d\n", port);
  if(listen(server_fd, 5) < 0) {
    perror("Listen zlyhal");
    close(server_fd);
    return -3;
  }
  printf("Cakam na pripojenie\n");
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
  if(client_fd < 0) {
    perror("Accept zlyhal");
    close(server_fd);
    return -4;
  }
  printf("Klient pripojeny %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
  // Spustenie hernej logiky v samostatnom vlákne
  pthread_t game_thread, comm_thread;

  GameSharedState shared;
  memset(&shared, 0, sizeof(shared));
  pthread_mutex_init(&shared.mutex, NULL);

  GameThreadArgs args = { .settings = &settings, .world = &world, .client_fd = client_fd, .shared = &shared };
  CommunicationThreadArgs comm_args = { .client_fd = client_fd, .world = &world, .shared = &shared };
  pthread_create(&game_thread, NULL, game_thread_func, &args);
  pthread_create(&comm_thread, NULL, communication_thread_func, &comm_args);
  pthread_join(game_thread, NULL);
  pthread_join(comm_thread, NULL);
  close(client_fd);
  close(server_fd);
  printf("Server je ukonceny\n");
  return 0;
}
