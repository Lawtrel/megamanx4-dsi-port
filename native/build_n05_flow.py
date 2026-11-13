"""Build native full-engine integration for physical DSi testing."""
from pathlib import Path
import subprocess,shutil,os,json,hashlib
ROOT=Path(__file__).resolve().parents[1];N06=True
OUT=ROOT/('build/native/n065-tiles' if N06 else 'build/native/n05-integrated');OUT.mkdir(parents=True,exist_ok=True)
SRC=OUT/'source';SRC.mkdir(exist_ok=True)
for n in ('x4_flow.h','x4_archive.h','x4_audio.h','x4_save.h','x4_n05_platform.h','x4_video.h','x4_sfx.h','x4_arm7_audio.h'):shutil.copyfile(ROOT/'native/src'/n,SRC/n)
shutil.copyfile(ROOT/'native/dsi/n05_flow_main.c',SRC/'main.c')
if N06:
 for name in ('n06_gpu.c','n063_arm7_audio.c'):shutil.copyfile(ROOT/'native/dsi'/name,SRC/name)
shutil.copyfile(ROOT/'build/native/n05-flow/flow.o',OUT/'flow.o')
nitro=OUT/'nitrofiles'
manifests=[('build/native/full-content/manifest.json','files','build/native/full-content','content'),
 ('build/native/full-audio/music/manifest.json','tracks','build/native/full-audio/music','music'),
 ('build/native/full-audio/sfx_manifest.json','banks','build/native/full-audio/sfx','sfx'),
 ('build/native/full-audio/sequences_manifest.json','sequences','build/native/full-audio/sfx','sfx')]
copied=[]
for mp,key,source,folder in manifests:
 m=json.loads((ROOT/mp).read_text());(nitro/folder).mkdir(parents=True,exist_ok=True)
 for row in m[key]:
  name=f'{row["selection"]:02x}.pcm' if folder=='music' else row.get('file',row.get('name'));p=ROOT/source/name
  assert hashlib.sha256(p.read_bytes()).hexdigest()==row['sha256'],str(p)
  dest=nitro/folder/name
  if not dest.is_file() or hashlib.sha256(dest.read_bytes()).hexdigest()!=row['sha256']:
   if N06 and not dest.exists():os.link(p,dest)
   else:shutil.copyfile(p,dest)
  copied.append({'path':str(dest.relative_to(nitro)),'sha256':row['sha256'],'bytes':p.stat().st_size})
# Reject stale or unexpected package assets, without deleting a directory.
expected={r['path'] for r in copied};actual={str(p.relative_to(nitro)) for p in nitro.rglob('*') if p.is_file()}
assert actual==expected,('unexpected NitroFS files',sorted(actual-expected))
build_id=hashlib.sha256((OUT/'flow.o').read_bytes()+b''.join(p.read_bytes() for p in sorted(SRC.glob('*.c')))+json.dumps(copied,sort_keys=True).encode()).hexdigest()[:20]
(SRC/'build_id.h').write_text('#define X4_BUILD_ID "'+build_id+'"\n')
(OUT/'build_id.txt').write_text(build_id+'\n')
make=(ROOT/'native/Makefile.nds').read_text()
(OUT/'Makefile').write_text(make)
env=os.environ.copy();sdk=ROOT/'ambiente/sdk'
env.update(DEVKITPRO=str(sdk),DEVKITARM=str(sdk/'devkitARM'),LIBNDS=str(sdk/'libnds'))
env['PATH']=str(sdk/'devkitARM/bin')+':'+str(sdk/'tools/bin')+':'+env['PATH']
subprocess.run(['make','-j4'],cwd=OUT,env=env,check=True)
binary=OUT/('mmx4_native_n065.nds' if N06 else 'mmx4_native_n05_flow.nds');assert binary.is_file()
(OUT/'assets.json').write_text(json.dumps(copied,indent=2)+'\n')
print('Built:',binary,'bytes',binary.stat().st_size,'sha256',hashlib.sha256(binary.read_bytes()).hexdigest())
