/* Host stress using the original stage/enemy/weapon logic. Test-only health,
 * checkpoint and position resets keep the same stage-4 corridor under load;
 * this is renderer qualification, not an automatic campaign playthrough. */
#include "main/player_weapon.h"
static int n089_frost_system(const char* root){
 unsigned previous=0,weapon_seen=0,draws=0,shared=0,aliases_seen=0;
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");assert(!x4_flow_begin_debug(root,0,4,0));
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);
 for(unsigned tick=0;tick<9000;++tick){
  unsigned held=0;
  if(tick==300){engine_obj.checkpoint=1;engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;}
  if(tick>900){
   g_Player.hp=48;g_Player.invincibility_timer=120;for(unsigned i=0;i<16;++i)g_Player.weapon_energy[i]=48;
   unsigned w=(tick/500)%9;weapon_seen|=1u<<w;
   if(g_Player.weapon!=w){g_Player.weapon=w;player_reset_charge_and_weapon(&g_Player);player_update_shot_types(&g_Player);}
   if(tick==901||g_Player.y_pos.i.hi>940){g_Player.x_pos.i.hi=4500;g_Player.y_pos.i.hi=907;g_Player.y_vel.val=0;}
   if(g_Player.x_pos.i.hi<4500)held|=X4_RIGHT;else if(g_Player.x_pos.i.hi>4700)held|=X4_LEFT;
   if(tick%240<200)held|=X4_SHOOT;
   if(tick%120<30)held|=X4_JUMP;if(tick%90<25)held|=X4_DASH;
  }
  assert(!x4_flow_step(held,held&~previous));previous=held;
  if(tick%2==0){
   x4_audio_update();int16_t pcm[630];x4_audio_read(pcm,630);
   int result=x4_flow_draw(NULL);X4VideoStats v=x4_video_stats();++draws;shared+=v.shared;aliases_seen+=v.alias_hits;
   if(result){X4RenderFault f;x4_n05_render_fault(&f);fprintf(stderr,"FAIL tick=%u x=%d y=%d weapon=%u reason=%u id=%d pose=%u commands=%u hits=%u misses=%u palettes=%u overflow=%u\n",tick,g_Player.x_pos.i.hi,g_Player.y_pos.i.hi,g_Player.weapon,f.reason,f.id,f.pose,v.commands,v.hits,v.misses,v.palette_uploads,v.overflow);return 2;}
   if(tick%1200==0)printf("PROBE tick=%u x=%d y=%d weapon=%u commands=%u misses=%u\n",tick,g_Player.x_pos.i.hi,g_Player.y_pos.i.hi,g_Player.weapon,v.commands,v.misses);
   int dirty;uint16_t slots[X4_VIDEO_TILES];unsigned n=x4_video_dirty_tiles(slots);for(unsigned i=0;i<n;++i)x4_video_rgba_texture(slots[i],&dirty);
  }
 }
 assert(weapon_seen==511&&draws==4500);
 x4_flow_close();x4_audio_close();printf("PASS stage4 Volcano X cp1, 9000 logic ticks, 4500 draws, all 9 weapons; shared=%u aliases=%u\n",shared,aliases_seen);return 0;
}
