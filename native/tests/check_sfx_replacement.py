"""Verify that a missing sample or invalid sequence cannot interrupt an active effect."""
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

with tempfile.TemporaryDirectory(prefix="x4-sfx-test-") as directory:
    folder = Path(directory)
    bank = b"X4SFX01\0" + struct.pack("<II", 4, 18900)
    bank += struct.pack("<IIII", 1, 0, 0, 0)
    bank += struct.pack("<IIII", 0x01000040, 0, 0, 0)
    bank += struct.pack("<IIII", 0x02000140, 0, 0, 0)
    bank += struct.pack("<IIII", 0x00000240, 0, 0, 0)
    (folder / "00_0.sfx").write_bytes(bank)
    (folder / "seq_0_0_0.pcm").write_bytes(struct.pack("<16h", *([123] * 16)))
    driver = folder / "test"
    subprocess.run([
        "gcc", "-std=c11", "-I" + str(ROOT / "src"), str(ROOT / "src/x4_sfx.c"),
        str(ROOT / "tests/sfx_replace_driver.c"), "-o", str(driver),
    ], check=True)
    subprocess.run([str(driver)], cwd=folder, check=True)
