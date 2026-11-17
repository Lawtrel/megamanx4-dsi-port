/* Debug-only accelerated inspection of the real original ending. */
static int n09_debug_system(const char* root,const char* mode,unsigned character){
 if(strcmp(mode,"debugcredits"))return 0;
 x4_audio_init("../../full-audio/music","../../full-audio/sfx");
 assert(!x4_flow_begin(root,character,1,0));x4_video_enabled=1;x4_video_viewport(32,24,256,192);
 unsigned previous=0;for(unsigned tick=0;tick<300;++tick)n07_tick(0,&previous);
 /* Exercise hitkill guards without scheduling a fabricated actor's AI. */
 struct EngineObj saved_engine=engine_obj;struct MainObj saved=main_objects[0];
 struct MainObj* boss=&main_objects[0];memset(boss,0,sizeof *boss);
 engine_obj.state=6;engine_obj.stage=12;engine_obj.substage=1;engine_obj.enable_boss=1;engine_obj.gameplay_locked=0;engine_obj.boss_ptr=boss;
 boss->active=1;boss->state=1;boss->id=0x44;boss->hp=48;
 x4_flow_debug_sigma_hitkill(1);assert(x4_flow_debug_sigma_hitkill_enabled()&&boss->hp==1);
 boss->hp=48;x4_flow_debug_sigma_hitkill(0);assert(boss->hp==48);
 boss->state=0;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==48);
 boss->state=1;boss->id=0x4a;boss->ext.main_74.target=0;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==48);
 boss->ext.main_74.target=1;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==1);
 boss->hp=48;boss->id=1;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==48);
 boss->id=0x44;engine_obj.stage=1;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==48);
 engine_obj.stage=12;engine_obj.gameplay_locked=1;x4_flow_debug_sigma_hitkill(1);assert(boss->hp==48);
 main_objects[0]=saved;engine_obj=saved_engine;
 assert(!x4_flow_debug_credits());assert(!x4_flow_debug_sigma_hitkill_enabled());
 assert(engine_obj.state==11&&engine_obj.unk1==0&&engine_obj.cur_character==character);
 unsigned substates=1,staff=0,last_state=999,last_sub=999;
 for(unsigned tick=0;tick<30000;++tick){
  assert(!x4_flow_step(0,0));x4_audio_update();int16_t pcm[315];x4_audio_read(pcm,315);assert(!x4_audio_errors());
  assert(!x4_flow_draw(screen));assert(!x4_video_stats().overflow);
  if(engine_obj.state==11){assert((u8)engine_obj.unk1<8);substates|=1u<<engine_obj.unk1;}
  staff+=x4_audio_track()==0;
  if(last_state!=(u8)engine_obj.state||last_sub!=(u8)engine_obj.unk1){printf("CREDITS tick=%u character=%u state=%u/%u stage=%u/%u alpha=%u track=%d\n",tick,character,(u8)engine_obj.state,(u8)engine_obj.unk1,(u8)engine_obj.stage,(u8)engine_obj.substage,x4_video_alpha(),x4_audio_track());fflush(stdout);last_state=(u8)engine_obj.state;last_sub=(u8)engine_obj.unk1;}
  if(tick==900&&x4_video_enabled){char name[64];snprintf(name,sizeof name,"credits_%u.ppm",character);n06_preview(name);}
  if((substates&128)&&engine_obj.state!=11){printf("CREDITS exit tick=%u state=%u\n",tick,(u8)engine_obj.state);break;}
 }
 printf("CREDITS summary character=%u states=%x staff=%u engine=%u/%u alpha=%u\n",character,substates,staff,(u8)engine_obj.state,(u8)engine_obj.unk1,x4_video_alpha());
 assert(substates==255&&staff);
 /* The original native scheduler closes slot0 after scheduling its own
  * replacement in that same slot. Post-credits currently remains black;
  * record that limit instead of pretending a title/X5 transition works. */
 assert(engine_obj.state==11&&engine_obj.unk1==7&&x4_video_alpha()==255);
 x4_flow_close();x4_audio_close();assert(!x4_flow_debug_sigma_hitkill_enabled());
 puts("PASS debug Sigma one-hit guards, clean original credits for selected character and session reset");return 1;
}
