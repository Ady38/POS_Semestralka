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

#define PORT 38200
#define BUFFER_SIZE 1024

// Zdieľaný smer hadíka (w/a/s/d), inicializovaný na 's' (dole)
volatile char snake_dir = 's';
pthread_mutex_t dir_mutex = PTHREAD_MUTEX_INITIALIZER;

// Argumenty pre herné vlákno
// Uchováva nastavenia, svet a socket klienta
typedef struct {
    const GameSettings* settings;
    World* world;
    int client_fd;
} GameThreadArgs;

// Herné vlákno: spúšťa hernú logiku v samostatnom vlákne
void* game_thread_func(void* arg) {
    GameThreadArgs* a = (GameThreadArgs*)arg;
    game_run(a->settings, a->world);
    return NULL;
}

// Argumenty pre komunikačné vlákno
// Uchováva socket klienta a pointer na svet
typedef struct {
    int client_fd;
    World* world;
} CommunicationThreadArgs;

// Serializuje stav sveta do textovej podoby (pre klienta)
void serialize_world(const World* world, char* buffer, size_t bufsize) {
    (void)bufsize;
    int idx = 0;
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
        // Prijímanie správ od klienta (polling každých 10 ms)
        char recvbuf[BUFFER_SIZE];
        int bytes = recv(comm->client_fd, recvbuf, BUFFER_SIZE - 1, 0);
        if (bytes > 0) {
            recvbuf[bytes] = '\0';
            char c = recvbuf[0];
            if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                pthread_mutex_lock(&dir_mutex);
                snake_dir = c;
                pthread_mutex_unlock(&dir_mutex);
            }
        } else if (bytes == 0) {
            printf("[COMM] Klient sa odpojil\n");
            break;
        }
        // Každú sekundu pošli stav sveta
        gettimeofday(&now, NULL);
        long elapsed_ms = (now.tv_sec - last_send.tv_sec) * 1000 + (now.tv_usec - last_send.tv_usec) / 1000;
        if (elapsed_ms >= 1000) {
            serialize_world(comm->world, buffer, sizeof(buffer));
            send(comm->client_fd, buffer, strlen(buffer), 0);
            last_send = now;
        }
        usleep(10000); // 10 ms
    }
    return NULL;
}

// Hlavná funkcia servera: inicializuje server, prijíma klienta, spúšťa vlákna
int main(int argc, char* argv[])
{
  // Spracovanie argumentov príkazového riadku
  if (argc < 4) {
    printf("Pouzitie: %s <size> <mode> <end_mode> [game_time]\n", argv[0]);
    printf("  size: 10-30\n  mode: 0=bez prekazok, 1=s prekazkami\n  end_mode: 0=standard, 1=casovy\n  game_time: sekundy (len pre end_mode=1)\n");
    return 1;
  }
  char* endptr;
  long size = strtol(argv[1], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny size!\n"); return 1; }
  long mode = strtol(argv[2], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny mode!\n"); return 1; }
  long end_mode = strtol(argv[3], &endptr, 10);
  if (*endptr != '\0') { printf("Neplatny end_mode!\n"); return 1; }
  long game_time = 0;
  if (end_mode == 1) {
    if (argc < 5) {
      printf("Chyba: Pri casovom rezime musite zadat aj game_time v sekundach!\n");
      return 1;
    }
    game_time = strtol(argv[4], &endptr, 10);
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
  server_addr.sin_port = htons(PORT);
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    perror("Chyba pri priradeni adresy");
    close(server_fd);
    return -2;
  }
  printf("Socket naviazany na port: %d\n", PORT);
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
  GameThreadArgs args = { .settings = &settings, .world = &world, .client_fd = client_fd };
  CommunicationThreadArgs comm_args = { .client_fd = client_fd, .world = &world };
  pthread_create(&game_thread, NULL, game_thread_func, &args);
  pthread_create(&comm_thread, NULL, communication_thread_func, &comm_args);
  pthread_join(game_thread, NULL);
  pthread_join(comm_thread, NULL);
  close(client_fd);
  close(server_fd);
  printf("Server je ukonceny\n");
  return 0;
}
