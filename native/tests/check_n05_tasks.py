"""Test native scheduler handoff with a host transport; compile actual Calico ARM9 adapter."""
from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n05-tasks';OUT.mkdir(parents=True,exist_ok=True)
REF=ROOT/'referencias/mmx4-decomp';SDK=ROOT/'ambiente/sdk';CODE=ROOT/'native/dsi/n05_tasks.c'
flags=['-std=gnu11','-O2','-DMMX4_PC=1','-D__psyz=1','-DVERSION_US=1','-I'+str(ROOT/'native/src')]
host=ROOT/'native/tests/thread_host_shim'
p=subprocess.run(['gcc',*flags,'-pthread','-fsanitize=address','-I'+str(REF/'include'),'-I'+str(REF/'tools/psyz/psyz/include'),
    '-I'+str(host),str(CODE),str(host/'shim.c'),str(ROOT/'native/tests/n05_tasks_driver.c'),'-o',str(OUT/'tasks_test')],capture_output=True,text=True)
(OUT/'compile.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3500:])
p=subprocess.run([str(OUT/'tasks_test')],capture_output=True,text=True,timeout=20)
(OUT/'result.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3000:])
print(p.stdout,end='')
p=subprocess.run([str(SDK/'devkitARM/bin/arm-none-eabi-gcc'),*flags,'-mcpu=arm946e-s','-mthumb','-DARM9=1','-D__NDS__=1',
    '-I'+str(ROOT/'build/native/gamecore/include'),'-I'+str(REF/'tools/psyz/psyz/include'),'-I'+str(SDK/'calico/include'),
    '-c',str(CODE),'-o',str(OUT/'n05_tasks.arm9.o')],capture_output=True,text=True)
(OUT/'arm9_compile.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3500:])
(OUT/'validation.json').write_text(json.dumps({'status':'passed','host_transport':'pthread Calico API shim',
    'yield_stress':5000,'native_calico_arm9_compiled':True,'physical_dsi_tested':False,
    'source_sha256':hashlib.sha256(CODE.read_bytes()).hexdigest()},indent=2)+'\n')
