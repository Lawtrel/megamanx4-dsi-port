#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <calico/nds/scfg.h>
#include <maxmod9.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/iosupport.h>
#include "x4_flow.h"
#include "x4_audio.h"
#include "x4_save.h"
#include "x4_n05_platform.h"
#include "x4_n05_actors.h"
#include "build_id.h"
#include "x4_video.h"
#include "x4_arm7_audio.h"
#include "x4_sfx.h"
#include "x4_controls.h"
#include "x4_lower_ui.h"
#include <calico/system/tick.h>
_Static_assert(KEY_A==X4_NDS_A&&KEY_B==X4_NDS_B&&KEY_SELECT==X4_NDS_SELECT&&KEY_START==X4_NDS_START&&
 KEY_RIGHT==X4_NDS_RIGHT&&KEY_LEFT==X4_NDS_LEFT&&KEY_UP==X4_NDS_UP&&KEY_DOWN==X4_NDS_DOWN&&
 KEY_L==X4_NDS_L&&KEY_R==X4_NDS_R&&KEY_X==X4_NDS_X&&KEY_Y==X4_NDS_Y,"Native controls must match the SDK keypad ABI");
#ifdef X4_N06_GPU
void x4_gpu_init(void);void x4_gpu_prepare(void);void x4_gpu_present(void);unsigned x4_gpu_words(void);
unsigned x4_gpu_upload_batches(void);unsigned x4_gpu_upload_tiles(void);
#define VERSION "BETA 9"
#define LOG_PREFIX "native_n09"
#else
#define VERSION "N05"
#define LOG_PREFIX "native_n05_flow"
#endif

