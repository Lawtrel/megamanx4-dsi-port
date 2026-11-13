#include "x4_music.h"
#include <stdio.h>
#include <string.h>
static int named(unsigned selection){return selection<0x1d||(selection>=0x20&&selection<=0x22)||(selection>=0x28&&selection<=0x57);}
void x4_music_close(X4Music* m){if(m->file)fclose(m->file);memset(m,0,sizeof *m);}
int x4_music_open(X4Music* m,const char* root,unsigned selection,unsigned gain){
 if(!m||!root||!named(selection)||gain>128)return -1;
 char path[2048];int n=snprintf(path,sizeof path,"%s/%02x.pcm",root,selection);
 if(n<0||(size_t)n>=sizeof path)return -1;
 FILE* f=fopen(path,"rb");if(!f)return -2;
 if(fseek(f,0,SEEK_END)){fclose(f);return -2;}long bytes=ftell(f);rewind(f);
 if(bytes<=0||(bytes&1)){fclose(f);return -3;}
 setvbuf(f,NULL,_IOFBF,8192);x4_music_close(m);
 *m=(X4Music){f,selection,gain,selection!=0&&selection<0x20,0,0,(size_t)bytes/2,0};return 0;
}
size_t x4_music_read(X4Music* m,int16_t* out,size_t frames){
 if(!m||!out)return 0;
 size_t done=0;
 if(m->file&&!m->finished)while(done<frames){
  size_t n=fread(out+done,2,frames-done,m->file);done+=n;m->position+=n;
  if(m->position==m->samples){
   if(m->loop){if(fseek(m->file,0,SEEK_SET)){m->error=1;break;}m->position=0;}
   else{m->finished=1;break;}
  }else if(!n){m->error=1;break;}
 }
 for(size_t i=0;i<done;++i)out[i]=(int32_t)out[i]*(int32_t)m->gain/128;
 if(done<frames)memset(out+done,0,(frames-done)*2);
 return done;
}
