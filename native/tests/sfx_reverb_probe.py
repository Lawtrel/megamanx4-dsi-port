"""Check the offline synthesizer with generated one-shot ADPCM."""

from array import array


def check_reverb(dsp):
    block = bytes((0, 0)) + bytes((0x77,)) * 14
    end = bytes((0, 1)) + bytes((0x77,)) * 14
    body = block * 31 + end
    frames = 65536

    def render(depth, voice, send):
        dsp.bake_reset(body, len(body), depth)
        dsp.bake_voice(voice, 0, 0x1000, 0x000F, 0, 0x3FFF, send)
        output = array("h", [0]) * (2 * frames)
        dsp.bake_pull(output, frames)
        return output

    dry = render(0, 0, 0)
    assert max(dry) > 1000 and not any(dry[2048 * 2:])
    assert dry == render(8, 0, 0) == render(0, 16, 1)
    wet = render(8, 0, 1)
    assert wet == render(8, 16, 1)
    assert max(abs(sample) for sample in wet[2048 * 2:]) > 4
    assert max(abs(sample) for sample in wet[-1024:]) <= 1
    assert dry == render(0, 0, 0)

    # Use the previous wet voice again with the room active, without a reset.
    render(8, 16, 1)
    dsp.bake_voice(16, 0, 0x1000, 0x000F, 0, 0x3FFF, 0)
    reused = array("h", [0]) * (2 * frames)
    dsp.bake_pull(reused, frames)
    assert max(abs(sample) for sample in reused[2048 * 2:]) <= 1

    dsp.bake_reset(bytes(16), 16)
    silence = array("h", [123]) * 882
    dsp.bake_pull(silence, 441)
    assert not any(silence)
