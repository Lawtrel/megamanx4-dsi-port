from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/music-tests';OUT.mkdir(parents=True,exist_ok=True)
source=ROOT/'native/src/x4_music.c';driver=ROOT/'native/tests/music_driver.c'
subprocess.run(['gcc','-O2','-fsanitize=address','-I'+str(ROOT/'native/src'),str(source),str(driver),'-o',str(OUT/'music_test')],check=True)
p=subprocess.run([str(OUT/'music_test'),str(ROOT/'build/native/full-audio/music')],capture_output=True,text=True)
(OUT/'result.log').write_text(p.stdout+p.stderr)
if p.returncode:raise RuntimeError(p.stderr[-3000:])
print(p.stdout,end='')
subprocess.run([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'),'-std=gnu11','-Os','-mcpu=arm946e-s','-mthumb','-I'+str(ROOT/'native/src'),'-c',str(source),'-o',str(OUT/'x4_music.arm9.o')],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','selections':80,'host_address_sanitizer':True,'arm9_compiled':True,'physical_playback_tested':False,
 'source_sha256':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in (source,driver)}},indent=2)+'\n')
