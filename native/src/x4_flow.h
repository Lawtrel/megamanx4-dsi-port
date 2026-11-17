#ifndef X4_FLOW_H
#define X4_FLOW_H
#include <stdint.h>
#include "x4_archive.h"
typedef struct {
 int state,substate,stage,area,character,checkpoint,x,y,hp,player_state,locked;
 unsigned frames,draw_errors,load_errors;
} X4FlowState;
typedef struct {
 int active,character,stage,area,hp,max_hp,lives,weapon;
 unsigned weapons,armor,upgrades;
 unsigned energy[9],tanks[3];
} X4PlayerPanel;
/* Read-only snapshot; panel cannot change campaign progress or player input. */
void x4_flow_player_panel(X4PlayerPanel* out);
enum {X4_RADAR_COLUMNS=32,X4_RADAR_ROWS=18,X4_RADAR_MAX_MARKERS=80};
typedef struct {int16_t x,y;uint8_t kind;} X4RadarMarker;
typedef struct {
 int active,origin_x,origin_y,player_x,player_y;
 unsigned marker_count;
 uint8_t cells[X4_RADAR_ROWS][X4_RADAR_COLUMNS];
 X4RadarMarker markers[X4_RADAR_MAX_MARKERS];
} X4Radar;
/* Coordinates are world pixels relative to origin; one cell is 16 pixels.
 * Cells: 0 empty, 1 collision terrain. Markers: 1 enemy, 2 visible pickup. */
void x4_flow_radar(X4Radar* out);
/* Queue an acquired X weapon (0 is Buster), consumed once on the next tick.
 * Return 0 accepted, -1 invalid/unavailable, -2 gameplay selection locked. */
int x4_flow_request_weapon(unsigned weapon);
/* Explicit debug actions. Hitkill leaves each active Sigma form at one HP;
 * its next valid original hit triggers the normal death/phase transition.
 * The setting resets on every new/closed session. No save is written. */
void x4_flow_debug_sigma_hitkill(int enabled);
int x4_flow_debug_sigma_hitkill_enabled(void);
/* Discard the live session and run the original credits for its character. */
int x4_flow_debug_credits(void);

/* Full-flow integration workbench. This API does not assert all content is
 * qualified; errors remain visible until the complete game is implemented. */
int x4_flow_begin(const char* content_root,unsigned character,unsigned stage,unsigned area);
/* Stage-selection debug entry: full equipment; normal campaign progress is
 * still handled by begin/new_game/title without this override. */
int x4_flow_begin_debug(const char* content_root,unsigned character,unsigned stage,unsigned area);
int x4_flow_new_game(const char* content_root);
int x4_flow_title(const char* content_root);
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
