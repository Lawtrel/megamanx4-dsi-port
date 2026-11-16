"""Compare ARM946 preparation instructions and exact FIFO bytes, not FPS."""
from pathlib import Path
import argparse,subprocess,json,hashlib,sys
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'build/native/n083-gpu-benchmark'
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--build-only',action='store_true');parser.add_argument('--run-only',action='store_true')
parser.add_argument('--python-path',type=Path);parser.add_argument('--baseline',default='cf348d9')
parser.add_argument('--candidate',type=Path,default=Path('native/dsi/n06_gpu.c'))
parser.add_argument('--output',type=Path,default=Path('build/native/n083-gpu-benchmark'))
args=parser.parse_args();OUT=ROOT/args.output;fixture=ROOT/'native/tests/n083_gpu_benchmark.c'
if not args.run_only:
 OUT.mkdir(parents=True,exist_ok=True);sdk=ROOT/'ambiente/sdk/devkitARM/bin'
 before=OUT/'before.c';before.write_bytes(subprocess.check_output(['git','show',args.baseline+':native/dsi/n06_gpu.c'],cwd=ROOT))
 flags=['-O3','-marm','-mcpu=arm946e-s','-fno-strict-aliasing','-ffunction-sections','-fdata-sections',
        '-I'+str(ROOT/'native/tests/nds_gpu_mock'),'-I'+str(ROOT/'native/src')]
 probes={}
 for name,source in [('before',before),('after',ROOT/args.candidate)]:
  objects=[]
  for i,p in enumerate([source,ROOT/'native/src/x4_video.c',fixture]):
   o=OUT/f'{name}_{i}.o';subprocess.run([str(sdk/'arm-none-eabi-gcc'),*flags,'-c',str(p),'-o',str(o)],check=True);objects.append(str(o))
  elf=OUT/(name+'.elf');subprocess.run([str(sdk/'arm-none-eabi-gcc'),'-nostdlib','-marm','-mcpu=arm946e-s','-Wl,--gc-sections,-Ttext=0x10000,-Tdata=0x20000,-e,unit_entry',*objects,'-lc','-lgcc','-o',str(elf)],check=True)
  subprocess.run([str(sdk/'arm-none-eabi-objcopy'),'-O','binary',str(elf),str(OUT/(name+'.bin'))],check=True)
  symbols=subprocess.check_output([str(sdk/'arm-none-eabi-nm'),str(elf)],text=True)
  addresses={key:int(next(l.split()[0] for l in symbols.splitlines() if l.endswith(' '+key)),16) for key in ('unit_entry','benchmark_start','benchmark_end')}
  probes[name]={**addresses,'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
                'binary_sha256':hashlib.sha256((OUT/(name+'.bin')).read_bytes()).hexdigest()}
 (OUT/'probes.json').write_text(json.dumps(probes,indent=2)+'\n')
if args.build_only:sys.exit(0)
if args.python_path:sys.path.insert(0,str(args.python_path.resolve()))
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_ARM,UC_HOOK_CODE
from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0,UC_ARM_REG_R1,UC_ARM_REG_PC,UC_CPU_ARM_946
probes=json.loads((OUT/'probes.json').read_text());results={}
for mode in range(4):
 case={}
 for name,row in probes.items():
  code=(OUT/(name+'.bin')).read_bytes();assert hashlib.sha256(code).hexdigest()==row['binary_sha256']
  machine=Uc(UC_ARCH_ARM,UC_MODE_ARM);machine.ctl_set_cpu_model(UC_CPU_ARM_946)
  machine.mem_map(0x10000,0x800000);machine.mem_write(0x10000,code)
  machine.reg_write(UC_ARM_REG_SP,0x7f0000);machine.reg_write(UC_ARM_REG_LR,0x7e0000)
  machine.reg_write(UC_ARM_REG_R0,600);machine.reg_write(UC_ARM_REG_R1,mode);counter=[False,0]
  def hook(machine,address,size,data):
   if address==row['benchmark_start']:counter[0]=True
   if address==row['benchmark_end']:counter[0]=False
   if counter[0]:counter[1]+=1
  machine.hook_add(UC_HOOK_CODE,hook);machine.emu_start(row['unit_entry'],0x7e0000,count=10000000)
  assert machine.reg_read(UC_ARM_REG_PC)==0x7e0000
  case[name]={'instructions':counter[1],'fifo_hash':machine.reg_read(UC_ARM_REG_R0)}
 assert case['before']['fifo_hash']==case['after']['fifo_hash'],case
 label=('world','mixed_hud','fade','gouraud')[mode];results[label]=case;print(label,case,flush=True)
report={'cpu':'ARM946','metric':'Executed preparation instructions; excludes cycles, cache and DMA; not FPS',
        'probes':probes,'results':results,'fixture_sha256':hashlib.sha256(fixture.read_bytes()).hexdigest(),
        'video_sha256':hashlib.sha256((ROOT/'native/src/x4_video.c').read_bytes()).hexdigest(),'physical_dsi_tested':False}
(OUT/'execution.json').write_text(json.dumps(report,indent=2)+'\n')
