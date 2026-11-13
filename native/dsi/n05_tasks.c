/* Native cooperative game tasks using Calico ARM9 threads.
 * Implements the portable game API from sozud/mmx4; no ucontext or PS1 CPU.
 * Scheduling/timers remain driven by the original per-frame game scheduler.
 * AGPL-3.0. */
#include "x4_game_tasks.h"
#include <calico/system/thread.h>
#include <calico/system/mutex.h>
#include <calico/system/condvar.h>
#include <setjmp.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
enum {SLOTS=4,STACK_BYTES=32768,GUARD_BYTES=64};
typedef struct {
    Thread worker;
    CondVar wake;
    jmp_buf restart_point;
    void (*entry)(void);
    int permit,finished,restart;
    unsigned id;
} NativeTask;
static NativeTask tasks[SLOTS];
static struct GameThread game_tasks[SLOTS];
static unsigned char stacks[SLOTS][STACK_BYTES] __attribute__((aligned(8)));
static Mutex gate;
static CondVar scheduler_wake;
static int initialized,current=-1;
struct GameThread* D_801F8300;

static void guard(unsigned slot){
    for(unsigned i=0;i<GUARD_BYTES;++i)assert(stacks[slot][i]==0xa5);
}
static void await_permit(NativeTask* t,int notify){
    mutexLock(&gate);
    if(notify){t->finished=1;condvarSignal(&scheduler_wake);}
    while(!t->permit)condvarWait(&t->wake,&gate);
    t->permit=0;mutexUnlock(&gate);
}
static int worker(void* arg){
    NativeTask* t=arg;await_permit(t,0);
    for(;;){
        (void)setjmp(t->restart_point);
        t->restart=0;
        if(t->entry)t->entry();
        game_tasks[t->id].state=0;
        await_permit(t,1);
    }
}
void mmx4_pc_threads_init(void){
    assert(current==-1);
    memset(game_tasks,0,sizeof game_tasks);
    if(!initialized){
        for(unsigned i=0;i<SLOTS;++i){
            NativeTask* t=&tasks[i];t->id=i;
            memset(stacks[i],0xa5,GUARD_BYTES);
            threadPrepare(&t->worker,worker,t,stacks[i]+STACK_BYTES,MAIN_THREAD_PRIO);
            threadAttachLocalStorage(&t->worker,NULL);threadStart(&t->worker);
        }
        initialized=1;
    }
    for(unsigned i=0;i<SLOTS;++i){tasks[i].entry=NULL;tasks[i].restart=1;}
    D_801F8300=&game_tasks[0];
}
struct GameThread* mmx4_pc_thread_slot(int slot){
    assert((unsigned)slot<SLOTS);return &game_tasks[slot];
}
void mmx4_pc_thread_create(int slot,void (*entry)(void)){
    assert(initialized&&(unsigned)slot<SLOTS&&entry);
    tasks[slot].entry=entry;tasks[slot].restart=1;game_tasks[slot].state=2;
}
void mmx4_pc_thread_run(int slot){
    assert(initialized&&current==-1&&(unsigned)slot<SLOTS);
    NativeTask* t=&tasks[slot];
    if(!t->entry||game_tasks[slot].state==0)return;
    guard(slot);mutexLock(&gate);
    current=slot;D_801F8300=&game_tasks[slot];t->finished=0;t->permit=1;
    condvarSignal(&t->wake);
    while(!t->finished)condvarWait(&scheduler_wake,&gate);
    current=-1;mutexUnlock(&gate);guard(slot);
}
void mmx4_pc_thread_yield(void){
    assert((unsigned)current<SLOTS);NativeTask* t=&tasks[current];
    await_permit(t,1);
    if(t->restart)longjmp(t->restart_point,1);
}
void mmx4_pc_thread_close(void){
    assert((unsigned)current<SLOTS);NativeTask* t=&tasks[current];
    game_tasks[current].state=0;t->restart=1;mmx4_pc_thread_yield();abort();
}
void mmx4_pc_thread_replace(void (*entry)(void)){
    assert((unsigned)current<SLOTS&&entry);NativeTask* t=&tasks[current];
    t->entry=entry;t->restart=1;game_tasks[current].state=2;
    mmx4_pc_thread_yield();abort();
}
