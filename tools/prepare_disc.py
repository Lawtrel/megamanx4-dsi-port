"""Verify the user's original X4 USA dump and extract its boot executable locally."""
from __future__ import annotations

import argparse
import os
import hashlib
import json
from pathlib import Path
import re
import struct

EXPECTED_SHA256 = "3ceab06ac99add4035f912188bcbf5b16c02056f46a33d38ff0c8dbce6cb613b"
EXPECTED_SIZE = 592217136
ROOT = Path(__file__).resolve().parents[1]

def disc_path() -> Path:
    """Resolve the user's local dump without a machine-specific public path."""
    def resolve(value):
        if os.name!='nt' and re.match(r'^[A-Za-z]:[\\/]',value):
            import shutil
            import subprocess
            if not shutil.which('wslpath'):
                raise RuntimeError('Windows dump path on Linux: register a local path or set X4_DISC.')
            value=subprocess.check_output(['wslpath','-u',value],text=True).strip()
        return Path(value).expanduser().resolve()
    configured = os.environ.get('X4_DISC')
    if configured:
        return resolve(configured)
    manifest = ROOT / 'private/disc_manifest.json'
    if manifest.is_file():
        return resolve(json.loads(manifest.read_text())['original_bin'])
    local = ROOT / 'private/disc.bin'
    if local.is_file():
        return local
    raise RuntimeError('Register your dump with tools/prepare_disc.py or set X4_DISC.')


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


class RawDisc:
    def __init__(self, path: Path):
        self.stream = path.open("rb")

    def sector(self, lba: int) -> bytes:
        self.stream.seek(lba * 2352)
        raw = self.stream.read(2352)
        if len(raw) != 2352 or raw[:12] != b"\0" + b"\xff" * 10 + b"\0":
            raise ValueError(f"Invalid raw sector at LBA {lba}")
        if raw[15] != 2 or raw[18] & 0x20:
            raise ValueError(f"Expected Mode 2 Form 1 data at LBA {lba}")
        return raw[24:24 + 2048]

    def read(self, lba: int, length: int) -> bytes:
        return b"".join(self.sector(lba + i) for i in range((length + 2047) // 2048))[:length]

    def directory(self, lba: int, size: int) -> dict:
        data = self.read(lba, size)
        entries = {}
        offset = 0
        while offset < len(data):
            length = data[offset]
            if not length:
                offset = ((offset // 2048) + 1) * 2048
                continue
            record = data[offset:offset + length]
            if length < 34 or len(record) != length:
                raise ValueError("Invalid ISO9660 directory record")
            raw_name = record[33:33 + record[32]]
            if raw_name not in (b"\0", b"\1"):
                name = raw_name.decode("ascii").split(";")[0]
                entries[name] = {
                    "lba": struct.unpack_from("<I", record, 2)[0],
                    "size": struct.unpack_from("<I", record, 10)[0],
                    "directory": bool(record[25] & 2),
                }
            offset += length
        return entries


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("bin", type=Path)
    parser.add_argument("--cue", type=Path)
    args = parser.parse_args()
    path = args.bin.resolve()
    cue = (args.cue or path.with_suffix(".cue")).resolve()
    if path.stat().st_size != EXPECTED_SIZE or sha256(path) != EXPECTED_SHA256:
        raise SystemExit("Dump differs from the supported original USA SLUS-00561 disc; no output written.")
    cue_text = cue.read_text(encoding="utf-8-sig")
    files = re.findall(r'^\s*FILE\s+"([^"]+)"\s+BINARY\s*$', cue_text, re.M | re.I)
    if len(files) != 1 or (cue.parent / files[0]).resolve() != path:
        raise SystemExit("CUE must reference this BIN as its only binary file.")
    if not re.search(r"TRACK\s+01\s+MODE2/2352", cue_text, re.I):
        raise SystemExit("Expected original MODE2/2352 CUE track.")
    disc = RawDisc(path)
    try:
        pvd = disc.sector(16)
        if pvd[:7] != b"\x01CD001\x01":
            raise ValueError("Missing ISO9660 primary volume descriptor")
        root_record = pvd[156:190]
        entries = disc.directory(struct.unpack_from("<I", root_record, 2)[0],
                                 struct.unpack_from("<I", root_record, 10)[0])
        config = disc.read(entries["SYSTEM.CNF"]["lba"], entries["SYSTEM.CNF"]["size"])
        if b"SLUS_005.61" not in config:
            raise ValueError("Unexpected SYSTEM.CNF boot target")
        entry = entries["SLUS_005.61"]
        exe = disc.read(entry["lba"], entry["size"])
        if exe[:8] != b"PS-X EXE":
            raise ValueError("Boot file lacks a PS-X EXE header")
        pc, gp, load, size = struct.unpack_from("<IIII", exe, 0x10)
        stack = struct.unpack_from("<I", exe, 0x30)[0]
        if (pc, load, size) != (0x800DAE8C, 0x80010000, 0x11F800):
            raise ValueError("Boot executable layout differs from the supported release")
        out = ROOT / "private"
        out.mkdir(exist_ok=True)
        (out / "SLUS_005.61").write_bytes(exe)
        (out / "SYSTEM.CNF").write_bytes(config)
        report = {
            "title": "Mega Man X4", "serial": "SLUS-00561", "region": "USA",
            "original_bin": str(path), "original_cue": str(cue),
            "disc_size": path.stat().st_size, "disc_sha256": EXPECTED_SHA256,
            "exe_size": len(exe), "exe_sha256": hashlib.sha256(exe).hexdigest(),
            "entry_pc": hex(pc), "load_address": hex(load), "text_size": size,
            "initial_gp": hex(gp), "initial_stack": hex(stack),
            "root_directory": entries,
            "originals_modified": False,
        }
        (out / "disc_manifest.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({k: v for k, v in report.items() if k != "root_directory"}, indent=2))
    finally:
        disc.stream.close()


if __name__ == "__main__":
    main()
