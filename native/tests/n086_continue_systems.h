/* Exercise the original death -> game over -> Continue flow, not a direct
 * restart. The menu uploads its own palette without reloading the player. */
static int n086_continue_system(const char* root,unsigned character){
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin(root,character,7,1));
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);
 unsigned previous=0;
 for(unsigned i=0;i<600;++i)n07_tick(0,&previous);
 assert(engine_obj.state==6&&g_Player.active&&g_Player.hp>0);
 uint16_t before[512];memcpy(before,SP_PALETTE,sizeof before);
 n06_preview(character?"continue_zero_before.ppm":"continue_x_before.ppm");
 engine_obj.player_lives=0;g_Player.hp=0;g_Player.state=3;
 uint16_t gameplay_palette[512];unsigned saved=0,restored=0;
 unsigned menu=0,confirmed=0,resumed=0;
 for(unsigned tick=0;tick<2400;++tick){
  unsigned held=0;
  if(engine_obj.state==8&&!engine_obj.unk1){memcpy(gameplay_palette,SP_PALETTE,sizeof gameplay_palette);saved=1;}
  if(engine_obj.state==8&&engine_obj.unk1==1&&!main_bss_state.transition.active){
   assert(main_bss_state.transition.selection==0);++menu;
   assert(memcmp(before,SP_PALETTE,sizeof before));
   held=X4_JUMP;confirmed=1;
  }
  unsigned previous_state=(u8)engine_obj.state;n07_tick(held,&previous);
  if(previous_state==8&&engine_obj.state==5){
   assert(saved);
   unsigned differences=0;for(unsigned i=0;i<512;++i)differences+=gameplay_palette[i]!=SP_PALETTE[i];
   printf("CONTINUE colors changed at menu exit=%u\n",differences);fflush(stdout);
   assert(!differences);restored=1;
  }
  if(confirmed&&engine_obj.state==6&&!engine_obj.unk1&&g_Player.active&&g_Player.hp>0){resumed=1;break;}
 }
 assert(menu&&confirmed&&resumed&&restored&&engine_obj.player_lives==2);
 for(unsigned i=0;i<600;++i)n07_tick(0,&previous);
 assert(!engine_obj.gameplay_locked&&engine_obj.unk1F&&g_Player.state==1);
 /* READY legitimately changes its text palette during the respawn. Check
  * the full borrowed palette at exit above, and stable player/HUD colors
  * again after the original READY animation has run. */
 assert(!memcmp(before,SP_PALETTE,16*sizeof *before));
 /* Original HUD uses CLUT 0x7841 (palette word 0x110); 0x100 is READY. */
 assert(!memcmp(before+0x110,SP_PALETTE+0x110,16*sizeof *before));
 n06_preview(character?"continue_zero_after.ppm":"continue_x_after.ppm");
 unsigned attacks=0;
 for(unsigned tick=0;tick<180;++tick){
  n07_tick(tick%30<6?X4_SHOOT:0,&previous);
  for(unsigned i=0;i<COUNT(weapon_objects);++i)if(weapon_objects[i].active)++attacks;
  if(character&&g_Player.unk5)++attacks;
 }
 assert(attacks);printf("CONTINUE attacks observed=%u\n",attacks);
 x4_flow_close();x4_audio_close();
 puts("PASS last-life death, original Continue, palette restoration and attacks after resuming");return 1;
}
