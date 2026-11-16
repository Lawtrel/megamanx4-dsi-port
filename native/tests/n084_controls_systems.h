#include "x4_controls.h"
static void n084_tick(unsigned held,unsigned* previous){
 g_Player.invincibility_timer=120; /* Isolate energy use from enemy damage. */
 n07_tick(held,previous);
}
static int n084_select_giga(const char* root,unsigned character){
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin_debug(root,character,1,0));x4_video_enabled=1;
 x4_video_viewport(32,24,256,192);unsigned previous=0;
 for(unsigned t=0;t<600;++t)n084_tick(0,&previous);
 assert(!engine_obj.gameplay_locked&&g_Player.weapon_energy[0]==48);
 unsigned action=x4_controls_buttons(X4_NDS_SELECT),seen=0;
 for(unsigned t=0;t<60;++t){
  n084_tick(action,&previous);
  if(character?g_Player.unk5==0x3A:g_Player.nova_strike_active)++seen;
 }
 assert(seen&&g_Player.weapon_energy[0]==(character?36:0));
 /* Holding the physical button generates one press, not repeated attacks. */
 for(unsigned t=0;t<120;++t)n084_tick(action,&previous);
 assert(g_Player.weapon_energy[0]==(character?36:0));
 n084_tick(0,&previous);
 for(unsigned t=0;t<120;++t)n084_tick(0,&previous);
 g_Player.weapon_energy[0]=48;
 unsigned mute=x4_controls_buttons(X4_NDS_SELECT|X4_NDS_X);
 unsigned leave=x4_controls_buttons(X4_NDS_SELECT|X4_NDS_L|X4_NDS_R);
 assert(!mute&&!leave);
 for(unsigned t=0;t<60;++t)n084_tick(t&1?mute:leave,&previous);
 assert(g_Player.weapon_energy[0]==48);
 x4_flow_close();x4_audio_close();x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin(root,character,1,0));previous=0;
 for(unsigned t=0;t<600;++t)n084_tick(0,&previous);
 for(unsigned t=0;t<60;++t)n084_tick(action,&previous);
 assert(g_Player.weapon_energy[0]==48&&!g_Player.nova_strike_active);
 if(character)assert(g_Player.unk5!=0x3A);
 x4_flow_close();x4_audio_close();
 puts("PASS physical SELECT activates original giga, spends energy once, respects ability requirements and consumes frontend chords");return 1;
}
