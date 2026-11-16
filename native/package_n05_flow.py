"""Verify and package a native integration build; no complete-game claim."""
from pathlib import Path
import hashlib,json,struct,subprocess,sys,shutil,os
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from nds_header import crc16
N06=True
BUILD=ROOT/('build/native/n083-render' if N06 else 'build/native/n05-integrated');DIST=ROOT/('dist/native-n083' if N06 else 'dist/native-n05-flow');FLOW=ROOT/'build/native/n05-flow'
target='mmx4_native_n083' if N06 else 'mmx4_native_n05_flow'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
inputs=json.loads((FLOW/'build_inputs.json').read_text())
assert sha(FLOW/'flow.o')==inputs['flow_sha256']==sha(BUILD/'flow.o')
for path,digest in inputs['sources'].items():assert sha(ROOT/path)==digest,path
assert sha(BUILD/'source/main.c')==sha(ROOT/'native/dsi/n05_flow_main.c')
build_id=hashlib.sha256((BUILD/'flow.o').read_bytes()+b''.join(p.read_bytes() for p in sorted((BUILD/'source').glob('*.c')))+json.dumps(json.loads((BUILD/'assets.json').read_text()),sort_keys=True).encode()).hexdigest()[:20]
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
 video=json.loads((ROOT/'build/native/n06-video-tests/validation.json').read_text());assert video['status']=='passed'
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
for banned in ('Psyz','psyz','CPUState','psx_dispatch','cpu_run','dyn_run','CdControl','SpuGet','SsUt','SsSep','DrawSync','MoveImage','unused_flow_','UNIMPLEMENTED','x4_movie_'):
 assert banned not in symbols,banned
for required in ('player_update_normal','update_main_objects','engine_update_funcs','x4_flow_begin','x4_flow_begin_debug','x4_n05_render_fault','x4_n05_render_cache_stats','x4_flow_title','layout_gate_apply_layout','func_8001C210','x4_audio_read','func_80018000','x4_sfx_mix','mmx4_pc_card_write','mmStreamOpen','threadPrepare'):
 assert required in symbols,required
DIST.mkdir(parents=True,exist_ok=True)
temporary=DIST/(binary.name+'.building');shutil.copyfile(binary,temporary);assert sha(temporary)==sha(binary)
os.replace(temporary,DIST/binary.name)
if N06:
 for required in ('x4_gpu_prepare','x4_gpu_present','x4_gpu_upload_batches','x4_video_dirty_tiles','x4_video_tile','x4_n06_draw_hud','glInit'):assert required in symbols,required
 for required in ('x4_arm7_audio_init','x4_sfx_set_backend','soundPreparePcm','soundSynchronize'):assert required in symbols,required
