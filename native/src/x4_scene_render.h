#ifndef X4_SCENE_RENDER_H
#define X4_SCENE_RENDER_H
#include <stdint.h>
#include <stddef.h>
#include "x4_scene.h"
typedef struct { uint8_t* data;size_t size; } X4Blob;
typedef struct {
    X4Blob graphics,assembly,palette_bank,weapon_texture;
    X4Blob layout,screens,attributes,tiles,palette;
    unsigned width,height,tile_count,screen_count,stage;
} X4SceneAssets;
int x4_assets_load(X4SceneAssets* a,const char* root,unsigned stage);
void x4_assets_free(X4SceneAssets* a);
int x4_scene_render(uint16_t* target,const X4SceneAssets* a,const X4SceneState* player,
                     const X4ShotState* shots,unsigned count,int camera_x,int camera_y);
#endif
