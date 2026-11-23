"""Paths for the optional browser build; no dump or generated game data is staged."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SDK = ROOT / "ambiente/sdk/calico"
BUILD = ROOT / "build/browser-tools/wasm"
PYODIDE = ROOT / "pages/node_modules/pyodide"
PSYZ = "referencias/mmx4-decomp/tools/psyz"
SOURCES = (
    "tools/prepare_disc.py",
    "native/prepare_full_content.py",
    "native/prepare_resource_index.py",
    "native/prepare_music.py",
    "native/prepare_sfx.py",
    "native/prepare_sequences.py",
    "native/prepare_lower_assets.py",
    "native/build_n05_flow.py",
    "native/sample_baker.py",
    "native/xa_to_pcm.c",
    "native/lower_icons_bake.c",
    "referencias/mmx4-decomp/include/main/xa_sequence_data.h",
    PSYZ + "/decomp/src/libsnd/vm_n2p.c",
)

FILES = {
    "build-worker.mjs": ROOT / "tools/web/build-worker.mjs",
    "browser-native.mjs": ROOT / "tools/web/browser-native.mjs",
    "nds-package.mjs": ROOT / "tools/web/nds-package.mjs",
    "kit/prepare.py": ROOT / "tools/web/browser-prepare.py",
    "kit/arm9.elf": ROOT / "build/native/engine/mmx4_ds_beta.elf",
    "kit/arm7.elf": SDK / "bin/ds7_maine.elf",
    "kit/icon.bmp": SDK / "share/nds-icon.bmp",
    "kit/ndstool-COPYING.txt": BUILD / "ndstool-COPYING.txt",
    **{f"kit/{tool}.{extension}": BUILD / f"{tool}.{extension}"
       for tool in ("ndstool", "xa", "samples", "atlas") for extension in ("mjs", "wasm")},
    **{f"kit/{name}": PYODIDE / name for name in (
        "pyodide.mjs", "pyodide.asm.js", "pyodide.asm.wasm",
        "python_stdlib.zip", "pyodide-lock.json",
    )},
    **{f"kit/{path}": ROOT / path for path in SOURCES},
}
