/* N07 integration checks: original stage events and persistent game menus. */
#include "main/menu.h"
#include "x4_save.h"
static void n07_tick(unsigned held,unsigned* previous){
 assert(!x4_flow_step(held,held&~*previous));*previous=held;
 x4_audio_update();int16_t pcm[315];x4_audio_read(pcm,315);
 assert(!x4_audio_errors());assert(!x4_flow_draw(screen));
 assert(!x4_video_stats().overflow);
}
static void n07_menu_image(const char* kind,unsigned character){
 unsigned n,foreground=0;const X4VideoCommand* cmds=x4_video_commands(&n);
 for(unsigned i=0;i<n;++i)if(cmds[i].screen_space&&cmds[i].order== -8193)++foreground;
 assert(foreground>10);char p[64];snprintf(p,sizeof p,"menu_%s_slot_%u.ppm",kind,character);n06_preview(p);
}
static int n07_system(const char* root,const char* mode,unsigned character){
 if(!strcmp(mode,"nocutscenes")){
  extern void func_80018000(arg_u8);
  x4_audio_init("../../full-audio/music","../../full-audio/sfx");
  assert(!x4_flow_begin(root,character,0,0));x4_video_enabled=1;
  unsigned previous=0;for(unsigned t=0;t<600;++t)n07_tick(0,&previous);
  X4FlowState before,after;x4_flow_state(&before);
  int track=x4_audio_track();unsigned held=controller_input.held,pressed=controller_input.pressed;
  /* Every original movie ID must return immediately even without a movies
   * directory. Keep tasks, controls and current music untouched. */
  for(unsigned id=0;id<11;++id){
   func_80018000((arg_u8)id);x4_flow_state(&after);
   assert(before.frames==after.frames&&before.state==after.state&&before.substate==after.substate);
   assert(before.stage==after.stage&&before.area==after.area&&before.character==after.character);
   assert(controller_input.held==held&&controller_input.pressed==pressed);
   assert(x4_audio_track()==track&&!x4_audio_errors());
  }
  for(unsigned t=0;t<120;++t)n07_tick(0,&previous);
  x4_flow_state(&after);assert(after.frames==before.frames+120&&!after.load_errors&&!after.draw_errors);
  x4_flow_close();x4_audio_close();
  puts("PASS all 11 cutscene calls skipped immediately; music, controls and gameplay continue");return 1;
 }
 if(!strcmp(mode,"titlemenu")){
  x4_audio_init("../../full-audio/music","../../full-audio/sfx");assert(!x4_flow_title(root));
  x4_video_enabled=1;x4_video_viewport(32,24,256,192);unsigned previous=0;int menu=0,selection=0,game=0;
  for(unsigned tick=0;tick<2700;++tick){
   unsigned held=tick%60==0&&tick>900?X4_JUMP:0;if(tick==600)held=1u<<16;
   if(engine_obj.state==1&&engine_obj.cur_character!=character)held=tick%10==0?X4_RIGHT:0;
   n07_tick(held,&previous);
   if(game_info.unk0==6&&game_info.mode==1)menu=1;
   if(engine_obj.state==1)selection=1;
   if(engine_obj.state==6&&!engine_obj.gameplay_locked)game=1;
   if(tick==480||tick==840){char p[64];snprintf(p,sizeof p,"title_%u_%u.ppm",character,tick);n06_preview(p);}
  }
  assert(menu&&selection&&game&&engine_obj.cur_character==character);
  x4_flow_close();x4_audio_close();puts("PASS animated title, original new-game menu, character selection, briefing and intro gameplay");return 1;
 }
 if(strcmp(mode,"traincars")&&strcmp(mode,"savecycle"))return 0;
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin(root,character,!strcmp(mode,"traincars")?8:1,!strcmp(mode,"traincars")?1:0));
 x4_video_enabled=1;x4_video_viewport(32,24,256,192);unsigned previous=0;
 for(unsigned t=0;t<600;++t)n07_tick(0,&previous);
 if(!strcmp(mode,"traincars")){
  extern void func_800C580C(struct ItemObj*);
  static const unsigned starts[]={4,4,4,11,13,15,19},lengths[]={5,5,5,2,2,3,3};
  static const unsigned char rows[7][5]={{0x3e,0x3f,0x43,0xb,0xc},{8,9,0x44,0x41,0x42},{0x3e,0x3f,0x40,0x41,0x42},{0x45,0x46},{0x47,0x48},{0x49,0x4a,0x4b},{0x4c,0x4d,0x4e}};
  assert(SP_BG_TILEMAP==D_80141BE8);
  for(unsigned type=0;type<7;++type){
   x4_flow_copy_layout();unsigned start=layout_width+starts[type];unsigned char old[5];memcpy(old,SP_BG_TILEMAP+start,lengths[type]);
   struct ItemObj gate={0};gate.unk2=type;gate.x_pos.i.hi=1168;gate.y_pos.i.hi=400;gate.unk7C.timer=200;
   /* Start just after the confirmed hit, then run the original explosion
    * countdown. Do not write the map or call its apply routine from here. */
   for(unsigned tick=0;tick<169;++tick){++main_bss_state.frame_counter;func_800C580C(&gate);}
   assert(!memcmp(old,SP_BG_TILEMAP+start,lengths[type]));
   ++main_bss_state.frame_counter;func_800C580C(&gate);
   assert(!memcmp(rows[type],SP_BG_TILEMAP+start,lengths[type]));
   assert(memcmp(old,SP_BG_TILEMAP+start,lengths[type]));
   for(unsigned i=0;i<lengths[type];++i)assert(get_layout_screen(0,(starts[type]+i)*256-background_objects[0].x_pos.i.hi,256-background_objects[0].y_pos.i.hi)==rows[type][i]);
   background_objects[0].x_pos.i.hi=(starts[type]+1)*256; background_objects[0].y_pos.i.hi=256;
   const X4Archive* a=x4_flow_archive();assert(!x4_n05_draw_scene(screen,a,a,0,0));
   if(type==0){char p[64];snprintf(p,sizeof p,"train_car_broken_%u.ppm",character);n06_preview(p);}
   printf("PASS train car variant=%u countdown=170 map/draw/collision share changed blocks\n",type);
  }
  x4_flow_close();x4_audio_close();return 1;
 }
 const char* save_root=getenv("X4_SAVE_TEST_ROOT");assert(save_root&& !x4_save_set_root(save_root));
 engine_obj.armor_parts=character?0:9;engine_obj.arm_type=1;engine_obj.palette_flags=1;engine_obj.max_hp=40;engine_obj.unk5F=1;
 engine_obj.collected_upgrades=7;engine_obj.state=9;engine_obj.unk1=engine_obj.unk2=engine_obj.unk3=0;
 unsigned char readback[8192];int written=0;
 for(unsigned tick=0;tick<3000;++tick){
  unsigned held=tick%40==0?X4_JUMP:0;
  n07_tick(held,&previous);
  if(tick==160)n07_menu_image("save",character);
  if(tick%40==0)fprintf(stderr,"SAVE tick=%u engine=%d/%d/%d cursor=%u op=%u result=%u\n",tick,engine_obj.state,engine_obj.unk1,engine_obj.unk2,main_bss_state.transition.selection,memcard_menu.operation,memcard_menu.result);
  if(!mmx4_pc_card_read(0,readback,sizeof readback)){written=1;fprintf(stderr,"SAVED tick=%u char=%u armor=%u\n",tick,readback[0x200],readback[0x203]);break;}
 }
 assert(written);
 x4_flow_close();x4_audio_close();x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 D_800F1D90.save.character=255;assert(!x4_flow_title(root));previous=0;int selected=0,loaded=0;
 for(unsigned tick=0;tick<5000;++tick){
  unsigned held=0;if(tick==600)held=1u<<16;
  if(tick>900&&tick%40==0){
   if(game_info.unk0==6&&game_info.mode==1&&!selected){held=8;selected=1;}else held=X4_JUMP;
  }
  n07_tick(held,&previous);
  if(tick==1100)n07_menu_image("load",character);
  if(tick%60==0)fprintf(stderr,"LOAD tick=%u info=%d/%d engine=%d/%d/%d cursor=%u char=%d\n",tick,game_info.unk0,game_info.mode,engine_obj.state,engine_obj.unk1,engine_obj.unk2,main_bss_state.transition.selection,engine_obj.cur_character);
  if(engine_obj.state==6&&!engine_obj.gameplay_locked&&engine_obj.max_hp==40&&engine_obj.cur_character==character){loaded=1;break;}
 }
 assert(loaded&&engine_obj.palette_flags==1&&engine_obj.armor_parts==(character?0:9)&&engine_obj.collected_upgrades==7&&g_Player.armor_parts==(character?0:9));
 x4_flow_close();x4_audio_close();puts("PASS original save menu, close session, original title/load menu, campaign and upgrades restored");return 1;
}
