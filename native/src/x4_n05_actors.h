#ifndef X4_N05_ACTORS_H
#define X4_N05_ACTORS_H
#include <stdint.h>
#include "x4_archive.h"
typedef struct {unsigned enemies,enemy_shots,items,visuals;int player_hp;} X4ActorCounts;
typedef struct {unsigned count,frame,reason;int id;unsigned pose,texture,clut;int background;} X4RenderFault;
typedef struct {unsigned bounds_hits,bounds_misses,asset_hits,asset_misses;} X4RenderCacheStats;
void x4_n05_render_fault(X4RenderFault* out);
void x4_n05_render_cache_stats(X4RenderCacheStats* out);
/* Development entry into the two intro areas; full engine progression pending. */
int x4_n05_begin_intro(const X4Archive* player,const X4Archive* stage,const X4Archive* palette,unsigned area);
void x4_n05_actors_reset(void);
int x4_n05_actors_step(int camera_x,int camera_y);
void x4_n05_actors_state(X4ActorCounts* out);
/* Common graphics must remain loaded while menus/ready/dialogue use them. */
void x4_n05_set_common_graphics(const X4Archive* common);
int x4_n05_install_ui(const X4Archive* common,const void* window_data,unsigned length);
int x4_n05_draw_actors(uint16_t* screen,const X4Archive* player,const X4Archive* stage,int camera_x,int camera_y);
int x4_n05_draw_scene(uint16_t* screen,const X4Archive* player,const X4Archive* stage,int camera_x,int camera_y);
void x4_n05_background_wrap(void);
void x4_n05_render_counts(unsigned backgrounds[3],unsigned* armor);
#endif
