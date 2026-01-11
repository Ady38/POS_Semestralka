#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <pthread.h>

typedef struct {
    volatile char snake_dir;
    pthread_mutex_t dir_mutex;
    volatile int snake_paused;
    volatile int snake_resume_tick;
    pthread_mutex_t pause_mutex;
} SharedState;

#endif // SHARED_STATE_H

