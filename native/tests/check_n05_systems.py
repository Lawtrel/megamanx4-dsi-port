from pathlib import Path
import subprocess,json,hashlib,concurrent.futures,os,tempfile
ROOT=Path(__file__).resolve().parents[2];HOST=ROOT/'build/native/n05-flow/host';binary=HOST/'systems_test'
cases=[(m,c) for m in ('pause','respawn','transition','weapons','campaign') for c in range(2)]
if os.environ.get('X4_GPU')=='1':cases.extend([('sigma',0),('sigma',1),('introclear',0),('introclear',1),('introleft',0),('pausejungle',1),('pauseoptions',0),('pauseoptions',1),('saveprompt',0),('saveprompt',1),('ice',1),('restart',0)])
if os.environ.get('X4_GPU')=='1':cases.extend([('train0',0),('train0',1),('train1',0),('train1',1),('armorparts',0),('capsule',0)])
if os.environ.get('X4_GPU')=='1':cases.extend([(m,c) for m in ('titlemenu','traincars','savecycle') for c in range(2)])
if os.environ.get('X4_GPU')=='1':cases.extend([('nocutscenes',0),('nocutscenes',1)])
if os.environ.get('X4_GPU')=='1':cases.extend([('debugloadout',0),('debugloadout',1)])
if os.environ.get('X4_GPU')=='1':cases.extend([(m,c) for m in ('sigmagear','refight5','refight7','refight9') for c in range(2)])
if os.environ.get('X4_GPU')=='1':cases.extend([('selectgiga',0),('selectgiga',1)])

gpu=os.environ.get('X4_GPU')=='1';prefix='gpu_' if gpu else ''
def check(case):
 mode,character=case
 try:
  with tempfile.TemporaryDirectory(prefix='n07-save-',dir=HOST) as save_root:
   env=dict(os.environ,X4_SAVE_TEST_ROOT=save_root)
   p=subprocess.run([str(binary),str(ROOT/'build/native/full-content'),mode,str(character)],cwd=HOST,env=env,capture_output=True,text=True,timeout=120)
  log=p.stdout+p.stderr;code=p.returncode
 except subprocess.TimeoutExpired as e:log=((e.stdout or b'')+(e.stderr or b'')).decode(errors='replace')+'\nTIMEOUT';code=-999
 (HOST/f'{prefix}system_{mode}_{character}.log').write_text(log)
 print(mode,character,code,log[-1200:],flush=True);return {'mode':mode,'character':character,'returncode':code}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(check,cases))
(HOST/('systems_gpu.json' if gpu else 'systems.json')).write_text(json.dumps({'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),'cases':results,'host_only':True,'graphics_backend':'ds-command-collector' if gpu else 'software','physical_dsi_tested':False},indent=2)+'\n')
if any(r['returncode'] for r in results):raise SystemExit(1)
