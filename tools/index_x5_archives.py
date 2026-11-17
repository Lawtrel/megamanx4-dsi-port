"""Validate the X5 asset/overlay tables and write a local, non-extracting index."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]
PRIVATE = ROOT / "private" / "x5-disc"
SECTOR = 2048


def digest(data: memoryview) -> str:
    return hashlib.sha1(data).hexdigest()


def index(path: Path, count: int, table_name: str) -> dict:
    data = path.read_bytes()
    if len(data) < SECTOR or len(data) % SECTOR:
        raise ValueError(f"{path.name}: size is not a whole number of sectors")
    members = []
    previous_end = SECTOR
    view = memoryview(data)
    for i in range(count):
        start_sector, size = struct.unpack_from("<II", data, i * 8)
        start = start_sector * SECTOR
        end = start + size
        if not size or start < previous_end or end > len(data):
            raise ValueError(f"{path.name}: invalid {table_name} row {i}")
        if any(view[previous_end:start]):
            raise ValueError(f"{path.name}: non-zero padding before row {i}")
        members.append({"index": i, "offset": start, "size": size,
                        "sha1": digest(view[start:end])})
        previous_end = (end + SECTOR - 1) // SECTOR * SECTOR
    sentinel = struct.unpack_from("<II", data, count * 8)
    if sentinel != (0, 0) or any(view[(count + 1) * 8:SECTOR]):
        raise ValueError(f"{path.name}: invalid {table_name} terminator")
    if previous_end != len(data):
        raise ValueError(f"{path.name}: unexpected trailing data or padding")
    return {"archive": path.name, "size": len(data), "sha1": digest(view),
            "members": members}


def main() -> None:
    dat_path = PRIVATE / "ROCK_X5.DAT"
    overlay_path = PRIVATE / "ROCK_X5.BIN"
    if not dat_path.is_file() or not overlay_path.is_file():
        raise SystemExit("Run tools/prepare_x5_disc.py first.")
    result = {
        "data_archive": index(dat_path, 172, "resource"),
        "overlay_archive": index(overlay_path, 59, "overlay"),
        "source_disc_sha1": json.loads((PRIVATE / "manifest.json").read_text())["disc_sha1"],
        "payloads_extracted": False,
    }
    out = PRIVATE / "archive-index.json"
    out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"data_members": len(result["data_archive"]["members"]),
                      "overlay_archive_entries": len(result["overlay_archive"]["members"]),
                      "code_overlays": 55, "data_overlays": 4,
                      "index": str(out.relative_to(ROOT)),
                      "payloads_extracted": False}, indent=2))


if __name__ == "__main__":
    main()
