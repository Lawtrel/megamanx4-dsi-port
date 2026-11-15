"""Compile original full-flow roots with the native stage/loading/task adapters."""
from pathlib import Path
import subprocess,json,re,hashlib
ROOT=Path(__file__).resolve().parents[1];REF=ROOT/'referencias/mmx4-decomp';CORE=ROOT/'build/native/gamecore'
OUT=ROOT/'build/native/n05-flow';OUT.mkdir(parents=True,exist_ok=True);SDK=ROOT/'ambiente/sdk/devkitARM/bin'
flags=['-std=gnu11','-Os','-mcpu=arm946e-s','-mthumb','-fno-strict-aliasing','-ffunction-sections','-fdata-sections',
 '-DMMX4_PC=1','-D__psyz=1','-DSKIP_MDEC=1','-DVERSION_US=1','-DX4_FULL_FLOW=1',
 '-Wno-incompatible-pointer-types','-Wno-int-conversion','-I'+str(CORE/'include'),'-I'+str(REF/'src/pc'),
 '-I'+str(REF/'tools/psyz/psyz/include'),'-I'+str(ROOT/'native/src'),'-I'+str(ROOT/'build/native/content-index')]
replacements={'55C4.c':['stop_sound','func_8001663C','func_80016F0C','func_80016448','is_sound_finished','func_800160F4','func_80016124','func_800170B0','func_800170E0','func_80017100','func_800169D8','func_80015284','func_800153D4','func_80015C10','func_80017340','func_800179BC'],
 '3D88.c':['func_80013AD8','func_80013890'], '323C.c':['func_80012A3C'],
 '144A4.c':['func_80023CE0','func_80023D68','init_objects'],'165A0.c':['func_80026118'],'E47C.c':['func_8001E980'],
 '102FC.c':[], '1EC20.c':[], '11184.c':[], 'game_info.c':[], '14A60.c':['func_80024260','func_80024E70']}
replacements['55C4.c'].append('func_80018000')
sources=[]
for name,names in replacements.items():
 text=(REF/'src/main'/name).read_text()
 # These high-level routes only save/restore Sony VRAM around movies. Native
 # rendering retains all source assets and redraws its own framebuffer, so no
 # GPU snapshot/copy or device fence is needed. No generic device stubs.
 if name in ('102FC.c','1EC20.c'):
  text=re.sub(r'(?m)^\s*(?:MoveImage\([^;]+|DrawSync\(0\));', '\n    /* Native framebuffer: source textures remain intact. */',text)
 if name=='11184.c':text=text.replace('SetDispMask(0);','/* Native frontend retains ownership of display during menu reset. */')
 if name=='game_info.c':text=text.replace('(u8*)0x80178000','MAIN_ARCHIVE_ARENA')
 for n in names:
  text,count=re.subn(r'(?m)^((?:void|s32) )'+n+r'(?=\([^;]*\)\s*\{)',r'\1unused_flow_'+n,text);assert count>=1,(n,count)
 p=OUT/(name[:-2]+'_native.c');p.write_text(text);sources.append(p)
p=OUT/'portable_game.c';text=(REF/'src/pc/placeholder.c').read_text()
for n in ('func_8001540C','func_80028FEC','func_800175AC'):
 text,count=re.subn(r'(?m)^((?:void|s32) )'+n+r'(?=\([^;]*\)\s*\{)',r'\1unused_flow_'+n,text);assert count==1,(n,count)
p.write_text(text);sources.append(p)
sources += [ROOT/'native/dsi'/n for n in ('n05_flow.c','n05_flow_loading.c','n05_flow_ui.c','n05_platform.c','n05_render.c','stage_spawn.c','n05_save.c','n08_media.c')]
sources += [ROOT/'native/src'/n for n in ('x4_archive.c','x4_content.c','x4_graphics_decode.c','x4_music.c','x4_sfx.c','x4_audio.c','x4_video.c')]+[REF/'src/pc/scratchpad.c']
objects=[]
def source_flags(source):return flags+(['-O3','-marm'] if source.name in ('n05_render.c','n05_flow_ui.c','x4_video.c') else [])
def dependency_key(source,dependencies):
 h=hashlib.sha256(source.read_bytes()+json.dumps(source_flags(source)).encode())
 for name in dependencies:
  p=Path(name)
  if not p.is_file():return ''
  h.update(str(p).encode());h.update(p.read_bytes())
 return h.hexdigest()
for s in sources:
 o=OUT/(s.stem+'.o');dep=OUT/(s.stem+'.d');keypath=OUT/(s.stem+'.deps.json')
 cache=json.loads(keypath.read_text()) if keypath.is_file() else {}
 if not(o.is_file() and cache.get('key')==dependency_key(s,cache.get('dependencies',[]))):
  p=subprocess.run([str(SDK/'arm-none-eabi-gcc'),*source_flags(s),'-MMD','-MF',str(dep),'-c',str(s),'-o',str(o)],capture_output=True,text=True)
  (OUT/(s.stem+'.log')).write_text(p.stdout+p.stderr)
  if p.returncode:raise RuntimeError(str(s)+'\n'+p.stderr[-3500:])
  dependencies=dep.read_text().replace('\\\n',' ').split(':',1)[1].split()
  keypath.write_text(json.dumps({'dependencies':dependencies,'key':dependency_key(s,dependencies)}))
 objects.append(str(o))
original=[r['object'] for r in json.loads((CORE/'report.json').read_text())['sources'] if Path(r['source']).name not in replacements]
objects+=original+[str(ROOT/'build/native/n05-tasks/n05_tasks.arm9.o')]
target=OUT/'flow.o';roots=['x4_flow_'+n for n in ('begin','new_game','title','step','state','debug','draw','close')]
roots+=['x4_audio_'+n for n in ('init','update','read','close','errors','track')]+['x4_save_set_root','x4_n05_missing_sfx']
roots+=['x4_video_'+n for n in ('commands','texture','rgba_texture','dirty_tiles','palette','clear','alpha','stats','viewport','get_viewport')]
roots+=['x4_sfx_set_backend','x4_sfx_stats']
p=subprocess.run([str(SDK/'arm-none-eabi-ld'),'-r','--gc-sections',*[a for n in roots for a in ('-u',n)],'-o',str(target),*objects],capture_output=True,text=True)
if p.returncode:raise RuntimeError(p.stderr[-4000:])
def run(*cmd):return subprocess.check_output([str(a) for a in cmd],text=True)
undefined={l.split()[-1] for l in run(SDK/'arm-none-eabi-nm','-u',target).splitlines()}
refs={re.split(r'[+-]0x',l.split()[-1])[0] for l in run(SDK/'arm-none-eabi-objdump','-r',target).splitlines() if ' R_ARM_' in l}
missing=sorted(undefined&refs)
(OUT/'required_services.json').write_text(json.dumps(missing,indent=2)+'\n')
(OUT/'adapter_sources.json').write_text(json.dumps([str(s.relative_to(ROOT)) for s in sources],indent=2)+'\n')
(OUT/'replaced_sources.json').write_text(json.dumps(list(replacements),indent=2)+'\n')
(OUT/'build_inputs.json').write_text(json.dumps({'flow_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),
 'sources':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources},
 'gamecore_report_sha256':hashlib.sha256((CORE/'report.json').read_bytes()).hexdigest(),
 'tasks_object_sha256':hashlib.sha256((ROOT/'build/native/n05-tasks/n05_tasks.arm9.o').read_bytes()).hexdigest()},indent=2)+'\n')
print(run(SDK/'arm-none-eabi-size',target));print('\n'.join(missing))
