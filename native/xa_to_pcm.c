/* Build-time XA conversion only. The DSi receives plain PCM audio.
 * Format follows the primary sozud/psyz XA decoder; no console runtime. */
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
static int previous[2],older[2];
static int clamp(int s){return s<-32768?-32768:s>32767?32767:s;}
static void output(FILE* f,int s){uint16_t v=(uint16_t)(int16_t)clamp(s);unsigned char b[2]={v,v>>8};if(fwrite(b,1,2,f)!=2)exit(3);}
int main(int argc,char** argv){
 if(argc!=7)return 2;FILE* in=fopen(argv[1],"rb"),*out=fopen(argv[6],"wb");if(!in||!out)return 3;
 previous[0]=previous[1]=older[0]=older[1]=0;
 unsigned long lba=strtoul(argv[2],0,0),channel=strtoul(argv[3],0,0),start=strtoul(argv[4],0,0),end=strtoul(argv[5],0,0);
 static const int pos[4]={0,60,115,98},neg[4]={0,0,-52,-55};unsigned char raw[2352];unsigned count=0,samples=0;
 for(unsigned long sector=start*8+channel;sector<end*8+channel;sector+=8){
  if(fseeko(in,(off_t)(lba+sector)*2352,SEEK_SET)||fread(raw,1,2352,in)!=2352)return 4;
  if(raw[16]!=1||raw[17]!=channel||(raw[18]&0x44)!=0x44){fprintf(stderr,"XA sector header mismatch at %lu\n",sector);return 5;}
  if(raw[19]&0x30){fprintf(stderr,"Unsupported XA width %02x\n",raw[19]);return 6;}
  unsigned channels=(raw[19]&1)?2:1,decimation=(raw[19]&4)?1:2;
  for(unsigned group=0;group<18;++group){const unsigned char* b=raw+24+group*128;
   int pcm[2][28];
   for(unsigned unit=0;unit<8;++unit){
    unsigned ch=unit%channels,h=b[4+unit],shift=(h&15)>12?9:12-(h&15),filter=(h>>4)&3;
    for(unsigned j=0;j<28;++j){int nibble=(b[16+unit/2+j*4]>>((unit&1)*4))&15;if(nibble&8)nibble-=16;
     int v=previous[ch]*pos[filter]+older[ch]*neg[filter]+32;
     int s=nibble*(1<<shift)+v/64;older[ch]=previous[ch];previous[ch]=s;pcm[ch][j]=clamp(s);}
    if(ch+1==channels)for(unsigned j=0;j<28;j+=decimation){int sum=0;
     for(unsigned c=0;c<channels;++c)for(unsigned k=0;k<decimation;++k)sum+=pcm[c][j+k];
     /* Match the established Python intro converter's floor division. */
     int denominator=channels*decimation;int value=sum>=0?sum/denominator:-((-sum+denominator-1)/denominator);
     output(out,value);++samples;}
   }
  }
  ++count;
 }
 fclose(in);if(fclose(out))return 7;printf("%u sectors %u samples\n",count,samples);return 0;
}
