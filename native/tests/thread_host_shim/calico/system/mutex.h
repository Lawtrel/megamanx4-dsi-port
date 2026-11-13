#pragma once
#include <pthread.h>
typedef struct Mutex {pthread_mutex_t value;} Mutex;
void mutexLock(Mutex*);
void mutexUnlock(Mutex*);
