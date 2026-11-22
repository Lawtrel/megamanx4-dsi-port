"""Verify and package a native integration build; no complete-game claim."""
from pathlib import Path
import hashlib,json,struct,subprocess,sys,shutil,os
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from nds_header import crc16
N06=True
BUILD=ROOT/('build/native/n09' if N06 else 'build/native/n05-integrated');DIST=ROOT/('dist/native-n09' if N06 else 'dist/native-n05-flow');FLOW=ROOT/'build/native/n05-flow'
target='mmx4_native_n09' if N06 else 'mmx4_native_n05_flow'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inputs=json.loads((FLOW/'build_inputs.json').read_text())
assert sha(FLOW/'flow.o')==inputs['flow_sha256']==sha(BUILD/'flow.o')
for path,digest in inputs['sources'].items():assert sha(ROOT/path)==digest,path
assert sha(BUILD/'source/main.c')==sha(ROOT/'native/dsi/n05_flow_main.c')
assert sha(BUILD/'source/x4_controls.h')==sha(ROOT/'native/src/x4_controls.h')
for name in ('x4_lower_ui.h','x4_lower_ui.c'):
 assert sha(BUILD/'source'/name)==sha(ROOT/'native/src'/name)
build_id=hashlib.sha256((BUILD/'flow.o').read_bytes()+b''.join(p.read_bytes() for p in sorted((BUILD/'source').iterdir()) if p.suffix in ('.c','.h') and p.name!='build_id.h')+json.dumps(json.loads((BUILD/'assets.json').read_text()),sort_keys=True).encode()).hexdigest()[:20]
assert (BUILD/'source/build_id.h').read_text()=='#define X4_BUILD_ID "'+build_id+'"\n'
reports=[('flow_test','matrix_gpu.json' if N06 else 'matrix.json'),('systems_test','systems_gpu.json' if N06 else 'systems.json')]
for name,report in reports:
 validation=json.loads((FLOW/'host'/report).read_text());source=json.loads((FLOW/'host'/(name+'_inputs.json')).read_text())
 assert validation['binary_sha256']==source['binary_sha256']==sha(FLOW/'host'/name)
 assert validation['cases'] and all(r['returncode']==0 for r in validation['cases']),report
 for path,digest in source['sources'].items():assert sha(ROOT/path)==digest,path
for p in ('music-tests','sfx-tests','save-tests'):
 report=json.loads((ROOT/'build/native'/p/'validation.json').read_text());assert report['status']=='passed'
 for path,digest in report.get('sources',report.get('source_sha256',{})).items():assert sha(ROOT/path)==digest,path
if N06:
 video=json.loads((ROOT/'build/native/n06-video-tests/validation.json').read_text());assert video['status']=='passed' and video.get('indexed_scalar_cases')==512
 for path,digest in video['sources'].items():assert sha(ROOT/path)==digest,path
 gpu=json.loads((ROOT/'build/native/n061-gpu-tests/validation.json').read_text());assert gpu['status']=='passed'
 for path,digest in gpu['sources'].items():assert sha(ROOT/path)==digest,path
 arm7=json.loads((ROOT/'build/native/n063-arm7-tests/validation.json').read_text());assert arm7['status']=='passed'
 for path,digest in arm7['sources'].items():assert sha(ROOT/path)==digest,path
 assert sha(BUILD/'source/n06_gpu.c')==sha(ROOT/'native/dsi/n06_gpu.c')
binary=BUILD/(target+'.nds');data=binary.read_bytes();assert data[0x12]==2
assert build_id.encode() in data, 'NDS does not contain the current build identifier; finish building before packaging'
assert crc16(data[:0x15e])==struct.unpack_from('<H',data,0x15e)[0]
assert crc16(data[0xc0:0x15c])==struct.unpack_from('<H',data,0x15c)[0]
segments={};spans=[]
for name,offset in [('arm9',0x20),('arm7',0x30),('arm9i',0x1c0),('arm7i',0x1d0)]:
 rom,entry,ram,size=struct.unpack_from('<4I',data,offset);assert rom>=0x4000 and size and rom+size<=len(data)
 if name in ('arm9','arm7'):assert ram<=entry<ram+size
 spans.append((rom,rom+size));segments[name]={'rom':rom,'ram':hex(ram),'bytes':size}
