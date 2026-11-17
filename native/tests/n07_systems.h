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
static void n0812_check_player_panel_snapshot(void){
 unsigned char player_before[sizeof g_Player],engine_before[sizeof engine_obj],input_before[sizeof controller_input];
 memcpy(player_before,&g_Player,sizeof g_Player);memcpy(engine_before,&engine_obj,sizeof engine_obj);
 memcpy(input_before,&controller_input,sizeof controller_input);
 X4PlayerPanel panel,again;x4_flow_player_panel(&panel);x4_flow_player_panel(&again);
 assert(!memcmp(&panel,&again,sizeof panel));
 assert(!memcmp(player_before,&g_Player,sizeof g_Player)&&!memcmp(engine_before,&engine_obj,sizeof engine_obj));
 assert(!memcmp(input_before,&controller_input,sizeof controller_input));
 assert(panel.active==(engine_obj.state==6&&g_Player.active));
 assert(panel.character==engine_obj.cur_character&&panel.stage==engine_obj.stage&&panel.area==engine_obj.substage);
 assert(panel.hp==((u8)g_Player.hp&127)&&panel.max_hp==(u8)engine_obj.max_hp);
 assert(panel.lives==engine_obj.player_lives&&panel.weapon==(u8)g_Player.weapon);
 assert(panel.weapons==(u8)g_Player.boss_flags&&panel.armor==(u8)engine_obj.armor_parts&&panel.upgrades==engine_obj.collected_upgrades);
 for(unsigned i=0;i<9;++i)assert(panel.energy[i]==(u8)g_Player.weapon_energy[i]);
 for(unsigned i=0;i<3;++i)assert(panel.tanks[i]==((u8)engine_obj.sub_tanks[i]&127));
}
static void n0813_check_radar_and_touch(unsigned character,unsigned* previous){
 X4Radar radar,again;unsigned char player_before[sizeof g_Player],engine_before[sizeof engine_obj],input_before[sizeof controller_input];
 memcpy(player_before,&g_Player,sizeof g_Player);memcpy(engine_before,&engine_obj,sizeof engine_obj);memcpy(input_before,&controller_input,sizeof controller_input);
 x4_flow_radar(&radar);x4_flow_radar(&again);
 assert(radar.active&&!memcmp(&radar,&again,sizeof radar));
 assert(!memcmp(player_before,&g_Player,sizeof g_Player)&&!memcmp(engine_before,&engine_obj,sizeof engine_obj)&&!memcmp(input_before,&controller_input,sizeof controller_input));
 assert(radar.origin_x>=0&&radar.origin_y>=0&&radar.marker_count<=80);
 assert(radar.player_x==g_Player.x_pos.i.hi-radar.origin_x&&radar.player_y==g_Player.y_pos.i.hi-radar.origin_y);
 for(unsigned y=0;y<18;++y)for(unsigned x=0;x<32;++x)assert(radar.cells[y][x]<=1);
 s16 old_x=g_Player.x_pos.i.hi,old_y=g_Player.y_pos.i.hi;
 g_Player.x_pos.i.hi=-100;g_Player.y_pos.i.hi=-100;x4_flow_radar(&again);assert(again.origin_x==0&&again.origin_y==0);
 g_Player.x_pos.i.hi=32767;g_Player.y_pos.i.hi=32767;x4_flow_radar(&again);
 assert(again.origin_x==((int)layout_width*256>512?(int)layout_width*256-512:0));
 assert(again.origin_y==((int)layout_height*256>288?(int)layout_height*256-288:0));
 g_Player.x_pos.i.hi=old_x;g_Player.y_pos.i.hi=old_y;
 /* Controlled active objects test filtering without running AI/spawn code. */
 struct MainObj saved_enemy=main_objects[0];struct ItemObj saved_item=item_objects[0];
 main_objects[0].active=1;main_objects[0].on_screen=1;main_objects[0].hp=10;main_objects[0].hurt_box=&saved_enemy;
 main_objects[0].bg_offset=g_Player.bg_offset;main_objects[0].x_pos.i.hi=g_Player.x_pos.i.hi;main_objects[0].y_pos.i.hi=g_Player.y_pos.i.hi;
 item_objects[0].active=1;item_objects[0].on_screen=1;item_objects[0].id=2;item_objects[0].state=1;
 item_objects[0].bg_offset=g_Player.bg_offset;item_objects[0].x_pos.i.hi=g_Player.x_pos.i.hi;item_objects[0].y_pos.i.hi=g_Player.y_pos.i.hi;
 x4_flow_radar(&radar);unsigned enemy=0,item=0;
 for(unsigned i=0;i<radar.marker_count;++i){assert(radar.markers[i].x>=0&&radar.markers[i].x<512&&radar.markers[i].y>=0&&radar.markers[i].y<288);enemy+=radar.markers[i].kind==1;item+=radar.markers[i].kind==2;}
 assert(enemy&&item);main_objects[0]=saved_enemy;item_objects[0]=saved_item;
 /* Terrain reads the live attribute bank and does not cache static content. */
 const X4Archive* archive=x4_flow_archive();const X4ArchiveMember* tiles=x4_archive_find(archive,0,0),*attrs=x4_archive_find(archive,1,0);
 int changed=0;
 for(unsigned row=0;row<18&&!changed;++row)for(unsigned col=0;col<32&&!changed;++col){
  unsigned x=radar.origin_x+col*16,y=radar.origin_y+row*16;
  if(x>=(unsigned)layout_width*256||y>=(unsigned)layout_height*256)continue;
  unsigned block=SP_BG_TILEMAP[g_Player.bg_offset*layout_size+(y/256)*layout_width+x/256];if(!block)continue;
  unsigned index=block*256+((y/16)&15)*16+((x/16)&15);if(index>=tiles->size/2)continue;
  unsigned tile=SP_BG_TILE_PIXELS[index]&0x3fff;if(tile>=attrs->size/4)continue;
  u32 saved=SP_BG_TILE_ATTRS[tile];SP_BG_TILE_ATTRS[tile]=(saved&~255u)|((saved&255)?0:1);
  x4_flow_radar(&again);assert(again.cells[row][col]!=radar.cells[row][col]);SP_BG_TILE_ATTRS[tile]=saved;changed=1;
 }
 assert(changed);
 int state=engine_obj.state;engine_obj.state=10;x4_flow_radar(&again);assert(!again.active&&!again.marker_count);engine_obj.state=state;
 assert(x4_flow_request_weapon(9)==-1);
 int weapon=g_Player.weapon;
 if(character){assert(x4_flow_request_weapon(1)==-1);}
 else{
  int lock=engine_obj.gameplay_locked;engine_obj.gameplay_locked=1;assert(x4_flow_request_weapon(1)==-2);engine_obj.gameplay_locked=lock;
  assert(!x4_flow_request_weapon(1));assert(g_Player.weapon==weapon);
  n07_tick(0,previous);assert(g_Player.weapon==1);
  assert(!x4_flow_request_weapon(6));n07_tick(0,previous);assert(g_Player.weapon==6);
  assert(g_Player.shot_types[0]==6);unsigned laser=0;
  for(unsigned i=0;i<COUNT(weapon_objects);++i)laser+=weapon_objects[i].active&&weapon_objects[i].id==6;
  assert(laser); /* Equipping Aiming Laser must initialize its real objects. */
  assert(!x4_flow_request_weapon(0));n07_tick(0,previous);assert(g_Player.weapon==0);
 }
 puts("PASS radar: deterministic read-only snapshot, live terrain, markers, inactive view and guarded touch weapon selection");
}
static int n07_system(const char* root,const char* mode,unsigned character){
 if(!strcmp(mode,"debugloadout")){
  extern u8 D_800F48EC[8];
  x4_audio_init("../../full-audio/music","../../full-audio/sfx");
  assert(!x4_flow_begin_debug(root,character,1,0));
  x4_video_enabled=1;x4_video_viewport(32,24,256,192);
  unsigned previous=0;
  n0812_check_player_panel_snapshot();
  assert(engine_obj.max_hp==48&&engine_obj.collected_upgrades==0xF0FF);
  assert((u8)engine_obj.sub_tanks[0]==0xA0&&(u8)engine_obj.sub_tanks[1]==0xA0&&engine_obj.sub_tanks[2]==32);
  assert(engine_obj.armor_parts==(character?0:15)&&engine_obj.arm_type==(character?0:2));
  assert(engine_obj.palette_flags==0&&engine_obj.unk5F==0);
  for(unsigned tick=0;tick<600;++tick)n07_tick(0,&previous);
  assert(engine_obj.state==6&&!engine_obj.gameplay_locked);
  n0812_check_player_panel_snapshot();
  n0813_check_radar_and_touch(character,&previous);
  assert((u8)g_Player.boss_flags==255&&g_Player.hp==48);
  assert(g_Player.armor_parts==(character?0:15));
  for(unsigned i=0;i<16;++i)assert(g_Player.weapon_energy[i]==48);
  if(!character){
   unsigned seen=1u<<g_Player.weapon;
   for(unsigned i=0;i<9;++i){
    n07_tick(512,&previous);for(unsigned t=0;t<10;++t)n07_tick(0,&previous);
    seen|=1u<<g_Player.weapon;
   }
   assert(seen==0x1FF);
  }
  n07_tick(1u<<16,&previous);for(unsigned t=0;t<180;++t)n07_tick(0,&previous);
  assert(bar_object.state||bar_object.unk5);
  if(character){for(unsigned i=0;i<8;++i)assert(bar_object.options.items[D_800F48EC[i]]);}
  else{for(unsigned i=2;i<10;++i)assert(bar_object.options.items[i]);}
  assert(bar_object.options.fields.unk20&&bar_object.options.fields.unk21&&bar_object.options.fields.unk22&&bar_object.options.fields.unk25);
  n07_tick(1u<<16,&previous);for(unsigned t=0;t<180;++t)n07_tick(0,&previous);
  assert(!bar_object.state&&!bar_object.unk5);
  /* The loadout is not invincibility or continuous refill. */
  g_Player.hp=45;g_Player.weapon_energy[0]=17;engine_obj.sub_tanks[0]=(s8)0x9C;
  for(unsigned t=0;t<60;++t)n07_tick(0,&previous);
  n0812_check_player_panel_snapshot();
  assert(g_Player.hp==45&&g_Player.weapon_energy[0]==17&&(u8)engine_obj.sub_tanks[0]==0x9C);
  engine_obj.state=5;engine_obj.unk1=0;engine_obj.unk1E=0;
  for(unsigned t=0;t<600;++t)n07_tick(0,&previous);
  assert(engine_obj.state==6&&(u8)g_Player.boss_flags==255&&g_Player.hp==48);
  assert(g_Player.armor_parts==(character?0:15)&&(u8)engine_obj.sub_tanks[0]==0x9C);
  assert(engine_obj.palette_flags==0);
  x4_flow_close();x4_audio_close();x4_audio_init("../../full-audio/music","../../full-audio/sfx");
  assert(!x4_flow_new_game(root));
  assert(engine_obj.max_hp==32&&!engine_obj.armor_parts&&!engine_obj.collected_upgrades);
  assert(!x4_flow_begin(root,character,1,0));previous=0;
  for(unsigned t=0;t<600;++t)n07_tick(0,&previous);
  n0812_check_player_panel_snapshot();
  assert(!g_Player.boss_flags&&!g_Player.armor_parts&&g_Player.hp==32&&!engine_obj.collected_upgrades);
  x4_flow_close();x4_audio_close();
  puts("PASS debug full loadout, weapon cycling/techniques, pause, finite resources, respawn and normal campaign reset");return 1;
 }
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
