#!/usr/bin/env python3
"""Audit only files selected for Git: source syntax, size and accidental assets."""
import ast
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
FORBIDDEN={'.bin','.cue','.iso','.img','.rom','.nds','.dsi','.elf','.o','.a','.so',
           '.dll','.exe','.pcm','.wav','.xa','.arc','.sav','.sst','.log','.ppm',
           '.srl','.zip','.7z','.gz','.tar','.png','.jpg','.jpeg'}
LOCAL_ROOTS={'private','build','dist','ambiente','referencias'}
TOKEN_PATTERNS=[re.compile(r'gh[pousr]_[A-Za-z0-9]{30,}'),
                re.compile(r'github_pat_[A-Za-z0-9_]{30,}'),
                re.compile(r'AKIA[A-Z0-9]{16}'),
                re.compile(r'-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----')]

def main():
    result=subprocess.run(['git','-C',str(ROOT),'ls-files','-z','--cached','--others','--exclude-standard'],capture_output=True,check=True)
    deleted=subprocess.run(['git','-C',str(ROOT),'ls-files','--deleted','-z'],capture_output=True,check=True)
    removed={name.decode('utf-8') for name in deleted.stdout.split(b'\0') if name}
    paths=sorted({name.decode('utf-8') for name in result.stdout.split(b'\0') if name}-removed)
    failures=[];total=0;python_files=0
    for name in paths:
        path=ROOT/name
        if not path.is_file():
            failures.append(name+': not an ordinary source file');continue
        if path.relative_to(ROOT).parts[0] in LOCAL_ROOTS or (path.suffix.lower() in FORBIDDEN and name!='native/Makefile.nds'):
            failures.append(name+': local/generated/game asset selected for Git');continue
        raw=path.read_bytes();total+=len(raw)
        if len(raw)>2*1024*1024:
            failures.append(name+': unexpectedly large source file')
        try:text=raw.decode('utf-8-sig')
        except UnicodeError:
            failures.append(name+': unexpected binary data');continue
        if any(pattern.search(text) for pattern in TOKEN_PATTERNS):
            failures.append(name+': possible credential; review locally without printing it')
        if path.suffix=='.py':
            try:ast.parse(text,filename=name);python_files+=1
            except SyntaxError as error:failures.append(name+': '+str(error))
    if failures:
        print('\n'.join(failures),file=sys.stderr);return 1
    print(f'PASS source tree: {len(paths)} files, {total} bytes, {python_files} Python sources; no selected game assets or known token patterns.')
    return 0

if __name__=='__main__':
    raise SystemExit(main())
