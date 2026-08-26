#include "debug_log.h"

#include <cstdio>
#include <cstring>
#include <ctime>

#include "platform_paths.h"

#if defined(__3DS__)
#include <3ds.h>
#endif

#if !ENABLE_LOGGING

namespace debug_log {

void init() {}
void set_enabled(bool /*enabled*/) {}
void vwrite(const char* /*fmt*/, va_list /*args*/) {}
void write(const char* /*fmt*/, ...) {}
void flush() {}
void close() {}

} // namespace debug_log

#else

namespace debug_log {
namespace {

static FILE* g_file = nullptr;
static bool g_enabled = true;
// storage_path() defaults to "sdmc:/3ds" on 3DS (identical to the old
// hardcoded path) and reflects whatever set_storage_root() was called with
// on other platforms (e.g. "sdmc:/switch/yokoi" on Switch).

static void write_banner() {
    if (!g_file) return;
#if defined(__3DS__)
    const u64 ms = osGetTime();
    std::fprintf(g_file, "\n===== Yokoi start (ms=%llu) =====\n", (unsigned long long)ms);
#else
    const std::time_t t = std::time(nullptr);
    std::fprintf(g_file, "\n===== Yokoi start (t=%lld) =====\n", (long long)t);
#endif
    std::fflush(g_file);
}

} // namespace

void set_enabled(bool enabled) {
    g_enabled = enabled;
    if (!g_enabled && g_file) {
        std::fflush(g_file);
        std::fclose(g_file);
        g_file = nullptr;
    }
}

void init() {
    if (!g_enabled) return;

#if defined(__3DS__)
    // Best-effort: ensure FS services are up before using sdmc:/ paths.
    // Do NOT call romfsInit() here; it's unrelated to SD logging and can be unsafe
    // to invoke early in startup.
    static bool fs_ok = false;
    if (!fs_ok) {
        Result rc = fsInit();
        if (R_SUCCEEDED(rc)) {
            fs_ok = true;
        }
    }
#endif

    if (g_file) return;
    g_file = std::fopen(storage_path("yokoi_log.txt").c_str(), "ab");
    if (!g_file) return;
    write_banner();
}

void vwrite(const char* fmt, va_list args) {
    if (!fmt || !g_enabled) return;
    if (!g_file) init();
    if (!g_file) return;

    std::vfprintf(g_file, fmt, args);
    std::fputc('\n', g_file);
    std::fflush(g_file);
}

void write(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vwrite(fmt, args);
    va_end(args);
}

void flush() {
    if (g_file) std::fflush(g_file);
}

void close() {
    if (!g_file) return;
    std::fflush(g_file);
    std::fclose(g_file);
    g_file = nullptr;
}

} // namespace debug_log

#endif
