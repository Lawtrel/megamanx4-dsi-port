"""Compile the source-only preparation helpers to WebAssembly with Emscripten."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "build/browser-tools/wasm"
UPSTREAM = ROOT / "referencias/mmx4-decomp"
PSYZ = UPSTREAM / "tools/psyz"
NATIVE = ROOT / "native"
SOURCES = ROOT / "build/native/n05-flow/host/preparer_sources.json"
COMMON = (
    "-std=gnu11", "-O2", "-sMODULARIZE=1", "-sENVIRONMENT=web,worker,node",
    "-sINVOKE_RUN=0", "-sALLOW_MEMORY_GROWTH=1", "-sSTACK_SIZE=2097152",
    "-sSTACK_OVERFLOW_CHECK=2",
)


def compile_tool(name, sources, *options):
    subprocess.run([
        "emcc", *COMMON, *options, *map(str, sources), "-o", str(OUTPUT / f"{name}.mjs"),
    ], check=True, cwd=ROOT)


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    compile_tool("xa", [NATIVE / "xa_to_pcm.c"],
                 "-sEXPORTED_RUNTIME_METHODS=FS,callMain", "-lproxyfs.js")
    compile_tool("samples", [
        NATIVE / "sfx_bake.c", PSYZ / "psyz/src/psyz/psyz_spu.c",
        PSYZ / "decomp/src/libspu/s_rmp.c", PSYZ / "decomp/src/libspu/s_sra.c",
    ],
                 "-D__psyz=1", "-DNDEBUG=1", "-I" + str(PSYZ / "psyz/include"),
                 "-sEXPORTED_FUNCTIONS=_malloc,_free,_bake_api_version,_bake_reset,_bake_voice,"
                 "_bake_keyoff,_bake_active,_bake_pull",
                 "-sEXPORTED_RUNTIME_METHODS=HEAPU8,HEAP16")
    sources = [ROOT / path for path in json.loads(SOURCES.read_text())]
    sources.extend((NATIVE / "browser/tasks.c", NATIVE / "lower_icons_bake.c"))
    compile_tool("atlas", sources, "-sASYNCIFY", "-sASYNCIFY_STACK_SIZE=131072",
                 "-sEXPORTED_RUNTIME_METHODS=FS,callMain", "-lproxyfs.js",
                 "-fno-strict-aliasing", "-fwrapv", "-ffunction-sections", "-fdata-sections",
                 "-Wno-incompatible-pointer-types", "-Wno-int-conversion",
                 "-DMMX4_PC=1", "-D__psyz=1", "-DSKIP_MDEC=1", "-DVERSION_US=1",
                 "-DX4_FULL_FLOW=1", "-DX4_N06_INDEXED=1",
                 "-I" + str(ROOT / "build/native/gamecore/include"),
                 "-I" + str(UPSTREAM / "src/pc"), "-I" + str(PSYZ / "psyz/include"),
                 "-I" + str(NATIVE / "src"), "-I" + str(NATIVE / "dsi"))


if __name__ == "__main__":
    main()
