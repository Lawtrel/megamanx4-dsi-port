"""Bake native PCM effects offline. No synthesizer/device code goes into ARM9.

Direct commands use original program, tone, pitch and ADSR. Sequence commands
are listed explicitly until their event player has been converted too.
"""
from pathlib import Path
import struct,json,re,hashlib,array
from sample_baker import load_baker
ROOT=Path(__file__).resolve().parents[1];REF=ROOT/'referencias/mmx4-decomp';PSYZ=REF/'tools/psyz'
OUT=ROOT/'build/native/full-audio';DEST=OUT/'sfx';DEST.mkdir(parents=True,exist_ok=True)
dsp=load_baker(OUT)
table=re.search(r'pitch_table\[\] = \{(.*?)\};',(PSYZ/'decomp/src/libsnd/vm_n2p.c').read_text(),re.S)[1]
pitches=[int(n,16) for n in re.findall(r'0x[0-9A-Fa-f]+',table)]
resources=json.loads((ROOT/'build/native/content-index/resource_index.json').read_text())['resources']
def u16(b,o):return struct.unpack_from('<H',b,o)[0]
def u32(b,o):return struct.unpack_from('<I',b,o)[0]
def members(r):
 raw=(ROOT/'build/native/full-content'/r['name']).read_bytes();cursor=2048;out=[]
 for i in range(u32(raw,0)):
  kind,size=struct.unpack_from('<II',raw,8+i*8);out.append((kind,raw[cursor:cursor+size]));cursor+=(size+2047)&~2047
 return out
def instruments(header,body):
 off=u32(header,0)&0xffffff;vab=header[off:];assert vab[:4]==b'pBAV'
 version=u32(vab,4);nprogs=u16(vab,18);nsamples=u16(vab,22);nslots=128 if version>=5 else 64
 tonebase=32+nslots*16;lengthbase=tonebase+nprogs*512
 lengths=[u16(vab,lengthbase+i*2)*(8 if version>=5 else 4) for i in range(nsamples+1)]
 offsets=[];pos=lengths[0]
 for n in range(nsamples):offsets.append(pos);pos+=lengths[n+1]
 assert pos<=len(body),(pos,len(body))
 progs={};dense=0
 for program in range(nslots):
  p=vab[32+program*16:48+program*16]
  if not p[0]:continue
  tones=[]
  for t in range(16):
   raw=vab[tonebase+dense*512+t*32:tonebase+dense*512+(t+1)*32]
   sample=u16(raw,22)
   tones.append({'raw':raw,'sample':sample,'offset':offsets[sample-1] if 0<sample<=len(offsets) else None,
    'length':lengths[sample] if 0<sample<len(lengths) else 0,'program_volume':p[1],'bank_volume':vab[24]})
  progs[program]=tones;dense+=1
 assert dense==nprogs
 return progs
def pitch(t,note,fine):
 step=(fine+t[5])//8;carry=int(step>=16);step-=carry*16
 semitones=carry+note+60-t[4];octave=semitones//12;index=(semitones%12)*16+step
 assert 0<=index<len(pitches)
 value=pitches[index];shift=octave-5
 return min(0x3fff,value<<shift if shift>0 else value>>-shift)
