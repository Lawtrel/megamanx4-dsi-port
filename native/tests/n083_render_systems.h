#include "main/player_weapon.h"
static int n083_render_system(const char* root,unsigned character,unsigned cp){
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin_debug(root,character,12,cp?0:1));
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);
 unsigned previous=0,boss_frames=0,weapon_seen=0,revealed=0;
 for(unsigned tick=0;tick<14000;++tick){
  if(cp&&tick==300){engine_obj.checkpoint=cp;engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;}
  unsigned held=0;
  if(tick>600){
   g_Player.hp=48;g_Player.invincibility_timer=120;
   for(unsigned i=0;i<16;++i)g_Player.weapon_energy[i]=48;
   if(!engine_obj.gameplay_locked){
    unsigned weapon=((tick-600)/600)%9;
    if(!character&&g_Player.weapon!=weapon){g_Player.weapon=weapon;player_reset_charge_and_weapon(&g_Player);player_update_shot_types(&g_Player);}
    weapon_seen|=1u<<weapon;
    struct MainObj* boss=engine_obj.enable_boss?engine_obj.boss_ptr:NULL;
    if(boss&&boss->active){
     ++boss_frames;
     if(boss->hp>0)boss->hp=48; /* Keep attacks alive for graphics coverage. */
     if(!cp&&tick>7000&&boss->id==68&&!revealed){boss->state=4;boss->unk5=boss->unk6=boss->unk7=0;revealed=1;}
     int dx=boss->x_pos.i.hi-g_Player.x_pos.i.hi;
     if(dx>100)held|=X4_RIGHT;else if(dx< -100)held|=X4_LEFT;
    }else held|=X4_RIGHT;
    unsigned phase=tick%300;if(phase<210)held|=X4_SHOOT;
    if(phase>=240&&phase<280)held|=32;
    if(tick%120<40)held|=X4_JUMP;
    if(tick%240<20)held|=X4_DASH;
    if(character&&tick%450<150)held|=4;else if(character&&tick%450<300)held|=8;
   }
   if(abc_object.unkC&&tick%30==0)held|=X4_JUMP;
  }
  assert(!x4_flow_step(held,held&~previous));previous=held;
  x4_audio_update();int16_t pcm[315];x4_audio_read(pcm,315);assert(!x4_audio_errors());
  int errors=x4_flow_draw(screen);
  int dirty;for(unsigned i=0;i<X4_VIDEO_TILES;++i)x4_video_texture(i,&dirty);
  for(unsigned i=0;i<X4_VIDEO_PALETTES;++i)x4_video_palette(i,&dirty);
  if(errors){fprintf(stderr,"STRESS ERROR tick=%u char=%u cp=%u weapon=%u player=%u/%u pose=%u errors=%d\n",tick,character,cp,(u8)g_Player.weapon,g_Player.state,g_Player.unk5,g_Player.animation_step.fields.frame_index,errors);return 0;}
  assert(!x4_video_stats().overflow);
  if(tick%1000==0)fprintf(stderr,"PROGRESS tick=%u char=%u cp=%u bossframes=%u reveal=%u weapon=%d x=%d y=%d\n",tick,character,cp,boss_frames,revealed,g_Player.weapon,g_Player.x_pos.i.hi,g_Player.y_pos.i.hi);
 }
 assert(boss_frames>300&&weapon_seen==511);
 printf("PASS render stress char=%u checkpoint=%u bossframes=%u reveal=%u weapons=%x\n",character,cp,boss_frames,revealed,weapon_seen);
 X4RenderFault fault;x4_n05_render_fault(&fault);assert(!fault.count);
 x4_flow_close();x4_audio_close();return 1;
}
