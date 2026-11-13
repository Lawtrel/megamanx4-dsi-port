#ifndef X4_ANIMATION_H
#define X4_ANIMATION_H
#include <stdint.h>
#include <stddef.h>
typedef struct {size_t position;uint8_t remaining,event,step,pose;} X4Animation;
int x4_animation_start(X4Animation *a,const uint8_t *bank,size_t length,size_t offset);
int x4_animation_tick(X4Animation *a,const uint8_t *bank,size_t length);
#endif
