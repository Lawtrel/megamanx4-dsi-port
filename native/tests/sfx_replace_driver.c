#include "x4_sfx.h"
#include <assert.h>
#include <stdio.h>

static unsigned playing;

static int play(unsigned voice, const int16_t* pcm, size_t samples, int loop,
                size_t loop_start, unsigned pan, unsigned gain) {
    assert(!loop && !loop_start);
    assert(samples == 16 && pcm[0] == 123 && pan == 64 && gain == 128);
    playing |= 1u << voice;
    return 0;
}

static void stop(unsigned voice) {
    playing &= ~(1u << voice);
}

static int active(unsigned voice) {
    return (playing >> voice) & 1u;
}

int main(void) {
    static const X4SfxBackend backend = {play, stop, active, NULL};
    for (unsigned hardware = 0; hardware < 2; ++hardware) {
        x4_sfx_set_backend(hardware ? &backend : NULL);
        x4_sfx_config(".");
        assert(x4_sfx_bank(0, 0) == 0);
        assert(x4_sfx_play(0, 0) == 0);
        assert(!x4_sfx_finished(0, 0));
        assert(x4_sfx_play(0, 2) == X4_SFX_INVALID_SEQUENCE);
        assert(x4_sfx_finished(0, 2));
        x4_sfx_stop(0, 2);
        assert(!x4_sfx_finished(0, 0));
        X4SfxStats stats = x4_sfx_stats();
        assert(stats.invalid_sequences == 1 && stats.invalid_resource == 0 && stats.invalid_bank == 0);
        assert(stats.invalid_index == 2 && stats.invalid_command == 0x02000140);
        assert(x4_sfx_play(0, 3) == X4_SFX_MISSING_PAYLOAD);
        assert(x4_sfx_finished(0, 3));
        x4_sfx_stop(0, 3);
        assert(!x4_sfx_finished(0, 0) && !x4_sfx_errors());
        assert(x4_sfx_stats().invalid_sequences == 1);
        assert(x4_sfx_play(0, 1) == -2);
        assert(!x4_sfx_finished(0, 0));
        assert(x4_sfx_errors() == 1);
        if (hardware) {
            assert(playing);
        } else {
            int16_t sample = 0;
            x4_sfx_mix(&sample, 1);
            assert(sample == 123);
        }
        x4_sfx_close();
    }
    puts("PASS missing and invalid sequence requests preserve active streamed and cached PCM");
}
