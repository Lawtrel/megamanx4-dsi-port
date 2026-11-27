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
        library = Path(kit) / ("sfx_bake.dll" if os.name == "nt" else "sfx_bake.so")
    else:
        library = output / "sfx_bake.so"
        subprocess.run([
            "gcc", "-O3", "-shared", "-fPIC", "-D__psyz=1", "-DNDEBUG=1",
            "-I" + str(PSYZ / "psyz/include"), str(ROOT / "native/sfx_bake.c"),
            str(PSYZ / "psyz/src/psyz/psyz_spu.c"),
            str(PSYZ / "decomp/src/libspu/s_rmp.c"),
            str(PSYZ / "decomp/src/libspu/s_sra.c"), "-o", str(library),
        ], check=True)
    dsp = ctypes.CDLL(str(library))
    version = getattr(dsp, "bake_api_version", None)
    if version is not None:
        version.argtypes = []
        version.restype = ctypes.c_uint
    if version is None or version() != 2:
        raise RuntimeError("Biblioteca de áudio incompatível. Reextraia o launcher completo.")

    reset = dsp.bake_reset
    reset.argtypes = [ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint]
    reset.restype = ctypes.c_int
    dsp.bake_voice.argtypes = [ctypes.c_uint] * 7
    dsp.bake_voice.restype = None
    dsp.bake_keyoff.argtypes = [ctypes.c_uint]
    dsp.bake_keyoff.restype = None
    dsp.bake_pull.argtypes = [ctypes.c_void_p, ctypes.c_uint]
    dsp.bake_pull.restype = None
    dsp.bake_active.argtypes = []
    dsp.bake_active.restype = ctypes.c_uint
    pull = dsp.bake_pull

    def reset_bank(body, size, reverb_depth=0):
        if len(body) != size or not 0 <= reverb_depth <= 127 or not reset(body, size, reverb_depth):
            raise ValueError("Invalid sample bank size or reverb depth")

    def pull_into_array(buffer, frames):
        samples = (ctypes.c_int16 * len(buffer)).from_buffer(buffer)
        pull(samples, frames)

    dsp.bake_reset = reset_bank
    dsp.bake_pull = pull_into_array
    return dsp
