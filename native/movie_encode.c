/* Offline LZ10 bitmap encoder; verified with the exact runtime decoder. */
#include "src/x4_movie.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static uint8_t raw[X4_MOVIE_PIXELS*2],decoded[sizeof raw],packed[sizeof raw*9/8+16];
static int head[65536],chain[sizeof raw];
static void word(FILE* f,uint32_t v){uint8_t b[]={v,v>>8,v>>16,v>>24};if(fwrite(b,1,4,f)!=4)exit(3);}
static unsigned hash(unsigned p){return (raw[p]*251u+raw[p+1]*31u+raw[p+2])&65535;}
static unsigned encode(void){
 memset(head,255,sizeof head);unsigned p=0,n=4;packed[0]=0x10;packed[1]=sizeof raw&255;packed[2]=sizeof raw>>8;packed[3]=sizeof raw>>16;
 while(p<sizeof raw){unsigned flag=n++;packed[flag]=0;
  for(unsigned bit=128;bit&&p<sizeof raw;bit>>=1){unsigned length=0,distance=0;
   if(p+2<sizeof raw){int old=head[hash(p)];for(unsigned tries=0;old>=0&&p-(unsigned)old<=4096&&tries<32;old=chain[old],++tries){
    unsigned k=0;while(k<18&&p+k<sizeof raw&&raw[old+k]==raw[p+k])++k;
    if(k>=3&&k>length){length=k;distance=p-old;if(k==18)break;}
   }}
   unsigned step=length?length:1;
   if(length){packed[flag]|=bit;unsigned value=((length-3)<<12)|(distance-1);packed[n++]=value>>8;packed[n++]=value;}
   else packed[n++]=raw[p];
   while(step--){if(p+2<sizeof raw){unsigned h=hash(p);chain[p]=head[h];head[h]=p;}++p;}
  }
 }
 return n;
}
int main(int argc,char** argv){
 if(argc!=3)return 2;FILE* in=fopen(argv[1],"rb"),*out=fopen(argv[2],"wb");if(!in||!out)return 2;
 fwrite("X4MV0001",1,8,out);word(out,256);word(out,192);word(out,15);word(out,0);word(out,18900);word(out,0);unsigned frames=0;
 for(;;){size_t got=fread(raw,1,sizeof raw,in);if(!got)break;if(got!=sizeof raw)return 3;
  for(unsigned i=1;i<sizeof raw;i+=2)raw[i]|=128;
  unsigned n=encode();if(x4_movie_decode(packed,n,decoded,sizeof decoded)||memcmp(raw,decoded,sizeof raw))return 4;
  word(out,n);if(fwrite(packed,1,n,out)!=n)return 3;++frames;
 }
 if(ferror(in)||!frames)return 3;fseek(out,20,SEEK_SET);word(out,frames);fclose(in);if(fclose(out))return 3;printf("Encoded and verified %u frames\n",frames);return 0;
}