/* Original C game logic on ARM9. No PS1 CPU, GPU, SPU or BIOS runtime. */
#ifdef X4_N06_GPU
/* All drawing emits GPU commands in this build. */
static uint16_t* const pixels=NULL;
#else
static uint16_t pixels[256*192] __attribute__((aligned(32)));
#endif
static volatile unsigned vblanks;
static unsigned character,stage,area,muted,audio_open,campaign=1,show_selector;
static unsigned lower_graphical,lower_valid;
static unsigned lower_updates,lower_skips,lower_tiles,lower_total_us,lower_max_us;
static unsigned lower_previous_mode,lower_previous_muted,lower_previous_selection;
static X4PlayerPanel lower_previous_panel;
static X4Radar lower_previous_radar;
static unsigned char lower_cache[X4_LOWER_TILE_BYTES] __attribute__((aligned(4)));
static void lower_console(void){
 consoleInit(NULL,0,BgType_Text4bpp,BgSize_T_256x256,15,0,false,true);
 lower_graphical=0;lower_valid=0;
}
static int lower_same_radar(const X4Radar* a,const X4Radar* b){
 if(a->active!=b->active)return 0;
 if(!a->active)return 1;
 if(a->player_x!=b->player_x||a->player_y!=b->player_y||a->marker_count!=b->marker_count||
    memcmp(a->cells,b->cells,sizeof a->cells))return 0;
 /* The marker struct may have padding. Compare only what is displayed. */
 for(unsigned i=0;i<a->marker_count&&i<X4_RADAR_MAX_MARKERS;++i){
  const X4RadarMarker* x=&a->markers[i];const X4RadarMarker* y=&b->markers[i];
  if(x->x!=y->x||x->y!=y->y||x->kind!=y->kind)return 0;
 }
 return 1;
}
static void lower_update(void){
 unsigned mode=x4_lower_mode(),selection=x4_lower_selection();
 if(mode==X4_LOWER_DEBUG)return;
 uint64_t started=tickGetCount();
 X4PlayerPanel player;X4Radar radar;
 if(mode==X4_LOWER_MAP)x4_flow_radar(&radar);
 else x4_flow_player_panel(&player);
 if(lower_valid&&mode==lower_previous_mode){
  int same=mode==X4_LOWER_MAP?lower_same_radar(&radar,&lower_previous_radar):
   (muted==lower_previous_muted&&selection==lower_previous_selection&&
    !memcmp(&player,&lower_previous_panel,sizeof player));
  if(same){++lower_skips;return;}
 }
 const unsigned char* image=x4_lower_render(&player,&radar,muted,(const unsigned char*)consoleGetDefault()->font.gfx);
 if(!lower_graphical){
  REG_BG0CNT_SUB=BG_COLOR_16|BG_MAP_BASE(15)|BG_TILE_BASE(0);
  volatile uint16_t* map=BG_MAP_RAM_SUB(15);
  for(unsigned i=0;i<32*24;++i)map[i]=i;
  for(unsigned i=0;i<16;++i)BG_PALETTE_SUB[i]=x4_lower_palette()[i];
  lower_graphical=1;lower_valid=0;
 }
 for(unsigned i=0;i<X4_LOWER_TILE_BYTES;i+=32)if(!lower_valid||memcmp(image+i,lower_cache+i,32)){
  volatile uint16_t* dest=(volatile uint16_t*)((unsigned char*)BG_GFX_SUB+i);
  const uint16_t* source=(const uint16_t*)(image+i);
  for(unsigned j=0;j<16;++j)dest[j]=source[j];
  memcpy(lower_cache+i,image+i,32);++lower_tiles;
 }
 if(mode==X4_LOWER_MAP)lower_previous_radar=radar;
 else lower_previous_panel=player;
 lower_previous_mode=mode;lower_previous_muted=muted;lower_previous_selection=selection;
 lower_valid=1;++lower_updates;
 unsigned us=(tickGetCount()-started)*1000000/TICK_FREQ;lower_total_us+=us;if(us>lower_max_us)lower_max_us=us;
}
static void lower_screen_begin(void){
 if(x4_lower_mode()==X4_LOWER_DEBUG){
  lower_console();
  iprintf("\x1b[2J\x1b[0;0HDIAGNOSTICO\x1b[17;0HSigma 1 golpe: %s\x1b[18;0H[FASE]\x1b[18;11H[SIGMA1H]\x1b[18;22H[CREDITOS]\x1b[20;0HFase/creditos: sai da partida.\x1b[22;0H[ PAINEL ] [ DEBUG ] [ MAPA ]",x4_flow_debug_sigma_hitkill_enabled()?"ON ":"OFF");
 }else lower_update();
}
static FILE* log_file;
static char log_path[128];
/* Ordinary samples fit in RAM until the next FAT directory update. */
static char log_buffer[8192];
enum {PROFILE_LOG_SYNC_INTERVAL=5};
static volatile unsigned diagnostic_phase;
static X4FlowState diagnostic_state;
static void commit_log(void){
 /* fclose commits the directory entry and survives a hardware reset. */
 if(log_file){
  fclose(log_file);
  log_file=fopen(log_path,"a");
  if(log_file)setvbuf(log_file,log_buffer,_IOFBF,sizeof log_buffer);
 }
}
static void crash_handler(ExcptContext* ctx,unsigned flags){
 /* Never access FAT/audio from the exception context. Photograph this. */
 lower_console();
 iprintf("\x1b[2JX4 " VERSION " - falha ARM9\nFase execucao:%u flags:%x\nPC:%08lx LR:%08lx\nSP:%08lx CPSR:%08lx\nFase:%d/%d X:%d Y:%d\nTick:%u\nFotografe esta tela.\n",diagnostic_phase,flags,
  (unsigned long)ctx->r[15],(unsigned long)ctx->r[14],(unsigned long)ctx->r[13],(unsigned long)ctx->cpsr,
  diagnostic_state.stage,diagnostic_state.area,diagnostic_state.x,diagnostic_state.y,diagnostic_state.frames);
 for(unsigned i=0;i<12;i+=2)iprintf("R%02u:%08lx R%02u:%08lx\n",i,(unsigned long)ctx->r[i],i+1,(unsigned long)ctx->r[i+1]);
 for(;;)__asm__ volatile("nop");
}
static const char* save_root;
static void on_vblank(void){++vblanks;}
static mm_word audio_callback(mm_word frames,mm_addr dest,mm_stream_formats format){
 (void)format;x4_audio_read_stereo(dest,frames);
 if(muted)memset(dest,0,(size_t)frames*2*sizeof(int16_t));
 return frames;
}
static void audio_stop(void){if(audio_open){mmStreamClose();audio_open=0;}x4_audio_close();}
static void audio_start(void){
 mm_stream stream={0};stream.sampling_rate=18900;stream.buffer_length=4096;
 stream.callback=audio_callback;stream.format=MM_STREAM_16BIT_STEREO;
 stream.timer=MM_TIMER0;stream.manual=true;mmStreamOpen(&stream);audio_open=1;
}
static void initialize_logs(void){
 const char* roots[]={"sd:/mmx4","fat:/mmx4"};char name[128];
 for(unsigned d=0;d<2&&!save_root;++d){
  if(FindDevice(roots[d])<0)continue;
  mkdir(roots[d],0777);struct stat info;if(stat(roots[d],&info)||!S_ISDIR(info.st_mode))continue;
  save_root=roots[d];
  for(unsigned i=0;i<1000;++i){
   snprintf(name,sizeof name,"%s/" LOG_PREFIX "_%03u.log",roots[d],i);
   FILE* exists=fopen(name,"rb");if(exists){fclose(exists);continue;}
   strcpy(log_path,name);log_file=fopen(name,"w");break;
  }
 }
 if(save_root)x4_save_set_root(save_root);
 if(log_file){
  setvbuf(log_file,log_buffer,_IOFBF,sizeof log_buffer);
  fprintf(log_file,VERSION "; native ARM9; DSi=%d; id=%s; build=%s %s\n",isDSiMode(),X4_BUILD_ID,__DATE__,__TIME__);
  fprintf(log_file,"PROFILE_HW mode=%s arm9_mhz=%u\n",isDSiMode()?"TWL":"NTR",
   isDSiMode()&&(REG_SCFG_CLK&SCFG_CLK_CPU_134MHz)?134u:67u);
  fprintf(log_file,"Development, incomplete qualification. All stage resources; 80 PCM tracks; native PCM effects. Cutscenes disabled; audio fidelity and hardware qualification pending.\n");commit_log();
 }
}
static void fail(const char* text,int code){
 audio_stop();lower_console();iprintf("\nFalha: %s\nCodigo: %d\n",text,code);
 if(log_file){fprintf(log_file,"FAIL %s code=%d\n",text,code);commit_log();}
 while(pmMainLoop())swiWaitForVBlank();
}
static int select_stage(void){
 lower_console();
 iprintf("\x1b[2J");
#ifndef X4_N06_GPU
 memset(pixels,0,sizeof pixels);
#endif
 while(pmMainLoop()){
  scanKeys();unsigned down=keysDown();
  if(down&KEY_SELECT){campaign^=1;iprintf("\x1b[2J");}
  if(down&(KEY_LEFT|KEY_RIGHT))character^=1;
  if(down&KEY_UP)stage=(stage+12)%13;
  if(down&KEY_DOWN)stage=(stage+1)%13;
  if(down&(KEY_L|KEY_R))area^=1;
  if(stage==9||stage==10)area=0;
  if(down&KEY_START)return 1;
  if(campaign)iprintf("\x1b[0;0HMEGA MAN X4 - " VERSION "\nIntegracao nativa\n\n"
   "MENU ORIGINAL\n\nSTART: abrir titulo\n\n"
   "Novo jogo e carregar campanha.\n"
   "Escolha X/Zero na tela do jogo.\nB confirma e avanca dialogos.\n\n"
   "SELECT: escolher fase de teste\n\nB:pulo A:dash Y:ataque\nX:especial L/R:arma\n"
   "START:menu SELECT:giga\n\nL+R+SELECT:voltar aqui\nSELECT+X:alternar som\n\n"
   "Versao em desenvolvimento.\nCutscenes desativadas.\n");
  else iprintf("\x1b[0;0HMEGA MAN X4 - " VERSION "\nTeste de integracao nativa\n\n"
   "Personagem: %-4s            \nFase: %02u  Area: %u            \n\n"
   "Esq/dir: X ou Zero\nCima/baixo: fase\nL/R: area\nSTART:jogar SELECT:menu original\n\n"
   "B: pulo  A: dash  Y: ataque\nX: arma especial\nL/R: trocar arma\nSTART:menu SELECT:giga\n\n"
   "L+R+SELECT: voltar aqui\nSELECT+X: ligar/desligar som\n\n"
   "DEBUG: armas/upgrades completos.\nCutscenes desativadas.\n",character?"Zero":"X",stage,area+1);
  swiWaitForVBlank();
 }
 return 0;
}
int main(int argc,char** argv){
 (void)argc;(void)argv;
#ifdef X4_N06_GPU
 /* H has room for the console font and a map at 30 KiB. Free C for textures. */
 videoSetModeSub(MODE_0_2D);vramSetBankH(VRAM_H_SUB_BG);
 consoleInit(NULL,0,BgType_Text4bpp,BgSize_T_256x256,15,0,false,true);
#else
 consoleDemoInit();
#endif
 lcdMainOnTop();
 setExceptionHandler(crash_handler);
 int dsi=isDSiMode();
 if(dsi)setCpuClock(true);
 x4_n05_set_dsi_mode(dsi);
 x4_sfx_set_cache_limit(dsi?4*1024*1024:96*1024);
 if(!nitroFSInit(NULL)){fail("Nao abriu os dados internos",-2);return 1;}
 if(fatInitDefault())initialize_logs();
 if(x4_lower_load_atlas("nitro:/ui/lower-ui.dat")){fail("Atlas do painel indisponivel",-3);return 1;}
#ifdef X4_N06_GPU
 x4_gpu_init();
#else
 videoSetMode(MODE_5_2D);vramSetBankA(VRAM_A_MAIN_BG);int bg=bgInit(3,BgType_Bmp16,BgSize_B16_256x256,0,0);
#endif
 mm_ds_system system={0};mmInit(&system);x4_arm7_audio_init();irqSet(IRQ_VBLANK,on_vblank);irqEnable(IRQ_VBLANK);
 for(;;){
  audio_stop();x4_flow_close();
#ifndef X4_N06_GPU
 memset(pixels,0,sizeof pixels);
#endif
#ifdef X4_N06_GPU
  x4_video_begin(0);swiWaitForVBlank();x4_gpu_present();
#else
  DC_FlushRange(pixels,sizeof pixels);dmaCopy(pixels,bgGetGfxPtr(bg),sizeof pixels);
#endif
  if(show_selector&&!select_stage())break;
  show_selector=0;
  x4_audio_init("nitro:/music","nitro:/sfx");
  lower_console();iprintf("\x1b[2J");
  if(log_file){fprintf(log_file,"BEGIN campaign=%u character=%u stage=%u area=%u debug_loadout=%u\n",campaign,character,stage,area,!campaign);commit_log();}
  int error=campaign?x4_flow_title("nitro:/content"):x4_flow_begin_debug("nitro:/content",character,stage,area);
  if(error){fail("Carregamento",error);break;}
  audio_start();unsigned previous=0,frames=0,last=vblanks,gfx_errors=0,logged_faults=0,logged_video_faults=x4_video_last_fault().count;
  uint64_t logic_ticks=0,draw_ticks=0,audio_ticks=0,wait_ticks=0,present_ticks=0,prepare_ticks=0;
  uint64_t simulation_clock=tickGetCount();unsigned last_simulation=0,dropped_ticks=0;
  /* Present-to-present intervals include last interval's screen/log work.
   * Phase averages alone hid the stalls produced by diagnostics and FAT. */
  uint64_t previous_present=0,frame_sum=0;
  unsigned previous_field=0,frame_samples=0,frame_max_us=0,work_max_us=0,late_frames=0;
  unsigned frame_histogram[101]={0},diagnostic_us=0;
  unsigned log_samples=0,log_sync_us=0,logged_invalid_sequences=0;
  int previous_log_stage=-1,previous_log_area=-1;
  lower_updates=lower_skips=lower_tiles=lower_total_us=lower_max_us=0;
  unsigned panel_field=vblanks;x4_lower_reset();lower_screen_begin();
  while(pmMainLoop()){
   uint64_t frame_start=tickGetCount();
   diagnostic_phase=1;x4_flow_state(&diagnostic_state);
   uint64_t t0=tickGetCount();x4_audio_update();if(audio_open)mmStreamUpdate();audio_ticks+=tickGetCount()-t0;
   scanKeys();unsigned held=keysHeld(),down=keysDown();
   if((held&(KEY_L|KEY_R|KEY_SELECT))==(KEY_L|KEY_R|KEY_SELECT)){
    show_selector=1;campaign=0;break;
   }
   if((held&(KEY_SELECT|KEY_X))==(KEY_SELECT|KEY_X)&&(down&(KEY_SELECT|KEY_X))){muted^=1;x4_arm7_audio_mute(muted);}
   if(down&KEY_TOUCH){
    touchPosition touch;touchRead(&touch);X4PlayerPanel player;x4_flow_player_panel(&player);
    unsigned mode=x4_lower_mode();int weapon=x4_lower_touch(touch.px,touch.py,&player);
    if(weapon==X4_LOWER_OPEN_TEST){show_selector=1;campaign=0;break;}
    if(weapon>=0)x4_flow_request_weapon((unsigned)weapon);
    if(weapon==X4_LOWER_SIGMA_HITKILL){
     x4_flow_debug_sigma_hitkill(!x4_flow_debug_sigma_hitkill_enabled());
     iprintf("\x1b[17;0HSigma 1 golpe: %s",x4_flow_debug_sigma_hitkill_enabled()?"ON ":"OFF");
     if(log_file)fprintf(log_file,"DEBUG sigma_hitkill=%d\n",x4_flow_debug_sigma_hitkill_enabled());
    }
    if(weapon==X4_LOWER_CREDITS){
     int result=x4_flow_debug_credits();
     if(log_file){fprintf(log_file,"DEBUG credits result=%d\n",result);commit_log();}
     if(result)iprintf("\x1b[17;0HCreditos indisponiveis (%d)   ",result);
     else{
      simulation_clock=tickGetCount();last_simulation=0;previous=0;
      lower_screen_begin();
     }
    }
    if(mode!=x4_lower_mode())lower_screen_begin();
   }
   unsigned input=x4_controls_buttons(held);
   t0=tickGetCount();
   diagnostic_phase=2;
#ifdef X4_N06_GPU
   /* Keep the original 60 Hz rules when drawing takes two display fields.
    * Bound catch-up after loads so a slow frame cannot create an endless lag. */
   uint64_t now=t0,period=ticksFromHz(60);unsigned steps=(now-simulation_clock)/period;
   if(!steps){steps=1;simulation_clock=now-period;}
   if(steps>4){dropped_ticks+=steps-4;steps=4;simulation_clock=now-steps*period;}
   for(unsigned step=0;step<steps;++step){
    error=x4_flow_step(input,input&~previous);previous=input;simulation_clock+=period;if(error)break;
   }
#else
   error=x4_flow_step(input,input&~previous);previous=input;
#endif
   logic_ticks+=tickGetCount()-t0;
   diagnostic_phase=3;t0=tickGetCount();x4_audio_update();audio_ticks+=tickGetCount()-t0;
   if(error){fail("Evento de fase",error);break;}
   diagnostic_phase=4;t0=tickGetCount();gfx_errors+=x4_flow_draw(pixels);draw_ticks+=tickGetCount()-t0;
#ifdef X4_N06_GPU
   diagnostic_phase=7;t0=tickGetCount();x4_gpu_prepare();prepare_ticks+=tickGetCount()-t0;
#endif
   t0=tickGetCount();if(audio_open)mmStreamUpdate();audio_ticks+=tickGetCount()-t0;
   diagnostic_phase=5;t0=tickGetCount();swiWaitForVBlank();uint64_t frame_wait=tickGetCount()-t0;wait_ticks+=frame_wait;t0=tickGetCount();
#ifdef X4_N06_GPU
   diagnostic_phase=6;x4_gpu_present();
#else
   DC_FlushRange(pixels,sizeof pixels);dmaCopy(pixels,bgGetGfxPtr(bg),sizeof pixels);
#endif
   uint64_t presented=tickGetCount();unsigned field=vblanks;
   present_ticks+=presented-t0;++frames;
   unsigned work_us=(presented-frame_start-frame_wait)*1000000/TICK_FREQ;
   if(work_us>work_max_us)work_max_us=work_us;
   if(previous_present){
    uint64_t span=presented-previous_present;unsigned us=span*1000000/TICK_FREQ;
    frame_sum+=span;++frame_samples;if(us>frame_max_us)frame_max_us=us;
    ++frame_histogram[us/1000<100?us/1000:100];
    if(field-previous_field>2)++late_frames;
   }
   previous_present=presented;previous_field=field;
   if(vblanks-panel_field>=6){lower_update();panel_field=vblanks;}
   unsigned elapsed=vblanks-last;
   if(elapsed>=60){
    uint64_t diagnostic_start=tickGetCount();
    X4FlowState s;x4_flow_state(&s);unsigned fps=frames*60/elapsed;last=vblanks;
    unsigned logic_us=logic_ticks*1000000/TICK_FREQ/frames,draw_us=draw_ticks*1000000/TICK_FREQ/frames;
    unsigned audio_us=audio_ticks*1000000/TICK_FREQ/frames,present_us=present_ticks*1000000/TICK_FREQ/frames,wait_us=wait_ticks*1000000/TICK_FREQ/frames;
    unsigned prepare_us=prepare_ticks*1000000/TICK_FREQ/frames;
    X4VideoStats v=x4_video_stats();X4SfxStats fx=x4_sfx_stats();frames=0;logic_ticks=draw_ticks=audio_ticks=present_ticks=wait_ticks=prepare_ticks=0;
    unsigned simulation_hz=(s.frames-last_simulation)*60/elapsed;last_simulation=s.frames;
    unsigned percentile=0,cumulative=0;
    for(unsigned i=0;i<101;++i){cumulative+=frame_histogram[i];if(cumulative*100>=frame_samples*95){percentile=i==100?frame_max_us:(i+1)*1000;break;}}
    /* Static controls stay on screen; redraw only changing diagnostic rows. */
    if(x4_lower_mode()==X4_LOWER_DEBUG)iprintf("\x1b[1;0H%s Fase %02d/%d  FPS:%2u    \nHP:%2d Checkpoint:%d          "
     "\x1b[3;0HSom:%s  Faixa:%2d            \nImagem:%u Audio:%u            \nEfeitos pendentes:%u          \nARM7 FX:%u Cache:%uKB        \nLog:%s                     "
     "\x1b[9;0HX:%5d Y:%5d              \nEstado:%d/%d Bloqueio:%d     \nCPU logica:%5uus          \nDesenho:%5uus GPU:%5uus    \nTiles:%u +%u  Limite:%u     \nPrep:%5uus                 \nQuadro max:%6uus         \nP95 <=%6uus atrasos:%u    ",
     s.character?"Zero":"X",s.stage,s.area+1,fps,s.hp,s.checkpoint,muted?"OFF":"ON",x4_audio_track(),gfx_errors,x4_audio_errors(),x4_n05_missing_sfx(),fx.active,fx.bytes/1024,log_file?"ativo":"indisponivel",s.x,s.y,s.state,s.substate,s.locked,logic_us,draw_us,present_us,v.hits,v.misses,v.overflow,prepare_us,frame_max_us,percentile,late_frames);
    if(log_file){fprintf(log_file,"frame=%u ticks=%u char=%d stage=%d area=%d fps=%u x=%d y=%d hp=%d checkpoint=%d engine=%d/%d player=%d locked=%d gfxerr=%u audioerr=%u missingfx=%u track=%d\n",
     vblanks,s.frames,s.character,s.stage,s.area,fps,s.x,s.y,s.hp,s.checkpoint,s.state,s.substate,s.player_state,s.locked,gfx_errors,x4_audio_errors(),x4_n05_missing_sfx(),x4_audio_track());
     fprintf(log_file,"PROFILE logic_us=%u draw_us=%u audio_us=%u present_us=%u wait_us=%u commands=%u hits=%u misses=%u palettes=%u upload=%u overflow=%u sim_hz=%u dropped_ticks=%u\n",logic_us,draw_us,audio_us,present_us,wait_us,v.commands,v.hits,v.misses,v.palette_uploads,v.uploaded_bytes,v.overflow,simulation_hz,dropped_ticks);
     fprintf(log_file,"ARM7 voices=%u pcm_bytes=%u cache_hits=%u cache_misses=%u starts=%u\n",fx.active,fx.bytes,fx.hits,fx.misses,fx.starts);
     if(fx.invalid_sequences!=logged_invalid_sequences){
      fprintf(log_file,"SEP_INVALID count=%u resource=%02x bank=%u index=%u command=%08lx\n",
       fx.invalid_sequences,fx.invalid_resource,fx.invalid_bank,fx.invalid_index,(unsigned long)fx.invalid_command);
      logged_invalid_sequences=fx.invalid_sequences;
     }
     fprintf(log_file,"PIPELINE prepare_us=%u\n",prepare_us);
     fprintf(log_file,"FRAME_TIME samples=%u mean_us=%u max_us=%u p95_upper_us=%u over_two_fields=%u work_max_us=%u diagnostic_previous_us=%u\n",frame_samples,frame_samples?(unsigned)(frame_sum*1000000/TICK_FREQ/frame_samples):0,frame_max_us,percentile,late_frames,work_max_us,diagnostic_us);
     X4GpuTransferStats transfer=x4_gpu_transfer_stats();
     fprintf(log_file,"VRAM_WINDOW waits=%u start_max=%u restore_max=%u late=%u gpu_flags=%x underflows=%u overflows=%u total_late=%u total_waits=%u\n",transfer.waits,transfer.start_max,transfer.restore_max,transfer.late,transfer.status,transfer.underflows,transfer.overflows,transfer.total_late,transfer.total_waits);
     fprintf(log_file,"LOWER_SCREEN mode=%u updates=%u tiles=%u mean_us=%u max_us=%u skips=%u\n",x4_lower_mode(),lower_updates,lower_tiles,lower_updates?lower_total_us/lower_updates:0,lower_max_us,lower_skips);
     fprintf(log_file,"TEXTURE_SHARING shared=%u alias_hits=%u\n",v.shared,v.alias_hits);
     X4VideoFault vf=x4_video_last_fault();
     if(vf.count!=logged_video_faults){
      fprintf(log_file,"VIDEO_FAULT count=%u frame=%u reason=%u commands=%u current=%u previous=%u\n",vf.count,vf.frame,vf.reason,vf.commands,vf.current,vf.previous);
      logged_video_faults=vf.count;
     }
     X4RenderCacheStats rc;x4_n05_render_cache_stats(&rc);
     fprintf(log_file,"RENDER_CACHE bounds_hits=%u bounds_misses=%u asset_hits=%u asset_misses=%u\n",rc.bounds_hits,rc.bounds_misses,rc.asset_hits,rc.asset_misses);
     X4RenderFault fault;x4_n05_render_fault(&fault);
     if(fault.count!=logged_faults){
      fprintf(log_file,"RENDER_FAULT count=%u tick=%u reason=%u id=%d pose=%u texture=%x clut=%x bg=%d\n",fault.count,fault.frame,fault.reason,fault.id,fault.pose,fault.texture,fault.clut,fault.background);
      logged_faults=fault.count;
     }
     char debug[256];x4_flow_debug(debug,sizeof debug);fprintf(log_file,"%s\n",debug);
#ifdef X4_N06_GPU
     fprintf(log_file,"FIFO words=%u old_words=%u\n",x4_gpu_words(),21*v.commands);
     fprintf(log_file,"UPLOAD tiles=%u batches=%u\n",x4_gpu_upload_tiles(),x4_gpu_upload_batches());
#endif
     fprintf(log_file,"LOG_IO previous_sync_us=%u\n",log_sync_us);
     log_sync_us=0;
     if(++log_samples>=PROFILE_LOG_SYNC_INTERVAL||s.stage!=previous_log_stage||s.area!=previous_log_area){
      uint64_t sync_start=tickGetCount();
      commit_log();
      log_sync_us=(tickGetCount()-sync_start)*1000000/TICK_FREQ;
      log_samples=0;previous_log_stage=s.stage;previous_log_area=s.area;
     }
    }
    lower_updates=lower_skips=lower_tiles=lower_total_us=lower_max_us=0;
    diagnostic_us=(tickGetCount()-diagnostic_start)*1000000/TICK_FREQ;
    frame_sum=0;frame_samples=frame_max_us=work_max_us=late_frames=0;
    memset(frame_histogram,0,sizeof frame_histogram);
   }
  }
  if(error||!pmMainLoop())break;
  commit_log();
 }
 audio_stop();x4_flow_close();if(log_file)fclose(log_file);return 0;
}
