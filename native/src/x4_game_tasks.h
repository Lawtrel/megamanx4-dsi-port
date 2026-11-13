#ifndef X4_GAME_TASKS_H
#define X4_GAME_TASKS_H
/* Platform boundary matching sozud/mmx4 obj.h GameThread exactly. Primitive
 * C types keep the game ABI independent of Calico's u32/s32 typedef names.
 * Include this in the platform adapter, not alongside the full game headers. */
#include <stddef.h>
struct GameThread {
    unsigned short state;short timer;
    unsigned int unk4,handle,unkC,stack,unk14[12],global_pointer;
    unsigned char pad48[0x80-0x48];
};
_Static_assert(sizeof(struct GameThread)==128,"game task ABI size");
_Static_assert(offsetof(struct GameThread,stack)==16,"game task ABI stack");
_Static_assert(offsetof(struct GameThread,global_pointer)==68,"game task ABI global pointer");
extern struct GameThread* D_801F8300;
void mmx4_pc_threads_init(void);
struct GameThread* mmx4_pc_thread_slot(int slot);
void mmx4_pc_thread_create(int slot,void (*entry)(void));
void mmx4_pc_thread_run(int slot);
void mmx4_pc_thread_yield(void);
void mmx4_pc_thread_close(void);
void mmx4_pc_thread_replace(void (*entry)(void));
#endif
