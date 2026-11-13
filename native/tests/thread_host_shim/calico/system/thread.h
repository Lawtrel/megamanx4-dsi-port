#pragma once
/* Host-only Calico test transport. Pthreads provide their own stack and libc
 * TLS. This tests handoff logic, not the DSi context switch implementation. */
#include <pthread.h>
#include <stddef.h>
typedef int (*ThreadFunc)(void*);
typedef struct Thread {pthread_t thread;ThreadFunc entry;void* arg;} Thread;
#define MAIN_THREAD_PRIO 28
void threadPrepare(Thread*,ThreadFunc,void*,void*,unsigned char);
void threadAttachLocalStorage(Thread*,void*);
void threadStart(Thread*);
