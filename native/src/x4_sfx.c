/* Native PCM effect transport. Fixed resource descriptors, no sound chip model. */
#include "x4_sfx.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
typedef struct {uint32_t command,offset,samples,loop;} Entry;
typedef struct {char path[1100];unsigned count;long data;Entry entries[256];} Bank;
enum {CACHE_SLOTS=64,CACHE_LIMIT=4*1024*1024};
typedef struct {void* allocation;int16_t* pcm;size_t bytes,samples;unsigned bank,index,loop,age,pins;} Cached;
typedef struct {FILE* file;unsigned bank,index,mask,priority,loop;long start;uint32_t position,samples;Cached* cached;unsigned gain;} Voice;
static Bank banks[6];static Voice voices[8];static char root[1024];static unsigned errors,character;
static Cached cache[CACHE_SLOTS];static unsigned cache_bytes,cache_age;static X4SfxStats stats;
static X4SfxBackend backend;
static size_t cache_limit=CACHE_LIMIT;
void x4_sfx_set_cache_limit(size_t bytes){cache_limit=bytes;}
void x4_sfx_character(unsigned value){character=!!value;}
static uint32_t le32(const unsigned char* b){return b[0]|(uint32_t)b[1]<<8|(uint32_t)b[2]<<16|(uint32_t)b[3]<<24;}
static void stop(Voice* v){
 if(v->cached){backend.stop((unsigned)(v-voices));--v->cached->pins;}
 if(v->file)fclose(v->file);memset(v,0,sizeof *v);
}
static int active(Voice* v){
 if(v->cached&&!backend.active((unsigned)(v-voices)))stop(v);
 return v->file||v->cached;
}
static void evict(Cached* c){cache_bytes-=c->bytes;free(c->allocation);memset(c,0,sizeof *c);}
void x4_sfx_close(void){for(unsigned i=0;i<8;++i)stop(&voices[i]);for(unsigned i=0;i<CACHE_SLOTS;++i)evict(&cache[i]);memset(banks,0,sizeof banks);}
void x4_sfx_config(const char* p){x4_sfx_close();errors=0;memset(&stats,0,sizeof stats);root[0]=0;if(p&&strlen(p)<sizeof root)strcpy(root,p);}
void x4_sfx_set_backend(const X4SfxBackend* b){x4_sfx_close();backend=b?*b:(X4SfxBackend){0};}
X4SfxStats x4_sfx_stats(void){stats.bytes=cache_bytes;stats.active=0;for(unsigned i=0;i<8;++i)stats.active+=!!active(&voices[i]);return stats;}
static Cached* find_cached(unsigned bank,unsigned index){
 for(unsigned i=0;i<CACHE_SLOTS;++i)if(cache[i].pcm&&cache[i].bank==bank&&cache[i].index==index){cache[i].age=++cache_age;return &cache[i];}
 return NULL;
}
static Cached* load_cached(FILE* f,unsigned bank,unsigned index,size_t samples,unsigned loop){
 if(!samples||samples>cache_limit/2)return NULL;
 /* ARM7 PCM16 lengths are measured in two-sample words. Repeat an odd
  * loop twice so hardware wraps on a word boundary without a silent sample. */
 size_t playback_samples=samples;
 if(loop&&(samples&1)){
  if(samples>cache_limit/4)return NULL;
  playback_samples=samples*2;
 }
 size_t bytes=(playback_samples*2+31)&~(size_t)31;Cached* slot=NULL;
 if(bytes>cache_limit)return NULL;
 for(unsigned i=0;i<8;++i)active(&voices[i]);
 for(;;){
  Cached* oldest=NULL;slot=NULL;
  for(unsigned i=0;i<CACHE_SLOTS;++i){Cached* c=&cache[i];if(!c->pcm){slot=c;continue;}
   if(!c->pins&&(!oldest||c->age<oldest->age))oldest=c;
  }
  if(slot&&cache_bytes+bytes<=cache_limit)break;
  if(!oldest)return NULL;evict(oldest);
 }
 void* memory=malloc(bytes+31);if(!memory)return NULL;
 int16_t* pcm=(int16_t*)(((uintptr_t)memory+31)&~(uintptr_t)31);
 if(fread(pcm,2,samples,f)!=samples){free(memory);return NULL;}
 if(playback_samples!=samples)memcpy(pcm+samples,pcm,samples*2);
 memset((uint8_t*)pcm+playback_samples*2,0,bytes-playback_samples*2);
 if(backend.prepare)backend.prepare(pcm,bytes);
 *slot=(Cached){memory,pcm,bytes,playback_samples,bank,index,loop,++cache_age,0};cache_bytes+=bytes;return slot;
}
static int start_cached(Voice* v,Cached* c,unsigned mask,unsigned priority,unsigned pan,unsigned gain){
 if(backend.play((unsigned)(v-voices),c->pcm,c->samples,c->loop,pan,gain)){++errors;return -2;}
 ++c->pins;*v=(Voice){.bank=c->bank,.index=c->index,.mask=mask,.priority=priority,.loop=c->loop,.samples=c->samples,.cached=c};++stats.starts;return 0;
}
unsigned x4_sfx_errors(void){return errors;}
int x4_sfx_bank(unsigned resource,unsigned bank){
 if(bank>=6||!root[0])return -1;
 Bank next={0};unsigned char header[16],raw[16];
 int n=snprintf(next.path,sizeof next.path,"%s/%02x_%u.sfx",root,resource,bank);if(n<0||(size_t)n>=sizeof next.path)return -1;
 FILE* f=fopen(next.path,"rb");if(!f)return -2;
 if(fread(header,1,16,f)!=16||memcmp(header,"X4SFX01\0",8)||le32(header+12)!=18900){fclose(f);return -3;}
 next.count=le32(header+8);next.data=16+(long)next.count*16;
 if(next.count>256||fseek(f,0,SEEK_END)){fclose(f);return -3;}
 long bytes=ftell(f);if(bytes<next.data||fseek(f,16,SEEK_SET)){fclose(f);return -3;}
 for(unsigned i=0;i<next.count;++i){
  if(fread(raw,1,16,f)!=16){fclose(f);return -3;}
  Entry* e=&next.entries[i];*e=(Entry){le32(raw),le32(raw+4),le32(raw+8),le32(raw+12)};
  if(e->offset>(unsigned long)(bytes-next.data)||e->samples>(unsigned long)(bytes-next.data-e->offset)/2||e->loop>3){fclose(f);return -3;}
 }
 fclose(f);for(unsigned i=0;i<8;++i)if(active(&voices[i])&&voices[i].bank==bank)stop(&voices[i]);
 for(unsigned i=0;i<CACHE_SLOTS;++i)if(cache[i].pcm&&cache[i].bank==bank)evict(&cache[i]);
 banks[bank]=next;return 0;
}
static unsigned mask_of(const Entry* e){
 if((e->command&0xc0)<0x80){unsigned slot=e->command>>20&15;return slot<4?1u<<(24+slot):0;}
 unsigned voice=e->command>>24&31,count=1+((e->command>>29)&3);
 return voice<24?(((1u<<count)-1)<<voice)&0xffffff:0;
}
int x4_sfx_play(unsigned bank,unsigned index){return x4_sfx_play_at(bank,index,-1,0);}
int x4_sfx_play_at(unsigned bank,unsigned index,int position,int stereo){
 if(bank>=6||index>=banks[bank].count)return -1;
 Bank* b=&banks[bank];Entry* e=&b->entries[index];if(!e->command)return -5;
 if(!e->samples&&(e->loop&2))return 0; /* Original zero-volume/empty tone. */
 unsigned type=e->command&0xc0,sequence=type<0x80;
 unsigned access=0,number=0;
 if(sequence){
  access=e->command>>8&127;
  /* Libsnd's SsSeqPlay selects sequence 0. SsSepPlay uses the index
   * stored in the command. Reject unknown entries before stopping a voice. */
  if(type==0x40)number=e->command>>24&31;
  if(access>1||number>=(access?2u:8u))return -6;
 }else if(!e->samples)return -6;
 unsigned pan=64,gain=128;
 if(stereo&&position>=0&&(e->command&0xc0)>=0x80&&!(e->command&0x8000)){
  if(position<5)position=5;if(position>27)position=27;
  unsigned left=position>16?32-position:16,right=position<16?position:16;
  /* Calico uses complementary linear gains. Normalize the original two
   * sides and compensate their sum, retaining the centered PCM level. */
  pan=(right*127+(left+right)/2)/(left+right);gain=(left+right)*4;
 }
 unsigned mask=mask_of(e),priority=e->command>>16&15;
 for(unsigned i=0;i<8;++i)if(active(&voices[i])&&(voices[i].mask&mask)&&voices[i].priority>priority)return -4;
 Voice* slot=NULL;
 for(unsigned i=0;i<8;++i){
  if((!active(&voices[i])||(voices[i].mask&mask))&&!slot)slot=&voices[i];
 }
 if(!slot)return -7;
 Cached* retained=backend.play?find_cached(bank,index):NULL;
 if(retained){
  for(unsigned i=0;i<8;++i)if(active(&voices[i])&&(voices[i].mask&mask))stop(&voices[i]);
  ++stats.hits;return start_cached(slot,retained,mask,priority,pan,gain);
 }
 if(backend.play)++stats.misses;
 char filename[1100];const char* path=b->path;long start=b->data+e->offset;unsigned samples=e->samples,loop=e->loop&1;
 if(sequence){
  int n=snprintf(filename,sizeof filename,"%s/seq_%u_%u_%u.pcm",root,character,access,number);if(n<0||(size_t)n>=sizeof filename)return -2;path=filename;
 }
 FILE* f=fopen(path,"rb");if(!f){++errors;return -2;}
 if(sequence){if(fseek(f,0,SEEK_END)){fclose(f);++errors;return -2;}long bytes=ftell(f);
  if(bytes<0||(bytes&1)){fclose(f);++errors;return -2;}samples=bytes/2;start=0;loop=!!(e->command&0x8000);
  if(!samples){fclose(f);return 0;}
 }
 if(fseek(f,start,SEEK_SET)){fclose(f);++errors;return -2;}
 /* A bad or absent resource must not interrupt the effect already playing. */
 for(unsigned i=0;i<8;++i)if(active(&voices[i])&&(voices[i].mask&mask))stop(&voices[i]);
 if(backend.play){Cached* c=load_cached(f,bank,index,samples,loop);
  if(c){fclose(f);return start_cached(slot,c,mask,priority,pan,gain);}
  /* Long effects and cache pressure use the existing streamed mixer. */
  if(fseek(f,start,SEEK_SET)){fclose(f);++errors;return -2;}
 }
 setvbuf(f,NULL,_IOFBF,2048);
 *slot=(Voice){.file=f,.bank=bank,.index=index,.mask=mask,.priority=priority,
               .loop=loop,.start=start,.samples=samples,.gain=gain};
 return 0;
}
void x4_sfx_stop(unsigned bank,unsigned index){
 unsigned mask=bank<6&&index<banks[bank].count?mask_of(&banks[bank].entries[index]):0;
 for(unsigned i=0;i<8;++i)if(bank==255||(active(&voices[i])&&(voices[i].mask&mask)))stop(&voices[i]);
}
int x4_sfx_finished(unsigned bank,unsigned index){
 if(bank>=6||index>=banks[bank].count)return 1;
 unsigned mask=mask_of(&banks[bank].entries[index]);for(unsigned i=0;i<8;++i)if(active(&voices[i])&&(voices[i].mask&mask))return 0;return 1;
}
void x4_sfx_mix(int16_t* out,size_t frames){
 int streaming=0;for(unsigned i=0;i<8;++i)streaming|=voices[i].file!=NULL;
 if(!streaming)return;
 int16_t temporary[256];int32_t accum[256];
 for(size_t base=0;base<frames;base+=256){size_t count=frames-base;if(count>256)count=256;
  for(size_t j=0;j<count;++j)accum[j]=out[base+j];
  for(unsigned i=0;i<8;++i){Voice* v=&voices[i];size_t done=0;
   while(v->file&&done<count){size_t n=count-done;if(n>v->samples-v->position)n=v->samples-v->position;
    size_t got=fread(temporary,2,n,v->file);
    if(v->gain==128){
     for(size_t j=0;j<got;++j)accum[done+j]+=temporary[j];
    }else{
     for(size_t j=0;j<got;++j)accum[done+j]+=((int32_t)temporary[j]*(int32_t)v->gain)/128;
    }
    done+=got;v->position+=got;
    if(got!=n){++errors;stop(v);break;}
    if(v->position==v->samples){if(v->loop){if(fseek(v->file,v->start,SEEK_SET)){++errors;stop(v);}else v->position=0;}else stop(v);}
   }
  }
  for(size_t j=0;j<count;++j){int32_t s=accum[j];out[base+j]=s>32767?32767:s< -32768?-32768:s;}
 }
}
