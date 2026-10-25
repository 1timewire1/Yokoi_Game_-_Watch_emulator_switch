#include "E0C6S46.h"

#include <cstring>
#include <ctime>

// Save state layout: a small header, then (version 2+) the wall-clock time
// the state was saved at, then the whole State block as-is. The header
// carries the model and sizeof(State), so a state written by a build with a
// different State layout (or for the other model) is rejected rather than
// misread - bump kStateVersion whenever the layout changes. Version 1 (no
// timestamp) still loads, with saved_unix_time() = 0 (unknown).
namespace {
constexpr uint32_t kStateMagic = 0x36434530;  // "0EC6"
constexpr uint8_t kStateVersion = 2;

struct Header {
    uint32_t magic;
    uint8_t version;
    uint8_t model;
    uint16_t reserved;
    uint32_t state_size;
};
}  // namespace

bool E0C6S46::save_state(FILE* file) {
    if (!file) return false;
    Header h{kStateMagic, kStateVersion, (uint8_t)model_, 0, (uint32_t)sizeof(State)};
    if (fwrite(&h, sizeof(h), 1, file) != 1) return false;
    const int64_t now = (int64_t)time(nullptr);
    if (fwrite(&now, sizeof(now), 1, file) != 1) return false;
    return fwrite(&s_, sizeof(State), 1, file) == 1;
}

bool E0C6S46::load_state(FILE* file) {
    if (!file) return false;
    Header h{};
    if (fread(&h, sizeof(h), 1, file) != 1) return false;
    if (h.magic != kStateMagic || h.version < 1 || h.version > kStateVersion || h.model != (uint8_t)model_ ||
        h.state_size != sizeof(State)) {
        return false;
    }
    int64_t saved_at = 0;
    if (h.version >= 2 && fread(&saved_at, sizeof(saved_at), 1, file) != 1) return false;
    State loaded{};
    if (fread(&loaded, sizeof(State), 1, file) != 1) return false;
    s_ = loaded;
    saved_unix_time_ = saved_at;
    segments_state_are_update = true;
    return true;
}
