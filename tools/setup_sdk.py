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
import time
import urllib.error
import urllib.parse
import urllib.request

ROOT = Path(__file__).resolve().parents[1] / 'ambiente'
PACKAGES = ROOT / "packages"
SDK = ROOT / "sdk"
IMAGE = "devkitpro/devkitarm"
MANIFEST = "sha256:15b79ce75822c289538d8153da5fa7aafe5e6adc32ad8a575a197beca0f0761b"
LAYER = "sha256:3c5026e520aeec9e208399f6d0a62098d5ee4c13d8cca97cdb9adbc6bb56a1c0"
LAYER_SIZE = 215133357


class RegistryRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, request, fp, code, message, headers, url):
        redirected = super().redirect_request(request, fp, code, message, headers, url)
        if redirected and urllib.parse.urlsplit(url).netloc != urllib.parse.urlsplit(request.full_url).netloc:
            redirected.remove_header("Authorization")
        return redirected


def download(archive):
    opener = urllib.request.build_opener(RegistryRedirect())
    auth_url = f"https://auth.docker.io/token?service=registry.docker.io&scope=repository:{IMAGE}:pull"
    url = f"https://registry-1.docker.io/v2/{IMAGE}/blobs/{LAYER}"
    partial = archive.with_suffix(".part")
    for attempt in range(4):
        try:
            with opener.open(auth_url, timeout=30) as response:
                token = json.load(response)["token"]
            request = urllib.request.Request(url, headers={"Authorization": "Bearer " + token})
            with opener.open(request, timeout=60) as response, partial.open("wb") as output:
                while data := response.read(1024 * 1024):
                    output.write(data)
            if partial.stat().st_size != LAYER_SIZE:
                raise OSError("Incomplete SDK download")
            with partial.open("rb") as stream:
                if hashlib.file_digest(stream, "sha256").hexdigest() != LAYER.split(":")[1]:
                    raise SystemExit("SDK layer checksum mismatch; download discarded")
            partial.replace(archive)
            return
        except urllib.error.HTTPError as error:
            # Signed blob URLs are refreshed on retry. Do not log their credentials.
            transient = error.code in (408, 429, 500, 502, 503, 504) or (
                error.code == 400 and urllib.parse.urlsplit(error.url).hostname != "registry-1.docker.io"
            )
            if not transient or attempt == 3:
                raise SystemExit(f"SDK download failed with HTTP {error.code}.") from None
        except (urllib.error.URLError, OSError) as error:
            if attempt == 3:
                raise SystemExit(f"SDK download failed: {type(error).__name__}.") from None
        finally:
            partial.unlink(missing_ok=True)
        print(f"Retrying SDK download ({attempt + 2}/4)...", flush=True)
        time.sleep(2 ** attempt)


def main():
    ROOT.mkdir(parents=True, exist_ok=True)
    PACKAGES.mkdir(exist_ok=True)
    SDK.mkdir(exist_ok=True)
    archive = PACKAGES / (LAYER.split(":")[1] + ".tar.gz")
    if not archive.exists():
        print("Downloading official SDK layer...", flush=True)
        download(archive)
    with archive.open("rb") as stream:
        digest = hashlib.file_digest(stream, "sha256").hexdigest()
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
