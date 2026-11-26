"""Convert all 80 named USA music/voice selections to native PCM offline."""
from pathlib import Path
import os,subprocess,sys,re,json,hashlib,struct
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from prepare_disc import RawDisc,sha256,EXPECTED_SHA256,disc_path
OUT=ROOT/'build/native/full-audio/music';OUT.mkdir(parents=True,exist_ok=True)
exe=(ROOT/'private/SLUS_005.61').read_bytes();assert hashlib.sha256(exe).hexdigest()=='dae43d44a9f3176208830ddbba0607bb627ebcd7faf9b36fc03dd0ffda34d2e8'
dump=disc_path();assert sha256(dump)==EXPECTED_SHA256
decoder=ROOT/'build/native/full-audio/xa_to_pcm'
if sys.platform=='emscripten':
 from x4_browser_native import run_xa
else:
 kit=os.environ.get('X4_HOST_TOOLS')
 if kit:
  decoder=Path(kit)/('xa_to_pcm.exe' if os.name=='nt' else 'xa_to_pcm')
 else:
  subprocess.run(['gcc','-std=gnu11','-O3',str(ROOT/'native/xa_to_pcm.c'),'-o',str(decoder)],check=True)
 def run_xa(*arguments):
  if os.name=='nt' and kit:
   source, lba, channel, start, end, target=arguments
   arguments=(os.path.relpath(source,ROOT),lba,channel,start,end,os.path.relpath(target,ROOT))
  subprocess.run([str(decoder),*arguments],check=True,cwd=ROOT)
archive_ids=[0x95,0x96,0x97,0x98,0x99,0x9b,0x9c,0x9d,0x9e,0x9f,0x9a]
enum=(ROOT/'referencias/mmx4-decomp/include/main/xa_sequence_data.h').read_text()
tracks=sorted((int(value,16),name) for name,value in re.findall(r'\b((?:MUSIC|VOICE)_\w+)\s*=\s*(0x[0-9A-Fa-f]+)',enum))
disc=RawDisc(dump);entries=[]
def offset(va):return va-0x80010000+2048
def word(va):return struct.unpack_from('<H',exe,offset(va))[0]
cachepath=OUT/'manifest.json';old=json.loads(cachepath.read_text()) if cachepath.exists() else {}
cache={t['selection']:t for t in old.get('tracks',[])} if old.get('decoder_sha256')==sha256(ROOT/'native/xa_to_pcm.c') else {}
try:
 directory=disc.directory(149185,2048)
 for selection,name in tracks:
  group,part=divmod(selection,8);ptr=struct.unpack_from('<I',exe,offset(0x800F19E0+group*4))[0];channel=0
  for _ in range(part):
   if word(ptr)&0x8000:channel+=1;ptr+=2
   ptr+=2
  start,end=word(ptr)&0x7fff,word(ptr+2)&0x7fff
  assert end>start and channel<8,(name,start,end,channel)
  # The last group is the boss-introduction XA; discover its actual name by
  # the executable's verified file LBA, rather than guessing a filename.
  metadata_lba=struct.unpack_from('<I',exe,offset(0x800F0E18+archive_ids[group]*12))[0]
  source=next((n for n,e in directory.items() if e['lba']==metadata_lba),None)
  assert source,(group,metadata_lba,list(directory))
  target=OUT/f'{selection:02x}.pcm';previous=cache.get(selection)
  if not(previous and target.exists() and sha256(target)==previous['sha256']):
   run_xa(str(dump),str(metadata_lba),str(channel),str(start),str(end),str(target))
  size=target.stat().st_size
  record={'selection':selection,'name':name,'source':source,'channel':channel,'start':start,'end':end,
   'sample_rate':18900,'format':'PCM16LE mono','bytes':size,'seconds':size/2/18900,'sha256':sha256(target),
   'one_shot':selection==0 or group>=4}
  entries.append(record)
  print(f'{selection:02X} {name}: {record["seconds"]:.2f}s',flush=True)
finally:disc.stream.close()
intro=next(t for t in entries if t['selection']==0x10)
assert intro['sha256']=='19464f2acbf1cdefd0f91007f00452145e843a9109d2d1378f4ec77eded7a3d6','Intro conversion regression'
cachepath.write_text(json.dumps({'disc_sha256':EXPECTED_SHA256,'decoder_sha256':sha256(ROOT/'native/xa_to_pcm.c'),
 'tracks':entries,'total_bytes':sum(t['bytes'] for t in entries),'physical_playback_tested':False},indent=2)+'\n')
print('Prepared',len(entries),'native PCM music/voice selections')
