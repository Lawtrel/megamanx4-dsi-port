#ifndef X4_FLOW_H
#define X4_FLOW_H
#include <stdint.h>
#include "x4_archive.h"
typedef struct {
 int state,substate,stage,area,character,checkpoint,x,y,hp,player_state,locked;
 unsigned frames,draw_errors,load_errors;
} X4FlowState;
/* Full-flow integration workbench. This API does not assert all content is
 * qualified; errors remain visible until the complete game is implemented. */
int x4_flow_begin(const char* content_root,unsigned character,unsigned stage,unsigned area);
int x4_flow_new_game(const char* content_root);
int x4_flow_step(unsigned held,unsigned pressed);
void x4_flow_state(X4FlowState* out);
void x4_flow_debug(char* text,unsigned capacity);
int x4_flow_draw(uint16_t* screen);
void x4_flow_close(void);
int x4_flow_load(unsigned id,unsigned mode,void* destination);
const X4Archive* x4_flow_archive(void);
int x4_flow_load_error(void);
void x4_flow_copy_layout(void);
#endif
