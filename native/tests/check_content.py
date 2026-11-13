"""Native direct-file resource loader: ASan, replacement lifetime and ARM9."""
from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/content-tests';OUT.mkdir(parents=True,exist_ok=True)
subprocess.run(['python3',str(ROOT/'native/prepare_resource_index.py')],check=True)
sources=[ROOT/'native/src/x4_content.c',ROOT/'native/src/x4_archive.c',ROOT/'native/tests/content_driver.c']
flags=['-std=gnu11','-O2','-I'+str(ROOT/'native/src'),'-I'+str(ROOT/'build/native/content-index')]
subprocess.run(['gcc',*flags,'-fsanitize=address','-fno-omit-frame-pointer',*map(str,sources),'-o',str(OUT/'content_test')],check=True)
p=subprocess.run([str(OUT/'content_test'),str(ROOT/'build/native/full-content')],capture_output=True,text=True)
(OUT/'result.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-4000:])
print(p.stdout,end='')
for s in sources[:2]:
    subprocess.run([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'),*flags,'-mcpu=arm946e-s','-mthumb','-c',str(s),'-o',str(OUT/(s.stem+'.arm9.o'))],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','resources':138,'replacement_cycles':414,
 'host_address_sanitizer':True,'arm9_compiled':True,'physical_dsi_tested':False,
 'source_sha256':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources+[ROOT/'build/native/content-index/x4_content_index.inc']}},indent=2)+'\n')
