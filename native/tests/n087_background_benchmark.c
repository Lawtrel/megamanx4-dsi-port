/* Synthetic live stage banks exercise the actual background renderer and
 * tile cache. No retail assets. Capture complete commands and decoded pixels. */
#include "common.h"
#include "x4_archive.h"
#include "x4_video.h"
#include <string.h>
#ifdef __linux__
#include <stdio.h>
#endif
struct EngineObj engine_obj;
struct BackgroundObj background_objects[3];
u8 layout_width,layout_height;u16 layout_size;
u8* pc_archive_slots[22];u8* mmx4_sp_background_cursor;
static u8 layout[4*3*3],image[262144];
static u16 tile_pixels[4*256],palette[2048];static u32 attributes[256];
static X4Archive archive;
u8 result[2*1024*1024];unsigned result_size;
extern int x4_original_camera_mode;
int x4_background_probe(uint16_t*,const X4Archive*,int,int);
__attribute__((noinline,noipa)) void benchmark_start(void){__asm__ volatile("":::"memory");}
__attribute__((noinline,noipa)) void benchmark_end(void){__asm__ volatile("":::"memory");}
static void append(const void* p,unsigned size){memcpy(result+result_size,p,size);result_size+=size;}
unsigned unit_entry(unsigned mode){
 unsigned random=0x7c31a9d5;
 for(unsigned i=0;i<sizeof image;++i){random=random*1664525+1013904223;image[i]=random>>24;}
 for(unsigned i=0;i<2048;++i)palette[i]=i%17?0x8000|(i*31&32767):0;
 for(unsigned i=0;i<sizeof layout;++i)layout[i]=1+i%3;
 for(unsigned i=0;i<4*256;++i)tile_pixels[i]=(1+i%127)|((i&1)?0x8000:0)|((i%7==0)?0x4000:0);
 for(unsigned i=1;i<256;++i){unsigned page=i%12;
  attributes[i]=(page<<24)|((i%16)<<16)|((i%13)<<20)|((i%12)<<8);
 }
 layout_width=4;layout_height=3;layout_size=12;
 pc_archive_slots[0]=(u8*)tile_pixels;pc_archive_slots[1]=(u8*)attributes;
 pc_archive_slots[5]=(u8*)palette;mmx4_sp_background_cursor=layout;
 archive.count=3;archive.members[0]=(X4ArchiveMember){0x10000,image,sizeof image};
 archive.members[1]=(X4ArchiveMember){0,(u8*)tile_pixels,sizeof tile_pixels};
 archive.members[2]=(X4ArchiveMember){1,(u8*)attributes,sizeof attributes};
 engine_obj.stage=1;engine_obj.substage=0;x4_original_camera_mode=1;
 for(unsigned i=0;i<3;++i){
  background_objects[i].unk3=1;background_objects[i].unk4A=2+i;background_objects[i].unk4B=4+i;
  background_objects[i].unk4D=0;background_objects[i].unk4E=3;
  background_objects[i].x_pos.i.hi=270+i*7;background_objects[i].y_pos.i.hi=80+i*11;
  if(mode==1){background_objects[i].x_pos.i.hi=-48;background_objects[i].y_pos.i.hi=-40;background_objects[i].unk4D=3;background_objects[i].unk4E=0;}
  if(mode==2){background_objects[i].unk4D=1;background_objects[i].unk4E=2;background_objects[i].x_pos.i.hi=736;}
  if(mode==4)background_objects[i].unk3=0;
 }
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);
 x4_background_probe(NULL,&archive,0,0); /* Warm cache outside measurement. */
 if(mode==5)archive.members[1].size=2; /* Bounds failure must be preserved. */
 for(unsigned frame=0;frame<3;++frame){
  if(mode==3){
   layout[3+frame]=frame?2:0;tile_pixels[300+frame]^=0x8001;
   attributes[3+frame]^=0x10000;palette[1050+frame]^=0x421;
  }
  benchmark_start();int error=x4_background_probe(NULL,&archive,0,0);benchmark_end();
  append(&error,sizeof error);
  unsigned count;const X4VideoCommand* commands=x4_video_commands(&count);append(&count,sizeof count);
  append(commands,count*sizeof *commands);
  for(unsigned i=0;i<count;++i){int dirty;const u16* rgba=x4_video_rgba_texture(commands[i].tile,&dirty);append(rgba,512);}
 }
 return result_size;
}
#ifdef __linux__
int main(int argc,char** argv){
 unsigned mode=argc>1?(unsigned)(argv[1][0]-'0'):0;unit_entry(mode);
 return fwrite(result,1,result_size,stdout)==result_size?0:1;
}
#endif
