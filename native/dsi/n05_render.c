/* Native drawing of original actor sprite assemblies. No PS1 GPU commands.
 * AGPL-3.0; data selection follows sozud/mmx4 get_pc_sprite_frame.
 * Flat and Gouraud QuadObj geometry is rasterized directly. */
#include "common.h"
#include "x4_archive.h"
#include "x4_graphics_decode.h"
#include "x4_n05_actors.h"
#include "x4_video.h"
#include "archive_memory.h"
#include "main/13A8C.h"
#include "main/128C4.h"
#include <stdint.h>
#include <string.h>
#ifdef __linux__
#include <stdio.h>
#endif
static const X4Archive* common_graphics;
/* Pose data is immutable until a content-bank replacement. Keep it instead
 * of decompressing X/Zero and shared saber/afterimages every draw. */
static struct {const u8* bank;unsigned pose,age;uint32_t generation,tag;size_t size;u8 data[16384];} poses[24];
static unsigned pose_age;static uint32_t pose_tag;
static int drawing_hud;
/* Private bridge switch: full engine owns all three camera transforms. */
int x4_original_camera_mode;
extern struct DialogueGlyphData D_801396C8;
extern u32 D_800F2F38[2];
extern s8 D_80173C6C[4];
static int actor_order(unsigned order){return (int)D_80173C6C[(order>>4)&3]*16+(order&7);}
static size_t window_data_size;
void x4_n05_set_common_graphics(const X4Archive* common){common_graphics=common;}
int x4_n05_install_ui(const X4Archive* common,const void* data,unsigned size){
    if(!common||!data||size<16||size>sizeof window_archive_data)return -1;
    memcpy(window_archive_data,data,size);window_data_size=size;common_graphics=common;return 0;
}
static unsigned le16(const u8* p){return p[0]|(unsigned)p[1]<<8;}
static unsigned le32(const u8* p){return le16(p)|(le16(p+2)<<16);}
static size_t available(const X4Archive* p,const X4Archive* s,const void* ptr){
    const X4Archive* banks[]={p,s};uintptr_t v=(uintptr_t)ptr;
    for(unsigned i=0;i<2;++i)for(unsigned j=0;j<banks[i]->count;++j){
        const X4ArchiveMember* m=&banks[i]->members[j];uintptr_t start=(uintptr_t)m->data;
        if(v>=start&&v-start<m->size)return m->size-(v-start);}
    uintptr_t window=(uintptr_t)window_archive_data;
    if(v>=window&&v-window<window_data_size)return window_data_size-(v-window);
    uintptr_t glyphs=(uintptr_t)&D_801396C8.count;
    size_t glyph_size=sizeof(D_801396C8)-((uintptr_t)&D_801396C8.count-(uintptr_t)&D_801396C8);
    if(v>=glyphs&&v-glyphs<glyph_size)return glyph_size-(v-glyphs);
    uintptr_t cursor=(uintptr_t)D_800F2F38;
    if(v>=cursor&&v-cursor<sizeof D_800F2F38)return sizeof D_800F2F38-(v-cursor);
    return 0;
}
static int draw(uint16_t* screen,const X4Archive* p,const X4Archive* s,
                const struct AnimatedObj* o,const u8* frames,int cx,int cy) {
    if(!o->active||!o->on_screen)return 0;
    if(x4_video_enabled){x4_video_order(drawing_hud==1?-16384:drawing_hud==2?-8193:actor_order(o->unk16));x4_video_screen_space(o->bg_offset<0||drawing_hud);}
    unsigned frame=o->animation_step.fields.frame_index;
    size_t avail=available(p,s,frames);
    if(avail<(frame+1)*4)return 1;
    unsigned count=le16(frames+frame*4),offset=le16(frames+frame*4+2)*4;
    if(offset>avail||count>(avail-offset)/4)return 1;
    const u8* raw=NULL;size_t size=0;unsigned base=o->unk40,absolute_atlas=0;uint32_t texture_tag=0;
    if(o->active&0x40){
        const X4ArchiveMember* texture=x4_archive_find(s,0x10102,0);
        if(!texture)return 1;raw=texture->data;size=texture->size;
    }else if(o->active&0x20){
        const X4ArchiveMember* texture=x4_archive_find(p,0x10206,0);
        if(!texture)return 1;raw=texture->data;size=texture->size;
    }else if(o->unk38||(o->unk40==g_Player.unk40&&g_Player.unk38)){
        /* Zero's saber reuses the player's decompressed texture for the
         * current pose; its separate assembly supplies the blade palette. */
        const u8* bank=o->unk38?o->unk38:g_Player.unk38;
        unsigned pose=o->unk38?frame:g_Player.animation_step.fields.frame_index;
        size_t n=available(p,s,bank);
        if(n<(pose+1)*4)return 1;
        unsigned entry=le32(bank+pose*4),start=entry&0xfffff;
        if(start>=n)return 1;
        unsigned slot=0,oldest=~0u;++pose_age;
        for(unsigned k=0;k<24;++k){
            if(poses[k].bank==bank&&poses[k].pose==pose&&poses[k].generation==x4_video_generation){slot=k;goto cached_pose;}
            if(poses[k].age<oldest){oldest=poses[k].age;slot=k;}
        }
        {size_t used;
         if(x4_graphics_decode(bank+start,n-start,poses[slot].data,sizeof poses[slot].data,&used,&size)!=X4_DECODE_OK||size!=(entry>>20)*128)return 1;
         poses[slot].bank=bank;poses[slot].pose=pose;poses[slot].generation=x4_video_generation;
         poses[slot].size=size;poses[slot].tag=++pose_tag;
        }
cached_pose:
        poses[slot].age=pose_age;size=poses[slot].size;raw=poses[slot].data;texture_tag=poses[slot].tag;base=0;
    }else absolute_atlas=1;
    if(size%128)return 1;
    unsigned tiles=size/128;
    if(o->bg_offset>=3)return 1;
    int ox=o->x_pos.i.hi-(o->bg_offset<0?0:background_objects[o->bg_offset].x_pos.i.hi);
    int oy=o->y_pos.i.hi-(o->bg_offset<0?0:background_objects[o->bg_offset].y_pos.i.hi);
    if(!x4_video_enabled&&x4_original_camera_mode&&o->bg_offset>=0){ox-=32;oy-=24;}
    const u16* palette=(const u16*)pc_archive_slots[5];if(!palette)return 1;
    for(unsigned i=0;i<count;++i){
        const u8* r=frames+offset+i*4;unsigned flags=r[0];
        unsigned tile=base+(r[1]|(flags&3)<<8);
        if(absolute_atlas){
            /* A few effects reuse background or object tiles by their original
             * atlas coordinates. Resolve the asset directly, without a VRAM
             * buffer, GPU commands or emulated graphics registers. */
            unsigned page=tile>>8,local=tile&255;const X4ArchiveMember* image=NULL;
            if(page>=0x15&&page<=0x1c){image=x4_archive_find(s,0x10000,0);tile=(page-0x15)*256+local;}
            else if(page==0x1d){image=x4_archive_find(p,0x10007,0);if(!image&&common_graphics)image=x4_archive_find(common_graphics,0x10007,0);tile=local;}
            else if(page==0x1e&&common_graphics){image=x4_archive_find(common_graphics,0x10008,0);tile=local;}
            else if(page==0x1f&&common_graphics){image=x4_archive_find(common_graphics,0x10001,0);tile=local;}
            else if(page>=6&&page<=15&&local<176){image=x4_archive_find(s,0x10102,0);tile=(page-6)*176+local;}
            else if(page>=5&&page<=15&&local>=176){image=x4_archive_find(p,0x10206,0);tile=(page-5)*80+local-176;}
            if(!image)return 1;raw=image->data;size=image->size;tiles=size/128;
        }
        if(tile>=tiles){
#ifdef __linux__
            fprintf(stderr,"  atlas overflow tile=%u tiles=%u base=%u image bytes=%zu piece=%u\n",tile,tiles,base,size,i);
#endif
            return 1;
        }
        unsigned clut=(o->unk42&0x8000)?0x7840:o->unk42+((flags&12)>>2);
        if(!(o->unk42&0x8000)&&(clut&0x30))clut+=0x30;
        if((clut>>6)<480)return 1;
        unsigned pi=((clut>>6)-480)*256+(clut&63)*16;
        if(pi+16>2048)return 1;
        int dx=(s8)r[2],dy=(s8)r[3];if(o->unk15)dx=-dx-16;
        unsigned row=tile/16,width=tiles-row*16;if(width>16)width=16;
        if(x4_video_enabled){
            int result=x4_video_tile(raw+row*2048+(tile%16)*8,width*8,4,palette+pi,16,
                ox+dx,oy+dy,16,16,(!!(flags&64)^!!o->unk15)|((!!(flags&128))<<1),o->active&0x10,texture_tag);
            if(result)return 1;continue;
        }
        for(unsigned y=0;y<16;++y)for(unsigned x=0;x<16;++x){
            int px=ox+dx+x,py=oy+dy+y;
            if(x4_original_camera_mode&&o->bg_offset<0){px=px*4/5;py=py*4/5;}
            if(px<0||px>=256||py<0||py>=192)continue;
            unsigned sx=(!!(flags&64)^!!o->unk15)?15-x:x,sy=(flags&128)?15-y:y;
            unsigned q=sy*(width*16)+(tile%16)*16+sx;
            unsigned index=(raw[row*2048+q/2]>>((q&1)*4))&15;
            u16 color=palette[pi+index];if(!color)continue;
            if(o->active&0x10){
                u16 old=screen[py*256+px];unsigned red=(old&31)+(color&31),green=((old>>5)&31)+((color>>5)&31),blue=((old>>10)&31)+((color>>10)&31);
                if(red>31)red=31;if(green>31)green=31;if(blue>31)blue=31;
                color=red|(green<<5)|(blue<<10);
            }
            screen[py*256+px]=color|0x8000;
        }
    }
    return 0;
}
static int checked_draw(uint16_t* screen,const X4Archive* p,const X4Archive* s,
                        const struct AnimatedObj* o,const u8* frames,int cx,int cy){
    int error=draw(screen,p,s,o,frames,cx,cy);
#ifdef __linux__
    if(error)fprintf(stderr,"Actor sprite unsupported: id=%d active=%02x state=%u pose=%u texture=%u clut=%04x\n",
                     o->id,(u8)o->active,o->state,o->animation_step.fields.frame_index,o->unk40,o->unk42);
    if(error){unsigned f=o->animation_step.fields.frame_index;size_t n=available(p,s,frames);
        fprintf(stderr,"  frames available=%zu count=%u offset=%u bg=%d palette=%p\n",n,n>=(f+1)*4?le16(frames+f*4):0,n>=(f+1)*4?le16(frames+f*4+2)*4:0,o->bg_offset,(void*)pc_archive_slots[5]);}
    if(error&&o->id==8)fprintf(stderr,"EREGION frame-offset=%zu table0=%u table8=%u index=%u base0=%u\n",(size_t)((uintptr_t)frames-(uintptr_t)SP_MENU_FRAMES),(unsigned)SP_MENU_FRAMES[0],(unsigned)SP_MENU_FRAMES[func_8002938C(8)],(unsigned)func_8002938C(8),(unsigned)(D_801406A8[0]>>7));
    if(error&&o->unk38){unsigned frame=o->animation_step.fields.frame_index;size_t n=available(p,s,o->unk38);
        fprintf(stderr,"  graphics bytes=%zu descriptor=%08x frame bytes=%zu\n",n,n>=(frame+1)*4?le32((const u8*)o->unk38+frame*4):0,available(p,s,frames));}
#endif
    return error;
}
int x4_n05_draw_actors(uint16_t* screen,const X4Archive* p,const X4Archive* s,int cx,int cy){
    int errors=0;
    for(unsigned i=0;i<COUNT(main_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&main_objects[i]),main_objects[i].sprite_frames,cx,cy);
    for(unsigned i=0;i<COUNT(shot_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&shot_objects[i]),shot_objects[i].unk3C,cx,cy);
    for(unsigned i=0;i<COUNT(item_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&item_objects[i]),item_objects[i].sprite_frames,cx,cy);
    for(unsigned i=0;i<COUNT(visual_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&visual_objects[i]),visual_objects[i].unk3C,cx,cy);
    for(unsigned i=0;i<COUNT(misc_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&misc_objects[i]),misc_objects[i].unk3C,cx,cy);
    return errors;
}
int x4_n05_draw_ui_sprite(uint16_t* screen,const X4Archive* a,const struct AnimatedObj* object,const u8* frames){
    drawing_hud=2;int result=checked_draw(screen,a,a,object,frames,0,0);drawing_hud=0;return result;
}
int x4_n05_draw_ui_overlay(uint16_t* screen,const X4Archive* a){
    int errors=0;for(unsigned i=0;i<COUNT(unk_objects);++i)errors+=checked_draw(screen,a,a,ANIMATED_OBJECT(&unk_objects[i]),unk_objects[i].unk3C,0,0);return errors;
}
extern u8 D_800F2B5C[21][12];
static int64_t edge(int ax,int ay,int bx,int by,int x,int y){return (int64_t)(x-ax)*(by-ay)-(int64_t)(y-ay)*(bx-ax);}
static void gradient_triangle(uint16_t* screen,const int* vx,const int* vy,const u8* colors,unsigned a,unsigned b,unsigned c,int semi){
    int ax=vx[a]*2,ay=vy[a]*2,bx=vx[b]*2,by=vy[b]*2,cx=vx[c]*2,cy=vy[c]*2;
    int64_t area=edge(ax,ay,bx,by,cx,cy);if(!area)return;
    if(area<0){unsigned t=b;b=c;c=t;bx=vx[b]*2;by=vy[b]*2;cx=vx[c]*2;cy=vy[c]*2;area=-area;}
    int l=vx[a],r=l,t=vy[a],bottom=t;
    unsigned ids[2]={b,c};for(unsigned j=0;j<2;++j){unsigned i=ids[j];if(vx[i]<l)l=vx[i];if(vx[i]>r)r=vx[i];if(vy[i]<t)t=vy[i];if(vy[i]>bottom)bottom=vy[i];}
    if(l<0)l=0;if(r>256)r=256;if(t<0)t=0;if(bottom>192)bottom=192;
    for(int y=t;y<bottom;++y)for(int x=l;x<r;++x){
        int64_t wa=edge(bx,by,cx,cy,x*2+1,y*2+1),wb=edge(cx,cy,ax,ay,x*2+1,y*2+1),wc=edge(ax,ay,bx,by,x*2+1,y*2+1);
        if(wa<0||wb<0||wc<0)continue;
        /* Half-open shared edge: the second triangle owns the diagonal. */
        if(a==0&&wa==0)continue;
        unsigned rgb[3];for(unsigned j=0;j<3;++j)rgb[j]=(unsigned)((wa*colors[a*3+j]+wb*colors[b*3+j]+wc*colors[c*3+j])/area)>>3;
        u16 color=rgb[0]|rgb[1]<<5|rgb[2]<<10;
        if(semi)color=((screen[y*256+x]&0x7bde)>>1)+((color&0x7bde)>>1);
        screen[y*256+x]=color|0x8000;
    }
}
static int draw_flat_quads(uint16_t* screen){
    int errors=0;
    for(unsigned i=0;i<COUNT(g_QuadObjects);++i){
        const struct QuadObj* q=&g_QuadObjects[i];if(!q->active||!q->on_screen)continue;
        if(q->bg_offset>=3){++errors;continue;}
        int ox=q->x_pos.i.hi-(q->bg_offset<0?0:background_objects[q->bg_offset].x_pos.i.hi);
        int oy=q->y_pos.i.hi-(q->bg_offset<0?0:background_objects[q->bg_offset].y_pos.i.hi);
        if(!x4_video_enabled&&x4_original_camera_mode&&q->bg_offset>=0){ox-=32;oy-=24;}
        int vx[4],vy[4];for(unsigned j=0;j<4;++j){vx[j]=ox+q->vertices[j].x.i.hi;vy[j]=oy+q->vertices[j].y.i.hi;}
        if(!x4_video_enabled&&x4_original_camera_mode&&q->bg_offset<0)for(unsigned j=0;j<4;++j){vx[j]=vx[j]*4/5;vy[j]=vy[j]*4/5;}
        if(x4_video_enabled){
            x4_video_order(actor_order(q->unk36));
            x4_video_screen_space(q->bg_offset<0);
            uint16_t colors[4];if((q->active&2)&&q->unk34>=21){++errors;continue;}
            for(unsigned j=0;j<4;++j){const u8* p=D_800F2B5C[q->unk34<21?q->unk34:0]+j*3;
                colors[j]=(q->active&2)?(p[0]>>3)|((p[1]>>3)<<5)|((p[2]>>3)<<10):q->unk34;
            }
            errors+=x4_video_quad(vx,vy,colors,q->active&0x10);continue;
        }
        if(q->active&2){
            if(q->unk34>=21){++errors;continue;}
            gradient_triangle(screen,vx,vy,D_800F2B5C[q->unk34],0,1,3,q->active&0x10);
            gradient_triangle(screen,vx,vy,D_800F2B5C[q->unk34],1,2,3,q->active&0x10);continue;
        }
        for(int y=0;y<192;++y){
            int intersections[4],count=0;
            for(unsigned j=0;j<4;++j){unsigned k=(j+1)&3;
                int a=vy[j],b=vy[k];if(a==b)continue;
                if(!((a<=y&&y<b)||(b<=y&&y<a)))continue;
                intersections[count++]=vx[j]*256+(int)(((int64_t)(2*y+1-2*a)*(vx[k]-vx[j])*128)/(b-a));
            }
            for(int a=0;a<count;++a)for(int b=a+1;b<count;++b)if(intersections[b]<intersections[a]){int t=intersections[a];intersections[a]=intersections[b];intersections[b]=t;}
            for(int j=0;j+1<count;j+=2){
                int x0=intersections[j]/256,x1=intersections[j+1]/256;if(x0<0)x0=0;if(x1>256)x1=256;
                for(int x=x0;x<x1;++x){u16 color=q->unk34&0x7fff;
                    if(q->active&0x10){u16 old=screen[y*256+x];
                        color=((old&0x7bde)>>1)+((color&0x7bde)>>1);
                    }
                    screen[y*256+x]=color|0x8000;
                }
            }
        }
    }
    return errors;
}
/* Read live map/palette banks. Palette animators and stage geometry writes
 * must affect the same data used for both graphics and terrain collision. */
