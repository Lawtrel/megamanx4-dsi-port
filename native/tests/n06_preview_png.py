"""Convert diagnostic PPM files to PNG using only the standard library."""
from pathlib import Path
import struct,zlib
ROOT=Path(__file__).resolve().parents[2];HOST=ROOT/'build/native/n05-flow/host'
def chunk(tag,data):return struct.pack('>I',len(data))+tag+data+struct.pack('>I',zlib.crc32(tag+data)&0xffffffff)
for path in sorted([*HOST.glob('sigma*.ppm'),*HOST.glob('jungle*.ppm'),*HOST.glob('menu*.ppm')]):
 header,w_h,maximum,rgb=path.read_bytes().split(b'\n',3)
 assert header==b'P6' and maximum==b'255';w,h=map(int,w_h.split());assert len(rgb)==w*h*3
 raw=b''.join(b'\0'+rgb[y*w*3:(y+1)*w*3] for y in range(h))
 png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>2I5B',w,h,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')
 path.with_suffix('.png').write_bytes(png);print(path.with_suffix('.png'))
