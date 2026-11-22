"""Host workbench for the full state machine. Missing services abort, no stubs."""
from pathlib import Path
import subprocess,json,concurrent.futures,hashlib,sys
ROOT=Path(__file__).resolve().parents[2];REF=ROOT/'referencias/mmx4-decomp';CORE=ROOT/'build/native/gamecore'
OUT=ROOT/'build/native/n05-flow';HOST=OUT/'host';HOST.mkdir(parents=True,exist_ok=True)
arguments=sys.argv[1:];systems='--systems' in arguments
if systems:arguments.remove('--systems')
executable=HOST/('systems_test' if systems else 'flow_test')
(HOST/'validation.json').write_text('{"status":"running","physical_dsi_tested":false}\n')
nm=ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-nm'
def run(*cmd):return subprocess.check_output([str(a) for a in cmd],text=True)
selection_key=hashlib.sha256((OUT/'flow.o').read_bytes()+(CORE/'report.json').read_bytes()).hexdigest()
selection_path=HOST/'selected_sources.json'
cached=json.loads(selection_path.read_text()) if selection_path.exists() else {}
if cached.get('key')==selection_key:selected={Path(p) for p in cached['sources']}
else:
 retained={l.split()[-1] for l in run(nm,'--defined-only',OUT/'flow.o').splitlines() if len(l.split())==3}
 byobj={r['object']:REF/r['source'] for r in json.loads((CORE/'report.json').read_text())['sources']}
 selected=set()
 for l in run(nm,'-A','--defined-only',*byobj).splitlines():
  if len(l.split())>=3 and l.split()[-1] in retained:selected.add(byobj[l.split(':',1)[0]])
 selection_path.write_text(json.dumps({'key':selection_key,'sources':sorted(map(str,selected))}))
for n in json.loads((OUT/'replaced_sources.json').read_text()):selected.discard(REF/'src/main'/n)
selected.update(ROOT/n for n in json.loads((OUT/'adapter_sources.json').read_text()))
selected.update([ROOT/'native/dsi/n05_tasks.c',ROOT/'native/tests/thread_host_shim/shim.c',ROOT/'native/tests'/('n05_systems_driver.c' if systems else 'n05_flow_driver.c')])
standard={'abort','memcpy','memmove','memset','memcmp','fopen','fclose','fflush','fgetc','fread','fwrite','fprintf','fseek','ftell','rewind','malloc','realloc','free','snprintf','strlen','strcpy','strcmp','strrchr','strtoul','getenv','__assert_func','__getreent',
 'calloc','setjmp','longjmp','setvbuf','condvarSignal','condvarWait','mutexLock','mutexUnlock','threadAttachLocalStorage','threadPrepare','threadStart'}
names=[n for n in json.loads((OUT/'required_services.json').read_text()) if n not in standard and not n.startswith(('__aeabi_','__gnu_'))]
trap=HOST/'diagnostic_traps.c';trap.write_text('#include <stdio.h>\n#include <stdlib.h>\n'+
 '\n'.join('__attribute__((noreturn)) void '+n+'(void){fprintf(stderr,"UNIMPLEMENTED FULL FLOW: '+n+'\\n");abort();}' for n in names)+'\n')
selected.add(trap)
flags=['-std=gnu11','-Os','-g','-fno-omit-frame-pointer','-fno-strict-aliasing','-fwrapv','-fsanitize=address','--param=asan-globals=0',
 '-ffunction-sections','-fdata-sections','-DMMX4_PC=1','-D__psyz=1','-DSKIP_MDEC=1','-DVERSION_US=1','-DX4_FULL_FLOW=1','-DX4_N06_INDEXED=1',
 '-Wno-incompatible-pointer-types','-Wno-int-conversion','-I'+str(REF/'include'),'-I'+str(REF/'src/pc'),
 '-I'+str(REF/'tools/psyz/psyz/include'),'-I'+str(ROOT/'native/src'),'-I'+str(ROOT/'build/native/content-index'),
 '-I'+str(ROOT/'native/tests/thread_host_shim'),'-pthread']
def dependency_key(s,dependencies):
 h=hashlib.sha256(s.read_bytes()+json.dumps(flags).encode())
 for name in dependencies:
  p=Path(name)
  if not p.is_file():return ''
  h.update(str(p).encode());h.update(p.read_bytes())
 return h.hexdigest()
def compile(s):
 name=hashlib.sha256(str(s).encode()).hexdigest()[:16];o=HOST/(name+'.o');keypath=HOST/(name+'.deps.json');depfile=HOST/(name+'.d')
 cache=json.loads(keypath.read_text()) if keypath.is_file() else {}
 key=dependency_key(s,cache.get('dependencies',[]))
 if not (o.is_file() and cache.get('key')==key):
  p=subprocess.run(['gcc',*flags,'-MMD','-MF',str(depfile),'-c',str(s),'-o',str(o)],capture_output=True,text=True);(HOST/(name+'.log')).write_text(p.stdout+p.stderr)
  if p.returncode:raise RuntimeError(str(s)+'\n'+p.stderr[-3000:])
  dependencies=depfile.read_text().replace('\\\n',' ').split(':',1)[1].split()
  keypath.write_text(json.dumps({'dependencies':dependencies,'key':dependency_key(s,dependencies)}))
 return str(o)
sources=sorted(selected);print('Full-flow host units:',len(sources),flush=True)
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:objects=list(pool.map(compile,sources))
p=subprocess.run(['gcc','-pthread','-fsanitize=address','-Wl,--gc-sections',*objects,'-o',str(executable)],capture_output=True,text=True)
(HOST/'link.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3000:])
fingerprints=sources+[ROOT/'native/src/x4_video.h',ROOT/'native/tests/n06_preview.h']+([ROOT/'native/tests/n07_systems.h',ROOT/'native/tests/n083_render_systems.h',ROOT/'native/tests/n084_controls_systems.h',ROOT/'native/tests/n086_continue_systems.h',ROOT/'native/tests/n089_frost_systems.h',ROOT/'native/tests/n09_debug_systems.h',ROOT/'native/src/x4_controls.h'] if systems else [])
(HOST/(executable.name+'_inputs.json')).write_text(json.dumps({'binary_sha256':hashlib.sha256(executable.read_bytes()).hexdigest(),
 'sources':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in fingerprints}},indent=2)+'\n')
arguments=arguments or (['pause','0'] if systems else ['0','0','0'])
try:p=subprocess.run([str(executable),str(ROOT/'build/native/full-content'),*arguments],cwd=HOST,capture_output=True,text=True,timeout=90)
except subprocess.TimeoutExpired as e:
 (HOST/'result.log').write_bytes((e.stdout or b'')+(e.stderr or b''));raise
(HOST/'result.log').write_text(p.stdout+p.stderr);print(p.stdout[-5000:]);print(p.stderr[-3000:])
(HOST/'validation.json').write_text(json.dumps({'status':'passed' if p.returncode==0 else 'failed','arguments':arguments,'host_only':True,'physical_dsi_tested':False})+'\n')
if p.returncode:raise SystemExit(1)
