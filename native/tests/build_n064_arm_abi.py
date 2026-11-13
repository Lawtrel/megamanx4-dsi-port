"""Build asset-free ARM9 regression probes; execute separately with Unicorn."""
from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n064-arm-abi';OUT.mkdir(parents=True,exist_ok=True)
SDK=ROOT/'ambiente/sdk/devkitARM/bin';REF=ROOT/'referencias/mmx4-decomp'
source=REF/'src/main/effects/effect_26_boss_death_fx.c'
text=source.read_text();fixed='func_8002B560((s8)0x82, 0);';old='((void (*)(s32, s32))func_8002B560)(0x82, 0);'
assert fixed in text
flags=['-Os','-mcpu=arm946e-s','-mthumb','-ffunction-sections','-fdata-sections','-DMMX4_PC=1','-D__psyz=1','-DVERSION_US=1','-Wno-incompatible-pointer-types','-Wno-int-conversion','-I'+str(ROOT/'build/native/gamecore/include'),'-I'+str(REF/'tools/psyz/psyz/include')]
rows={}
for name,content in [('before',text.replace(fixed,old)),('after',text)]:
 caller=OUT/(name+'.c');caller.write_text(content)
 objects=[]
 for i,p in enumerate([caller,REF/'src/main/objects.c',ROOT/'native/tests/n064_arm_abi_driver.c']):
  o=OUT/f'{name}_{i}.o';subprocess.run([str(SDK/'arm-none-eabi-gcc'),*flags,'-c',str(p),'-o',str(o)],check=True);objects.append(o)
 elf=OUT/(name+'.elf');subprocess.run([str(SDK/'arm-none-eabi-gcc'),'-nostdlib','-mcpu=arm946e-s','-mthumb','-Wl,--gc-sections,-Ttext=0x10000,-Tdata=0x20000,-e,unit_entry',*map(str,objects),'-o',str(elf)],check=True)
 subprocess.run([str(SDK/'arm-none-eabi-objcopy'),'-O','binary',str(elf),str(OUT/(name+'.bin'))],check=True)
 symbols=subprocess.check_output([str(SDK/'arm-none-eabi-nm'),str(elf)],text=True)
 entry=int(next(l.split()[0] for l in symbols.splitlines() if l.endswith(' unit_entry')),16)
 rows[name]={'entry':entry,'sha256':hashlib.sha256((OUT/(name+'.bin')).read_bytes()).hexdigest()}
(OUT/'probes.json').write_text(json.dumps(rows,indent=2)+'\n')
print('Built isolated ARM9 probes: before must return 3; after must return 0.')
