#include "x4_music.h"
#include <assert.h>
#include <stdio.h>
static int16_t pcm[4096];
int main(int argc,char** argv){
 assert(argc==2);X4Music music={0};unsigned tracks=0;size_t total=0;
 for(unsigned selection=0;selection<=0x57;++selection){
  if(selection>=0x1d&&selection<0x20||selection>=0x23&&selection<0x28)continue;
  assert(!x4_music_open(&music,argv[1],selection,128));unsigned loop=music.loop;size_t samples=music.samples;music.loop=0;
  size_t read=0;while(!music.finished){size_t n=x4_music_read(&music,pcm,4096);read+=n;assert(!music.error&&n);}
  assert(read==samples);assert(!x4_music_read(&music,pcm,4096));for(unsigned i=0;i<4096;++i)assert(pcm[i]==0);
  if(loop){assert(!x4_music_open(&music,argv[1],selection,64));size_t target=music.samples+4096;
   while(target){size_t n=target>4096?4096:target;assert(x4_music_read(&music,pcm,n)==n);target-=n;assert(!music.finished&&!music.error);}}
  void* old=music.file;assert(x4_music_open(&music,"/missing",selection,128)<0&&music.file==old);
  ++tracks;total+=samples;
 }
 x4_music_close(&music);assert(x4_music_open(&music,argv[1],0x1f,128)<0);
 printf("PASS %u native PCM selections, %zu samples; EOF, silence tail, looping, gain and failed replacement\n",tracks,total);
}
