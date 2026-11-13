from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n063-arm7-tests';OUT.mkdir(parents=True,exist_ok=True)
mock=ROOT/'native/tests/arm7_audio_mock'
sources=[ROOT/p for p in ('native/dsi/n063_arm7_audio.c','native/src/x4_sfx.c','native/src/x4_sfx.h','native/src/x4_arm7_audio.h','native/tests/n063_arm7_audio_test.c')]+sorted(mock.rglob('*.h'))
subprocess.run(['gcc','-std=c11','-O2','-g','-fsanitize=address,undefined','-I'+str(mock),'-I'+str(ROOT/'native/src'),str(sources[0]),str(sources[1]),str(sources[4]),'-o',str(OUT/'arm7_test')],check=True)
p=subprocess.run([str(OUT/'arm7_test'),str(OUT)],capture_output=True,text=True);print(p.stdout,p.stderr);assert not p.returncode
sdk=ROOT/'ambiente/sdk'
subprocess.run([str(sdk/'devkitARM/bin/arm-none-eabi-gcc'),'-std=gnu11','-O2','-mcpu=arm946e-s','-marm','-DARM9=1','-D__NDS__=1','-I'+str(sdk/'libnds/include'),'-I'+str(sdk/'calico/include'),'-I'+str(ROOT/'native/src'),'-c',str(sources[0]),'-o',str(OUT/'arm7_control.arm9.o')],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','sources':{str(s.relative_to(ROOT)):hashlib.sha256(s.read_bytes()).hexdigest() for s in sources},'real_arm9_control_backend':True,'arm7_protocol_recording':True,'physical_dsi_tested':False},indent=2)+'\n')
