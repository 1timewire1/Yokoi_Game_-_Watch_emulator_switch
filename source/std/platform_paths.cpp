#include "platform_paths.h"

#include <string>

namespace {
std::string& storage_root_mut() {
#if defined(__3DS__)
    static std::string root = "sdmc:/3ds";
#else
    static std::string root = ".";
#endif
    return root;
}

static bool ends_with_slash(const std::string& s) {
    if (s.empty()) return false;
    char c = s.back();
    return c == '/' || c == '\\';
}
}

void set_storage_root(const std::string& root) {
    if (root.empty()) {
        return;
    }

    std::string normalized = root;
    while (ends_with_slash(normalized)) {
        normalized.pop_back();
    }

    storage_root_mut() = normalized;
}

const std::string& storage_root() {
    return storage_root_mut();
}

std::string storage_path(const char* relative) {
    if (!relative || !*relative) {
        return storage_root();
    }

    std::string base = storage_root();
    if (base.empty()) {
        return std::string(relative);
    }

    if (base.back() == '/') {
        return base + relative;
    }
    return base + "/" + relative;
}
