"""Package only the static page, its verifier and the verifier's dependency."""
import argparse
import json
from pathlib import Path
import shutil

from prepare_disc import EXPECTED_SHA256, EXPECTED_SIZE

ROOT = Path(__file__).resolve().parents[1]
PAGE = ROOT / "pages"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("out", type=Path)
    out = parser.parse_args().out
    vendor = PAGE / "node_modules/hash-wasm"
    files = {
        "index.html": PAGE / "index.html",
        "style.css": PAGE / "style.css",
        "verify.js": PAGE / "verify.js",
        "verify-worker.js": PAGE / "verify-worker.js",
        "sha256.js": vendor / "dist/sha256.umd.min.js",
        "hash-wasm-LICENSE.txt": vendor / "LICENSE",
    }
    out.mkdir(parents=True, exist_ok=True)
    allowed = set(files) | {"supported-disc.js"}
    unexpected = [path.name for path in out.iterdir()
                  if path.name not in allowed or path.is_symlink() or not path.is_file()]
    if unexpected:
        parser.error("arquivos inesperados na saída pública: " + ", ".join(sorted(unexpected)))
    for name, source in files.items():
        shutil.copyfile(source, out / name)
    supported = {"bytes": EXPECTED_SIZE, "sha256": EXPECTED_SHA256}
    (out / "supported-disc.js").write_text(
        "const supportedDisc = " + json.dumps(supported) + ";\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
