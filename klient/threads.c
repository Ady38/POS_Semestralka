#include "threads.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

// Vlákno pre čítanie vstupu od používateľa a odosielanie na server
// menu: pointer na Menu, kde je socket a príznak running
void* menu_input_thread(void* arg) {
    Menu* menu = (Menu*)arg;
    char buffer[BUFFER_SIZE];
    enable_raw_mode(); // zapne raw mód pre okamžité čítanie kláves
    while (1) {
        pthread_mutex_lock(&menu->lock);
        int running = menu->running;
        int paused = menu->paused;
        pthread_mutex_unlock(&menu->lock);
        if (!running) break;
        if (paused) {
            sleep(10); // Počas pauzy nečítaj vstup
            continue;
        }
        int c = getchar(); // načítaj znak
        if (c == EOF) break;
        if (c == 'p' || c == 'P') {
            pthread_mutex_lock(&menu->lock);
            menu->paused = 1;
            menu->hra_pozastavena = 1;
            pthread_mutex_unlock(&menu->lock);
            printf("\nHra pozastavena.\n");
            disable_raw_mode();
            menu_zobraz(menu); // zobraz menu po pozastavení
            enable_raw_mode();
            continue;
        }
        buffer[0] = (char)c;
        buffer[1] = '\0';
        pthread_mutex_lock(&menu->lock);
        int fd = menu->client_fd;
        pthread_mutex_unlock(&menu->lock);
        send(fd, buffer, 1, 0); // pošli znak na server
        // Kláves 'q' už neukončuje hru na klientovi; prípadné ukončenie rieši server cez GAMEOVER správu
    }
    disable_raw_mode(); // obnoví pôvodné nastavenia terminálu
    return NULL;
}

// Vlákno pre prijímanie správ zo servera a ich zobrazovanie
// menu: pointer na Menu, kde je socket a príznak running
void* menu_recv_thread(void* arg) {
    Menu* menu = (Menu*)arg;
    char buffer[BUFFER_SIZE];
    while (1) {
        pthread_mutex_lock(&menu->lock);
        int running = menu->running;
        int paused = menu->paused;
        int fd = menu->client_fd;
        pthread_mutex_unlock(&menu->lock);
        if (!running) break;
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read <= 0) {
            printf("\nServer sa odpojil\n");
            pthread_mutex_lock(&menu->lock);
            menu->running = 0;
            pthread_mutex_unlock(&menu->lock);
            break;
        }
        buffer[bytes_read] = '\0';
        // Skontroluj, či nejde o správu o konci hry
        if (strncmp(buffer, "GAMEOVER:", 9) == 0) {
            int score = 0, time = 0;
            system("clear"); // vyčistí obrazovku pred vypísaním konca hry
            sscanf(buffer, "GAMEOVER:score=%d;time=%d", &score, &time);
            printf("\n--- KONIEC HRY ---\n");
            printf("Skore: %d\n", score);
            printf("Cas hry: %d s\n", time);
            printf("Stlac lubovolne tlacidlo pre otvorenie menu\n");
            pthread_mutex_lock(&menu->lock);
            menu->running = 0;
            pthread_mutex_unlock(&menu->lock);
            break;
        }
        // Pridaj ošetrenie pre RESUME správu
        if (strncmp(buffer, "RESUME", 6) == 0) {
            pthread_mutex_lock(&menu->lock);
            menu->paused = 0;
            menu->hra_pozastavena = 0;
            pthread_mutex_unlock(&menu->lock);
            // Môžeš pridať informáciu pre užívateľa
            printf("\nHra pokračuje!\n");
            continue;
        }
        int local_paused = 0;
        pthread_mutex_lock(&menu->lock);
        local_paused = menu->paused;
        pthread_mutex_unlock(&menu->lock);
        if (!local_paused) {
            system("clear"); // vyčistí obrazovku na Linux
            printf("\r%s\n", buffer); // vypíše stav sveta
        }
        // Ak je pauza, správy sa prijímajú, ale nevypisujú
    }
    return NULL;
}
