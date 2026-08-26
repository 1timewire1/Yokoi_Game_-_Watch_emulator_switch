#include "yokoi_controller_state.h"

#include <atomic>

namespace {
std::atomic<uint32_t> g_controller_mask{0};
}

uint32_t yokoi_controller_get_mask() {
    return g_controller_mask.load();
}

void yokoi_controller_set_mask(uint32_t mask) {
    g_controller_mask.store(mask);
}