def volume(t):
 raw=t['raw'];v=raw[2]*t['bank_volume']*16383//16129
 v=v*t['program_volume']*raw[2]//16129
 # Native mono: strongest of the two instrument pan channels, matching SsSetMono.
 return min(16383,v*v//16383)
def main():
 records=[];missing=[];cache={};total=0
 # Reuse existing, hash-verified bakes when the descriptor layout is adjusted.
 old={};previous=OUT/'sfx_manifest.json';diskcache=OUT/'sfx-cache';diskcache.mkdir(exist_ok=True)
 if previous.is_file():
  for record in json.loads(previous.read_text())['banks']:
   p=DEST/record['file']
   if not p.is_file():continue
   raw=p.read_bytes()
   if hashlib.sha256(raw).hexdigest()!=record['sha256']:continue
   start=16+len(record['entries'])*16
   for d in record['entries']:
    if d['samples']:old[(record['resource'],record['bank'],d['index'],d['command'])]=(raw[start+d['offset']:start+d['offset']+d['samples']*2],bool(d['loop']))
 for r in resources:
  if r['format']!='container':continue
  entries=members(r);header=None
  bankmap={}
  for kind,data in entries:
   if kind in (4,6,7,8):header=data
   elif kind>>16==2:
    assert header;bankmap[(u32(header,0)>>24)]=(header,data,instruments(header,data))
  header=None
  for kind,data in entries:
   if kind in (4,6,7,8):header=data
   elif kind>>16==2:
    bank=(kind>>8)&255;off=u32(header,0)&0xffffff;commands=[u32(header,p) for p in range(8,off,4)]
    descriptors=[];pcm=bytearray()
    for index,c in enumerate(commands):
     descriptor={'index':index,'command':c,'offset':len(pcm),'samples':0,'loop':0,'status':'empty'}
     if not c:descriptors.append(descriptor);continue
     packed=c.to_bytes(4,'little');which=packed[0]>>6
     if which<2:
      access=c>>8&127;sequence=(c>>24&31) if which==1 else 0
      descriptor['status']='sequence_external'
      if access>1 or sequence>=(2 if access else 8):
       descriptor['status']='sequence_pending';missing.append({'resource':r['id'],'bank':bank,'index':index,'command':c})
      descriptors.append(descriptor);continue
     target=bankmap.get(packed[0]&63)
     if target is None:
      # These overlays keep the character banks already loaded by the engine.
      retained={(0x6b,0):0x4d,(0x71,1):0x4b}.get((r['id'],packed[0]&63))
      if retained is not None:
       base=next(item for item in resources if item['id']==retained);base_header=None
       for base_kind,base_data in members(base):
        if base_kind in (4,6,7,8):base_header=base_data
        elif base_kind>>16==2 and u32(base_header,0)>>24==packed[0]&63:
         target=(base_header,base_data,instruments(base_header,base_data));descriptor['retained_resource']=retained;break
     if target is None:
      descriptor['status']='cross_bank_pending';missing.append({'resource':r['id'],'bank':bank,'index':index,'command':c});descriptors.append(descriptor);continue
     h,body,progs=target;program=packed[1]&127;tone=packed[2]>>4;voices=1+((packed[3]>>5)&3)
     signature=hashlib.sha256(h+body+packed).hexdigest()
     cp=diskcache/(signature+'.pcm');meta=diskcache/(signature+'.json')
     if signature not in cache and cp.is_file() and meta.is_file():cache[signature]=(cp.read_bytes(),json.loads(meta.read_text())['loop'])
     if signature not in cache and (r['id'],kind&255,index,c) in old:cache[signature]=old[(r['id'],kind&255,index,c)]
     if signature in cache:rendered,loop=cache[signature]
     else:
      dsp.bake_reset(body,len(body));loop=False;valid=0
      for voice in range(voices):
       if program not in progs or tone>=16:break
       t=progs[program][tone];raw=t['raw']
       if t['offset'] is not None:
        assert t['offset']%8==0 and len(body)+0x1000<=512*1024
        for p in range(t['offset'],t['offset']+t['length'],16):
         flags=body[p+1]
         if flags&1:loop|=bool(flags&2);break
        dsp.bake_voice(voice,t['offset'],pitch(raw,raw[6],raw[5]),u16(raw,16),u16(raw,18),volume(t));valid+=1
       tone+=1
       if tone==16:program+=1;tone=0
      buf=array.array('h',[0])*(441*2);mono=array.array('h');clock=0;last_nonzero=0;limit=44100*(3 if loop else 20)
      while valid and clock<limit:
       dsp.bake_pull(buf,441)
       for j in range(441):
        # Exact rational rate reduction 44100 -> 18900 (3/7).
        if ((clock+j)*3)//7!=((clock+j+1)*3)//7:mono.append((int(buf[2*j])+int(buf[2*j+1]))//2)
       clock+=441
       if dsp.bake_active():last_nonzero=clock
       elif clock>441 and clock-last_nonzero>441:break
      if not loop:
       while mono and abs(mono[-1])<=1:mono.pop()
      rendered=mono.tobytes();cache[signature]=(rendered,loop)
     if not cp.is_file():cp.write_bytes(rendered);meta.write_text(json.dumps({'loop':loop}))
     descriptor.update(samples=len(rendered)//2,loop=int(loop),status='silent' if not rendered else 'loop_approximation' if loop else 'rendered')
     pcm.extend(rendered);descriptors.append(descriptor)
    # Fixed little-endian descriptors, stable ABI read by native mixer.
    filename=f'{r["id"]:02x}_{bank}.sfx';table_bytes=b''.join(struct.pack('<IIII',d['command'],d['offset'],d['samples'],d['loop']|(2 if d['status']=='silent' else 0)) for d in descriptors)
    payload=b'X4SFX01\0'+struct.pack('<II',len(descriptors),18900)+table_bytes+pcm
    (DEST/filename).write_bytes(payload);total+=len(payload)
    records.append({'resource':r['id'],'name':r['name'],'bank':bank,'file':filename,'sha256':hashlib.sha256(payload).hexdigest(),'bytes':len(payload),'entries':descriptors})
  print('Prepared SFX',r['name'],flush=True)
 report={'bank_count':len(records),'bytes':total,'banks':records,'pending':missing,
  'limitations':['SEP commands with no matching sequence remain unsupported','Looped direct effects use an approximate three-second PCM cycle','PCM conversion is mono; instrument stereo and reverb remain pending'],
  'runtime':'Native PCM mixing; host synthesizer excluded from NDS'}
 (OUT/'sfx_manifest.json').write_text(json.dumps(report,indent=2)+'\n')
 print('SFX banks',len(records),'bytes',total,'pending',len(missing),flush=True)

if __name__=='__main__':main()
