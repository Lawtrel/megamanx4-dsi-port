#include "x4_animation.h"
int x4_animation_start(X4Animation *a,const uint8_t *bank,size_t length,size_t offset) {
    if(offset>length || length-offset<4)return 0;
    a->position=offset;a->remaining=bank[offset];a->event=bank[offset+1];
    a->step=bank[offset+2];a->pose=bank[offset+3];return 1;
}
/* Original 0x80015DC8..0x80015E08: byte countdown, signed record stride,
 * then load all four bytes. A zero duration wraps to 255 on the first tick.
 * Events are preserved as data; gameplay event handling belongs to the actor. */
int x4_animation_tick(X4Animation *a,const uint8_t *bank,size_t length) {
    a->remaining=(uint8_t)(a->remaining-1);
    if(a->remaining)return 1;
    int delta=(int)(int8_t)a->step*4;
    size_t next;
    if(delta<0) {
        if(a->position<(size_t)-delta)return 0;
        next=a->position-(size_t)-delta;
    } else {
        if(a->position>length || (size_t)delta>length-a->position)return 0;
        next=a->position+(size_t)delta;
    }
    return x4_animation_start(a,bank,length,next);
}
