#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
extern void mmx4_pc_threads_init(void);
extern struct GameThread* mmx4_pc_thread_slot(s32);
extern void mmx4_pc_thread_create(s32,void (*)(void));
extern void mmx4_pc_thread_run(s32);
extern void mmx4_pc_thread_yield(void);
extern void mmx4_pc_thread_close(void);
extern void mmx4_pc_thread_replace(void (*)(void));
static char trace[32];static unsigned length,steps;
static void record(char c){assert(length<sizeof trace-1);trace[length++]=c;}
static void task_a(void){int local=17;record('A');mmx4_pc_thread_yield();assert(local==17);record('B');mmx4_pc_thread_close();assert(0);}
static void task_b(void){int local=23;record('C');mmx4_pc_thread_yield();assert(local==23);record('D');}
static void replaced(void){record('S');}
static void replace(void){record('R');mmx4_pc_thread_replace(replaced);assert(0);}
static void z(void){record('Z');}
static void stress(void){for(unsigned i=0;i<5000;++i){++steps;mmx4_pc_thread_yield();assert(steps==i+1);}}
int main(void){
    mmx4_pc_threads_init();mmx4_pc_thread_create(0,task_a);mmx4_pc_thread_create(1,task_b);
    mmx4_pc_thread_run(0);mmx4_pc_thread_run(1);mmx4_pc_thread_run(0);mmx4_pc_thread_run(1);
    assert(!strcmp(trace,"ACBD"));assert(!mmx4_pc_thread_slot(0)->state&&!mmx4_pc_thread_slot(1)->state);
    mmx4_pc_thread_create(2,replace);mmx4_pc_thread_run(2);mmx4_pc_thread_run(2);assert(!strcmp(trace,"ACBDRS"));
    mmx4_pc_thread_create(0,task_a);mmx4_pc_thread_run(0);mmx4_pc_thread_create(0,z);mmx4_pc_thread_run(0);
    assert(!strcmp(trace,"ACBDRSAZ"));
    mmx4_pc_thread_create(3,stress);for(unsigned i=0;i<5001;++i)mmx4_pc_thread_run(3);
    assert(steps==5000&&!mmx4_pc_thread_slot(3)->state);
    mmx4_pc_threads_init();assert(!mmx4_pc_thread_slot(0)->state);mmx4_pc_thread_create(1,z);mmx4_pc_thread_run(1);
    assert(!strcmp(trace,"ACBDRSAZZ"));
    puts("PASS task interleaving, local stack, close, replace, recreate, reset, 5000 yields");
}
