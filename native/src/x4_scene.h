#ifndef X4_SCENE_H
#define X4_SCENE_H
#include <stdint.h>
/* Native integration slice. Terrain solver/animation/shot routines are from
 * sozud/mmx4, including the original player state controller. */
enum { X4_RIGHT=1,X4_LEFT=2,X4_SHOOT=16,X4_JUMP=128,X4_DASH=256 };
typedef struct {int x,y,pose,animation,facing,contacts,shots,deaths,shot_sound;} X4SceneState;
typedef struct {int active,x,y,pose,facing;} X4ShotState;
void x4_scene_init(void* graphics,void* assembly,void* palette_bank,void* palette,void* layout,void* screens,
                   void* attributes,unsigned width,unsigned height,int x,int y);
void x4_scene_step(unsigned held,unsigned pressed);
void x4_scene_state(X4SceneState* state);
void x4_scene_shot(unsigned i,X4ShotState* state);
void x4_scene_camera(int x,int y);
#endif
