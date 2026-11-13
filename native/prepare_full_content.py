"""Prepare a private native asset pack for all ARC resources, never install it.

This supplies data for future complete game integration, not a game executable.
"""
from pathlib import Path
import sys,json,re,struct
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from prepare_disc import RawDisc,sha256,EXPECTED_SHA256,disc_path
OUT=ROOT/'build/native/full-content';OUT.mkdir(parents=True,exist_ok=True)
dump=disc_path();assert sha256(dump)==EXPECTED_SHA256
disc=RawDisc(dump);entries=[]
try:
    for name,info in sorted(disc.directory(23,10240).items()):
        assert re.fullmatch(r'[A-Z0-9_.]+',name)
        if info['directory'] or not name.endswith('.ARC'):continue
        target=OUT/name
        raw=disc.read(info['lba'],info['size'])
        target.write_bytes(raw)
        container=len(raw)>=2048 and len(raw)%2048==0 and struct.unpack_from('<I',raw,4)[0]==len(raw)
        entries.append({'name':name,'bytes':len(raw),'sha256':sha256(target),'lba':info['lba'],
                        'format':'container' if container else 'raw-bank'})
finally:disc.stream.close()
(OUT/'manifest.json').write_text(json.dumps({'disc_sha256':EXPECTED_SHA256,
    'scope':'All original ARC resources; complete game runtime not yet integrated',
    'files':entries,'total_bytes':sum(e['bytes'] for e in entries)},indent=2)+'\n')
print('Prepared',len(entries),'ARC files,',sum(e['bytes'] for e in entries),'bytes')
