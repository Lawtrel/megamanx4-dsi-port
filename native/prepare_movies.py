"""Prepare local 256x192/15fps bitmap movies and PCM from the verified dump.
FFmpeg decodes STR on the build computer; only bounded LZ10/PCM runs on DSi.
Set X4_FFMPEG to a local executable; FFmpeg is never bundled into the game.
"""
from pathlib import Path
import subprocess,sys,os,struct,json,hashlib,shutil
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from prepare_disc import RawDisc,disc_path,sha256,EXPECTED_SHA256
OUT=ROOT/'build/native/movies';DEST=OUT/'converted';DEST.mkdir(parents=True,exist_ok=True)

def run(*args):subprocess.run(list(map(str,args)),check=True)
def ffmpeg_path():
 value=os.environ.get('X4_FFMPEG') or shutil.which('ffmpeg')
 if not value:
  legacy=ROOT/'private/video-tools/imageio_ffmpeg/binaries/ffmpeg-win-x86_64-v7.1.exe'
  if legacy.is_file():value=str(legacy)
 if not value:raise RuntimeError('Install FFmpeg or set X4_FFMPEG to its executable.')
 return value
ffmpeg=ffmpeg_path()
def ffpath(p):
 if str(ffmpeg).endswith('.exe') and sys.platform=='linux':return subprocess.check_output(['wslpath','-w',str(p)],text=True).strip()
 return str(p)
encoder=OUT/'movie_encode'
run('gcc','-O3','-I'+str(ROOT/'native/src'),ROOT/'native/movie_encode.c',ROOT/'native/src/x4_movie.c','-o',encoder)
source_key=sha256(ROOT/'native/movie_encode.c')+sha256(ROOT/'native/src/x4_movie.c')
exe=(ROOT/'private/SLUS_005.61').read_bytes();dump=disc_path();assert sha256(dump)==EXPECTED_SHA256
old_path=OUT/'manifest.json';old=json.loads(old_path.read_text()) if old_path.is_file() else {};cache={e['id']:e for e in old.get('movies',[])} if old.get('encoder_key')==source_key else {}
records=[];disc=RawDisc(dump)
try:
 for i in range(11):
  file,expected,unused,arg9,skip=struct.unpack_from('<IHHHH',exe,0x800F1D04-0x80010000+2048+i*12)
  lba,size,_=struct.unpack_from('<III',exe,0x800F0E18-0x80010000+2048+file*12);assert size%2336==0
  video=DEST/f'{i:02d}.xmv';audio=DEST/f'{i:02d}.pcm';previous=cache.get(i)
  if not(previous and video.is_file() and audio.is_file() and sha256(video)==previous['video_sha256'] and sha256(audio)==previous['audio_sha256']):
   raw=OUT/f'{i:02d}.str';rgb=OUT/f'{i:02d}.raw';disc.stream.seek(lba*2352);raw.write_bytes(disc.stream.read(size//2336*2352))
   base=[ffmpeg,'-nostdin','-v','error','-y','-f','psxstr','-i',ffpath(raw)]
   run(*base,'-map','0:v:0','-an','-vf','scale=256:192:flags=area','-fps_mode','passthrough','-frames:v',expected,'-pix_fmt','bgr555le','-f','rawvideo',ffpath(rgb))
   run(encoder,rgb,video)
   with video.open('rb') as f:header=f.read(32)
   frames=struct.unpack_from('<I',header,20)[0]
   # Real packet count can differ from the maximum requested by the game.
   assert abs(frames-expected)<=3,(i,frames,expected)
   run(*base,'-map','0:a:0','-vn','-ac','1','-ar','18900','-f','s16le',ffpath(audio))
   with audio.open('r+b') as f:f.truncate(frames*1260*2)
   # Only two explicit intermediate files inside this movie build directory.
   rgb.unlink();raw.unlink()
  with video.open('rb') as f:frames=struct.unpack_from('<I',f.read(32),20)[0]
  record={'id':i,'source_file_id':file,'lba':lba,'frames':frames,'skip_mask':skip,'width':256,'height':192,'fps':15,'sample_rate':18900,'video_file':video.name,'audio_file':audio.name,'video_bytes':video.stat().st_size,'audio_bytes':audio.stat().st_size,'video_sha256':sha256(video),'audio_sha256':sha256(audio)}
  records.append(record);print('MOVIE',i,frames,'frames',record['video_bytes'],'video bytes',flush=True)
finally:disc.stream.close()
assets=[{'file':r[k+'_file'],'sha256':r[k+'_sha256'],'bytes':r[k+'_bytes']} for r in records for k in ('video','audio')]
old_path.write_text(json.dumps({'disc_sha256':EXPECTED_SHA256,'encoder_key':source_key,'movies':records,'files':assets,'native_codec':'independent LZ10 RGB555 bitmaps and PCM16 mono','physical_playback_tested':False},indent=2)+'\n')
print('Prepared and roundtrip-verified',len(records),'native movies',flush=True)
