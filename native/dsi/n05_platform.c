/* Native high-level services for the N05 development integration.
 * No CD/SPU devices, BIOS or CPU interpreter. Incomplete audio is reported,
 * never advertised as a complete implementation of the original soundscape. */
#include "common.h"
#include "main/3D88.h"
#include "x4_content.h"
#include "x4_n05_platform.h"
#include <string.h>
#ifdef X4_FULL_FLOW
#include "x4_sfx.h"
#include "main/55C4.h"
#include "main/323C.h"
extern s32 D_80139534;
#endif
static char root_path[1024];
static X4Content music_bank;
static int failure,selection,pending;
static unsigned volume,missing_sfx;
void x4_n05_platform_close(void){x4_content_close(&music_bank);
#ifdef X4_FULL_FLOW
 x4_sfx_close();
#endif
}
void x4_n05_platform_init(const char* root){
    x4_n05_platform_close();failure=0;missing_sfx=0;pending=0;volume=128;
    if(!root||strlen(root)>=sizeof root_path){failure=-100;root_path[0]=0;return;}
    strcpy(root_path,root);
}
int x4_n05_platform_error(void){return failure;}
unsigned x4_n05_missing_sfx(void){return missing_sfx;}
int x4_n05_music_request(int* out,unsigned* gain){
    if(!pending)return 0;*out=selection;*gain=volume;pending=0;return 1;
}
s32 func_8001540C(s32 group,arg_u8 index,void* source){
#ifdef X4_FULL_FLOW
    (void)source;int result=x4_sfx_play(group&127,index);
    if(result==-6)++missing_sfx;return result;
#else
    (void)group;(void)index;(void)source;++missing_sfx;return -1;
#endif
}
void stop_sound(u8 group,u8 index){
#ifdef X4_FULL_FLOW
 x4_sfx_stop(group,index);
#else
 (void)group;(void)index;
#endif
}
/* No effect has been accepted for playback by func_8001540C above. Thus
 * there is no active effect to wait for. This does not claim it was played. */
s32 is_sound_finished(s32 index,struct MainObj* owner){(void)owner;
#ifdef X4_FULL_FLOW
 return x4_sfx_finished(2,index&255);
#else
 (void)index;return 1;
#endif
}
void func_8001663C(u8 track,u8 gain){selection=track;volume=gain;pending=1;
#ifdef X4_FULL_FLOW
 D_80173C84=1;D_80141BD4=2;D_80139530=7;D_80141BD0=0;D_80139528=gain;
#endif
}
void func_80016F0C(void){selection=-1;pending=1;D_80173C84=0;D_80141BD4=0;D_80139530=0;}
void func_80016448(u8 gain){volume=gain;D_80139528=gain;}
#ifdef X4_FULL_FLOW
unsigned x4_n05_music_gain(void){return volume;}
void x4_n05_music_status(int playing){D_80173C84=playing?2:0;D_80139530=playing?2:0;D_80139564=playing?2:0;D_8013955C=playing?1:0;}
/* PCM transport owns completion; fades advance on original game frames. */
s32 func_800169D8(void){
 if(D_80141BD0&&D_80173C84){if((main_bss_state.frame_counter&1)==0){
  int next=(int)volume-D_80139534;if(next<4){func_80016F0C();D_80141BD0=0;}else func_80016448(next);
 }}return D_80173C84?0:-1;
}
void func_80015284(void){
 x4_sfx_close();for(unsigned i=0;i<6;++i)D_8013E198[i]=-1;
 for(unsigned i=0;i<4;++i){D_8013E1C8[i]=-1;D_8013924C[i]=255;}
 for(unsigned i=0;i<24;++i)D_80139234[i]=-1;
}
/* Mixing is mono in this development build; this setting is retained for
 * the native stereo transport without invoking a legacy sound library. */
void func_800153D4(u8 stereo){D_80171EA9=stereo;}
void func_80015C10(void){
 extern union SepBundle D_801459C8;extern u8 D_800F1654[4];
 x4_sfx_character((u8)engine_obj.cur_character);
 /* Retain original sequence source for conversion validation. The native
  * transport uses baked command assets and does not open a PS1 SEP player. */
 func_80013890(D_800F1654[(u8)engine_obj.cur_character],D_801459C8.raw);
}
#endif
/* Stage event 04 requests a replacement sound-data ARC in mode 4. Load its
 * real data synchronously. Other legacy arena modes require the full-flow
 * adapter and fail explicitly in this development slice. */
#ifndef X4_FULL_FLOW
void func_80013AD8(s32 id,u8 mode,CdLoadAddress address){
    (void)address;
    int status=mode==4?x4_content_open(&music_bank,root_path,(unsigned)id):-101;
    if(status){failure=status;D_801406AC=0x80;return;}
    D_801406AC=2;D_8013BD40=0;
}
#endif
