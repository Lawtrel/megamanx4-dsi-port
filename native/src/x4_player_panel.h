#ifndef X4_PLAYER_PANEL_H
#define X4_PLAYER_PANEL_H
#include "x4_flow.h"
#define X4_PANEL_ROWS 24
#define X4_PANEL_COLUMNS 31
/* Leave the final console column unused to avoid bottom-row scrolling; no escape codes in data. */
void x4_player_panel_rows(const X4PlayerPanel* state,char rows[X4_PANEL_ROWS][X4_PANEL_COLUMNS+1],unsigned muted);
#endif
