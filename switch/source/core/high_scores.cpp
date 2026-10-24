#include "high_scores.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

#include "std/debug_log.h"
#include "std/platform_paths.h"
#include "switch_settings.h"

namespace {

std::map<std::string, uint32_t> g_scores;  // "REF.mode" -> best
bool g_loaded = false;
bool g_dirty = false;

std::string key_of(const std::string& ref, const char* mode) {
    return ref + "." + (mode ? mode : "");
}

void load_if_needed() {
    if (g_loaded) return;
    g_loaded = true;
    const std::string path = storage_path("yokoi_highscores.txt");
    FILE* file = fopen(path.c_str(), "r");
    if (!file) return;
    char line[160];
    while (fgets(line, sizeof(line), file)) {
        char* eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        const unsigned long v = strtoul(eq + 1, nullptr, 10);
        if (line[0] != '\0') g_scores[line] = (uint32_t)v;
    }
    fclose(file);
    YOKOI_LOG("high_scores: loaded %u record(s) from '%s'", (unsigned)g_scores.size(), path.c_str());
}

}  // namespace

uint32_t high_score_get(const std::string& ref, const char* mode) {
    load_if_needed();
    auto it = g_scores.find(key_of(ref, mode));
    return it == g_scores.end() ? 0 : it->second;
}

bool high_score_note(const std::string& ref, const char* mode, uint32_t score) {
    load_if_needed();
    if (score == 0) return false;
    uint32_t& best = g_scores[key_of(ref, mode)];
    if (score <= best) return false;
    best = score;
    g_dirty = true;
    return true;
}

void high_score_flush() {
    if (!g_dirty || g_debug_mode) return;
    const std::string path = storage_path("yokoi_highscores.txt");
    FILE* file = fopen(path.c_str(), "w");
    if (!file) {
        YOKOI_LOG("high_scores: couldn't write '%s'", path.c_str());
        return;
    }
    for (const auto& kv : g_scores) {
        if (kv.second > 0) fprintf(file, "%s=%u\n", kv.first.c_str(), (unsigned)kv.second);
    }
    fclose(file);
    g_dirty = false;
}