static int background(uint16_t* screen,const X4Archive* stage,int cx,int cy){
    uint16_t clear=(engine_obj.stage==0&&engine_obj.substage==0)?0x9861:0x8000;
    if(x4_video_enabled)x4_video_begin(clear);else for(unsigned i=0;i<256*192;++i)screen[i]=clear;
    if(x4_original_camera_mode&&!background_objects[0].unk3&&!background_objects[1].unk3&&!background_objects[2].unk3)return 0;
    const X4ArchiveMember* image=x4_archive_find(stage,0x10000,0);
    const X4ArchiveMember* map=x4_archive_find(stage,0,0);
    const X4ArchiveMember* attributes=x4_archive_find(stage,1,0);
    if(!image||!map||!attributes||!SP_BG_TILEMAP||!SP_PALETTE)return 1;
    if(!layout_width||!layout_height)return 1;
    for(int layer=2;layer>=0;--layer){
    if(x4_original_camera_mode){
        if(!background_objects[layer].unk3)continue;
        cx=background_objects[layer].x_pos.i.hi+(x4_video_enabled?0:32);cy=background_objects[layer].y_pos.i.hi+(x4_video_enabled?0:24);
    }
    for(int ty=cy/16;ty<=(cy+(x4_video_enabled?239:191))/16;++ty)for(int tx=cx/16;tx<=(cx+(x4_video_enabled?319:255))/16;++tx){
        if(tx<0||ty<0||tx>=layout_width*16||ty>=layout_height*16)continue;
        unsigned block=SP_BG_TILEMAP[layer*layout_size+(ty/16)*layout_width+tx/16];
        if(!block)continue;
        unsigned pos=block*256+(ty&15)*16+(tx&15);
        if((pos+1)*2>map->size)return 1;
        unsigned entry=SP_BG_TILE_PIXELS[pos],tile=entry&0x3fff;if(!tile)continue;
        if((tile+1)*4>attributes->size)return 1;
        unsigned val=SP_BG_TILE_ATTRS[tile],page=(val>>24)&15,ox=(val>>16)&15,oy=(val>>20)&15,clut=(val>>8)&255;
        if(page>=12)return 1;
        int dx=tx*16-cx,dy=ty*16-cy;
        if(x4_video_enabled){
            unsigned offset,bits=page<8?4:8,pi=(clut+64)*16;
            if(page<8)offset=page*32768+oy*16*128+ox*8;
            else{unsigned px=ox*16;offset=(page&3)*65536+(px/128)*32768+oy*16*128+px%128;}
            unsigned colors=bits==4?16:256;if(pi>=2048)return 1;if(pi+colors>2048)colors=2048-pi;
            if(offset+15*128+(bits==4?8:16)>image->size)return 1;
            x4_video_screen_space(0);x4_video_order((int)((entry&0x8000)?background_objects[layer].unk4B:background_objects[layer].unk4A)*16+page);
            if(x4_video_tile(image->data+offset,128,bits,SP_PALETTE+pi,colors,dx,dy,16,16,0,entry&0x4000,0))return 1;
            continue;
        }
        int x0=dx<0?-dx:0,x1=dx+16>256?256-dx:16,y0=dy<0?-dy:0,y1=dy+16>192?192-dy:16;
        for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x){
            unsigned sx=x,sy=y,index,offset;
            if(page<8){
                offset=page*32768+(oy*16+sy)*128+ox*8+sx/2;
                if(offset>=image->size)return 1;
                index=(image->data[offset]>>(4*(sx&1)))&15;
            }else{
                unsigned px=ox*16+sx;
                offset=(page&3)*65536+(px/128)*32768+(oy*16+sy)*128+px%128;
                if(offset>=image->size)return 1;
                index=image->data[offset];
            }
            unsigned pi=(clut+64)*16+index;if(pi>=2048)return 1;
            u16 color=SP_PALETTE[pi];if(color)screen[(dy+y)*256+dx+x]=color|0x8000;
        }
    }
    }
    return 0;
}
int x4_n05_draw_scene(uint16_t* screen,const X4Archive* p,const X4Archive* s,int cx,int cy){
    int errors=background(screen,s,cx,cy);
    errors+=draw_flat_quads(screen);
    errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&g_Player),g_Player.unk3C,cx,cy);
    if(x4_original_camera_mode){
        errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&g_Entity),g_Entity.unk3C,cx,cy);
        for(unsigned i=0;i<COUNT(baz_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&baz_objects[i]),baz_objects[i].unk3C,cx,cy);
        for(int i=COUNT(foo_objects)-1;i>=0;--i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&foo_objects[i]),foo_objects[i].unk3C,cx,cy);
        errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&qux_object),qux_object.unk3C,cx,cy);
    }
    for(unsigned i=0;i<COUNT(weapon_objects);++i)errors+=checked_draw(screen,p,s,ANIMATED_OBJECT(&weapon_objects[i]),weapon_objects[i].unk3C,cx,cy);
    return errors+x4_n05_draw_actors(screen,p,s,cx,cy);
}
/* Original HUD assets and coordinates. This was missing from the old native
 * submission path: changing the viewport alone cannot restore health gauges. */
