"""Link the ARM executable independently of the disc, then optionally package NitroFS."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SDK = ROOT / "ambiente/sdk"
TARGET = "mmx4_ds_beta"
TITLE = "Megaman X4 Ds;Native ARM9 engine;Beta 9"
FLOW = ROOT / "build/native/n05-flow/flow.o"
HEADERS = ("x4_flow.h", "x4_archive.h", "x4_audio.h", "x4_save.h",
           "x4_n05_platform.h", "x4_n05_actors.h", "x4_video.h", "x4_sfx.h",
           "x4_arm7_audio.h", "x4_controls.h", "x4_player_panel.h", "x4_lower_ui.h")
FRONTEND = ("main.c", "x4_lower_ui.c", "n06_gpu.c", "n063_arm7_audio.c")


def sha(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def build_id(output):
    inputs = [output / "flow.o", output / "Makefile"]
    inputs.extend(output / "source" / name for name in sorted((*HEADERS, *FRONTEND)))
    return hashlib.sha256("".join(map(sha, inputs)).encode()).hexdigest()[:20]


def link_engine(output):
    source = output / "source"
    source.mkdir(parents=True, exist_ok=True)
    for name in (*HEADERS, "x4_lower_ui.c"):
        shutil.copyfile(ROOT / "native/src" / name, source / name)
    shutil.copyfile(ROOT / "native/dsi/n05_flow_main.c", source / "main.c")
    for name in ("n06_gpu.c", "n063_arm7_audio.c"):
        shutil.copyfile(ROOT / "native/dsi" / name, source / name)
    shutil.copyfile(FLOW, output / "flow.o")
    shutil.copyfile(ROOT / "native/Makefile.nds", output / "Makefile")
    identifier = build_id(output)
    (source / "build_id.h").write_text(f'#define X4_BUILD_ID "{identifier}"\n')
    (output / "build_id.txt").write_text(identifier + "\n")
    env = os.environ.copy()
    env.update(DEVKITPRO=str(SDK), DEVKITARM=str(SDK / "devkitARM"), LIBNDS=str(SDK / "libnds"))
    env["PATH"] = os.pathsep.join((str(SDK / "devkitARM/bin"), str(SDK / "tools/bin"), env["PATH"]))
    subprocess.run(["make", "-j4", "X4_ENGINE_ONLY=1"], cwd=output, env=env, check=True)
    return output / (TARGET + ".elf")


def collect_assets():
    content = ROOT / "build/native/full-content"
    audio = ROOT / "build/native/full-audio"
    manifests = [
        (content / "manifest.json", "files", content, "content"),
        (audio / "music/manifest.json", "tracks", audio / "music", "music"),
        (audio / "sfx_manifest.json", "banks", audio / "sfx", "sfx"),
        (audio / "sequences_manifest.json", "sequences", audio / "sfx", "sfx"),
    ]
    assets = [
        ("content/resources.idx", content / "resources.idx", None),
        ("ui/lower-ui.dat", ROOT / "build/native/lower-assets/lower-ui.dat", None),
    ]
    for manifest, key, folder, target_folder in manifests:
        for row in json.loads(manifest.read_text())[key]:
            name = f'{row["selection"]:02x}.pcm' if key == "tracks" else row.get("file", row.get("name"))
            assets.append((f"{target_folder}/{name}", folder / name, row["sha256"]))
    return assets


def validate_icon(icon):
    with icon.open("rb") as stream:
        header = stream.read(54)
    if len(header) != 54 or header[:2] != b"BM":
        raise ValueError("O ícone deve ser um BMP indexado de 32×32 e 4 bits.")
    offset = struct.unpack_from("<I", header, 10)[0]
    dib, width, height, planes, bits, compression = struct.unpack_from("<IiiHHI", header, 14)
    if (dib, width, abs(height), planes, bits, compression) != (40, 32, 32, 1, 4, 0):
        raise ValueError("O ícone deve ser um BMP indexado de 32×32 e 4 bits, sem compressão.")
    if offset < 118 or offset + 512 > icon.stat().st_size:
        raise ValueError("O BMP precisa conter a paleta de 16 cores e todos os pixels.")


def package(engine, icon=None):
    if icon:
        validate_icon(icon)
    output = engine.parent
    nitro = output / "nitrofiles"
    nitro.mkdir(exist_ok=True)
    assets = []
    for path, source, expected_hash in collect_assets():
        digest = sha(source)
        if expected_hash and digest != expected_hash:
            raise RuntimeError(f"Recurso com SHA-256 incorreto: {path}")
        dest = nitro / path
        dest.parent.mkdir(exist_ok=True)
        if not dest.is_file() or sha(dest) != digest:
            shutil.copyfile(source, dest)
        assets.append({"path": path, "sha256": digest, "bytes": source.stat().st_size})
    expected = {asset["path"] for asset in assets}
    actual = {path.relative_to(nitro).as_posix() for path in nitro.rglob("*") if path.is_file()}
    if actual != expected:
        raise RuntimeError(f"Arquivos inesperados no NitroFS: {sorted(actual - expected)}")

    binary = output / (TARGET + ".nds")
    pending = binary.with_suffix(".tmp")
    try:
        subprocess.run([
            str(SDK / "tools/bin/ndstool"), "-c", str(pending), "-9", str(engine),
            "-7", str(SDK / "calico/bin/ds7_maine.elf"),
            "-b", str(icon or SDK / "calico/share/nds-icon.bmp"), TITLE, "-d", str(nitro),
        ], check=True)
        pending.replace(binary)
    finally:
        pending.unlink(missing_ok=True)
    (output / "assets.json").write_text(json.dumps(assets, indent=2) + "\n")
    print(f"Built: {binary} ({binary.stat().st_size} bytes, SHA-256 {sha(binary)})")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine-only", action="store_true")
    parser.add_argument("--icon-bmp", type=Path, help="User-provided 32×32 BMP for the console banner")
    args = parser.parse_args()
    if args.icon_bmp and (args.engine_only or not args.icon_bmp.is_file()):
        parser.error("--icon-bmp requires an existing 32×32 BMP and a full build")
    output = ROOT / ("build/native/engine" if args.engine_only else "build/native/n09")
    engine = link_engine(output)
    if args.engine_only:
        print(f"ARM executable: {engine}")
    else:
        package(engine, args.icon_bmp)


if __name__ == "__main__":
    main()
