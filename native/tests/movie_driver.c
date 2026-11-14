#include "x4_movie.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t hash(uint32_t h,const uint8_t* b,size_t n){for(size_t i=0;i<n;++i)h=(h^b[i])*16777619u;return h;}
int main(int argc,char** argv){
 assert(argc==2);uint8_t out[8];
 uint8_t literal[]={0x10,8,0,0,0,1,2,3,4,5,6,7,8};
 assert(!x4_movie_decode(literal,sizeof literal,out,sizeof out));for(unsigned i=0;i<8;++i)assert(out[i]==i+1);
 for(unsigned i=0;i<sizeof literal;++i)assert(x4_movie_decode(literal,i,out,sizeof out));
 uint8_t repeat[]={0x10,8,0,0,0x40,7,0x40,0};
 assert(!x4_movie_decode(repeat,sizeof repeat,out,sizeof out));for(unsigned i=0;i<8;++i)assert(out[i]==7);
 repeat[4]=0x80;assert(x4_movie_decode(repeat,sizeof repeat,out,sizeof out));
 x4_movie_config(argv[1]);assert(x4_movie_start(11)<0);
 unsigned total=0;
 for(unsigned id=0;id<11;++id){
  assert(!x4_movie_start(id));assert(x4_movie_active()&&x4_movie_id()==id&&x4_movie_frame()==0);
  unsigned count=0;uint32_t video=2166136261u,audio=video;int16_t pcm[1260],reference[1260];
  char name[2048];snprintf(name,sizeof name,"%s/%02u.pcm",argv[1],id);FILE* f=fopen(name,"rb");assert(f);
  while(x4_movie_active()){
   assert(x4_movie_frame()==count);video=hash(video,(const uint8_t*)x4_movie_pixels(),X4_MOVIE_PIXELS*2);
   if((id==0&&count==80)||(id==2&&count==20)){
    char p[64];snprintf(p,sizeof p,"movie_%u.ppm",id);FILE* image=fopen(p,"wb");assert(image);fprintf(image,"P6\n256 192\n255\n");
    const uint16_t* pixels=x4_movie_pixels();for(unsigned i=0;i<X4_MOVIE_PIXELS;++i){uint16_t c=pixels[i];uint8_t rgb[]={(c&31)*255/31,((c>>5)&31)*255/31,((c>>10)&31)*255/31};assert(fwrite(rgb,1,3,image)==3);}fclose(image);
   }
   x4_movie_audio(pcm,1260);assert(fread(reference,2,1260,f)==1260&&!memcmp(reference,pcm,sizeof pcm));audio=hash(audio,(uint8_t*)pcm,sizeof pcm);
   for(unsigned i=0;i<4;++i)x4_movie_tick();++count;
  }
  assert(!x4_movie_error()&&count&&fgetc(f)==EOF);fclose(f);total+=count;
  assert(!x4_movie_start(id));x4_movie_skip();assert(!x4_movie_active()&&!x4_movie_pixels());
  printf("PASS movie=%u frames=%u video=%08x audio=%08x natural end, PCM order and skip/reopen\n",id,count,video,audio);fflush(stdout);
 }
 x4_movie_close();printf("PASS %u movie frames and malformed/truncated LZ10 records\n",total);return 0;
}
