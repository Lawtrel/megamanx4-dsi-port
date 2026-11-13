#!/usr/bin/env python3
"""Extract the SDK from one digest-pinned official devkitPro container layer.

No Docker daemon, root privileges, package installation, or system changes.
Run with Linux/WSL Python so Linux symlinks and executable permissions work.
"""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import urllib.request

ROOT = Path(__file__).resolve().parents[1] / 'ambiente'
PACKAGES = ROOT / "packages"
SDK = ROOT / "sdk"
IMAGE = "devkitpro/devkitarm"
MANIFEST = "sha256:15b79ce75822c289538d8153da5fa7aafe5e6adc32ad8a575a197beca0f0761b"
LAYER = "sha256:3c5026e520aeec9e208399f6d0a62098d5ee4c13d8cca97cdb9adbc6bb56a1c0"
LAYER_SIZE = 215133357


def main():
    ROOT.mkdir(parents=True, exist_ok=True)
    PACKAGES.mkdir(exist_ok=True)
    SDK.mkdir(exist_ok=True)
    archive = PACKAGES / (LAYER.split(":")[1] + ".tar.gz")
    if not archive.exists():
        auth_url = f"https://auth.docker.io/token?service=registry.docker.io&scope=repository:{IMAGE}:pull"
        with urllib.request.urlopen(auth_url) as response:
            token = json.load(response)["token"]
        request = urllib.request.Request(
            f"https://registry-1.docker.io/v2/{IMAGE}/blobs/{LAYER}",
            headers={"Authorization": "Bearer " + token},
        )
        partial = archive.with_suffix(".part")
        print("Downloading official SDK layer...", flush=True)
        with urllib.request.urlopen(request, timeout=60) as response, partial.open("wb") as output:
            while data := response.read(1024 * 1024):
                output.write(data)
        partial.replace(archive)
    digest = hashlib.file_digest(archive.open("rb"), "sha256").hexdigest()
    if digest != LAYER.split(":")[1] or archive.stat().st_size != LAYER_SIZE:
        raise SystemExit("SDK layer checksum/size mismatch; extraction refused")
    # The root is newly owned by this project. Only opt/devkitpro content is
    # extracted, with the two directory components removed.
    print("SHA256 verified; extracting SDK into project directory...", flush=True)
    subprocess.run([
        "tar", "-xzf", str(archive), "--strip-components=2", "-C", str(SDK),
        "--wildcards", "opt/devkitpro/*", "--no-same-owner",
    ], check=True)
    (ROOT / "sdk-source.json").write_text(json.dumps({
        "image": IMAGE,
        "platform": "linux/amd64",
        "manifest": MANIFEST,
        "layer": LAYER,
        "size": LAYER_SIZE,
        "source": "https://hub.docker.com/r/devkitpro/devkitarm",
        "extracted": "opt/devkitpro/*",
    }, indent=2) + "\n")
    print("SDK ready at " + str(SDK), flush=True)


if __name__ == "__main__":
    main()
