"""Run already-built full-flow integration cases without mutating its binary."""
from pathlib import Path
import subprocess,json,sys,hashlib,os,concurrent.futures
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n05-flow/host'
binary=OUT/'flow_test';assert binary.is_file()
cases=[(0,0,1),(1,0,1),(0,1,0),(1,1,0)]
if '--all' in sys.argv:cases=[(c,s,a) for c in range(2) for s in range(13) for a in range(2) if not(s in (9,10) and a)]
results=[];gpu=os.environ.get('X4_GPU')=='1';prefix='gpu_' if gpu else ''
def check(case):
 print('CASE',case,flush=True)
 try:
  p=subprocess.run([str(binary),str(ROOT/'build/native/full-content'),*map(str,case)],cwd=OUT,capture_output=True,text=True,timeout=90)
  log=p.stdout+p.stderr;code=p.returncode
 except subprocess.TimeoutExpired as e:
  log=((e.stdout or b'')+(e.stderr or b'')).decode(errors='replace')+'\nTIMEOUT';code=-999
 (OUT/(prefix+'case_'+'_'.join(map(str,case))+'.log')).write_text(log)
 print('RESULT',case,code,log[-400:],flush=True);return {'case':case,'returncode':code}
with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:results=list(pool.map(check,cases))
(OUT/('matrix_gpu.json' if gpu else 'matrix.json')).write_text(json.dumps({'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest(),
 'host_only':True,'graphics_backend':'ds-command-collector' if gpu else 'software','physical_dsi_tested':False,'cases':results},indent=2)+'\n')
if any(r['returncode'] for r in results):raise SystemExit(1)
