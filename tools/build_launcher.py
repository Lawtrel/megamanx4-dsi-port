"""Bundle the precompiled DS engine and native preparation tools for Linux/WSL."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile

from browser_kit import ROOT, SOURCES

NAME = "megaman-x4-ds-launcher-linux-wsl-x64"
SDK = ROOT / "ambiente/sdk"
NATIVE = ROOT / "native"
UPSTREAM = ROOT / "referencias/mmx4-decomp"
PSYZ = UPSTREAM / "tools/psyz"
SHIM = NATIVE / "tests/thread_host_shim"
CORE = ROOT / "build/native/gamecore/include"
NDSTOOL = ROOT / "build/browser-tools/ndstool"
NDSTOOL_REVISION = "76e8b681bb225d945a48852821e03114e6c7ce1c"


def compile_tools(folder):
    subprocess.run([
        "gcc", "-std=gnu11", "-O3", str(NATIVE / "xa_to_pcm.c"),
        "-o", str(folder / "xa_to_pcm"),
    ], check=True)
    subprocess.run([
        "gcc", "-O3", "-shared", "-fPIC", "-D__psyz=1", "-DNDEBUG=1",
        "-I" + str(PSYZ / "psyz/include"), str(NATIVE / "sfx_bake.c"),
        str(PSYZ / "psyz/src/psyz/psyz_spu.c"), "-o", str(folder / "sfx_bake.so"),
    ], check=True)
    manifest = ROOT / "build/native/n05-flow/host/preparer_sources.json"
    sources = [ROOT / path for path in json.loads(manifest.read_text())]
    sources.extend((NATIVE / "dsi/n05_tasks.c", SHIM / "shim.c", NATIVE / "lower_icons_bake.c"))
    headers = (CORE, UPSTREAM / "src/pc", PSYZ / "psyz/include", NATIVE / "src",
               NATIVE / "dsi", SHIM)
    subprocess.run([
        "gcc", "-std=gnu11", "-O2", "-pthread", "-fno-strict-aliasing", "-fwrapv",
        "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",
        "-Wno-incompatible-pointer-types", "-Wno-int-conversion", "-DMMX4_PC=1",
        "-D__psyz=1", "-DSKIP_MDEC=1", "-DVERSION_US=1", "-DX4_FULL_FLOW=1",
        "-DX4_N06_INDEXED=1", *("-I" + str(path) for path in headers),
        *map(str, sources), "-o", str(folder / "atlas"),
    ], check=True)


def bundle(archive):
    runtime_files = {
        **{path: ROOT / path for path in SOURCES},
        "tools/nds_header.py": ROOT / "tools/nds_header.py",
        "engine.elf": ROOT / "build/native/engine/mmx4_ds_beta.elf",
        "ambiente/sdk/calico/bin/ds7_maine.elf": SDK / "calico/bin/ds7_maine.elf",
        "ambiente/sdk/calico/share/nds-icon.bmp": SDK / "calico/share/nds-icon.bmp",
        "ambiente/sdk/tools/bin/ndstool": SDK / "tools/bin/ndstool",
    }
    bundle_files = {
        "ABRIR_NO_WINDOWS.cmd": ROOT / "tools/launcher/ABRIR_NO_WINDOWS.cmd",
        "Abrir-No-Windows.ps1": ROOT / "tools/launcher/Abrir-No-Windows.ps1",
        "launcher.py": ROOT / "tools/launcher/launcher.py",
        "LEIA-ME.md": ROOT / "tools/launcher/LEIA-ME.md",
        "LICENSE.txt": ROOT / "LICENSE",
        "LICENSE-upstream.txt": UPSTREAM / "LICENSE",
        "LICENSE-psyz.txt": PSYZ / "LICENSE",
        "LICENSE-ndstool.txt": NDSTOOL / "COPYING",
        **{f"runtime/{name}": path for name, path in runtime_files.items()},
    }
    for source in bundle_files.values():
        if not source.is_file():
            raise FileNotFoundError(source)
    archive.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="mmx4-kit-") as temporary:
        directory = Path(temporary)
        compile_tools(directory)
        for name in ("xa_to_pcm", "sfx_bake.so", "atlas"):
            bundle_files[f"runtime/host/{name}"] = directory / name
        source_archive = directory / "ndstool-source.tar.gz"
        subprocess.run(["git", "-C", str(NDSTOOL), "archive", "--format=tar.gz",
                        f"--output={source_archive}", NDSTOOL_REVISION], check=True)
        bundle_files[source_archive.name] = source_archive
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as output:
            for name, source in sorted(bundle_files.items()):
                output.write(source, f"{NAME}/{name}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    bundle(args.output)
    print(args.output)
