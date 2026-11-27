"""Smoke test the launcher archive without using any game data."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
from zipfile import ZipFile

from build_launcher import PREFIX
from nds_header import validate_native_package
from prepare_disc import EXPECTED_SIZE


def run(python, workspace, *command):
    return subprocess.run([python, "-X", "utf8", *command], cwd=workspace,
                          encoding="utf-8", capture_output=True, check=False)


def smoke(archive):
    with tempfile.TemporaryDirectory(prefix="mmx4-launcher-smoke-") as directory:
        root = Path(directory)
        with ZipFile(archive) as bundle:
            assert bundle.testzip() is None
            assert not any(Path(name).suffix.lower() in
                           {".bin", ".cue", ".iso", ".nds", ".arc", ".pcm"}
                           for name in bundle.namelist())
            bundle.extractall(root)
        windows = os.name == "nt"
        exe = ".exe" if windows else ""
        shared = ".dll" if windows else ".so"
        target = "windows-x64" if windows else "linux-wsl-x64"
        launcher = root / (PREFIX + target)
        python = str(launcher / "python/python.exe") if windows else sys.executable
        runtime = launcher / "runtime"
        assert json.loads((runtime / "platform.json").read_text()) == target
        sys.path.insert(0, str(launcher))
        try:
            from launcher import script_command
        finally:
            sys.path.pop(0)
        sibling = runtime / "tools/smoke_sibling.py"
        sibling.write_text("value = 42\n")
        probe = runtime / "tools/smoke_probe.py"
        probe.write_text("from smoke_sibling import value\nassert value == 42\n")
        subprocess.run(script_command(python, str(probe), ()), cwd=runtime, check=True)

        synthetic = root / "arquivo de teste.bin"
        synthetic.touch()
        command = ("launcher.py", "--dump", str(synthetic), "--output-dir", str(root))
        bad_size = run(python, launcher, *command)
        assert bad_size.returncode == 1 and "BIN incompatível" in bad_size.stderr, bad_size
        with synthetic.open("r+b") as stream:
            stream.truncate(EXPECTED_SIZE)
        bad_hash = run(python, launcher, *command)
        assert bad_hash.returncode == 1 and "SHA-256 não corresponde" in bad_hash.stderr, bad_hash
        assert not (root / "mmx4_ds_beta.nds").exists()

        # The packaging entry point receives the same manifest types as a real build.
        content = runtime / "build/native/full-content"
        audio = runtime / "build/native/full-audio"
        lower = runtime / "build/native/lower-assets"
        for folder in (content, audio / "music", lower):
            folder.mkdir(parents=True)
        fixture = b"synthetic NitroFS payload\n"
        (content / "fake.ARC").write_bytes(fixture)
        (content / "resources.idx").write_bytes(b"synthetic index")
        (lower / "lower-ui.dat").write_bytes(b"synthetic atlas")
        manifests = {
            content / "manifest.json": {"files": [
                {"name": "fake.ARC", "sha256": hashlib.sha256(fixture).hexdigest()}
            ]},
            audio / "music/manifest.json": {"tracks": []},
            audio / "sfx_manifest.json": {"banks": []},
            audio / "sequences_manifest.json": {"sequences": []},
        }
        for path, manifest in manifests.items():
            path.write_text(json.dumps(manifest))
        if not windows:
            for tool in ("ambiente/sdk/tools/bin/ndstool", "host/xa_to_pcm", "host/atlas",
                         "host/sfx_bake.so"):
                binary = runtime / tool
                binary.chmod(0o700)
                versions = subprocess.check_output(["readelf", "--version-info", str(binary)], text=True)
                glibc = [tuple(map(int, version.split(".")))
                         for version in re.findall(r"\bGLIBC_([0-9.]+)", versions)]
                assert glibc and max(glibc) <= (2, 35), (tool, glibc)
            dependencies = subprocess.check_output(["ldd", str(runtime / "host/atlas")], text=True)
            assert "not found" not in dependencies, dependencies
        sector = bytearray(2352)
        sector[16:20] = bytes((1, 0, 0x44, 4))
        for group in range(18):
            offset = 24 + group * 128
            sector[offset + 16:offset + 128] = bytes([0x21]) * 112
        xa = root / "fixture.xa"
        xa.write_bytes(sector)
        pcm = root / "fixture.pcm"
        subprocess.run([str(runtime / ("host/xa_to_pcm" + exe)), str(xa), "0", "0", "0", "1",
                        str(pcm)], check=True)
        samples = pcm.read_bytes()
        assert len(samples) == 8064 and any(samples)
        sample_probe = runtime / "native/smoke_samples.py"
        fixture_path = Path(__file__).resolve().parents[1] / "native/tests/sfx_reverb_probe.py"
        sample_probe.write_text(
            fixture_path.read_text(encoding="utf-8")
            + "\nfrom sample_baker import load_baker\ncheck_reverb(load_baker(None))\n",
            encoding="utf-8",
        )
        assert (runtime / ("host/sfx_bake" + shared)).is_file()
        subprocess.run(script_command(python, str(sample_probe), ()), cwd=runtime, check=True,
                       env=dict(os.environ, X4_HOST_TOOLS=str(runtime / "host")))

        package = run(python, runtime, "native/build_n05_flow.py", "--package-engine", "engine.elf")
        assert package.returncode == 0, package
        output = runtime / "mmx4_ds_beta.nds"
        nitro = runtime / "nitrofiles"
        validate_native_package(output, nitro)
        with output.open("r+b") as stream:
            stream.seek(0x15e)
            original = stream.read(1)
            stream.seek(0x15e)
            stream.write(bytes([original[0] ^ 1]))
        try:
            validate_native_package(output, nitro)
        except ValueError as error:
            assert "CRC" in str(error), error
        else:
            raise AssertionError("Corrupted header was accepted")
    print("PASS launcher archive, native tools, BIN rejection and synthetic DS/DSi packaging")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    smoke(parser.parse_args().archive)
