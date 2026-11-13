from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/sfx-tests';OUT.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'build/native/full-audio/sfx_manifest.json').read_text())
seen=set()
for b in manifest['banks']:
 assert (b['resource'],b['bank']) not in seen;seen.add((b['resource'],b['bank']))
 data=(ROOT/'build/native/full-audio/sfx'/b['file']).read_bytes();assert hashlib.sha256(data).hexdigest()==b['sha256']
cases=OUT/'cases.txt';cases.write_text(''.join(f'{r} {b}\n' for r,b in sorted(seen)))
source=ROOT/'native/src/x4_sfx.c';driver=ROOT/'native/tests/sfx_driver.c'
subprocess.run(['gcc','-O2','-fsanitize=address','-I'+str(ROOT/'native/src'),str(source),str(driver),'-o',str(OUT/'sfx_test')],check=True)
p=subprocess.run([str(OUT/'sfx_test'),str(ROOT/'build/native/full-audio/sfx'),str(cases)],capture_output=True,text=True)
(OUT/'result.log').write_text(p.stdout+p.stderr)
print(p.stdout,end='')
if p.returncode:raise RuntimeError(p.stderr[-3000:])
subprocess.run([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'),'-std=gnu11','-Os','-mcpu=arm946e-s','-mthumb','-I'+str(ROOT/'native/src'),'-c',str(source),'-o',str(OUT/'x4_sfx.arm9.o')],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','host_address_sanitizer':True,'banks':len(seen),'physical_dsi_tested':False,
 'sources':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (source,driver)}},indent=2)+'\n')
