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
    parser.add_argument("--with-browser", action="store_true",
                        help="Stage the optional browser build for local testing")
    args = parser.parse_args()
    out = args.out
    vendor = PAGE / "node_modules/hash-wasm"
    files = {
        "index.html": PAGE / "index.html",
        "style.css": PAGE / "style.css",
        "verify.js": PAGE / "verify.js",
        "verify-worker.js": PAGE / "verify-worker.js",
        "sha256.js": vendor / "dist/sha256.umd.min.js",
        "hash-wasm-LICENSE.txt": vendor / "LICENSE",
    }
    generated = {"supported-disc.js", "browser-config.js"}
    if args.with_browser:
        from browser_kit import FILES, SOURCES
        files.update(FILES)
        generated.add("kit/sources.json")
    out.mkdir(parents=True, exist_ok=True)
    allowed = set(files) | generated
    directories = {str(parent) for name in allowed for parent in Path(name).parents if parent != Path(".")}
    unexpected = []
    for path in out.rglob("*"):
        name = path.relative_to(out).as_posix()
        if path.is_symlink() or (name not in directories if path.is_dir() else name not in allowed):
            unexpected.append(name)
    if unexpected:
        parser.error("arquivos inesperados na saída pública: " + ", ".join(sorted(unexpected)))
    for name, source in files.items():
        destination = out / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
    (out / "browser-config.js").write_text(
        "const browserBuildAvailable = " + json.dumps(args.with_browser) + ";\n",
        encoding="utf-8",
    )
    if args.with_browser:
        (out / "kit/sources.json").write_text(json.dumps(SOURCES) + "\n", encoding="utf-8")
    supported = {"bytes": EXPECTED_SIZE, "sha256": EXPECTED_SHA256}
    (out / "supported-disc.js").write_text(
        "const supportedDisc = " + json.dumps(supported) + ";\n", encoding="utf-8"
    )


if __name__ == "__main__":
    main()