extern struct HudSpriteOrigin D_800F30F4[8];
extern struct PlayerGaugePosition D_800F3124[2];
extern u16 D_800F312C[4];
static void hud_bar(uint16_t* screen,int x,int bottom,int amount,unsigned color){
    if(amount<0)amount=0;if(amount>127)amount=127;
    if(x4_video_enabled){int vx[4]={x,x+5,x+5,x},vy[4]={bottom-amount,bottom-amount,bottom,bottom};
        uint16_t c[4]={color,color,color,color};x4_video_screen_space(1);x4_video_order(-16384);x4_video_quad(vx,vy,c,0);
    }else for(int y=(bottom-amount)*4/5;y<bottom*4/5;++y)for(int px=x*4/5;px<(x+5)*4/5;++px)
        if(px>=0&&px<256&&y>=0&&y<192)screen[y*256+px]=color|0x8000;
}
static int hud_sprite(uint16_t* screen,const X4Archive* a,unsigned slot,unsigned index){
    const u8* archive=(const u8*)SP_SPRITE_FRAMES;size_t n=available(a,a,archive);if(n<8||slot>=8)return 1;
    unsigned offset=le32(archive+4);if(offset>=n)return 1;
    struct AnimatedObj o={0};o.active=0x20;o.on_screen=1;o.bg_offset=-1;
    o.x_pos.i.hi=D_800F30F4[slot].x;o.y_pos.i.hi=D_800F30F4[slot].y;
    o.unk42=D_800F30F4[slot].clut;o.animation_step.fields.frame_index=index;
    return checked_draw(screen,a,a,&o,archive+offset,0,0);
}
int x4_n06_draw_hud(uint16_t* screen,const X4Archive* a){
    if(!engine_obj.unk1F||bar_object.state||bar_object.unk5)return 0;
    drawing_hud=1;int errors=0,hp=(u8)g_Player.hp&127;
    hud_bar(screen,D_800F3124[0].x,D_800F3124[0].bottom,hp,D_800F312C[hp<engine_obj.max_hp/3]);
    errors+=hud_sprite(screen,a,0,(engine_obj.max_hp-32)/2+0x45);
    errors+=hud_sprite(screen,a,1,engine_obj.player_lives+0x3b);
    if(engine_obj.enable_boss&&engine_obj.boss_ptr){
        hp=(u8)engine_obj.boss_ptr->hp&127;
        hud_bar(screen,D_800F3124[1].x,D_800F3124[1].bottom,hp,D_800F312C[hp<16]);
        errors+=hud_sprite(screen,a,7,(u8)engine_obj.unk25+0x57);
    }
    unsigned weapon=(u8)g_Player.weapon;
    if((!g_Player.unk2&&weapon)||(g_Player.unk2&&(g_Player.boss_flags&0x20))){
        unsigned energy=(u8)g_Player.weapon_energy[g_Player.unk2?0:weapon];
        hud_bar(screen,0x24,0x4f,energy,D_800F312C[0]);errors+=hud_sprite(screen,a,2,0x4e);
        if(!g_Player.unk2)errors+=hud_sprite(screen,a,weapon==3?6:5,weapon+0x4e);
    }
    drawing_hud=0;return errors;
}
