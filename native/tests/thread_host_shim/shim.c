#include "calico/system/thread.h"
#include "calico/system/mutex.h"
#include "calico/system/condvar.h"
#include <assert.h>
void threadPrepare(Thread* t,ThreadFunc f,void* arg,void* top,unsigned char p){(void)top;(void)p;t->entry=f;t->arg=arg;}
void threadAttachLocalStorage(Thread* t,void* storage){(void)t;(void)storage;/* pthreads have libc TLS automatically */}
static void* start(void* ptr){Thread* t=ptr;t->entry(t->arg);return NULL;}
void threadStart(Thread* t){assert(!pthread_create(&t->thread,NULL,start,t));}
void mutexLock(Mutex* m){assert(!pthread_mutex_lock(&m->value));}
void mutexUnlock(Mutex* m){assert(!pthread_mutex_unlock(&m->value));}
void condvarSignal(CondVar* c){assert(!pthread_cond_signal(&c->value));}
void condvarWait(CondVar* c,Mutex* m){assert(!pthread_cond_wait(&c->value,&m->value));}
