/* Original movie calls yield while native bitmap/PCM playback advances. */
#include "common.h"
#include "main/2824.h"
#include "x4_movie.h"
#include "x4_sfx.h"
extern void func_80016F0C(void);
void func_80018000(arg_u8 movie){
 static const u16 skip[]={0x800,0x8f0,0x840,0x840,0x840,0x840,0x840,0x840,0x840,0x840,0x840};
 func_80016F0C();x4_sfx_stop(255,0);
 if(x4_movie_start((u8)movie))return;
 while(x4_movie_active()){
  func_800127C8(1);
  if(controller_input.pressed&skip[(u8)movie])x4_movie_skip();
 }
}
