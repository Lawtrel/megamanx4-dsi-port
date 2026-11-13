from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n061-gpu-tests';OUT.mkdir(parents=True,exist_ok=True)
sources=[ROOT/p for p in ['native/dsi/n06_gpu.c','native/src/x4_video.c','native/src/x4_video.h','native/tests/n061_gpu_abi_test.c','native/tests/nds_gpu_mock/nds.h']]
subprocess.run(['gcc','-std=c11','-O2','-fsanitize=address,undefined','-I'+str(ROOT/'native/tests/nds_gpu_mock'),'-I'+str(ROOT/'native/src'),str(sources[0]),str(sources[1]),str(sources[3]),'-o',str(OUT/'gpu_test')],check=True)
p=subprocess.run([str(OUT/'gpu_test')],capture_output=True,text=True);print(p.stdout,p.stderr)
(OUT/'validation.json').write_text(json.dumps({'status':'passed' if not p.returncode else 'failed','sources':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources},'host_gpu_command_recording':True,'physical_dsi_tested':False},indent=2)+'\n')
assert not p.returncode
