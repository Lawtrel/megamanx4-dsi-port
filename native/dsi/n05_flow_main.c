#include <nds.h>
#include <filesystem.h>
#include <fat.h>
#include <maxmod9.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "x4_flow.h"
#include "x4_audio.h"
#include "x4_save.h"
#include "x4_n05_platform.h"
#include "x4_n05_actors.h"
#include "build_id.h"
#include "x4_video.h"
#include "x4_arm7_audio.h"
#include "x4_sfx.h"
#include <calico/system/tick.h>
#ifdef X4_N06_GPU
void x4_gpu_init(void);void x4_gpu_prepare(void);void x4_gpu_present(void);unsigned x4_gpu_words(void);
unsigned x4_gpu_upload_batches(void);unsigned x4_gpu_upload_tiles(void);
#define VERSION "BETA 8.3"
#define LOG_PREFIX "native_n083"
#else
#define VERSION "N05"
#define LOG_PREFIX "native_n05_flow"
#endif

/* Original C game logic on ARM9. No PS1 CPU, GPU, SPU or BIOS runtime. */
static uint16_t pixels[256*192] __attribute__((aligned(32)));
static volatile unsigned vblanks;
static unsigned character,stage,area,muted,audio_open,campaign=1;
static FILE* log_file;
static char log_path[128];
static volatile unsigned diagnostic_phase;
static X4FlowState diagnostic_state;
static void commit_log(void){
 /* fclose also commits the FAT file size; fflush alone left logs empty
  * after a hardware reset. Reopen only once per profile interval. */
 if(log_file){fclose(log_file);log_file=fopen(log_path,"a");}
}
static void crash_handler(ExcptContext* ctx,unsigned flags){
 /* Never access FAT/audio from the exception context. Photograph this. */
 iprintf("\x1b[2JX4 " VERSION " - falha ARM9\nFase execucao:%u flags:%x\nPC:%08lx LR:%08lx\nSP:%08lx CPSR:%08lx\nFase:%d/%d X:%d Y:%d\nTick:%u\nFotografe esta tela.\n",diagnostic_phase,flags,
  (unsigned long)ctx->r[15],(unsigned long)ctx->r[14],(unsigned long)ctx->r[13],(unsigned long)ctx->cpsr,
  diagnostic_state.stage,diagnostic_state.area,diagnostic_state.x,diagnostic_state.y,diagnostic_state.frames);
 for(unsigned i=0;i<12;i+=2)iprintf("R%02u:%08lx R%02u:%08lx\n",i,(unsigned long)ctx->r[i],i+1,(unsigned long)ctx->r[i+1]);
 for(;;)__asm__ volatile("nop");
}
static const char* save_root;
static void on_vblank(void){++vblanks;}
static mm_word audio_callback(mm_word frames,mm_addr dest,mm_stream_formats format){
 (void)format;x4_audio_read(dest,frames);if(muted)memset(dest,0,frames*2);return frames;
}
static void audio_stop(void){if(audio_open){mmStreamClose();audio_open=0;}x4_audio_close();}
static void audio_start(void){
 mm_stream stream={0};stream.sampling_rate=18900;stream.buffer_length=4096;
 stream.callback=audio_callback;stream.format=MM_STREAM_16BIT_MONO;
 stream.timer=MM_TIMER0;stream.manual=true;mmStreamOpen(&stream);audio_open=1;
}
static void initialize_logs(void){
 const char* roots[]={"sd:/mmx4","fat:/mmx4"};char name[128];
 for(unsigned d=0;d<2&&!save_root;++d){
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
  fprintf(log_file,VERSION "; native ARM9; DSi=%d; id=%s; build=%s %s\n",isDSiMode(),X4_BUILD_ID,__DATE__,__TIME__);
  fprintf(log_file,"Development, incomplete qualification. All stage resources; 80 PCM tracks; native PCM effects. Cutscenes disabled; six sound commands and effect fidelity pending.\n");commit_log();
 }
}
static void fail(const char* text,int code){
 audio_stop();iprintf("\nFalha: %s\nCodigo: %d\n",text,code);
 if(log_file){fprintf(log_file,"FAIL %s code=%d\n",text,code);fflush(log_file);}
 while(pmMainLoop())swiWaitForVBlank();
}
static unsigned buttons(unsigned keys){
 unsigned b=0;if(keys&KEY_RIGHT)b|=1;if(keys&KEY_LEFT)b|=2;
 if(keys&KEY_UP)b|=4;
 if(keys&KEY_DOWN)b|=8;
 if(keys&KEY_Y)b|=16;
 if(keys&KEY_X)b|=32;
 if(keys&KEY_R)b|=512;
 if(keys&KEY_B)b|=128;
 if(keys&KEY_A)b|=256;
 if(keys&KEY_L)b|=1024;
 if(keys&KEY_START)b|=1u<<16;
 if(keys&KEY_SELECT)b|=1u<<17;
 return b;
}
static int select_stage(void){
 iprintf("\x1b[2J");memset(pixels,0,sizeof pixels);
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
   "START:menu do jogo\n\nL+R+SELECT:voltar aqui\nSELECT+X:alternar som\n\n"
   "Versao em desenvolvimento.\nCutscenes desativadas.\n");
  else iprintf("\x1b[0;0HMEGA MAN X4 - " VERSION "\nTeste de integracao nativa\n\n"
   "Personagem: %-4s            \nFase: %02u  Area: %u            \n\n"
   "Esq/dir: X ou Zero\nCima/baixo: fase\nL/R: area\nSTART:jogar SELECT:menu original\n\n"
   "B: pulo  A: dash  Y: ataque\nX: arma especial\nL/R: trocar arma\nSTART: menu do jogo\n\n"
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
 if(!isDSiMode()){fail("Inicie em modo DSi",-1);return 1;}
 if(!nitroFSInit(NULL)){fail("Nao abriu os dados internos",-2);return 1;}
 fatInitDefault();initialize_logs();
#ifdef X4_N06_GPU
 x4_gpu_init();
#else
 videoSetMode(MODE_5_2D);vramSetBankA(VRAM_A_MAIN_BG);int bg=bgInit(3,BgType_Bmp16,BgSize_B16_256x256,0,0);
#endif
 mm_ds_system system={0};mmInit(&system);x4_arm7_audio_init();irqSet(IRQ_VBLANK,on_vblank);irqEnable(IRQ_VBLANK);
 for(;;){
  audio_stop();x4_flow_close();memset(pixels,0,sizeof pixels);
#ifdef X4_N06_GPU
  x4_video_begin(0);swiWaitForVBlank();x4_gpu_present();
#else
  DC_FlushRange(pixels,sizeof pixels);dmaCopy(pixels,bgGetGfxPtr(bg),sizeof pixels);
#endif
  if(!select_stage())break;
  x4_audio_init("nitro:/music","nitro:/sfx");
  iprintf("\x1b[2JCarregando %s fase %02u/%u...",character?"Zero":"X",stage,area);
  if(log_file){fprintf(log_file,"BEGIN campaign=%u character=%u stage=%u area=%u debug_loadout=%u\n",campaign,character,stage,area,!campaign);commit_log();}
  int error=campaign?x4_flow_title("nitro:/content"):x4_flow_begin_debug("nitro:/content",character,stage,area);
  if(error){fail("Carregamento",error);break;}
  audio_start();unsigned previous=0,frames=0,last=vblanks,gfx_errors=0,logged_faults=0;
  uint64_t logic_ticks=0,draw_ticks=0,audio_ticks=0,wait_ticks=0,present_ticks=0,prepare_ticks=0;
  uint64_t simulation_clock=tickGetCount();unsigned last_simulation=0,dropped_ticks=0;
  iprintf("\x1b[2J");
  while(pmMainLoop()){
   diagnostic_phase=1;x4_flow_state(&diagnostic_state);
   uint64_t t0=tickGetCount();x4_audio_update();if(audio_open)mmStreamUpdate();audio_ticks+=tickGetCount()-t0;
   scanKeys();unsigned held=keysHeld(),down=keysDown();
   if((held&(KEY_L|KEY_R|KEY_SELECT))==(KEY_L|KEY_R|KEY_SELECT))break;
   if((held&KEY_SELECT)&&(down&KEY_X)){muted^=1;x4_arm7_audio_mute(muted);}
   unsigned input=buttons(held);
   if((held&(KEY_SELECT|KEY_X))==(KEY_SELECT|KEY_X))input&=~((1u<<17)|32u);
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
   diagnostic_phase=5;t0=tickGetCount();swiWaitForVBlank();wait_ticks+=tickGetCount()-t0;t0=tickGetCount();
#ifdef X4_N06_GPU
   diagnostic_phase=6;x4_gpu_present();
#else
   DC_FlushRange(pixels,sizeof pixels);dmaCopy(pixels,bgGetGfxPtr(bg),sizeof pixels);
#endif
   present_ticks+=tickGetCount()-t0;++frames;
   unsigned elapsed=vblanks-last;
   if(elapsed>=60){
    X4FlowState s;x4_flow_state(&s);unsigned fps=frames*60/elapsed;last=vblanks;
    unsigned logic_us=logic_ticks*1000000/TICK_FREQ/frames,draw_us=draw_ticks*1000000/TICK_FREQ/frames;
    unsigned audio_us=audio_ticks*1000000/TICK_FREQ/frames,present_us=present_ticks*1000000/TICK_FREQ/frames,wait_us=wait_ticks*1000000/TICK_FREQ/frames;
    unsigned prepare_us=prepare_ticks*1000000/TICK_FREQ/frames;
    X4VideoStats v=x4_video_stats();X4SfxStats fx=x4_sfx_stats();frames=0;logic_ticks=draw_ticks=audio_ticks=present_ticks=wait_ticks=prepare_ticks=0;
    unsigned simulation_hz=(s.frames-last_simulation)*60/elapsed;last_simulation=s.frames;
    iprintf("\x1b[0;0HMEGA MAN X4 " VERSION "\n%s Fase %02d/%d  FPS:%2u    \nHP:%2d Checkpoint:%d          \n\n"
     "B:pulo A:dash Y:ataque\nX:especial L/R:arma\nSTART:menu\nL+R+SELECT:selecao de teste\n\n"
     "Som:%s  Faixa:%2d            \nImagem:%u Audio:%u            \nEfeitos pendentes:%u          \nARM7 FX:%u Cache:%uKB        \nLog:%s                     \n"
     "X:%5d Y:%5d              \nEstado:%d/%d Bloqueio:%d     \nCPU logica:%5uus          \nDesenho:%5uus GPU:%5uus    \nTiles:%u +%u  Limite:%u     \nPrep:%5uus                 \n",
     s.character?"Zero":"X",s.stage,s.area+1,fps,s.hp,s.checkpoint,muted?"OFF":"ON",x4_audio_track(),gfx_errors,x4_audio_errors(),x4_n05_missing_sfx(),fx.active,fx.bytes/1024,log_file?"ativo":"indisponivel",s.x,s.y,s.state,s.substate,s.locked,logic_us,draw_us,present_us,v.hits,v.misses,v.overflow,prepare_us);
    if(log_file){fprintf(log_file,"frame=%u ticks=%u char=%d stage=%d area=%d fps=%u x=%d y=%d hp=%d checkpoint=%d engine=%d/%d player=%d locked=%d gfxerr=%u audioerr=%u missingfx=%u track=%d\n",
     vblanks,s.frames,s.character,s.stage,s.area,fps,s.x,s.y,s.hp,s.checkpoint,s.state,s.substate,s.player_state,s.locked,gfx_errors,x4_audio_errors(),x4_n05_missing_sfx(),x4_audio_track());
     fprintf(log_file,"PROFILE logic_us=%u draw_us=%u audio_us=%u present_us=%u wait_us=%u commands=%u hits=%u misses=%u palettes=%u upload=%u overflow=%u sim_hz=%u dropped_ticks=%u\n",logic_us,draw_us,audio_us,present_us,wait_us,v.commands,v.hits,v.misses,v.palette_uploads,v.uploaded_bytes,v.overflow,simulation_hz,dropped_ticks);
     fprintf(log_file,"ARM7 voices=%u pcm_bytes=%u cache_hits=%u cache_misses=%u starts=%u\n",fx.active,fx.bytes,fx.hits,fx.misses,fx.starts);
     fprintf(log_file,"PIPELINE prepare_us=%u\n",prepare_us);
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
     commit_log();}
   }
  }
  if(log_file)fflush(log_file);
  if(error)break;
 }
 audio_stop();x4_flow_close();if(log_file)fclose(log_file);return 0;
}
