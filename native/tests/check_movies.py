"""Verify all movie assets and full native playback, separately from game routes."""
from pathlib import Path
import subprocess,json,hashlib
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/movie-tests';OUT.mkdir(parents=True,exist_ok=True)
manifest=json.loads((ROOT/'build/native/movies/manifest.json').read_text())
assert len(manifest['movies'])==11
for record in manifest['files']:
 p=ROOT/'build/native/movies/converted'/record['file']
 assert p.stat().st_size==record['bytes'] and hashlib.sha256(p.read_bytes()).hexdigest()==record['sha256']
source=ROOT/'native/src/x4_movie.c';driver=ROOT/'native/tests/movie_driver.c'
subprocess.run(['gcc','-O2','-fsanitize=address,undefined','-I'+str(ROOT/'native/src'),str(source),str(driver),'-o',str(OUT/'movie_test')],check=True)
p=subprocess.run([str(OUT/'movie_test'),str(ROOT/'build/native/movies/converted')],cwd=OUT,capture_output=True,text=True)
(OUT/'result.log').write_text(p.stdout+p.stderr);print(p.stdout,flush=True)
if p.returncode:raise RuntimeError(p.stderr[-3000:])
subprocess.run([str(ROOT/'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'),'-O2','-mcpu=arm946e-s','-mthumb','-I'+str(ROOT/'native/src'),'-c',str(source),'-o',str(OUT/'movie.arm9.o')],check=True)
(OUT/'validation.json').write_text(json.dumps({'status':'passed','movies':11,'frames':sum(m['frames'] for m in manifest['movies']),'host_sanitizers':True,'arm9_compiled':True,'physical_dsi_tested':False,'sources':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (source,driver,ROOT/'native/src/x4_movie.h')},'assets_manifest_sha256':hashlib.sha256((ROOT/'build/native/movies/manifest.json').read_bytes()).hexdigest()},indent=2)+'\n')
