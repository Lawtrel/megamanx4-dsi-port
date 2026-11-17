"""Prepare original pause icons offline; generated game images stay local."""
from pathlib import Path
import json,subprocess,hashlib,struct
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/native/lower-assets';OUT.mkdir(parents=True,exist_ok=True)
REF=ROOT/'referencias/mmx4-decomp'
inputs=json.loads((ROOT/'build/native/n05-flow/host/systems_test_inputs.json').read_text())['sources']
sources=[ROOT/p for p in inputs if p.endswith('.c') and not p.endswith('n05_systems_driver.c')]
sources.append(ROOT/'native/lower_icons_bake.c')
flags=['-std=gnu11','-Os','-fno-strict-aliasing','-fwrapv','-ffunction-sections','-fdata-sections','-D__psyz=1','-DSKIP_MDEC=1','-DX4_N06_INDEXED=1','-Wno-incompatible-pointer-types','-Wno-int-conversion','-DMMX4_PC=1','-DVERSION_US=1','-DX4_FULL_FLOW=1','-Wl,--gc-sections']
flags += ['-I'+str(p) for p in (ROOT/'build/native/gamecore/include',REF/'src/pc',REF/'tools/psyz/include',REF/'tools/psyz/psyz/include',ROOT/'native/src',ROOT/'native/dsi',ROOT/'native/tests/thread_host_shim',ROOT/'build/native/content-index')]
dependencies=sources+list((ROOT/'build/native/gamecore/include').rglob('*.h'))+list((ROOT/'native/src').glob('*.h'))
key=hashlib.sha256(json.dumps(flags).encode()+b''.join(p.read_bytes() for p in sorted(dependencies))).hexdigest()
cache=OUT/'bake_key.txt'
if not (cache.is_file() and cache.read_text()==key and (OUT/'sprites.rgb555').is_file()):
 subprocess.run(['gcc',*flags,*map(str,sources),'-o',str(OUT/'bake')],check=True,cwd=ROOT)
 subprocess.run([str(OUT/'bake'),str(ROOT/'build/native/full-content'),str(OUT/'sprites.rgb555')],check=True,cwd=ROOT)
 cache.write_text(key)
raw=(OUT/'sprites.rgb555').read_bytes()
assert len(raw)==2*54*128*128*2
sprites=[]
for i in range(108):
 values=struct.unpack_from('<16384H',raw,i*32768)
 points=[(n%128,n//128) for n,v in enumerate(values) if v&0x8000]
 assert points,('empty source sprite',i)
 x0=min(x for x,y in points);y0=min(y for x,y in points)
 w=max(x for x,y in points)-x0+1;h=max(y for x,y in points)-y0+1
 # Keep small sprites at native size, shrink larger assemblies with nearest sampling.
 scale=min(1,24/w,24/h);dw=max(1,round(w*scale));dh=max(1,round(h*scale));out=[0]*576
 for y in range(dh):
  for x in range(dw):out[((24-dh)//2+y)*24+(24-dw)//2+x]=values[(y0+y*h//dh)*128+x0+x*w//dw]
 sprites.append(out)
# Weapon order follows acquisition bits, matching the original pause menu.
x_indices=[1,5,7,9,11,13,15,17,19]
z_indices=[49,5,13,3,7,9,1,11,15]
icons=[]
for character,indices in enumerate((x_indices,z_indices)):
 sheet=[sprites[character*54+i] for i in indices]
 sheet += [sprites[50+i] for i in range(4)]
 sheet += [sprites[character*54+45],sprites[character*54+35],sprites[character*54+36],sprites[character*54+37],sprites[character*54+48]]
 icons.append(sheet)
# Reserved UI colors plus deterministic weighted median-cut for original art.
palette=[0x1041,0x7fff,0x3106,0x7fe0,0x001f,0x03e0,0x03ff]
from collections import Counter
hist=Counter(v&32767 for sheet in icons for icon in sheet for v in icon if v&0x8000)
boxes=[list(hist)]
while len(boxes)<9:
 candidates=[(sum(hist[v] for v in b)*max(max((v>>c)&31 for v in b)-min((v>>c)&31 for v in b) for c in (0,5,10)),i) for i,b in enumerate(boxes) if len(b)>1]
 if not candidates:break
 _,i=max(candidates);b=boxes.pop(i)
 channel=max((0,5,10),key=lambda c:max((v>>c)&31 for v in b)-min((v>>c)&31 for v in b))
 b.sort(key=lambda v:((v>>channel)&31,v));total=sum(hist[v] for v in b);accum=0;split=1
 for j,v in enumerate(b[:-1],1):
  accum+=hist[v]
  if accum*2>=total:split=j;break
 boxes.extend((b[:split],b[split:]))
for box in boxes:
 n=sum(hist[v] for v in box);palette.append(sum((sum(((v>>c)&31)*hist[v] for v in box)//n)<<c for c in (0,5,10)))
palette=(palette+[0]*16)[:16]
def nearest(v):
 if not v&0x8000:return 0
 return min(range(1,16),key=lambda i:sum((((v>>c)&31)-((palette[i]>>c)&31))**2 for c in (0,5,10)))
lookup={v:nearest(v) for sheet in icons for icon in sheet for v in icon}
pixels=[[[lookup[v] for v in icon] for icon in sheet] for sheet in icons]
header='/* Generated locally from original X4 sprite assemblies; never commit. */\n#include <stdint.h>\n'
header+='static const uint16_t x4_lower_icon_palette[16]={'+','.join(map(str,palette))+'};\n'
header+='static const uint8_t x4_lower_icon_pixels[2][18][576]={'+','.join('{'+','.join('{'+','.join(map(str,icon))+'}' for icon in sheet)+'}' for sheet in pixels)+'};\n'
(OUT/'x4_lower_icons.h').write_text(header)
# Standalone visual contact sheet for reviewing the prepared resources.
with (OUT/'icons.ppm').open('wb') as f:
 f.write(b'P6\n432 48\n255\n')
 for y in range(48):
  for x in range(432):
   c=palette[pixels[y//24][x//24][(y%24)*24+x%24]]
   f.write(bytes(((c&31)*255//31,((c>>5)&31)*255//31,((c>>10)&31)*255//31)))
report={'status':'passed','icons':36,'palette_colors':16,'header_sha256':hashlib.sha256(header.encode()).hexdigest(),'bake_key':key,
 'sources':{str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (Path(__file__).resolve(),ROOT/'native/lower_icons_bake.c')},'physical_dsi_tested':False}
(OUT/'validation.json').write_text(json.dumps(report,indent=2)+'\n')
print('Prepared 36 original lower-screen icons and shared palette',flush=True)
