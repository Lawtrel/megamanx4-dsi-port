"""Execute the asset-free ARM probes (optional dependency: unicorn==2.1.4).

Build with build_n064_arm_abi.py under the SDK's Linux/WSL environment first.
This tests ARM calling conventions, not a console or full game emulator.
"""
from pathlib import Path
import argparse,json,hashlib,sys
ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--python-path',type=Path,help='Optional local directory containing the Unicorn Python package')
args=parser.parse_args()
if args.python_path:sys.path.insert(0,str(args.python_path.resolve()))
from unicorn import Uc,UC_ARCH_ARM,UC_MODE_THUMB
from unicorn.arm_const import UC_ARM_REG_SP,UC_ARM_REG_LR,UC_ARM_REG_R0,UC_ARM_REG_PC,UC_CPU_ARM_946
folder=ROOT/'build/native/n064-arm-abi';probes=json.loads((folder/'probes.json').read_text())
results={}
for name,row in probes.items():
 code=(folder/(name+'.bin')).read_bytes();assert hashlib.sha256(code).hexdigest()==row['sha256']
 machine=Uc(UC_ARCH_ARM,UC_MODE_THUMB);machine.ctl_set_cpu_model(UC_CPU_ARM_946)
 machine.mem_map(0x10000,0x40000);machine.mem_write(0x10000,code)
 machine.reg_write(UC_ARM_REG_SP,0x4f000);machine.reg_write(UC_ARM_REG_LR,0x4e001)
 machine.emu_start(row['entry']|1,0x4e000,count=100000)
 assert machine.reg_read(UC_ARM_REG_PC)==0x4e000,'Probe did not return within its instruction budget'
 results[name]=machine.reg_read(UC_ARM_REG_R0)
assert results=={'before':3,'after':0},results
(folder/'execution.json').write_text(json.dumps({'cpu':'ARM946','results':results,'probes':probes,'physical_dsi_tested':False},indent=2)+'\n')
print('PASS actual ARM instructions: old caller retains both filters (3), fixed caller removes both (0).')
