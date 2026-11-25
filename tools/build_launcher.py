"""Bundle the precompiled DS engine and native preparation tools for Linux/WSL."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import urllib.request
import zipfile

from browser_kit import ROOT, SOURCES

PREFIX = "megaman-x4-ds-launcher-"
DEFAULT_TARGET = "linux-wsl-x64"
TARGETS = (DEFAULT_TARGET, "windows-x64", "android-arm64")
NAME = PREFIX + DEFAULT_TARGET
PYTHON_VERSION = "3.13.12"
PYTHON_EMBED_SHA256 = "76f238f606250c87c6beac75dccd35ee99070a13490555936abb6cb64ecce3d0"
SDK = ROOT / "ambiente/sdk"
NATIVE = ROOT / "native"
UPSTREAM = ROOT / "referencias/mmx4-decomp"
PSYZ = UPSTREAM / "tools/psyz"
SHIM = NATIVE / "tests/thread_host_shim"
CORE = ROOT / "build/native/gamecore/include"
NDSTOOL = ROOT / "build/browser-tools/ndstool"
NDSTOOL_REVISION = "76e8b681bb225d945a48852821e03114e6c7ce1c"


def compile_tools(folder, target):
    windows = target == "windows-x64"
    android = target == "android-arm64"
    if windows:
        cc, cxx = "x86_64-w64-mingw32-gcc-posix", "x86_64-w64-mingw32-g++-posix"
    elif android:
        tools = Path(os.environ["ANDROID_NDK_HOME"]) / "toolchains/llvm/prebuilt/linux-x86_64/bin"
        cc, cxx = str(tools / "aarch64-linux-android26-clang"), str(tools / "aarch64-linux-android26-clang++")
    else:
        cc, cxx = "gcc", "g++"
    exe = ".exe" if windows else ""
    shared = ".dll" if windows else ".so"
    static = ["-static"] if windows else []
    subprocess.run([
        cc, "-std=gnu11", "-O3", *static, str(NATIVE / "xa_to_pcm.c"),
        "-o", str(folder / ("xa_to_pcm" + exe)),
    ], check=True)
    subprocess.run([
        cc, "-std=gnu11", "-O3", "-shared", "-fPIC", *static, "-D__psyz=1", "-DNDEBUG=1",
        "-I" + str(PSYZ / "psyz/include"), str(NATIVE / "sfx_bake.c"),
        str(PSYZ / "psyz/src/psyz/psyz_spu.c"), "-o", str(folder / ("sfx_bake" + shared)),
    ], check=True)
    manifest = ROOT / "build/native/n05-flow/host/preparer_sources.json"
    sources = [ROOT / path for path in json.loads(manifest.read_text())]
    sources.extend((NATIVE / "dsi/n05_tasks.c", SHIM / "shim.c", NATIVE / "lower_icons_bake.c"))
    headers = (CORE, UPSTREAM / "src/pc", PSYZ / "psyz/include", NATIVE / "src",
               NATIVE / "dsi", SHIM)
    subprocess.run([
        cc, "-std=gnu11", "-O2", "-pthread", *static, "-fno-strict-aliasing", "-fwrapv",
        "-ffunction-sections", "-fdata-sections", "-Wl,--gc-sections",
        "-Wno-incompatible-pointer-types", "-Wno-int-conversion", "-DMMX4_PC=1",
        "-D__psyz=1", "-DSKIP_MDEC=1", "-DVERSION_US=1", "-DX4_FULL_FLOW=1",
        "-DX4_N06_INDEXED=1", *("-I" + str(path) for path in headers),
        *map(str, sources), "-o", str(folder / ("atlas" + exe)),
    ], check=True)
    tools = {f"runtime/host/{name}": folder / name for name in
             ("xa_to_pcm" + exe, "sfx_bake" + shared, "atlas" + exe)}
    if not (windows or android):
        tools["runtime/ambiente/sdk/tools/bin/ndstool"] = SDK / "tools/bin/ndstool"
        return tools
    ndstool = folder / ("ndstool" + exe)
    source = NDSTOOL / "source"
    objects = []
    for unit in sorted(source.glob("*.c")):
        obj = folder / (unit.stem + ".o")
        subprocess.run([cc, "-O2", "-c", str(unit), "-o", str(obj)], check=True)
        objects.append(obj)
    subprocess.run([
        cxx, "-std=c++11", "-O2", '-DPACKAGE_VERSION="2.3.1"',
        *(["-D__WIN32__", "-static"] if windows else ["-static-libstdc++"]),
        *map(str, sorted(source.glob("*.cpp"))), *map(str, objects),
        "-o", str(ndstool),
    ], check=True)
    tools["runtime/ambiente/sdk/tools/bin/" + ndstool.name] = ndstool
    return tools


def add_windows_python(archive, base):
    name = f"python-{PYTHON_VERSION}-embed-amd64.zip"
    url = f"https://www.python.org/ftp/python/{PYTHON_VERSION}/{name}"
    with urllib.request.urlopen(url, timeout=60) as response:
        payload = response.read()
    if hashlib.sha256(payload).hexdigest() != PYTHON_EMBED_SHA256:
        raise RuntimeError("Windows Python package SHA-256 mismatch")
    from io import BytesIO
    with zipfile.ZipFile(BytesIO(payload)) as python:
        for file in python.namelist():
            archive.writestr(f"{base}/python/{file}", python.read(file))


def bundle(archive, target=DEFAULT_TARGET):
    runtime_files = {
        **{path: ROOT / path for path in SOURCES},
        "tools/nds_header.py": ROOT / "tools/nds_header.py",
        "engine.elf": ROOT / "build/native/engine/mmx4_ds_beta.elf",
        "ambiente/sdk/calico/bin/ds7_maine.elf": SDK / "calico/bin/ds7_maine.elf",
        "ambiente/sdk/calico/share/nds-icon.bmp": SDK / "calico/share/nds-icon.bmp",
    }
    bundle_files = {
        "launcher.py": ROOT / "tools/launcher/launcher.py",
        "LEIA-ME.md": ROOT / "tools/launcher/LEIA-ME.md",
        "LICENSE.txt": ROOT / "LICENSE",
        "LICENSE-upstream.txt": UPSTREAM / "LICENSE",
        "LICENSE-psyz.txt": PSYZ / "LICENSE",
        "LICENSE-ndstool.txt": NDSTOOL / "COPYING",
        **{f"runtime/{name}": path for name, path in runtime_files.items()},
    }
    launcher = ROOT / "tools/launcher"
    if target == "android-arm64":
        bundle_files["abrir-no-android.sh"] = launcher / "abrir-no-android.sh"
    else:
        for name in ("ABRIR_NO_WINDOWS.cmd", "Abrir-No-Windows.ps1"):
            bundle_files[name] = launcher / name
    for source in bundle_files.values():
        if not source.is_file():
            raise FileNotFoundError(source)
    archive.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="mmx4-kit-") as temporary:
        directory = Path(temporary)
        bundle_files.update(compile_tools(directory, target))
        source_archive = directory / "ndstool-source.tar.gz"
        subprocess.run(["git", "-C", str(NDSTOOL), "archive", "--format=tar.gz",
                        f"--output={source_archive}", NDSTOOL_REVISION], check=True)
        bundle_files[source_archive.name] = source_archive
        base = PREFIX + target
        with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as output:
            for name, source in sorted(bundle_files.items()):
                output.write(source, f"{base}/{name}")
            output.writestr(f"{base}/runtime/platform.json", json.dumps(target))
            if target == "windows-x64":
                add_windows_python(output, base)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--target", choices=TARGETS, default=DEFAULT_TARGET)
    args = parser.parse_args()
    bundle(args.output, args.target)
    print(args.output)
