from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n06-video-tests';OUT.mkdir(parents=True,exist_ok=True)
sources=[ROOT/'native/src/x4_video.c',ROOT/'native/src/x4_video.h',ROOT/'native/tests/n06_video_test.c']
subprocess.run(['gcc','-std=c11','-O2','-g','-Wall','-Wextra','-fsanitize=address,undefined','-I'+str(ROOT/'native/src'),str(sources[0]),str(sources[2]),'-o',str(OUT/'video_test')],check=True)
p=subprocess.run([str(OUT/'video_test')],capture_output=True,text=True);print(p.stdout,p.stderr)
(OUT/'validation.json').write_text(json.dumps({'status':'passed' if not p.returncode else 'failed','sources':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources},'host_only':True},indent=2)+'\n')
assert not p.returncode
