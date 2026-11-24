"""Use the same sample renderer through the native or browser ABI."""
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PSYZ = ROOT / "referencias/mmx4-decomp/tools/psyz"


def load_baker(output):
    if sys.platform == "emscripten":
        import x4_browser_native
        return x4_browser_native

    import ctypes

    kit = os.environ.get("X4_HOST_TOOLS")
    if kit:
        library = Path(kit) / "sfx_bake.so"
    else:
        library = output / "sfx_bake.so"
        subprocess.run([
            "gcc", "-O3", "-shared", "-fPIC", "-D__psyz=1", "-DNDEBUG=1",
            "-I" + str(PSYZ / "psyz/include"), str(ROOT / "native/sfx_bake.c"),
            str(PSYZ / "psyz/src/psyz/psyz_spu.c"), "-o", str(library),
        ], check=True)
    dsp = ctypes.CDLL(str(library))
    dsp.bake_reset.argtypes = [ctypes.c_void_p, ctypes.c_uint]
    dsp.bake_pull.argtypes = [ctypes.c_void_p, ctypes.c_uint]
    dsp.bake_active.restype = ctypes.c_uint
    pull = dsp.bake_pull

    def pull_into_array(buffer, frames):
        samples = (ctypes.c_int16 * len(buffer)).from_buffer(buffer)
        pull(samples, frames)

    dsp.bake_pull = pull_into_array
    return dsp
