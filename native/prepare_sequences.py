"""Host-only conversion of original short SEP instruments into native PCM.
No SEP/MIDI player or sound-chip implementation is linked into the NDS.
"""
from prepare_sfx import ROOT,OUT,DEST,resources,members,u16,u32,instruments,pitch,dsp,REVERB_DEPTH,TONE_MODE_REVERB
import struct,array,json,hashlib
def sequences(raw,offset,count):
 result=[];position=offset
 for i in range(count):
  position+=8 if i==0 else 2
  resolution=int.from_bytes(raw[position:position+2],'big');tempo=int.from_bytes(raw[position+2:position+5],'big');position+=7
  size=int.from_bytes(raw[position:position+4],'big');position+=4;data=raw[position:position+size];position+=size
  assert len(data)==size and resolution>0 and tempo>0
  result.append((resolution,tempo,data))
 return result
def events(sequence):
 resolution,tempo,raw=sequence;pos=0;tick=0;clock=0;running=0;out=[]
 def byte():
  nonlocal pos
  assert pos<len(raw);value=raw[pos];pos+=1;return value
 def vlq():
  value=0
  for _ in range(4):
   n=byte();value=(value<<7)|(n&127)
   if n<128:return value
  raise ValueError('Oversized delta')
 while pos<len(raw):
  delta=vlq();clock+=delta*tempo/(resolution*1000000);status=byte()
  if status<128:pos-=1;status=running
  else:running=status
  assert status>=128
  if status==255:
   kind=byte();n=vlq();value=raw[pos:pos+n];pos+=n
   if kind==81:assert n==3;tempo=int.from_bytes(value,'big')
   elif kind==47:break
   else:assert kind in (88,89,0,1,3),('unsupported meta',kind)
   continue
  kind=status>>4;channel=status&15
  assert 8<=kind<=14,('unsupported event',status)
  a=byte();b=0 if kind in (12,13) else byte()
  out.append((round(clock*44100),kind,channel,a,b))
 return out,round(clock*44100)
manifest=[]
for character,resource in enumerate((0x4b,0x4d)):
 r=next(r for r in resources if r['id']==resource);banks={};header=None
 for kind,data in members(r):
  if kind in (4,6,7,8):header=data
  elif kind>>16==2:banks[kind>>8&255]=(header,data,instruments(header,data))
 sr=next(r for r in resources if r['name']==f'PL0{character}SEP.ARC')
 raw=(ROOT/'build/native/full-content'/sr['name']).read_bytes();offsets=struct.unpack_from('<II',raw)
 for access,count in enumerate((8,2)):
  h,body,progs=banks[access]
  for index,seq in enumerate(sequences(raw,offsets[access],count)):
   ev,end=events(seq);dsp.bake_reset(body,len(body),REVERB_DEPTH);program=list(range(16));vol=[127]*16;pan=[64]*16
   voices={};allocator=0;clock=0;mono=array.array('h');buf=array.array('h',[0])*(441*2);controls=set()
   def pull(until):
    global clock
    while clock<until:
     n=min(441,until-clock);dsp.bake_pull(buf,n)
     for j in range(n):
      if ((clock+j)*3)//7!=((clock+j+1)*3)//7:mono.append((int(buf[2*j])+int(buf[2*j+1]))//2)
     clock+=n
   for when,kind,ch,a,b in ev:
    pull(when)
    if kind==12:program[ch]=a
    elif kind==11:
     controls.add((a,b))
     if a==7:vol[ch]=b
     elif a==10:pan[ch]=b
     else:assert a in (0,32,64,91,98,99,100,101,6,38,121,123),('unhandled controller',a,b)
    elif kind==8 or (kind==9 and b==0):
     for voice in voices.pop((ch,a),[]):dsp.bake_keyoff(voice)
    elif kind==9:
     selected=progs.get(program[ch],[]);started=[]
     for tone in selected:
      t=tone['raw']
      if not(t[6]<=a<=t[7]) or tone['offset'] is None:continue
      voice=allocator%24;allocator+=1
      # Sequence velocity and channel volume feed the original instrument gain.
      gain=b*vol[ch]//127;seed=gain*tone['bank_volume']*16383//16129
      seed=seed*tone['program_volume']*t[2]//16129
      volume=min(16383,seed*seed//16383)
      p=pitch(t,a,0)
      dsp.bake_voice(voice,tone['offset'],p,u16(t,16),u16(t,18),volume,t[1]==TONE_MODE_REVERB);started.append(voice)
     voices[(ch,a)]=started
    elif kind in (10,13,14):raise ValueError(('Unconverted pressure or pitch event',kind,ch,a,b))
   for active in voices.values():
    for voice in active:dsp.bake_keyoff(voice)
   pull(end+44100*2)
   while mono and abs(mono[-1])<=1:mono.pop()
   filename=f'seq_{character}_{access}_{index}.pcm';payload=mono.tobytes();(DEST/filename).write_bytes(payload)
   record={'character':character,'access':access,'sequence':index,'file':filename,'samples':len(mono),'bytes':len(payload),
    'sha256':hashlib.sha256(payload).hexdigest(),'events':len(ev),'controls':sorted(controls)}
   manifest.append(record);print(filename,len(mono),len(ev),sorted(controls),flush=True)
(OUT/'sequences_manifest.json').write_text(json.dumps({'sequences':manifest,'limitations':['Mono PCM with per-tone reverb rendered offline; sequence controller modulation is incomplete']},indent=2)+'\n')
