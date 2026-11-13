#include "common.h"
#include "main/object_motion.h"
#include "x4_flow.h"
#include "x4_scene.h"
#include "x4_audio.h"
#include "x4_video.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "n06_preview.h"
static uint16_t screen[256*192];
extern void ice_core_shed_ice(struct MainObj*);
extern void eregion_death_blink(struct MainObj*);
static void check_native_boss_safety(void){
 assert(cur_random==0xD37);unsigned first=get_random(),different=0;
 for(unsigned i=0;i<64;++i)different|=get_random()!=first;
 assert(different);
 /* The old startup seed was zero: every mask without bit zero hung here. */
 for(unsigned mask=1;mask<1024;++mask){struct MainObj eye={0};
  eye.ext.main_18.ice_pieces=mask;eye.ext.main_18.shed_timer=0x21;cur_random=0;
  ice_core_shed_ice(&eye);
  unsigned left=eye.ext.main_18.ice_pieces;assert(!(left&~mask)&&__builtin_popcount(left)==__builtin_popcount(mask)-1);
 }
 /* Full effect pool must retry allocation before entering the wait state. */
 static struct EffectObj saved[COUNT(effect_objects)];memcpy(saved,effect_objects,sizeof saved);
 for(unsigned i=0;i<COUNT(effect_objects);++i)effect_objects[i].active=1;
 struct MainObj dragon={0};dragon.unk7C=1;dragon.unk6=1;
 eregion_death_blink(&dragon);assert(dragon.unk6==1&&dragon.unk7C==1&&!dragon.ext.main_9.effect);
 memcpy(effect_objects,saved,sizeof saved);cur_random=0xD37;
 puts("PASS native startup RNG, all 1023 ice masks with zero seed, full dragon effect pool");
}
extern int x4_flow_ui_pause_palette_active(void);
int main(int argc,char** argv){
 assert(argc>=3);const char* mode=argv[2];unsigned character=argc>3?strtoul(argv[3],NULL,0):0;
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 int sigma=!strcmp(mode,"sigma"),introclear=!strcmp(mode,"introclear"),pausejungle=!strcmp(mode,"pausejungle"),ice=!strcmp(mode,"ice");
 assert(!(!strcmp(mode,"campaign")?x4_flow_new_game(argv[1]):x4_flow_begin(argv[1],character,sigma?12:pausejungle?1:ice?2:0,(sigma||introclear||pausejungle)?1:0)));
 check_native_boss_safety();
 x4_video_enabled=getenv("X4_GPU")!=NULL;
 if(x4_video_enabled)x4_video_viewport(32,24,256,192);
 unsigned previous=0,changes=0,boss_start=0;int seen_pause=0,seen_resume=0,seen_game=0,seen_boss=0,seen_boss_actor=0;
 unsigned defeated=0;int left_intro=0;
 unsigned ice_ticks=0,ice_damage=0,ice_shed=0;int ice_hp=0;
 uint16_t palette_before[512],pause_expected[512];unsigned palette_restores=0;
 for(unsigned tick=0;tick<(introclear?12000:ice?6000:sigma?3600:2400);++tick){
  unsigned held=0;
  if(!strcmp(mode,"campaign")){if(tick%180==150)held=1u<<16;if(tick%40==10&&tick>150)held|=X4_JUMP;if(character&&tick==100)held|=X4_RIGHT;}
  else if(!strcmp(mode,"pause")||pausejungle){
   if(pausejungle&&tick==300){engine_obj.checkpoint=2;engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;}
   if(tick==(pausejungle?800:400)||tick==(pausejungle?1100:700)||tick==(pausejungle?1500:1000))held=1u<<16;
  }else if(!strcmp(mode,"respawn")){
   if(tick==400){g_Player.hp=0;g_Player.state=3;}
  }else if(!strcmp(mode,"transition")){
   if(tick==400){engine_obj.state=10;engine_obj.unk1=0;}
   if(tick%80==10)held=X4_JUMP;
  }else if(!strcmp(mode,"weapons")){
   if(tick>=400){
    g_Player.hp=32;g_Player.invincibility_timer=120;g_Player.boss_flags=-1;g_Player.armor_parts=15;
    if(tick%200==0){g_Player.weapon=((tick-400)/200)%9;++changes;}
    for(unsigned i=0;i<16;++i)g_Player.weapon_energy[i]=48;
    if(tick%200<150)held|=X4_SHOOT;
    if(tick%120<20)held|=X4_JUMP;
    if(tick%90<15)held|=X4_DASH;
    if(character&&tick%300<90)held|=4;
    if(character&&tick%300>190)held|=8;
   }
  }else if(sigma){
   if(tick>240){held=X4_RIGHT;if(tick%70<28)held|=X4_JUMP;if(tick%19==0)held|=X4_SHOOT;
    g_Player.hp=32;g_Player.invincibility_timer=120;/* Survive for graphics coverage, host test only. */
   }
  }else if(ice){
   /* Stage 2/0 checkpoint FOUR is the corridor before the eye. Checkpoint
    * two is much earlier, before a wall that this scripted input can't climb. */
   if(tick==300){engine_obj.checkpoint=4;engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;}
   if(tick>600){
    struct MainObj* eye=engine_obj.enable_boss?engine_obj.boss_ptr:NULL;
    g_Player.hp=32;g_Player.invincibility_timer=120;
    if(eye&&eye->id==18){
     ++ice_ticks;if(eye->hp>ice_hp)ice_hp=eye->hp;
     if(eye->hp>0&&eye->hp<ice_hp)++ice_damage;
     unsigned pieces=eye->ext.main_18.ice_pieces&1023;if(pieces&&pieces!=1023)++ice_shed;
     if(g_Player.x_pos.i.hi<eye->x_pos.i.hi-20)held|=X4_RIGHT;
     else if(g_Player.x_pos.i.hi>eye->x_pos.i.hi+20)held|=X4_LEFT;
    }else if(g_Player.x_pos.i.hi<6240)held|=X4_RIGHT;
    if(tick%90<35)held|=X4_JUMP;if(tick%19==0)held|=X4_SHOOT;
   }
  }else if(introclear){
   held=X4_RIGHT;if(tick%90<35)held|=X4_JUMP;if(tick%19==0)held|=X4_SHOOT;
   if(tick==300){engine_obj.checkpoint=2;engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;}
   if(tick==500){extern int x4_flow_load(unsigned,unsigned,void*);assert(!x4_flow_load(0x76,4,NULL));}
   g_Player.hp=32;g_Player.invincibility_timer=120;
   if(engine_obj.enable_boss&&engine_obj.boss_ptr&&engine_obj.boss_ptr->on_screen){
    if(++defeated==200){/* Enter the original death dispatcher; do not skip its sequence. */
     engine_obj.boss_ptr->hp=0;engine_obj.boss_ptr->state=2;engine_obj.boss_ptr->unk5=0;engine_obj.boss_ptr->unk6=0;
    }
   }
  }else abort();
  int palette_was_active=x4_flow_ui_pause_palette_active();
  if(pausejungle&&!palette_was_active&&SP_PALETTE)memcpy(palette_before,SP_PALETTE+1024,sizeof palette_before);
  assert(!x4_flow_step(held,held&~previous));previous=held;
  if(pausejungle){int active=x4_flow_ui_pause_palette_active();
   if(!palette_was_active&&active)memcpy(pause_expected,palette_before,sizeof pause_expected);
   if(palette_was_active&&!active){assert(!memcmp(SP_PALETTE+1024,pause_expected,sizeof pause_expected));++palette_restores;}
  }
  x4_audio_update();int16_t pcm[315];x4_audio_read(pcm,315);assert(!x4_audio_errors());
  X4FlowState s;x4_flow_state(&s);
  if(ice&&tick%240==0){fprintf(stderr,"ICE tick=%u x=%d y=%d boss=%d lock=%d\n",tick,s.x,s.y,engine_obj.enable_boss,s.locked);
   for(unsigned i=0;i<COUNT(main_objects);++i)if(main_objects[i].active&&main_objects[i].id==18){struct MainObj* eye=&main_objects[i];fprintf(stderr,"EYE state=%d/%d/%d x=%d y=%d hp=%d pieces=%x\n",eye->state,eye->unk5,eye->unk6,eye->x_pos.i.hi,eye->y_pos.i.hi,eye->hp,eye->ext.main_18.ice_pieces);}}
  if(introclear&&defeated>200&&s.state!=6){left_intro=1;printf("LEFT INTRO tick=%u state=%d/%d\n",tick,s.state,s.substate);break;}
  if(introclear&&tick%120==0){printf("ENDCHECK x=%d camera=%d dialogue=%d script=%d boss=%d hp=%d\n",s.x,background_objects[0].x_pos.i.hi,abc_object.unkC,g_Player.script_state,engine_obj.enable_boss,engine_obj.boss_ptr?engine_obj.boss_ptr->hp:-1);
   for(unsigned i=0;i<COUNT(misc_objects);++i)if(misc_objects[i].active&&misc_objects[i].id==46)printf("MESSENGER state=%d step=%d x=%d\n",misc_objects[i].state,misc_objects[i].unk5,misc_objects[i].x_pos.i.hi);
  }
  if(s.state==6&&!s.locked)seen_game=1;
  if(bar_object.state||bar_object.unk5)seen_pause=1;
  if(tick>700&&seen_pause&&!bar_object.state&&!bar_object.unk5)seen_resume=1;
  if(tick%30==0){printf("%s tick=%u engine=%d/%d stage=%d/%d player=%d hp=%d bar=%d/%d lives=%d\n",mode,tick,s.state,s.substate,s.stage,s.area,s.player_state,s.hp,bar_object.state,bar_object.unk5,engine_obj.player_lives);fflush(stdout);}
  int errors=x4_flow_draw(screen);
  if(pausejungle&&x4_video_enabled&&(tick==1000||tick==1400)){char name[64];snprintf(name,sizeof name,"jungle_%s.ppm",tick==1000?"pause":"resume");n06_preview(name);}
  if(sigma&&engine_obj.enable_boss&&engine_obj.boss_ptr&&!seen_boss){
   seen_boss=1;boss_start=tick;assert(engine_obj.unk1F);unsigned n;const X4VideoCommand* cmds=x4_video_commands(&n);int left_bar=0,right_bar=0;
   for(unsigned i=0;i<n;++i)if(!cmds[i].textured&&cmds[i].order== -16384){if(cmds[i].x[0]==22)left_bar=1;if(cmds[i].x[0]==290)right_bar=1;}
   if(x4_video_enabled){assert(left_bar&&right_bar);char name[64];snprintf(name,sizeof name,"sigma_%u.ppm",character);n06_preview(name);}
  }
  if(sigma&&seen_boss&&!seen_boss_actor&&tick>boss_start+180&&engine_obj.enable_boss&&engine_obj.boss_ptr&&engine_obj.boss_ptr->active&&engine_obj.boss_ptr->on_screen){
   seen_boss_actor=1;if(x4_video_enabled){char name[64];snprintf(name,sizeof name,"sigma_battle_%u.ppm",character);n06_preview(name);}
  }
  if(x4_video_enabled){X4VideoStats v=x4_video_stats();if(errors||tick%240==0)fprintf(stderr,"GPU tick=%u commands=%u hits=%u misses=%u overflow=%u\n",tick,v.commands,v.hits,v.misses,v.overflow);assert(!v.overflow);int dirty;for(unsigned i=0;i<X4_VIDEO_TILES;++i)x4_video_texture(i,&dirty);for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)x4_video_palette(i,&dirty);}
  if(errors){fprintf(stderr,"DRAW FAILED mode=%s tick=%u errors=%d\n",mode,tick,errors);return 2;}
 }
 if(sigma)assert(seen_boss&&seen_boss_actor);
 if(ice){fprintf(stderr,"ICE fight_ticks=%u damage_ticks=%u shed_ticks=%u max_hp=%d\n",ice_ticks,ice_damage,ice_shed,ice_hp);assert(ice_ticks>300&&ice_damage&&ice_shed);}
 if(introclear)assert(left_intro);
 assert(seen_game);
 if(!strcmp(mode,"campaign"))assert(engine_obj.cur_character==character);
 if(!strcmp(mode,"pause")||pausejungle)assert(seen_pause&&seen_resume);
 if(pausejungle)assert(palette_restores);
 if(!strcmp(mode,"transition"))assert(engine_obj.substage==1);
 if(!strcmp(mode,"respawn"))assert(g_Player.hp>0&&engine_obj.player_lives==1);
 if(!strcmp(mode,"weapons"))assert(changes>=9);
 x4_flow_close();x4_audio_close();printf("PASS native systems %s character=%u\n",mode,character);
}
