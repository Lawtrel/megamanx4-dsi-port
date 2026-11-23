"""Run the existing preparation pipeline in a browser-local filesystem."""
import os
from pathlib import Path
import runpy
import sys

from x4_browser_status import status

PROJECT = Path("/project")
DISC = PROJECT / "disc/original.bin"


def run(script, *args):
    sys.argv = [str(PROJECT / script), *map(str, args)]
    return runpy.run_path(sys.argv[0], run_name="__main__")


def main():
    os.chdir(PROJECT)
    os.environ["X4_DISC"] = str(DISC)
    sys.path[:0] = [str(PROJECT / "tools"), str(PROJECT / "native")]
    cue = PROJECT / "original.cue"
    cue.write_text('FILE "disc/original.bin" BINARY\n  TRACK 01 MODE2/2352\n    INDEX 01 00:00:00\n')
    stages = (
        ("Verificando a edição e o SHA-256…", "tools/prepare_disc.py", (DISC, "--cue", cue)),
        ("Preparando os arquivos de dados…", "native/prepare_full_content.py", ()),
        ("Montando o índice de recursos…", "native/prepare_resource_index.py", ()),
        ("Convertendo música e vozes…", "native/prepare_music.py", ()),
        ("Preparando efeitos sonoros…", "native/prepare_sfx.py", ()),
        ("Preparando sequências de áudio…", "native/prepare_sequences.py", ()),
        ("Preparando o painel da tela inferior…", "native/prepare_lower_assets.py", ()),
    )
    for message, script, args in stages:
        status(message)
        run(script, *args)
    from build_n05_flow import collect_assets
    return [(name, str(path)) for name, path, _ in collect_assets()]


asset_paths = main()
