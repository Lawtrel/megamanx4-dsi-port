#include "x4_flow.h"
#include "x4_scene.h"
#include "x4_audio.h"
#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static uint16_t screen[256*192];
int main(int argc,char** argv){
 assert(argc>=2);unsigned character=argc>2?strtoul(argv[2],NULL,0):0;
 unsigned stage=argc>3?strtoul(argv[3],NULL,0):0,area=argc>4?strtoul(argv[4],NULL,0):0;
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin(argv[1],character,stage,area));unsigned previous=0;
 x4_video_enabled=getenv("X4_GPU")!=NULL;
 if(x4_video_enabled)x4_video_viewport(32,24,256,192);
 int unlocked=0,moved=0,start_x=0;
 for(unsigned tick=0;tick<1000;++tick){
  unsigned held=tick>240?X4_RIGHT:0;if(tick%70<28)held|=X4_JUMP;if(tick%19==0)held|=X4_SHOOT;
  assert(!x4_flow_step(held,held&~previous));previous=held;
  x4_audio_update();int16_t pcm[315];x4_audio_read(pcm,315);assert(!x4_audio_errors());
  X4FlowState s;x4_flow_state(&s);if(!tick)start_x=s.x;
  if(s.state==6&&!s.locked)unlocked=1;if(unlocked&&s.x>start_x+100)moved=1;
  if(tick%30==0){printf("tick=%u engine=%d/%d stage=%d/%d character=%d checkpoint=%d player=%d x=%d y=%d hp=%d lock=%d\n",
   tick,s.state,s.substate,s.stage,s.area,s.character,s.checkpoint,s.player_state,s.x,s.y,s.hp,s.locked);fflush(stdout);}
  int errors=x4_flow_draw(screen);
  if(x4_video_enabled){
   X4VideoStats v=x4_video_stats();if(errors||tick%120==0)fprintf(stderr,"GPU tick=%u commands=%u hits=%u misses=%u overflow=%u\n",tick,v.commands,v.hits,v.misses,v.overflow);
   assert(!v.overflow);int dirty;for(unsigned i=0;i<X4_VIDEO_TILES;++i)x4_video_texture(i,&dirty);for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)x4_video_palette(i,&dirty);
  }
  if(errors){fprintf(stderr,"FULL FLOW DRAW FAILED tick=%u errors=%d\n",tick,errors);return 2;}
 }
 assert(unlocked&&moved);x4_flow_close();printf("PASS original flow boot and control character=%u stage=%u/%u\n",character,stage,area);
}
