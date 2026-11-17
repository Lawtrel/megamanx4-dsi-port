"""Verify a local USA Mega Man X5 disc and extract its native inputs privately."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from prepare_disc import RawDisc  # noqa: E402

EXPECTED_DISC_SIZE = 582_954_960
EXPECTED_DISC_SHA1 = "10709231f857636b5ccd3cd9acebc91458dcb5fd"
EXPECTED_EXE_SHA1 = "476e55ff4ac698a1ee0654c0f08804a5837bd2f3"
EXPECTED_FILES = {"SLUS_013.34": 534_528, "ROCK_X5.BIN": 1_384_448}


def digest(path: Path, algorithm: str) -> str:
    h = hashlib.new(algorithm)
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bin", type=Path)
    parser.add_argument("--cue", type=Path)
    args = parser.parse_args()
    image = args.bin.resolve()
    cue = (args.cue or image.with_suffix(".cue")).resolve()
    if image.stat().st_size != EXPECTED_DISC_SIZE or image.stat().st_size % 2352:
        raise SystemExit("Unexpected X5 raw-track size; no output written.")
    if digest(image, "sha1") != EXPECTED_DISC_SHA1:
        raise SystemExit("Dump does not match the supported USA X5 data track; no output written.")
    cue_text = cue.read_text(encoding="utf-8-sig")
    files = re.findall(r'^\s*FILE\s+"([^"]+)"\s+BINARY\s*$', cue_text, re.M | re.I)
    if len(files) != 1 or (cue.parent / files[0]).resolve() != image:
        raise SystemExit("CUE must reference this BIN as its only file.")
    if not re.search(r"TRACK\s+01\s+MODE2/2352", cue_text, re.I):
        raise SystemExit("Expected MODE2/2352 data track.")

    disc = RawDisc(image)
    try:
        pvd = disc.sector(16)
        if pvd[:7] != b"\x01CD001\x01":
            raise ValueError("Missing ISO9660 primary volume descriptor")
        record = pvd[156:190]
        root = disc.directory(int.from_bytes(record[2:6], "little"),
                              int.from_bytes(record[10:14], "little"))
        config = disc.read(root["SYSTEM.CNF"]["lba"], root["SYSTEM.CNF"]["size"])
        if b"SLUS_013.34" not in config:
            raise ValueError("Unexpected X5 boot target")
        extracted: dict[str, bytes] = {"SYSTEM.CNF": config}
        for name, expected_size in EXPECTED_FILES.items():
            entry = root[name]
            if entry["size"] != expected_size or entry["directory"]:
                raise ValueError(f"Unexpected ISO9660 entry for {name}")
            extracted[name] = disc.read(entry["lba"], entry["size"])
        exe = extracted["SLUS_013.34"]
        if exe[:8] != b"PS-X EXE" or hashlib.sha1(exe).hexdigest() != EXPECTED_EXE_SHA1:
            raise ValueError("Boot executable does not match USA SLUS-01334")
        if "ROCK_X5.DAT" not in root or root["ROCK_X5.DAT"]["directory"]:
            raise ValueError("Missing X5 data archive")
        data_entry = root["ROCK_X5.DAT"]
        extracted["ROCK_X5.DAT"] = disc.read(data_entry["lba"], data_entry["size"])

        out = ROOT / "private" / "x5-disc"
        out.mkdir(parents=True, exist_ok=True)
        for name, data in extracted.items():
            (out / name).write_bytes(data)
        manifest = {
            "title": "Mega Man X5", "serial": "SLUS-01334", "region": "USA",
            "disc_sha1": EXPECTED_DISC_SHA1, "disc_size": image.stat().st_size,
            "cue_track": "MODE2/2352", "exe_sha1": EXPECTED_EXE_SHA1,
            "exe_size": len(exe), "boot_config": config.decode("ascii").strip(),
            "disc_files": {name: {"lba": item["lba"], "size": item["size"]}
                           for name, item in sorted(root.items())},
            "extracted_private": sorted(extracted),
            "original_disc_modified": False,
        }
        (out / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({k: v for k, v in manifest.items() if k != "disc_files"}, indent=2))
    finally:
        disc.stream.close()


if __name__ == "__main__":
    main()
