/* HOST ONLY: render original pause sprites for the lower-screen icon atlas. */
#include "common.h"
#include "x4_flow.h"
#include "x4_video.h"
#include "x4_audio.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Layout {s16 x,y;u8 tile,character,clut,alternate;};
extern struct Layout D_800F1AD8[38];
extern int x4_n05_draw_ui_sprite(uint16_t*,const X4Archive*,const struct AnimatedObj*,const u8*);
static uint16_t screen[256*192];
int main(int argc,char** argv){
 assert(argc==3);FILE* out=fopen(argv[2],"wb");assert(out);
 for(unsigned character=0;character<2;++character){
  x4_audio_init("build/native/full-audio/music","build/native/full-audio/sfx");
  assert(!x4_flow_begin_debug(argv[1],character,1,0));x4_video_enabled=1;x4_video_set_indexed(1);
  for(unsigned i=0;i<600;++i){assert(!x4_flow_step(0,0));assert(!x4_flow_draw(screen));}
  assert(!x4_flow_step(1u<<16,1u<<16));
  for(unsigned i=0;i<180;++i){assert(!x4_flow_step(0,0));assert(!x4_flow_draw(screen));}
  for(unsigned index=0;index<54;++index){
   struct Layout l=D_800F1AD8[index%38];struct AnimatedObj o={0};o.active=o.on_screen=1;o.bg_offset=-1;
   o.x_pos.i.hi=64;o.y_pos.i.hi=64;o.animation_step.fields.frame_index=l.tile;o.unk40=0x1d60;
   unsigned clut=l.alternate;o.unk42=(clut&15)|(((clut>>4)+480)<<6);
   const u8* frames=SP_TITLE_FRAMES;
   if(index>=38&&index<49){static const unsigned poses[]={0x1b,1,0x22,0xa,0x60,0x3d,0x2e,0x49,0x63,0x6b,0x73};
    o.active=0x21;o.unk40=0;o.unk42=index==42||index==48?0x780b:0x7807;
    o.animation_step.fields.frame_index=poses[index-38];frames=(const u8*)SP_ARCHIVE_ENTRY(SP_SPRITE_FRAMES,3);
   }else if(index==49){o=*(struct AnimatedObj*)&g_Player;o.x_pos.i.hi=o.y_pos.i.hi=64;o.bg_offset=-1;frames=g_Player.unk3C;}
   else if(index>=50){o=*(struct AnimatedObj*)&g_Player;o.x_pos.i.hi=o.y_pos.i.hi=64;o.bg_offset=-1;
    o.unk42=index==52&&g_Player.arm_type==2?0x7844:0x7843;
    frames=character?g_Player.unk3C:(const u8*)SP_SPRITE_FRAMES+SP_SPRITE_FRAMES[index-50+5];}
   x4_video_begin(0);assert(!x4_n05_draw_ui_sprite(screen,x4_flow_archive(),&o,frames));
   uint16_t image[128*128]={0};unsigned count;const X4VideoCommand* c=x4_video_commands(&count);
   for(unsigned j=0;j<count;++j){uint16_t tile[256];x4_video_command_rgba(c+j,tile);
    for(unsigned y=0;y<c[j].h;++y)for(unsigned x=0;x<c[j].w;++x){int dx=c[j].x[0]+x,dy=c[j].y[0]+y;
     unsigned sx=c[j].flip&1?c[j].w-1-x:x,sy=c[j].flip&2?c[j].h-1-y:y;
     if(dx>=0&&dx<128&&dy>=0&&dy<128&&(tile[sy*16+sx]&0x8000))image[dy*128+dx]=tile[sy*16+sx];
    }
   }
   assert(fwrite(image,sizeof image,1,out)==1);
  }
  x4_flow_close();x4_audio_close();
 }
 assert(!fclose(out));return 0;
}
