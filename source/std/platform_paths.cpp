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

namespace {
std::string g_config_subdir, g_log_subdir;
std::string sub_path(const std::string& sub, const char* relative) {
    if (sub.empty()) return storage_path(relative);
    const std::string dir = storage_path(sub.c_str());
    if (!relative || !*relative) return dir;
    return dir + "/" + relative;
}
}  // namespace

void set_config_subdir(const std::string& subdir) { g_config_subdir = subdir; }
void set_log_subdir(const std::string& subdir) { g_log_subdir = subdir; }
std::string config_path(const char* relative) { return sub_path(g_config_subdir, relative); }
std::string log_path(const char* relative) { return sub_path(g_log_subdir, relative); }

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