shutil.copyfile(ROOT/'docs/STATUS_N083.md',DIST/'LEIA-ME.md')
shutil.copyfile(ROOT/'referencias/mmx4-decomp/LICENSE',DIST/'LICENSE-upstream.txt')
for _,p in reports:shutil.copyfile(FLOW/'host'/p,DIST/p)
if N06:shutil.copyfile(ROOT/'build/native/n06-video-tests/validation.json',DIST/'video_tests.json')
if N06:shutil.copyfile(ROOT/'build/native/n061-gpu-tests/validation.json',DIST/'gpu_backend_tests.json')
if N06:shutil.copyfile(ROOT/'build/native/n063-arm7-tests/validation.json',DIST/'arm7_audio_tests.json')
if N06:
 shutil.copyfile(ROOT/'docs/research/N06.md',DIST/'PESQUISA_N06_HISTORICO.md')
 for p in (FLOW/'host').glob('sigma*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for p in (FLOW/'host').glob('jungle*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for p in (FLOW/'host').glob('menu*.png'):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
 for pattern in ('train_[01]_*.png','train_car_broken_[01].png','armor*.png','title_[01]_*.png'):
  for p in (FLOW/'host').glob(pattern):shutil.copyfile(p,DIST/('diagnostico_'+p.name))
manifest={'version':'Beta 8.3' if N06 else 'N05 FLOW integration 2','not_complete_game':True,'physical_dsi_tested':False,
 'binary':binary.name,'sha256':sha(binary),'build_id':build_id,'size':len(data),'unit_code':2,'bios_or_cpu_emulator_linked':False,
 'upstream':'https://github.com/sozud/mmx4','upstream_revision':'4e16dfc7c06597fd5719dd112654d8cf85d343c2',
 'host_stage_entry_cases':48,'host_system_cases':len(json.loads((FLOW/'host'/reports[1][1]).read_text())['cases']),
 'debug_loadout':'Stage selection only: all weapons/techniques, 48 HP, tanks, X full armor and Plasma; original resource consumption; no defeated-boss flag override',
 'assets':{'arc':138,'music_voice_tracks':80,'sfx_banks':79,'short_sequences':20,'native_movies':0},
 'pending':['Full physical DSi qualification and complete playthrough','Six unsupported sound commands and precise looping/reverb/panning','Complete graphics priority/filter fidelity','Original title/load menu qualification','30/60 FPS optimization'],
 'flow_object_sha256':sha(BUILD/'flow.o'),'frontend_sha256':sha(BUILD/'source/main.c'),'nitrofs_files':files,'segments':segments}
if N06:manifest.update(movies='disabled: original calls return immediately; no movie files or playback code packaged',renderer='DS 3D RGB555-alpha tiles',logical_viewport=[320,240],world_crop=[32,24,256,192],physical_viewport=[256,192],texture_cache_bytes=524288,palette_cache_bytes=0,melonDS_tested=False,
 gpu_submission='packed four-opcode FIFO via DMA0; retained state; VTX16 then VTX_XY',gpu_compilation='ARM -O3',rng_startup_seed='0x0D37',log_commit='fclose/reopen per profile interval',
 audio_effects='Calico ARM7 driver and native hardware channels 8..15; acknowledged ownership',sfx_pcm_cache_limit=4194304,
 sfx_publication='cache insertion only; acknowledged stop before reuse',boss_trace=True,
 graphics_pipeline='CPU preparation before VBlank; adjacent dirty uploads batched within each bank',tile_conversion='4bpp pair lookup and 8bpp color lookup; scalar reference validated',palette_reuse='raw color snapshot checked on first use in each new frame',
 campaign_menu='original animated title, new-game/load menus and native two-bank save journal',mutable_layout='shared original D_80141BE8 for stage events, drawing and collision',train_background='original camera-wrap rule and repeated-map seam sampling',player_armor='original part assemblies and palettes composed over shared player pose',
 pending=['30/60 FPS measurement and visual qualification on physical DSi','melonDS white-screen diagnosis','DS blending approximates some original effects','Six remaining sound commands and audio effect fidelity','Complete campaign and all Sigma phases'])
(DIST/'build_manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
benchmark=ROOT/'build/native/n066-video-benchmark/execution.json'
if benchmark.is_file():
 report=json.loads(benchmark.read_text());probe=report['probes']['after']
 if (probe['source_sha256']==sha(ROOT/'native/src/x4_video.c') and
     probe.get('fixture_sha256')==sha(ROOT/'native/tests/n066_video_benchmark.c') and
     probe.get('header_sha256')==sha(ROOT/'native/src/x4_video.h')):
  shutil.copyfile(benchmark,DIST/'tile_instruction_benchmark.json')
assert sha(DIST/binary.name)==manifest['sha256']
gpu_benchmark=ROOT/'build/native/n083-gpu-benchmark/execution.json'
if gpu_benchmark.is_file():
 report=json.loads(gpu_benchmark.read_text())
 if (report['probes']['after']['source_sha256']==sha(ROOT/'native/dsi/n06_gpu.c') and
     report['fixture_sha256']==sha(ROOT/'native/tests/n083_gpu_benchmark.c') and
     report['video_sha256']==sha(ROOT/'native/src/x4_video.c')):
  assert all(r['before']['fifo_hash']==r['after']['fifo_hash'] and r['after']['instructions']<=r['before']['instructions'] for r in report['results'].values())
  shutil.copyfile(gpu_benchmark,DIST/'gpu_instruction_benchmark.json')
print(json.dumps({k:manifest[k] for k in ('binary','sha256','size','not_complete_game','physical_dsi_tested')},indent=2))
