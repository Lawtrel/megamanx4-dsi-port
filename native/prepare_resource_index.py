"""Resolve original resource IDs to local filenames offline, never at runtime.

The native runtime reads ordinary files. It receives neither PS1 addresses nor
CD sector commands. Keep the index tied to the user's verified USA executable.
"""
from pathlib import Path
import hashlib,json,struct
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/native/content-index';OUT.mkdir(parents=True,exist_ok=True)
exe=(ROOT/'private/SLUS_005.61').read_bytes()
assert hashlib.sha256(exe).hexdigest()=='dae43d44a9f3176208830ddbba0607bb627ebcd7faf9b36fc03dd0ffda34d2e8'
manifest=json.loads((ROOT/'build/native/full-content/manifest.json').read_text())
by_location={(r['lba'],r['bytes']):r for r in manifest['files']}
entries=[]
for resource_id in range(138):
    offset=0x800F0E18-0x80010000+2048+resource_id*12
    lba,size,flags=struct.unpack_from('<III',exe,offset)
    resource=by_location[(lba,size)]
    entries.append(dict(resource,id=resource_id))
assert len({r['name'] for r in entries})==138
lines=['/* Generated from the verified private dump. No game code. */']
for r in entries:
    lines.append('{"%s", %du, %du}, /* %02X */'%(r['name'],r['bytes'],r['format']=='container',r['id']))
(OUT/'x4_content_index.inc').write_text('\n'.join(lines)+'\n')
(OUT/'resource_index.json').write_text(json.dumps({'exe_sha256':hashlib.sha256(exe).hexdigest(),'resources':entries},indent=2)+'\n')
print('Resolved 138 original IDs to filenames and exact sizes')
