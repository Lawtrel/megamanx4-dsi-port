#!/usr/bin/env python3
"""Prepare pinned source dependencies and the local SDK; never publish assets."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
UPSTREAM = 'https://github.com/sozud/mmx4.git'
REVISION = '4e16dfc7c06597fd5719dd112654d8cf85d343c2'
REF = ROOT / 'referencias/mmx4-decomp'

def git(*args, capture=False):
    command = ['git', '-C', str(REF), *args]
    if capture:
        return subprocess.check_output(command, text=True).strip()
    subprocess.run(command, check=True)

def prepare_sources(offline=False):
    if not (REF / '.git').exists():
        if offline:
            raise RuntimeError('Upstream absent. Run setup without --offline first.')
        REF.mkdir(parents=True, exist_ok=True)
        git('init')
        git('remote', 'add', 'origin', UPSTREAM)
        git('fetch', '--depth=1', 'origin', REVISION)
        git('checkout', '--detach', REVISION)
    if git('rev-parse', 'HEAD', capture=True) != REVISION:
        raise RuntimeError('Upstream revision differs; existing checkout was preserved.')
    psyz = REF / 'tools/psyz'
    expected = git('ls-tree', 'HEAD', 'tools/psyz', capture=True).split()[2]
    actual = ''
    if (psyz / '.git').exists():
        actual = subprocess.check_output(['git', '-C', str(psyz), 'rev-parse', 'HEAD'], text=True).strip()
    if actual != expected:
        if offline:
            raise RuntimeError('Pinned psyz source is missing; run online setup first.')
        git('submodule', 'update', '--init', '--depth=1', '--', 'tools/psyz')
        actual = subprocess.check_output(['git', '-C', str(psyz), 'rev-parse', 'HEAD'], text=True).strip()
        if actual != expected:
            raise RuntimeError('psyz revision differs from upstream gitlink')
    for patch in sorted((ROOT / 'patches').glob('*.patch')):
        reverse = subprocess.run(['git', '-C', str(REF), 'apply', '--reverse', '--check', str(patch)], capture_output=True)
        if reverse.returncode:
            git('apply', '--check', str(patch))
            git('apply', str(patch))
        git('apply', '--reverse', '--check', str(patch))
    print('Pinned upstream and native safety patches verified.', flush=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--offline', action='store_true', help='Verify local dependencies without downloading')
    parser.add_argument('--skip-sdk', action='store_true')
    parser.add_argument('--disc', type=Path, help='Original supported USA BIN; kept outside Git')
    parser.add_argument('--cue', type=Path)
    args = parser.parse_args()
    if sys.platform != 'linux' or sys.version_info < (3, 11):
        raise RuntimeError('Use Linux/WSL with Python 3.11 or newer.')
    for tool in ('git', 'gcc', 'make', 'tar'):
        import shutil
        if not shutil.which(tool):
            raise RuntimeError('Missing required tool: ' + tool)
    prepare_sources(args.offline)
    if not args.skip_sdk:
        compiler = ROOT / 'ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc'
        if not compiler.is_file():
            if args.offline:
                raise RuntimeError('Local SDK absent; run online setup first.')
            subprocess.run([sys.executable, str(ROOT / 'tools/setup_sdk.py')], check=True)
        print(subprocess.check_output([str(compiler), '--version'], text=True).splitlines()[0])
    if args.disc:
        command = [sys.executable, str(ROOT / 'tools/prepare_disc.py'), str(args.disc)]
        if args.cue:
            command.extend(['--cue', str(args.cue)])
        subprocess.run(command, check=True)
    print('Setup complete. Compile with: python3 native/compile.py --n06')

if __name__ == '__main__':
    main()
