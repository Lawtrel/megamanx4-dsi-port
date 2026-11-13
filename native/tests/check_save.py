from pathlib import Path
import subprocess,tempfile,json
ROOT=Path(__file__).resolve().parents[2];REF=ROOT/'referencias/mmx4-decomp';OUT=ROOT/'build/native/save-tests';OUT.mkdir(parents=True,exist_ok=True)
flags=['-std=gnu11','-O2','-DMMX4_PC=1','-D__psyz=1','-DVERSION_US=1','-I'+str(ROOT/'native/src'),'-I'+str(REF/'tools/psyz/psyz/include')]
source=ROOT/'native/dsi/n05_save.c'
subprocess.run(['gcc','-I'+str(REF/'include'),*flags,'-fsanitize=address',str(source),str(ROOT/'native/tests/save_driver.c'),'-o',str(OUT/'save_test')],check=True)
with tempfile.TemporaryDirectory(prefix='native-save-',dir=OUT) as folder:
 p=subprocess.run([str(OUT/'save_test'),folder],capture_output=True,text=True)
(OUT/'result.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3000:])
print(p.stdout,end='')
subprocess.run([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'),'-I'+str(ROOT/'build/native/gamecore/include'),*flags,'-mcpu=arm946e-s','-mthumb','-c',str(source),'-o',str(OUT/'n05_save.arm9.o')],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','host_address_sanitizer':True,'arm9_compiled':True,'physical_dsi_tested':False})+'\n')