spans.sort();assert all(a[1]<=b[0] for a,b in zip(spans,spans[1:]))
fntoff,fntsize,fatoff,fatsize=struct.unpack_from('<4I',data,0x40)
assert fntoff+fntsize<=len(data) and fatoff+fatsize<=len(data)
fnt=data[fntoff:fntoff+fntsize];fat=data[fatoff:fatoff+fatsize];files={}
def directory(index,prefix=''):
 off,fid,parent=struct.unpack_from('<IHH',fnt,index*8)
 while fnt[off]:
  c=fnt[off];off+=1;name=fnt[off:off+(c&127)].decode();off+=c&127
  if c&128:child=struct.unpack_from('<H',fnt,off)[0]&0xfff;off+=2;directory(child,prefix+name+'/')
  else:
   a,b=struct.unpack_from('<II',fat,fid*8);assert 0<=a<=b<=len(data)
   files[prefix+name]=hashlib.sha256(data[a:b]).hexdigest();fid+=1
directory(0)
assert not any(p.startswith('movies/') or p.endswith(('.xmv','.str')) for p in files), 'Cutscenes must not be packaged'
expected={r['path']:r['sha256'] for r in json.loads((BUILD/'assets.json').read_text())};assert files==expected
symbols=subprocess.check_output([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-nm'),str(BUILD/(target+'.elf'))],text=True)
# Bound the static footprint on DS and verify that both render profiles have
# backing memory in the segment initialized by their respective startup path.
cache_layout={}
if N06:
 defined=subprocess.check_output([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-nm'),'-S','--defined-only',str(BUILD/(target+'.elf'))],text=True)
 records={parts[-1]:parts for line in defined.splitlines() if len(parts:=line.split())>=3}
 ntr_end=int(records['__heap_start_ntr'][0],16);twl_end=int(records['__heap_start_twl'][0],16)
 assert ntr_end<=0x2300000 and twl_end<0x2fc0000
 for name,size in {'indexed':524288,'gpu_palettes':104448,'ds_poses':65632,'dsi_poses':393792}.items():
  row=records[name];address=int(row[0],16)
  start,end=(ntr_end,twl_end) if name=='dsi_poses' else (0x02000000,ntr_end)
  assert len(row)==4 and int(row[1],16)==size and start<=address and address+size<=end,name
  cache_layout[name]={'address':hex(address),'bytes':size}
 assert int(records['indexed'][0],16)%32==0
 assert not {'rgba','rgba_tables','aliases'}&records.keys()
 assert '__dldi_start' in records and b'\xed\xa5\x8d\xbf Chishm\x00' in data
assets=json.loads((ROOT/'build/native/lower-assets/validation.json').read_text());assert assets['status']=='passed'
assert sha(BUILD/'source/x4_lower_icons.h')==sha(ROOT/'build/native/lower-assets/x4_lower_icons.h')==assets['header_sha256']
for path,digest in assets['sources'].items():assert sha(ROOT/path)==digest,path
lower=json.loads((ROOT/'build/native/lower-ui-tests/validation.json').read_text());assert lower['status']=='passed'
assert lower['atlas_sha256']==assets['header_sha256']
for path,digest in lower['sources'].items():assert sha(ROOT/path)==digest,path
panel=json.loads((ROOT/'build/native/player-panel-tests/validation.json').read_text());assert panel['status']=='passed'
for path,digest in panel['sources'].items():assert sha(ROOT/path)==digest,path
for name in ('x4_flow_radar','x4_flow_request_weapon','x4_lower_touch','x4_lower_render','x4_flow_debug_sigma_hitkill','x4_flow_debug_sigma_hitkill_enabled','x4_flow_debug_credits'):assert name in symbols,name
for banned in ('Psyz','psyz','CPUState','psx_dispatch','cpu_run','dyn_run','CdControl','SpuGet','SsUt','SsSep','DrawSync','MoveImage','unused_flow_','UNIMPLEMENTED','x4_movie_'):
 assert banned not in symbols,banned
for required in ('player_update_normal','update_main_objects','engine_update_funcs','x4_flow_begin','x4_flow_begin_debug','x4_n05_render_fault','x4_n05_render_cache_stats','x4_flow_title','layout_gate_apply_layout','func_8001C210','x4_audio_read','func_80018000','x4_sfx_mix','mmx4_pc_card_write','mmStreamOpen','threadPrepare'):
 assert required in symbols,required
DIST.mkdir(parents=True,exist_ok=True)
temporary=DIST/(binary.name+'.building');shutil.copyfile(binary,temporary);assert sha(temporary)==sha(binary)
os.replace(temporary,DIST/binary.name)
if N06:
 for required in ('x4_gpu_prepare','x4_gpu_present','x4_gpu_transfer_stats','x4_gpu_upload_batches','x4_video_dirty_tiles','x4_video_dirty_palettes','x4_video_peek_indexed','x4_video_last_fault','x4_video_tile','x4_n06_draw_hud','glInit'):assert required in symbols,required
 for required in ('x4_arm7_audio_init','x4_sfx_set_backend','soundPreparePcm','soundSynchronize'):assert required in symbols,required
shutil.copyfile(ROOT/'docs/STATUS_N09.md',DIST/'LEIA-ME.md')
shutil.copyfile(ROOT/'referencias/mmx4-decomp/LICENSE',DIST/'LICENSE-upstream.txt')
for _,p in reports:shutil.copyfile(FLOW/'host'/p,DIST/p)
if N06:shutil.copyfile(ROOT/'build/native/n06-video-tests/validation.json',DIST/'video_tests.json')
if N06:shutil.copyfile(ROOT/'build/native/n061-gpu-tests/validation.json',DIST/'gpu_backend_tests.json')
if N06:shutil.copyfile(ROOT/'build/native/n063-arm7-tests/validation.json',DIST/'arm7_audio_tests.json')
shutil.copyfile(ROOT/'build/native/lower-ui-tests/validation.json',DIST/'lower_ui_tests.json')
shutil.copyfile(ROOT/'build/native/lower-assets/validation.json',DIST/'lower_assets.json')
if N06:
 shutil.copyfile(ROOT/'docs/research/N06.md',DIST/'PESQUISA_N06_HISTORICO.md')
 for p in (FLOW/'host').glob('sigma*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for p in (FLOW/'host').glob('jungle*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for p in (FLOW/'host').glob('menu*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for pattern in ('train_[01]_*.png','train_car_broken_[01].png','armor*.png','title_[01]_*.png','continue*.png'):
  for p in (FLOW/'host').glob(pattern):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
manifest={'version':'Beta 9' if N06 else 'N05 FLOW integration 2','not_complete_game':True,'physical_dsi_tested':False,
 'binary':binary.name,'sha256':sha(binary),'build_id':build_id,'size':len(data),'unit_code':2,'bios_or_cpu_emulator_linked':False,
 'upstream':'https://github.com/sozud/mmx4','upstream_revision':'4e16dfc7c06597fd5719dd112654d8cf85d343c2',
 'host_stage_entry_cases':48,'host_system_cases':len(json.loads((FLOW/'host'/reports[1][1]).read_text())['cases']),
 'controls':'SELECT sends original GIGA/R2 action; full SELECT+X and L+R+SELECT chords consumed by frontend',
 'debug_loadout':'Stage selection only: all weapons/techniques, 48 HP, tanks, X full armor and Plasma; original resource consumption; no defeated-boss flag override',
 'assets':{'arc':138,'music_voice_tracks':80,'sfx_banks':79,'short_sequences':20,'native_movies':0},
 'pending':['Four unresolved SEP commands and precise looping/reverb','Complete graphics priority/filter fidelity','Original title/load menu qualification','30/60 FPS optimization'],
 'flow_object_sha256':sha(BUILD/'flow.o'),'frontend_sha256':sha(BUILD/'source/main.c'),'nitrofs_files':files,'segments':segments}
if N06:manifest.update(movies='disabled: original calls return immediately; no movie files or playback code packaged',renderer='DS 3D lossless RGB256 indexed tiles and retained RGB555 CLUTs',logical_viewport=[320,240],world_crop=[32,24,256,192],physical_viewport=[256,192],texture_cache_bytes=524288,texture_cache_slots=2048,texture_tile_bytes=256,gpu_palette_slots=192,palette_cache_bytes=98304,melonDS_tested=False,
 gpu_submission='packed four-opcode FIFO via DMA0; retained state; VTX16 then VTX_XY',fifo_builder='local cursor and pending opcode word; commit once per four commands and at end',gpu_compilation='ARM -O3',graphics_sort='stable two-byte radix; joint histograms; no identity/copy passes',frame_timing='present-to-present includes diagnostics; mean/max/p95 upper bound; fields beyond two; no FPS cap',rng_startup_seed='0x0D37',log_commit='fclose/reopen per profile interval',
 audio_effects='Calico ARM7 driver and native hardware channels 8..15; acknowledged ownership',sfx_pcm_cache_limit=4194304,sfx_pcm_cache_limit_ntr=98304,
 sfx_publication='cache insertion only; acknowledged stop before reuse',boss_trace=True,
 graphics_pipeline='CPU list/cache preparation before VBlank; per-bank 8KiB DMA chunks start only at VCOUNT 192..204; wait with all textures mapped; retain prior image until uploads complete',tile_conversion='4/8bpp to 8-bit indices; palette colors versioned separately from transparency/index remapping',texture_sharing='2048 native indexed slots; original source/pose/remap key; pixels reused across color-only palette animation; host tests retain RGBA for reference',palette_reuse='raw color snapshot checked on first use in each new frame',
 campaign_menu='original animated title, new-game/load menus and native two-bank save journal',continue_palette='restore borrowed live palette when leaving engine state 8; nested state 12 retains menu ownership',background_iteration='frame-local pointer/size snapshots and row bounds; live map/palette contents read each draw',mutable_layout='shared original D_80141BE8 for stage events, drawing and collision',train_background='original camera-wrap rule and repeated-map seam sampling',player_armor='original part assemblies and palettes composed over shared player pose',
 post_credits='return to the X4 title',
 pending=['Sustained 30 FPS qualification on physical DSi','DS blending fidelity','Four unresolved SEP commands and audio effect fidelity'])
if N06:manifest.update(console_cache_layout=cache_layout,physical_ds_tested=False,dspico_tested=False,campaign_user_validated=['X','Zero'],radar_user_feedback='No perceived slowdown; no new timing log supplied',startup='original title directly; stage selector through Debug or L+R+SELECT')
# Emulator observations qualify one exact ROM. Rebuilding must not inherit
# an earlier run if the linked binary or its embedded assets changed.
evidence=ROOT/'build/native/compat-smoke/verified.json'
if evidence.is_file():
 report=json.loads(evidence.read_text())
 if report['sha256']==manifest['sha256'] and report['build_id']==build_id:
  manifest.update(melonDS_tested=True,compatibility_smoke=report)
(DIST/'build_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
benchmark=ROOT/'build/native/n0811-tile-benchmark/execution.json'
if benchmark.is_file():
 report=json.loads(benchmark.read_text());probe=report['probes']['after']
 if (probe.get('indexed_candidate') and probe['source_sha256']==sha(ROOT/'native/src/x4_video.c') and
     probe.get('fixture_sha256')==sha(ROOT/'native/tests/n066_video_benchmark.c') and
     probe.get('header_sha256')==sha(ROOT/'native/src/x4_video.h')):
  shutil.copyfile(benchmark,DIST/'tile_instruction_benchmark.json')
assert sha(DIST/binary.name)==manifest['sha256']
gpu_benchmark=ROOT/'build/native/n0811-gpu-benchmark/execution.json'
if gpu_benchmark.is_file():
 report=json.loads(gpu_benchmark.read_text())
 if (report['probes']['after']['source_sha256']==sha(ROOT/'native/dsi/n06_gpu.c') and
     report['fixture_sha256']==sha(ROOT/'native/tests/n083_gpu_benchmark.c') and
     report['video_sha256']==sha(ROOT/'native/src/x4_video.c')):
  assert all(r['before']['fifo_sha256']==r['after']['fifo_sha256'] for r in report['results'].values())
  shutil.copyfile(gpu_benchmark,DIST/'gpu_instruction_benchmark.json')
background_benchmark=ROOT/'build/native/n0811-background-benchmark'
if (background_benchmark/'execution.json').is_file():
 report=json.loads((background_benchmark/'execution.json').read_text())
 host=json.loads((background_benchmark/'host_validation.json').read_text())
 if (report['probes']['after']['source_sha256']==sha(ROOT/'native/dsi/n05_render.c') and
     report['fixture_sha256']==sha(ROOT/'native/tests/n087_background_benchmark.c') and
     report['video_sha256']==sha(ROOT/'native/src/x4_video.c') and
     report['archive_sha256']==sha(ROOT/'native/src/x4_archive.c')):
  assert host['status']=='passed' and host['after']['source_sha256']==report['probes']['after']['source_sha256']
  assert all(r['before']['output_sha256']==r['after']['output_sha256'] for r in report['results'].values())
  assert all(report['results'][key]['after']['instructions']<report['results'][key]['before']['instructions'] for key in ('three_layers','negative_edges','train_seam','live_mutations'))
  shutil.copyfile(background_benchmark/'execution.json',DIST/'background_instruction_benchmark.json')
  shutil.copyfile(background_benchmark/'host_validation.json',DIST/'background_equivalence_tests.json')
print(json.dumps({k:manifest[k] for k in ('binary','sha256','size','not_complete_game','physical_dsi_tested')},indent=2))
