"""Validate the independent lower renderer and touch actions, with real atlas/font previews."""
from pathlib import Path
import subprocess,json,hashlib,os
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/lower-ui-tests';OUT.mkdir(parents=True,exist_ok=True)
SDK=ROOT/'ambiente/sdk/devkitARM/bin';ASSETS=ROOT/'build/native/lower-assets'
sources=[ROOT/p for p in ('native/src/x4_lower_ui.c','native/src/x4_lower_ui.h','native/src/x4_flow.h','native/tests/lower_ui_test.c')]
font_object=subprocess.check_output([str(SDK/'arm-none-eabi-ar'),'p',str(ROOT/'ambiente/sdk/libnds/lib/libnds9.a'),'default_font.bin.o']);(OUT/'font.o').write_bytes(font_object)
subprocess.run([str(SDK/'arm-none-eabi-objcopy'),'-O','binary','-j','.rodata.default_font_bin',str(OUT/'font.o'),str(OUT/'font.bin')],check=True)
subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-g','-fsanitize=address,undefined','-I'+str(ROOT/'native/src'),'-I'+str(ASSETS),str(sources[0]),str(sources[3]),'-o',str(OUT/'lower_ui_test')],check=True)
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0'
subprocess.run([str(OUT/'lower_ui_test'),str(OUT/'font.bin'),str(OUT)],env=env,check=True)
subprocess.run([str(SDK/'arm-none-eabi-gcc'),'-std=gnu11','-Os','-mcpu=arm946e-s','-mthumb','-I'+str(ROOT/'native/src'),'-I'+str(ASSETS),'-c',str(sources[0]),'-o',str(OUT/'lower_ui.arm9.o')],check=True)
try:
 from PIL import Image
 for p in OUT.glob('*.ppm'):Image.open(p).save(p.with_suffix('.png'))
except ImportError:pass
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(OUT/'validation.json').write_text(json.dumps({'status':'passed','physical_dsi_tested':False,'sources':{str(s.relative_to(ROOT)):sha(s) for s in sources},'atlas_sha256':sha(ASSETS/'x4_lower_icons.h')},indent=2)+'\n')
