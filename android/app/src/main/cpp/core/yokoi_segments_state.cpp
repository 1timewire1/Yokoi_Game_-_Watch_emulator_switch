#include "yokoi_segments_state.h"

#include <memory>
#include <vector>

#include "SM5XX/SM5XX.h"
#include "yokoi_runtime_state.h"

void update_segments_from_cpu(SM5XX* cpu) {
    if (!cpu || !cpu->segments_state_are_update) {
        return;
    }

    std::shared_ptr<const std::vector<Segment>> meta;
    std::shared_ptr<std::vector<uint8_t>> back;
    static thread_local std::vector<uint8_t> state;
    static thread_local std::vector<uint8_t> buffer;
    static thread_local uint32_t last_gen = 0;

    const uint32_t gen = g_seg_generation.load();
    if (gen != last_gen) {
        state.clear();
        buffer.clear();
        last_gen = gen;
    }

    {
        std::lock_guard<std::mutex> snap_lock(g_segment_snapshot_mutex);
        meta = g_segments_meta;
        back = g_seg_on_back;
    }

    if (!meta || !back) {
        cpu->segments_state_are_update = false;
        return;
    }

    const size_t n = meta->size();
    if (state.size() != n) state.assign(n, 0);
    if (buffer.size() != n) buffer.assign(n, 0);
    if (back->size() != n) back->assign(n, 0);

    for (size_t i = 0; i < n; i++) {
        const Segment& seg = (*meta)[i];
        const bool new_state = cpu->get_segments_state(seg.id[0], seg.id[1], seg.id[2]);

        bool s = state[i] != 0;
        bool b = buffer[i] != 0;

        // Same blink-protection behavior as 3DS renderer.
        s = s && (new_state || b);
        s = s || (new_state && b);

        buffer[i] = s ? 1 : 0;
        state[i] = new_state ? 1 : 0;
        (*back)[i] = buffer[i];
    }

    // Publish the snapshot by swapping front/back pointers, but only if the generation
    // didn't change mid-update (prevents cross-game contamination).
    {
        std::lock_guard<std::mutex> snap_lock(g_segment_snapshot_mutex);
        if (g_seg_generation.load() == gen && g_seg_on_back == back) {
            std::swap(g_seg_on_front, g_seg_on_back);
        }
    }

    cpu->segments_state_are_update = false;
}
