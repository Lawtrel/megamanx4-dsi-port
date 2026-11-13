#include "x4_scene_render.h"
#include "x4_graphics_decode.h"
#include "x4_sprite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned u16(const uint8_t* p){return p[0]|(unsigned)p[1]<<8;}
static unsigned u32(const uint8_t* p){return u16(p)|(u16(p+2)<<16);}
static int load(X4Blob* b,const char* root,const char* file) {
    char path[1024];snprintf(path,sizeof path,"%s/%s",root,file);
    FILE* f=fopen(path,"rb");if(!f)return 0;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;}
    long size=ftell(f);rewind(f);
    if(size<=0||size>2*1024*1024){fclose(f);return 0;}
    b->data=malloc(size);b->size=size;
    int ok=b->data&&fread(b->data,1,size,f)==(size_t)size;
    fclose(f);return ok;
}
void x4_assets_free(X4SceneAssets* a) {
    free(a->graphics.data);free(a->assembly.data);free(a->palette_bank.data);free(a->weapon_texture.data);
    free(a->layout.data);free(a->screens.data);free(a->attributes.data);free(a->tiles.data);free(a->palette.data);
    memset(a,0,sizeof *a);
}
int x4_assets_load(X4SceneAssets* a,const char* root,unsigned stage) {
    memset(a,0,sizeof *a);if(stage>1)return 0;
    if(!load(&a->graphics,root,"graphics.bin")||!load(&a->assembly,root,"assembly.bin")||
       !load(&a->palette_bank,root,"palette_bank.bin")||!load(&a->weapon_texture,root,"weapon_texture.bin"))goto fail;
    char dir[1024];snprintf(dir,sizeof dir,"%s/intro%u",root,stage);
    X4Blob info={0};if(!load(&info,dir,"info.bin"))goto fail;
    if(info.size!=16){free(info.data);goto fail;}
    a->width=u32(info.data);a->height=u32(info.data+4);a->tile_count=u32(info.data+8);a->screen_count=u32(info.data+12);
    free(info.data);a->stage=stage;
    if(!load(&a->layout,dir,"layout.bin")||!load(&a->screens,dir,"screens.bin")||
       !load(&a->attributes,dir,"attributes.bin")||!load(&a->tiles,dir,"tiles.bin")||!load(&a->palette,dir,"palette.bin"))goto fail;
    if(a->width!=32||a->height!=8||a->graphics.size!=188276||a->assembly.size!=36696||a->palette_bank.size!=2464||
       a->weapon_texture.size!=112640||a->palette.size!=4096||a->layout.size!=a->width*a->height*3||
       a->screens.size!=a->screen_count*512||a->tiles.size!=a->tile_count*512||a->attributes.size!=a->tile_count*4)goto fail;
    for(size_t i=0;i<a->layout.size;++i)if(a->layout.data[i]>=a->screen_count)goto fail;
    for(size_t i=0;i<a->screens.size;i+=2)if((u16(a->screens.data+i)&0x3fff)>=a->tile_count)goto fail;
    return 1;
fail:x4_assets_free(a);return 0;
}
static void layer(uint16_t* target,const X4SceneAssets* a,unsigned layer,int cx,int cy) {
    /* Iterate visible 16x16 tiles; map divisions stay outside pixel loops. */
    for(int ty=cy/16;ty<=(cy+191)/16;++ty)for(int tx=cx/16;tx<=(cx+255)/16;++tx) {
        if(tx<0||ty<0||tx>=(int)a->width*16||ty>=(int)a->height*16)continue;
        unsigned block=a->layout.data[layer*a->width*a->height+(ty/16)*a->width+tx/16];
        if(!block)continue;
        unsigned entry=u16(a->screens.data+block*512+((ty&15)*16+(tx&15))*2);
        unsigned tile=entry&0x3fff;if(!tile)continue;
        const uint16_t* pixels=(const uint16_t*)a->tiles.data+tile*256;
        int dx=tx*16-cx,dy=ty*16-cy;
        int x0=dx<0?-dx:0,x1=dx+16>256?256-dx:16;
        int y0=dy<0?-dy:0,y1=dy+16>192?192-dy:16;
        for(int y=y0;y<y1;++y){
            unsigned sy=(entry&0x8000)?15-y:y;
            uint16_t* row=target+(dy+y)*256+dx+x0;
            for(int x=x0;x<x1;++x){
                unsigned sx=(entry&0x4000)?15-x:x;
                uint16_t color=pixels[sy*16+sx];if(color)*row=color;++row;
            }
        }
    }
}
static int sprite(uint16_t* target,const X4SceneAssets* a,unsigned bank,unsigned pose,int x,int y,unsigned facing) {
    uint8_t decoded[4096];const uint8_t* data;size_t length;
    unsigned base=u32(a->assembly.data+bank*4),end=u32(a->assembly.data+(bank+1)*4);
    if(base+pose*4+4>end)return 0;
    unsigned count=u16(a->assembly.data+base+pose*4),off=base+u16(a->assembly.data+base+pose*4+2)*4;
    if(off+count*4>end)return 0;
    if(bank==0) {
        if(pose>=237)return 0;
        unsigned gb=u32(a->graphics.data),entry=u32(a->graphics.data+gb+pose*4),start=gb+(entry&0xfffff);
        if(start>=a->graphics.size)return 0;
        size_t used;
        if(x4_graphics_decode(a->graphics.data+start,a->graphics.size-start,decoded,sizeof decoded,&used,&length)!=X4_DECODE_OK||length!=(entry>>20)*128)return 0;
        data=decoded;
    }else {data=a->weapon_texture.data;length=a->weapon_texture.size;}
    const uint16_t* pal=(const uint16_t*)a->palette.data+(bank?32:0);
    return x4_sprite_draw(target,256,192,data,length,a->assembly.data+off,count,pal,x,y,1,facing);
}
int x4_scene_render(uint16_t* target,const X4SceneAssets* a,const X4SceneState* p,
                     const X4ShotState* shots,unsigned count,int cx,int cy) {
    for(unsigned i=0;i<256*192;++i)target[i]=0x8000;
    layer(target,a,2,cx,cy);layer(target,a,1,cx,cy);layer(target,a,0,cx,cy);
    int errors=!sprite(target,a,0,p->pose,p->x-cx,p->y-cy,p->facing);
    for(unsigned i=0;i<count;++i)if(shots[i].active)
        errors+=!sprite(target,a,1,shots[i].pose,shots[i].x-cx,shots[i].y-cy,shots[i].facing);
    return errors;
}
