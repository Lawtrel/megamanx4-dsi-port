"""Verify read-only lower-screen formatting without console hardware."""
from pathlib import Path
import hashlib,json,os,subprocess
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/native/player-panel-tests';OUT.mkdir(parents=True,exist_ok=True)
sources=[ROOT/'native/src/x4_player_panel.c',ROOT/'native/src/x4_player_panel.h',ROOT/'native/src/x4_flow.h',ROOT/'native/tests/player_panel_test.c']
subprocess.run(['gcc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-I'+str(ROOT/'native/src'),str(sources[0]),str(sources[-1]),'-o',str(OUT/'player_panel_test')],check=True)
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0'
subprocess.run([str(OUT/'player_panel_test')],env=env,check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','physical_dsi_tested':False,'sources':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}},indent=2)+'\n')
