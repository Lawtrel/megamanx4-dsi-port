/* Cooperative task transport for host preparation in a browser worker. */
#include "x4_game_tasks.h"
#include <assert.h>
#include <emscripten/fiber.h>
#include <stdlib.h>
#include <string.h>

enum {TASKS=4, STACK_BYTES=128*1024};
typedef struct {
    emscripten_fiber_t fiber;
    void (*entry)(void);
    int restart;
    int replace_current;
    unsigned char c_stack[STACK_BYTES] __attribute__((aligned(16)));
    unsigned char async_stack[STACK_BYTES] __attribute__((aligned(16)));
} BrowserTask;

static BrowserTask tasks[TASKS];
static struct GameThread slots[TASKS];
static emscripten_fiber_t scheduler;
static unsigned char scheduler_async[STACK_BYTES] __attribute__((aligned(16)));
static int current=-1;
struct GameThread* D_801F8300;

void mmx4_pc_threads_init(void){
    assert(current==-1);
    memset(slots,0,sizeof slots);
    for(unsigned i=0;i<TASKS;++i){
        tasks[i].entry=NULL;
        tasks[i].restart=1;
    }
    D_801F8300=slots;
}

struct GameThread* mmx4_pc_thread_slot(int slot){
    assert((unsigned)slot<TASKS);
    return &slots[slot];
}

void mmx4_pc_thread_create(int slot,void (*entry)(void)){
    assert((unsigned)slot<TASKS&&entry);
    tasks[slot].entry=entry;
    tasks[slot].restart=1;
    if(current==slot)tasks[slot].replace_current=1;
    slots[slot].state=2;
}

static void start(void* arg){
    BrowserTask* task=arg;
    task->entry();
    slots[current].state=0;
    task->restart=1;
    mmx4_pc_thread_yield();
    abort();
}

void mmx4_pc_thread_run(int slot){
    assert(current==-1&&(unsigned)slot<TASKS);
    BrowserTask* task=&tasks[slot];
    if(!task->entry||!slots[slot].state)return;
    if(task->restart){
        emscripten_fiber_init(&task->fiber,start,task,
                              task->c_stack,sizeof task->c_stack,
                              task->async_stack,sizeof task->async_stack);
        task->restart=task->replace_current=0;
    }
    current=slot;
    D_801F8300=&slots[slot];
    emscripten_fiber_init_from_current_context(&scheduler,scheduler_async,sizeof scheduler_async);
    emscripten_fiber_swap(&scheduler,&task->fiber);
    current=-1;
}

void mmx4_pc_thread_yield(void){
    assert((unsigned)current<TASKS);
    emscripten_fiber_swap(&tasks[current].fiber,&scheduler);
}

void mmx4_pc_thread_close(void){
    assert((unsigned)current<TASKS);
    BrowserTask* task=&tasks[current];
    slots[current].state=task->replace_current?2:0;
    task->restart=1;
    mmx4_pc_thread_yield();
    abort();
}

void mmx4_pc_thread_replace(void (*entry)(void)){
    assert((unsigned)current<TASKS&&entry);
    mmx4_pc_thread_create(current,entry);
    mmx4_pc_thread_yield();
    abort();
}
