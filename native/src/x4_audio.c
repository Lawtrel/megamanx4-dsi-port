#include "x4_audio.h"
#include "x4_music.h"
#include "x4_sfx.h"
#include "x4_n05_platform.h"
#include "x4_movie.h"
#include <string.h>
static X4Music music;static char root[1024];static unsigned errors;static int track=-1;
void x4_audio_close(void){x4_music_close(&music);x4_sfx_close();track=-1;}
void x4_audio_init(const char* music_root,const char* sfx_root){
 x4_audio_close();errors=0;root[0]=0;if(music_root&&strlen(music_root)<sizeof root)strcpy(root,music_root);x4_sfx_config(sfx_root);
}
unsigned x4_audio_errors(void){return errors+x4_sfx_errors();}
int x4_audio_track(void){return track;}
void x4_audio_update(void){
 int selection;unsigned gain;
 if(x4_n05_music_request(&selection,&gain)){
  if(selection<0){x4_music_close(&music);track=-1;x4_n05_music_status(0);}
  else if(x4_music_open(&music,root,selection,gain)){++errors;x4_music_close(&music);track=-1;x4_n05_music_status(0);}
  else{track=selection;x4_n05_music_status(1);}
 }
 music.gain=x4_n05_music_gain();
}
void x4_audio_read(int16_t* output,size_t frames){
 if(x4_movie_active()){x4_movie_audio(output,frames);return;}
 x4_music_read(&music,output,frames);x4_sfx_mix(output,frames);
 if(music.file&&(music.finished||music.error)){
  errors+=!!music.error;x4_music_close(&music);track=-1;x4_n05_music_status(0);
 }
}
