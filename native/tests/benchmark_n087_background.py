"""Actual background renderer: exact commands/pixels and ARM946 instructions, not FPS."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys,struct
ROOT=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--build-only',action='store_true');p.add_argument('--run-only',action='store_true')
p.add_argument('--python-path',type=Path);p.add_argument('--baseline',default='5557bce')
p.add_argument('--output',type=Path,default=Path('build/native/n087-background-benchmark'))
args=p.parse_args();out=ROOT/args.output;fixture=ROOT/'native/tests/n087_background_benchmark.c'
labels=('three_layers','negative_edges','train_seam','live_mutations','disabled_layers','invalid_bounds')
sha=lambda b:hashlib.sha256(b).hexdigest()
if not args.run_only:
 out.mkdir(parents=True,exist_ok=True);sdk=ROOT/'ambiente/sdk/devkitARM/bin';ref=ROOT/'referencias/mmx4-decomp'
 source_inputs={}
 for label,source in [('before',subprocess.check_output(['git','show',args.baseline+':native/dsi/n05_render.c'],cwd=ROOT)),('after',(ROOT/'native/dsi/n05_render.c').read_bytes())]:
  target=out/(label+'.c');target.write_bytes(source+b'\nint x4_background_probe(uint16_t* s,const X4Archive* a,int x,int y){return background(s,a,x,y);}\n')
  source_inputs[label]={'source_sha256':sha(source)}
 defines=['-DMMX4_PC=1','-D__psyz=1','-DSKIP_MDEC=1','-DVERSION_US=1','-DX4_FULL_FLOW=1']
 common=['-std=gnu11','-O3','-fno-strict-aliasing','-fwrapv','-ffunction-sections','-fdata-sections',*defines]
 includes=[ref/'src/pc',ref/'tools/psyz/psyz/include',ROOT/'native/src']
 probes={}
 for label in source_inputs:
  sources=[out/(label+'.c'),ROOT/'native/src/x4_video.c',ROOT/'native/src/x4_archive.c',fixture]
  for host in (True,False):
   inc=[ref/'include' if host else ROOT/'build/native/gamecore/include',*includes]
   flags=[*common,*['-I'+str(d) for d in inc]]
   compiler='gcc' if host else str(sdk/'arm-none-eabi-gcc')
   flags+=['-fsanitize=address,undefined','--param=asan-globals=0'] if host else ['-marm','-mcpu=arm946e-s']
   objects=[]
   for i,source in enumerate(sources):
    obj=out/f'{label}_{"host" if host else "arm"}_{i}.o'
    subprocess.run([compiler,*flags,'-c',str(source),'-o',str(obj)],check=True);objects.append(str(obj))
   binary=out/(label+('_host' if host else '.elf'))
   link=['-fsanitize=address,undefined','-Wl,--gc-sections'] if host else ['-nostdlib','-marm','-mcpu=arm946e-s','-Wl,--gc-sections,-Ttext=0x10000,-Tdata=0x20000,-e,unit_entry']
   subprocess.run([compiler,*link,*objects,*([] if host else ['-lc','-lgcc']),'-o',str(binary)],check=True)
   if not host:
    subprocess.run([str(sdk/'arm-none-eabi-objcopy'),'-O','binary',str(binary),str(out/(label+'.bin'))],check=True)
    symbols=subprocess.check_output([str(sdk/'arm-none-eabi-nm'),str(binary)],text=True)
    addresses={k:int(next(l.split()[0] for l in symbols.splitlines() if l.endswith(' '+k)),16) for k in ('unit_entry','benchmark_start','benchmark_end','result','result_size')}
    probes[label]={**addresses,**source_inputs[label],'binary_sha256':sha((out/(label+'.bin')).read_bytes())}
 host_results={}
 for mode,name in enumerate(labels):
  before=subprocess.check_output([str(out/'before_host'),str(mode)])
  after=subprocess.check_output([str(out/'after_host'),str(mode)])
  assert before==after,name
  host_results[name]={'bytes':len(after),'sha256':sha(after)}
  print('PASS host commands/RGBA',name,len(after),flush=True)
 (out/'probes.json').write_text(json.dumps(probes,indent=2)+'\n')
 (out/'host_validation.json').write_text(json.dumps({'status':'passed','results':host_results,'before':source_inputs['before'],'after':source_inputs['after'],'fixture_sha256':sha(fixture.read_bytes()),'video_sha256':sha((ROOT/'native/src/x4_video.c').read_bytes()),'archive_sha256':sha((ROOT/'native/src/x4_archive.c').read_bytes())},indent=2)+'\n')
if args.build_only:sys.exit(0)
if args.python_path:sys.path.insert(0,str(args.python_path.resolve()))
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0,UC_ARM_REG_PC,UC_CPU_ARM_946
probes=json.loads((out/'probes.json').read_text());results={}
for mode,name in enumerate(labels):
 case={};outputs={}
 for label,row in probes.items():
  code=(out/(label+'.bin')).read_bytes();assert sha(code)==row['binary_sha256']
  machine=Uc(UC_ARCH_ARM,UC_MODE_ARM);machine.ctl_set_cpu_model(UC_CPU_ARM_946)
  machine.mem_map(0x10000,0x800000);machine.mem_write(0x10000,code)
  machine.reg_write(UC_ARM_REG_SP,0x7f0000);machine.reg_write(UC_ARM_REG_LR,0x7e0000);machine.reg_write(UC_ARM_REG_R0,mode)
  count=[False,0]
  def hook(machine,address,size,data):
   if address==row['benchmark_start']:count[0]=True
   if address==row['benchmark_end']:count[0]=False
   if count[0]:count[1]+=1
  machine.hook_add(UC_HOOK_CODE,hook);machine.emu_start(row['unit_entry'],0x7e0000,count=30000000)
  assert machine.reg_read(UC_ARM_REG_PC)==0x7e0000,name
  size=struct.unpack('<I',machine.mem_read(row['result_size'],4))[0]
  outputs[label]=bytes(machine.mem_read(row['result'],size))
  case[label]={'instructions':count[1],'bytes':size,'output_sha256':sha(outputs[label])}
 assert outputs['before']==outputs['after'],name
 results[name]=case;print(name,case,flush=True)
(out/'execution.json').write_text(json.dumps({'cpu':'ARM946','metric':'executed background/tile instructions; excludes cycles/cache/DMA; not FPS','probes':probes,'results':results,'fixture_sha256':sha(fixture.read_bytes()),'video_sha256':sha((ROOT/'native/src/x4_video.c').read_bytes()),'archive_sha256':sha((ROOT/'native/src/x4_archive.c').read_bytes()),'physical_dsi_tested':False},indent=2)+'\n')
