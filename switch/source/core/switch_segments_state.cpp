#include "switch_segments_state.h"

#include "SM5XX/SM5XX.h"
#include "switch_runtime_state.h"

void update_segments_from_cpu(SM5XX* cpu) {
    if (!cpu || !cpu->segments_state_are_update) {
        return;
    }

    const size_t n = g_segments.size();
    if (g_seg_raw_prev.size() != n) g_seg_raw_prev.assign(n, 0);
    if (g_seg_on.size() != n) g_seg_on.assign(n, 0);

    for (size_t i = 0; i < n; i++) {
        const Segment& seg = g_segments[i];
        const bool new_state = cpu->get_segments_state(seg.id[0], seg.id[1], seg.id[2]);

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
