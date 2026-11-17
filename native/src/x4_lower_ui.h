#ifndef X4_LOWER_UI_H
#define X4_LOWER_UI_H
#include "x4_flow.h"
enum {X4_LOWER_SIGMA_HITKILL=-4,X4_LOWER_CREDITS=-3,X4_LOWER_OPEN_TEST=-2,X4_LOWER_PANEL=0,X4_LOWER_DEBUG,X4_LOWER_MAP,X4_LOWER_TILE_BYTES=24576};
/* Actions are returned to the frontend; this renderer never mutates game state. */
void x4_lower_reset(void);
unsigned x4_lower_mode(void);
int x4_lower_touch(unsigned x,unsigned y,const X4PlayerPanel* player);
const unsigned char* x4_lower_render(const X4PlayerPanel*,const X4Radar*,unsigned muted,const unsigned char* font);
const unsigned short* x4_lower_palette(void);
#endif
