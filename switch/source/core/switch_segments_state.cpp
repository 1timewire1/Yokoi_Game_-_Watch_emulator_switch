#include "switch_segments_state.h"

#include "SM5XX/SM5XX.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"

void update_segments_from_cpu(SM5XX* cpu) {
    if (!cpu || !cpu->segments_state_are_update) {
        return;
    }

    // The blink protection below is for the Game & Watch CPUs, whose
    // segments flicker as they're multiplexed. The E0C6S46/S48 (Tamagotchi,
    // Digital Monster) latch a whole, steady LCD picture 32 times a second,
    // and there it only hid things: a dot had to be lit on two pictures in
    // a row to show, so a screen scrolling one dot per picture (a Digimon's
    // clock sliding in and out) lost its 1-dot strokes - digits fell apart
    // into dashes. The BrickEmuPy-ported cores (Pocket Pikachu, Tamagotchi
    // Mini, D-Tector, Chibi Pachi... and the VFD tabletops, already averaged
    // per frame) also hand over their whole display memory - same thing.
    // Settings > Screen > Anti-Flicker Filter: Auto (off for those, on for
    // the Game & Watch CPUs), On or Off for every game.
    // Auto's exceptions: BrickEmuPy-core games whose LCD is multiplexed
    // like a Game & Watch's, so they flicker without the filter.
    static const char* const kFilterOn[] = {"BIGCAT9IN1"};
    bool filter_on_auto = false;
    if (g_game)
        for (const char* r : kFilterOn) filter_on_auto = filter_on_auto || g_game->ref == r;
    const uint8_t type = cpu->get_cpu_type_id();
    const bool steady_lcd = g_flicker_filter == 2 ? true
                          : g_flicker_filter == 1 ? false
                          : (type == 5 || type == 6) && !filter_on_auto;  // CPU_TYPE_E0C6S46 / CPU_TYPE_BRICK (std/savestate.h)

    const size_t n = g_segments.size();
    if (g_seg_raw_prev.size() != n) g_seg_raw_prev.assign(n, 0);
    if (g_seg_on.size() != n) g_seg_on.assign(n, 0);

    for (size_t i = 0; i < n; i++) {
        const Segment& seg = g_segments[i];
        const bool new_state = cpu->get_segments_state(seg.id[0], seg.id[1], seg.id[2]);

        if (steady_lcd) {
            g_seg_on[i] = new_state ? 1 : 0;
            g_seg_raw_prev[i] = g_seg_on[i];
            continue;
        }

        bool s = g_seg_raw_prev[i] != 0;
        bool b = g_seg_on[i] != 0;

        // Same blink-protection behavior as 3DS/Android: a segment that just
        // turned off still shows for one more frame if it was on the frame
        // before, smoothing out single-frame CPU flicker.
        s = s && (new_state || b);
        s = s || (new_state && b);

        g_seg_on[i] = s ? 1 : 0;
        g_seg_raw_prev[i] = new_state ? 1 : 0;
    }

    cpu->segments_state_are_update = false;
}
