"""Build and validate the current native ARM9 port; assets stay local."""
from pathlib import Path
import argparse
import datetime
import os
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
LOGS=ROOT/'build/native/logs'
LOGS.mkdir(parents=True,exist_ok=True)
logpath=LOGS/(datetime.datetime.now().strftime('compile_%Y%m%d_%H%M%S_%f')+'.log')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--n09','--n0815','--n0814','--n0813','--n0812','--n0811','--n0810','--n089','--n088','--n087','--n086','--n085','--n084','--n083','--n082','--n081','--n08','--n07','--n06',action='store_true',help='Compile the current beta 9 native target')
    parser.parse_args()
    if sys.platform!='linux':
        raise RuntimeError('Use COMPILAR_JOGO.cmd on Windows, or run under Linux/WSL.')
    sys.path.insert(0,str(ROOT/'tools'))
    from setup_native import prepare_sources
    prepare_sources(offline=True)
    for path in ('ambiente/sdk/devkitARM/bin/arm-none-eabi-gcc','native/Makefile.nds','private/SLUS_005.61'):
        if not (ROOT/path).is_file():
            raise RuntimeError('Missing '+path+'. Run tools/setup_native.py --disc /path/to/game.bin first.')
    os.environ.update(X4_N06='1',X4_GPU='1')
    with logpath.open('w',encoding='utf-8') as log:
        def say(message):
            print(message,flush=True);log.write(message+'\n');log.flush()
        def run(script,*arguments):
            say('\nExecutando: '+script)
            env=os.environ.copy();env['PYTHONUNBUFFERED']='1'
            with subprocess.Popen([sys.executable,script,*arguments],cwd=ROOT,env=env,stdout=subprocess.PIPE,
                                  stderr=subprocess.STDOUT,text=True,errors='replace') as process:
                for line in process.stdout:
                    print(line,end='',flush=True);log.write(line);log.flush()
                if process.wait():raise RuntimeError('Etapa interrompida: '+script)
        say('Beta 9: X4 nativo com retorno ao titulo apos os creditos.')
        say('Log: '+str(logpath))
        run('native/build_gamecore.py')
        if not (ROOT/'build/native/full-content/manifest.json').is_file():run('native/prepare_full_content.py')
        for script in ('prepare_resource_index.py','prepare_music.py','prepare_sfx.py','prepare_sequences.py'):
            run('native/'+script)
        for script in ('check_content.py','check_n05_tasks.py','check_music.py','check_save.py','check_sfx.py','check_n063_arm7_audio.py','check_player_panel.py'):
            run('native/tests/'+script)
        run('native/probe_n05_flow.py')
        for script in ('check_n06_video.py','check_n061_gpu.py','check_n05_flow.py'):
            run('native/tests/'+script)
        run('native/tests/check_n05_flow_matrix.py','--all')
        run('native/tests/check_n05_flow.py','--systems','pause','0')
        run('native/tests/check_n05_systems.py')
        run('native/prepare_lower_assets.py')
        run('native/tests/check_lower_ui.py')
        run('native/tests/n06_preview_png.py')
        run('native/build_n05_flow.py')
        run('native/package_n05_flow.py')
        say('\nSUCESSO DE COMPILACAO (EM TESTE): '+str(ROOT/'dist/native-n09/mmx4_native_n09.nds'))
        say('Use somente no seu ambiente local. O NDS contem recursos extraidos do seu dump.')

if __name__=='__main__':
    try:main()
    except Exception as error:
        print('\nFALHA:',error,file=sys.stderr)
        print('Log:',logpath,file=sys.stderr)
        sys.exit(1)
