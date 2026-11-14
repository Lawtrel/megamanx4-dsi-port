/* Native bitmap/PCM movie transport. Source-video decoding happens offline.
 * Independent LZ10 frames allow bounded memory and precise format checks. */
#include "x4_movie.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char root[1024];
static FILE *video,*audio;
static uint16_t* pixels;
static uint8_t* packed;
static unsigned active,error,id,frame,frames,ticks,generation;
static uint32_t le32(const uint8_t* p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
int x4_movie_decode(const uint8_t* data,size_t bytes,uint8_t* out,size_t capacity){
 if(bytes<4||data[0]!=0x10)return -1;
 size_t size=le32(data)>>8,pos=4,done=0;
 if(size!=capacity)return -1;
 while(done<size){
  if(pos>=bytes)return -1;unsigned flags=data[pos++];
  for(unsigned bit=128;bit&&done<size;bit>>=1){
   if(flags&bit){
    if(bytes-pos<2)return -1;unsigned a=data[pos++],b=data[pos++];size_t n=(a>>4)+3,d=((a&15)<<8)+b+1;
    if(d>done||n>size-done)return -1;
    while(n--){out[done]=out[done-d];++done;}
   }else{if(pos>=bytes)return -1;out[done++]=data[pos++];}
  }
 }
 return pos==bytes?0:-1;
}
void x4_movie_close(void){
 if(video)fclose(video);if(audio)fclose(audio);video=audio=NULL;
 free(pixels);free(packed);pixels=NULL;packed=NULL;active=0;
}
void x4_movie_config(const char* path){
 x4_movie_close();root[0]=0;error=0;
 if(path&&strlen(path)<sizeof root)strcpy(root,path);
}
static int next_frame(void){
 uint8_t size[4];if(fread(size,1,4,video)!=4)return -1;
 unsigned n=le32(size);if(n<4||n>X4_MOVIE_PIXELS*2*9/8+16||fread(packed,1,n,video)!=n)return -1;
 return x4_movie_decode(packed,n,(uint8_t*)pixels,X4_MOVIE_PIXELS*2);
}
int x4_movie_start(unsigned selection){
 x4_movie_close();error=0;id=selection;frame=ticks=0;
 if(selection>10||!root[0])return -(int)(error=120);
 char name[1100];snprintf(name,sizeof name,"%s/%02u.xmv",root,selection);video=fopen(name,"rb");
 uint8_t h[32];
 if(!video||fread(h,1,32,video)!=32||memcmp(h,"X4MV0001",8)||le32(h+8)!=256||le32(h+12)!=192||le32(h+16)!=15||le32(h+24)!=18900||le32(h+28)!=0){error=121;goto fail;}
 frames=le32(h+20);if(!frames||frames>100000){error=122;goto fail;}
 snprintf(name,sizeof name,"%s/%02u.pcm",root,selection);audio=fopen(name,"rb");
 if(!audio||fseek(audio,0,SEEK_END)||ftell(audio)!=(long)frames*1260*2||fseek(audio,0,SEEK_SET)){error=123;goto fail;}
 setvbuf(video,NULL,_IOFBF,32768);setvbuf(audio,NULL,_IOFBF,8192);
 pixels=malloc(X4_MOVIE_PIXELS*2);packed=malloc(X4_MOVIE_PIXELS*2*9/8+16);
 if(!pixels||!packed||next_frame()){error=124;goto fail;}
 active=1;++generation;return 0;
fail:x4_movie_close();return -(int)error;
}
void x4_movie_skip(void){x4_movie_close();}
void x4_movie_tick(void){
 if(!active||++ticks%4)return;
 if(++frame==frames){x4_movie_close();return;}
 if(next_frame()){error=125;x4_movie_close();}
}
int x4_movie_active(void){return active;}
int x4_movie_error(void){return error?-(int)error:0;}
unsigned x4_movie_id(void){return id;}
unsigned x4_movie_frame(void){return frame;}
unsigned x4_movie_generation(void){return generation;}
const uint16_t* x4_movie_pixels(void){return pixels;}
void x4_movie_audio(int16_t* out,size_t samples){
 size_t got=active?fread(out,2,samples,audio):0;
 if(active&&ferror(audio))error=126;
 if(got<samples)memset(out+got,0,(samples-got)*2);
}
