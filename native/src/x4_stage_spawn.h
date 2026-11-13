#ifndef X4_STAGE_SPAWN_H
#define X4_STAGE_SPAWN_H
/* Requires live banks installed by x4_stage_install_resources. Returns number
 * created, or -1 for invalid stage/window/direction. Does not update/draw AI. */
int x4_stage_spawn_visible(short left,short right,short top,short bottom,unsigned direction);
#endif
