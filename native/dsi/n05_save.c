#include "common.h"
#include "x4_save.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
enum {CAPACITY=8192,HEADER=24};
static char save_root[768];
static struct {unsigned generation,size;int valid;unsigned char data[CAPACITY];} journal[2];
static unsigned read32(const unsigned char* p){return p[0]|(unsigned)p[1]<<8|(unsigned)p[2]<<16|(unsigned)p[3]<<24;}
static void write32(unsigned char* p,unsigned v){for(unsigned i=0;i<4;++i)p[i]=v>>(8*i);}
static unsigned crc32(const unsigned char* p,unsigned n){
 unsigned c=~0u;for(unsigned i=0;i<n;++i){c^=p[i];for(unsigned j=0;j<8;++j)c=(c>>1)^((0u-(c&1))&0xedb88320u);}return ~c;
}
int x4_save_set_root(const char* root){
 save_root[0]=0;if(!root||!root[0]||strlen(root)>=sizeof save_root)return -1;strcpy(save_root,root);return 0;
}
static int path(char* p,size_t n,int port,unsigned bank){
 return snprintf(p,n,"%s/mmx4_save%d_%c.dat",save_root,port,'a'+bank)>=(int)n?-1:0;
}
static int load_journals(int port){
 if((unsigned)port>1||!save_root[0])return -1;
 for(unsigned i=0;i<2;++i){
  journal[i].valid=0;char name[1024];if(path(name,sizeof name,port,i))return -1;
  FILE* f=fopen(name,"rb");if(!f)continue;unsigned char h[HEADER];
  if(fread(h,1,sizeof h,f)!=sizeof h||memcmp(h,"X4DSISV1",8)){fclose(f);continue;}
  unsigned size=read32(h+12);if(size!=0&&size!=CAPACITY){fclose(f);continue;}
  unsigned generation=read32(h+8),payload_crc=read32(h+16);
  if(read32(h+20)!=crc32(h,20)||fread(journal[i].data,1,size,f)!=size||fgetc(f)!=EOF||crc32(journal[i].data,size)!=payload_crc){fclose(f);continue;}
  fclose(f);journal[i].generation=generation;journal[i].size=size;journal[i].valid=1;
 }
 if(!journal[0].valid)return journal[1].valid?1:-1;
 if(!journal[1].valid)return 0;
 return (int32_t)(journal[1].generation-journal[0].generation)>0?1:0;
}
int mmx4_pc_card_info(int port){return (unsigned)port>1||!save_root[0]?1:0;}
int mmx4_pc_card_load(int port){return mmx4_pc_card_info(port);}
void mmx4_pc_card_list(int port,struct MemcardFileList* list,const char* pattern){
 memset(list,0,sizeof *list);int latest=load_journals(port);
 if(latest<0||!journal[latest].size||(*pattern&&strcmp(pattern,"BASLUS-00561")))return;
 strcpy(list->names[0],"BASLUS-00561");list->count=1;list->sizes[0]=list->total_size=CAPACITY;
}
int mmx4_pc_card_read(int port,void* buffer,int size){
 if(!buffer||size<0||size>CAPACITY||mmx4_pc_card_info(port))return 2;
 int latest=load_journals(port);if(latest<0||!journal[latest].size)return 1;
 memcpy(buffer,journal[latest].data,size);return 0;
}
static int store(int port,const void* buffer,int size,int empty){
 if(mmx4_pc_card_info(port)||size<0||size>CAPACITY||(!buffer&&!empty))return 2;
 int latest=load_journals(port);unsigned next=latest<0?0:(unsigned)latest^1;
 unsigned generation=latest<0?1:journal[latest].generation+1;
 if(!generation)return 2;/* Do not wrap the generation ordering. */
 unsigned payload=empty?0:CAPACITY;
 if(!empty){
  if(latest>=0&&journal[latest].size)memcpy(journal[next].data,journal[latest].data,CAPACITY);
  else memset(journal[next].data,0,CAPACITY);
  memcpy(journal[next].data,buffer,size);
 }
 unsigned char h[HEADER]={0};memcpy(h,"X4DSISV1",8);write32(h+8,generation);write32(h+12,payload);
 write32(h+16,crc32(journal[next].data,payload));write32(h+20,crc32(h,20));
 char name[1024];if(path(name,sizeof name,port,next))return 2;
 FILE* f=fopen(name,"wb");if(!f)return 2;
 int ok=fwrite(h,1,sizeof h,f)==sizeof h&&fwrite(journal[next].data,1,payload,f)==payload;
 if(fflush(f))ok=0;if(fclose(f))ok=0;if(!ok)return 2;
 int checked=load_journals(port);return checked>=0&&journal[checked].generation==generation?0:2;
}
int mmx4_pc_card_write(int port,const void* buffer,int size){return store(port,buffer,size,0);}
int mmx4_pc_card_format(int port){return store(port,NULL,0,1)?1:0;}
