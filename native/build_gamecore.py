"""Compile decompiled game C for ARM9; report unresolved platform/game services.

This does not link an NDS and must never be described as playable.
"""
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import subprocess
import shutil

ROOT=Path(__file__).resolve().parents[1]
REF=ROOT/'referencias/mmx4-decomp'
OUT=ROOT/'build/native/gamecore'
SDK=ROOT/'ambiente/sdk/devkitARM/bin'

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    # Upstream MMX4_PC includes 64-bit-only size assertions. Keep the layout
    # checks enabled and select its existing 32-bit expectations on ARM9.
    includes=OUT/'include'
    shutil.copytree(REF/'include',includes,dirs_exist_ok=True)
    obj_header=includes/'obj.h'
    text=obj_header.read_text()
    for marker in ('pc_object_header_size','pc_effect_object_size'):
        old='#ifdef MMX4_PC\nMMX4_STATIC_ASSERT('+marker
        assert text.count(old)==1
        text=text.replace(old,'#if defined(MMX4_PC) && __SIZEOF_POINTER__ == 8\nMMX4_STATIC_ASSERT('+marker)
    obj_header.write_text(text)
    sources=[]
    for folder in ('','data','effects','items','layers','mains','misc','quads','shots','unks','visuals','weapons'):
        sources.extend(sorted((REF/'src/main'/folder).glob('*.c')))
    flags=['-std=gnu11','-Os','-mcpu=arm946e-s','-mthumb','-mthumb-interwork',
           '-mfloat-abi=soft','-fno-strict-aliasing','-ffunction-sections','-fdata-sections',
           '-DMMX4_PC=1','-D__psyz=1','-DSKIP_MDEC=1','-DVERSION_US=1',
           '-Wno-incompatible-pointer-types','-Wno-int-conversion',
           '-I'+str(includes),'-I'+str(REF/'tools/psyz/psyz/include')]
    # Reuse objects only when flags, upstream headers and source content match.
    key=hashlib.sha256(json.dumps(flags).encode())
    for folder in (includes,REF/'tools/psyz/psyz/include'):
        for header in sorted(folder.rglob('*.h')):
            key.update(str(header.relative_to(folder)).encode());key.update(header.read_bytes())
    build_key=key.hexdigest()
    old_path=OUT/'report.json'
    old=json.loads(old_path.read_text()) if old_path.is_file() else {}
    cached={r['source']:r for r in old.get('sources',[])} if old.get('build_key')==build_key else {}
    def compile(path):
        name=str(path.relative_to(REF/'src')).replace('/','_')
        obj=OUT/(name+'.o')
        digest=hashlib.sha256(path.read_bytes()).hexdigest()
        previous=cached.get(str(path.relative_to(REF)))
        if previous and previous['ok'] and previous['sha256']==digest and obj.is_file():
            return {**previous,'reused':True}
        p=subprocess.run([str(SDK/'arm-none-eabi-gcc'),*flags,'-c',str(path),'-o',str(obj)],
                         stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        (OUT/(name+'.log')).write_text(p.stdout)
        return {'source':str(path.relative_to(REF)),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                'object':str(obj),'ok':p.returncode==0,'diagnostic':p.stdout[-1600:] if p.returncode else ''}
    first=compile(REF/'src/main/player.c')
    if not first['ok']:raise SystemExit(first['diagnostic'])
    results=[]
    with ThreadPoolExecutor(max_workers=4) as pool:
        for i,result in enumerate(pool.map(compile,sources),1):
            results.append(result)
            if i%50==0 or i==len(sources):
                print(f'ARM9: {i}/{len(sources)} fontes verificadas',flush=True)
    failures=[r for r in results if not r['ok']]
    report={'build_key':build_key,'reused':sum(bool(r.get('reused')) for r in results),
            'revision':subprocess.check_output(['git','-C',str(REF),'rev-parse','HEAD'],text=True).strip(),
            'compiled':len(results)-len(failures),'failed':len(failures),'runtime_linked':False,
            'playable':False,'sources':results}
    if not failures:
        obj=OUT/'gamecore.o'
        subprocess.run([str(SDK/'arm-none-eabi-ld'),'-r','-o',str(obj),*[r['object'] for r in results]],check=True)
        undefined=subprocess.check_output([str(SDK/'arm-none-eabi-nm'),'-u',str(obj)],text=True)
        (OUT/'required_services.txt').write_text(undefined)
        report['size']=subprocess.check_output([str(SDK/'arm-none-eabi-size'),str(obj)],text=True)
        report['undefined_count']=len(undefined.splitlines())
    (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k!='sources'},indent=2))
    for failure in failures[:5]:print(failure['source']+'\n'+failure['diagnostic'])
    if failures:raise SystemExit(1)

if __name__=='__main__':main()
