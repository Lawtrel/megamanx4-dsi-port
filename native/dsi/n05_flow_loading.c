/* Native synchronous archive loading for the original game flow.
 * The arena is an ordinary native allocation containing data banks, not PS1
 * RAM. Image/sample payloads have their own owned buffers. No CD interface. */
#include "common.h"
#include "main/3D88.h"
#include "main/323C.h"
#include "x4_content.h"
#include "x4_flow.h"
#include "x4_n05_actors.h"
#include "x4_sfx.h"
#include "x4_video.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static const char* content_root;
static struct {unsigned type;u8* data;size_t size;} images[11],samples[6];
static size_t bank_sizes[22];
static X4Archive view;
static u16 native_palette[2048];
static int error;
/* Stage add-on files contain partial sprite uploads at a later column of the
 * same logical asset. They are not a separate drawable image. Preserve and
 * patch the native packed tile bank instead of silently ignoring arg 9/10. */
static int patch_stage_graphics(unsigned type,const u8* data,size_t size){
 static const unsigned tile_offsets[11]={0,0,0,528,704,880,0,0,0,176,352};
 unsigned arg=type&255;
 if((type&0xffff00)!=0x10100||arg>=11||(arg!=2&&!tile_offsets[arg]))return 0;
 size_t offset=tile_offsets[arg]*128u,required=offset+size;
 size_t previous=images[2].type==0x10102?images[2].size:0;
 if(required<previous)required=previous;
 u8* next=calloc(1,required);if(!next)return X4_ARC_NOMEM;
 if(previous)memcpy(next,images[2].data,previous);memcpy(next+offset,data,size);
 free(images[2].data);images[2].data=next;images[2].size=required;images[2].type=0x10102;return 0;
}
extern union SepBundle D_801459C8;
static int within_arena(const void* ptr,size_t size){
 uintptr_t start=(uintptr_t)MAIN_ARCHIVE_ARENA,p=(uintptr_t)ptr;
 return p>=start&&p-start<=sizeof pc_archive_arena&&size<=sizeof pc_archive_arena-(p-start);
}
static void rebuild_view(void){
 memset(&view,0,sizeof view);
 for(unsigned i=0;i<22;++i){
  u8* p=i==12?(u8*)D_801406A8:i==14?cur_draw_info_drawenv:i==15?cur_draw_info_dispenv_screen_w:pc_archive_slots[i];
  if(p&&bank_sizes[i])view.members[view.count++]=(X4ArchiveMember){i,p,bank_sizes[i]};
 }
 for(unsigned i=0;i<11;++i)if(images[i].data)
  view.members[view.count++]=(X4ArchiveMember){images[i].type,images[i].data,images[i].size};
}
const X4Archive* x4_flow_archive(void){rebuild_view();return &view;}
int x4_flow_load_error(void){return error;}
void x4_flow_loading_init(const char* root){
 content_root=root;error=0;memset(bank_sizes,0,sizeof bank_sizes);
 memset(pc_archive_slots,0,sizeof pc_archive_slots);D_801406A8=NULL;
 D_8015D9C8=MAIN_ARCHIVE_ARENA;D_80137DC4=D_8015D9C8;D_80173C80=D_8015D9C8;
 D_801406AC=2;D_8013BD40=0;D_8013BD44=1;
}
void x4_flow_loading_close(void){
 x4_video_invalidate();
 for(unsigned i=0;i<11;++i){free(images[i].data);images[i].data=NULL;}
 for(unsigned i=0;i<6;++i){free(samples[i].data);samples[i].data=NULL;}
}
int x4_flow_load(unsigned id,unsigned mode,void* destination){
 x4_video_invalidate(); /* Old arena/image addresses may be reused by this load. */
 X4Content resource={0};int result=x4_content_open(&resource,content_root,id);
 if(result){error=result;return result;}
 if(!resource.is_container){
  /* Destinations are explicit native banks with known capacity. */
  size_t capacity=destination==WINDOW_ARCHIVE_DATA?sizeof window_archive_data:
   destination==REPLAY_DATA?sizeof replay_data:
   destination==D_801459C8.raw?sizeof D_801459C8:0;
  result=x4_content_copy(&resource,destination,capacity);
  if(!result&&destination==WINDOW_ARCHIVE_DATA)
   result=x4_n05_install_ui(&view,WINDOW_ARCHIVE_DATA,resource.archive.size);
 }else{
  u8* cursor=mode==0?MAIN_ARCHIVE_ARENA:mode==3?D_80173C80:
   mode==2?(u8*)destination:D_8015D9C8;
  if(mode>4||!within_arena(cursor,0)){result=-110;goto done;}
  size_t bytes=0;
  for(unsigned i=0;i<resource.archive.count;++i){
   const X4ArchiveMember* m=&resource.archive.members[i];
   unsigned handler=m->type>>16,arg=handler==2||handler==3?(m->type>>8)&255:m->type&255;
   if(!handler){if(arg>=22||(arg==5&&m->size>sizeof native_palette)){result=-111;goto done;}bytes+=m->size;}
   else if((handler==1&&arg>=11)||(handler==2&&arg>=6)||handler>3){result=-112;goto done;}
  }
  if(!within_arena(cursor,bytes)){result=-113;goto done;}
  /* Preallocate uploads before changing live pointers. */
  u8* uploads[128]={0};
  for(unsigned i=0;i<resource.archive.count;++i){
   const X4ArchiveMember* m=&resource.archive.members[i];
   if(m->type>>16){uploads[i]=malloc(m->size?m->size:1);
    if(!uploads[i]){for(unsigned j=0;j<i;++j)free(uploads[j]);result=X4_ARC_NOMEM;goto done;}
    memcpy(uploads[i],m->data,m->size);}
  }
  /* Reusing the arena ends the lifetime of the old tail banks. Retaining
   * their sizes would let an obsolete player bank overlap a new menu bank
   * and incorrectly truncate its sprite assemblies during bounds checks. */
  if(mode==0||mode==3)for(unsigned j=0;j<22;++j){
   u8* p=j==12?(u8*)D_801406A8:j==14?cur_draw_info_drawenv:j==15?cur_draw_info_dispenv_screen_w:pc_archive_slots[j];
   if(p&&within_arena(p,0)&&(uintptr_t)p+bank_sizes[j]>(uintptr_t)cursor)bank_sizes[j]=0;
  }
  for(unsigned i=0;i<resource.archive.count;++i){
   const X4ArchiveMember* m=&resource.archive.members[i];unsigned handler=m->type>>16,arg=handler==2||handler==3?(m->type>>8)&255:m->type&255;
   if(!handler){
    memcpy(cursor,m->data,m->size);
    if(arg==12)D_801406A8=(u32*)cursor;
    else if(arg==4||arg==6||arg==7||arg==8)D_80141F00=cursor;
    else if(arg==14)cur_draw_info_drawenv=cursor;
    else if(arg==15)cur_draw_info_dispenv_screen_w=cursor;
    else if(arg==5){memcpy(native_palette,m->data,m->size);pc_archive_slots[5]=(u8*)native_palette;}
    else pc_archive_slots[arg]=cursor;
    bank_sizes[arg]=arg==5?sizeof native_palette:m->size;cursor+=m->size;
   }else if(handler==1){
    result=patch_stage_graphics(m->type,uploads[i],m->size);if(result)goto done;
    if(m->type==0x10102){free(uploads[i]);uploads[i]=NULL;continue;}
    free(images[arg].data);images[arg].data=uploads[i];images[arg].size=m->size;images[arg].type=m->type;
   }else if(handler==2){
    free(samples[arg].data);samples[arg].data=uploads[i];samples[arg].size=m->size;samples[arg].type=m->type;
    /* Preserve original command/header bindings for the native audio adapter. */
    struct SoundArchive* h=(struct SoundArchive*)D_80141F00;
    D_80141F50[arg]=h->sound_entries;D_80141EE8[arg]=(u8*)h+h->vab_offset;
    D_8013E198[arg]=(u8)h->vab_id;
    /* Native PCM descriptors are prepared from this exact resource. Missing
     * conversion is a load error, never reported as successful playback. */
    result=x4_sfx_bank(id,arg);if(result)goto done;
   }else{free(uploads[i]);/* Sequence data is supplied separately to native audio. */}
  }
  D_8015D9C8=cursor;D_80137DC4=cursor;
 }
done:
 x4_content_close(&resource);rebuild_view();
 if(result){error=result;D_801406AC=0x80;}else{D_801406AC=2;D_8013BD40=0;}
 return result;
}
void func_80013AD8(s32 id,u8 mode,CdLoadAddress address){x4_flow_load(id,mode,(void*)(uintptr_t)address);}
void func_80013890(u32 id,u8* destination){x4_flow_load(id,2,destination);}
void func_80014C70(void){if(error)abort();D_8013BD44=1;D_80141BD2=0x78;}
void func_80014A90(s32 a,s32 b){(void)a;(void)b;func_80014C70();}
void func_80012E18(u8* start,u8* end){if(end<start||!within_arena(start,end-start))abort();memset(start,0,end-start);}
