#include "save_slots.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include <sys/stat.h>

#include "std/load_file.h"
#include "std/platform_paths.h"
#include "std/savestate.h"
#include "std/debug_log.h"

#include "render/switch_gl.h"
#include "render/switch_screen.h"
#include "switch_runtime_state.h"
#include "switch_settings.h"

namespace {

constexpr int kThumbW = 256, kThumbH = 144;
constexpr uint32_t kThumbMagic = 0x4D485459;  // "YTHM"
struct ThumbHeader {
    uint32_t magic;
    uint16_t w, h;
    int64_t saved_at;
};

// Same folder/naming as savestate.cpp's .sav files, .thm instead.
std::string thumb_path(uint16_t game_index, int slot) {
    std::string sav = get_save_path_slot(game_index, slot);
    if (sav.size() < 4) return std::string();
    return sav.substr(0, sav.size() - 4) + ".thm";
}

SaveSlotInfo g_info[kNumSaveSlots + 1];

void free_info() {
    for (auto& i : g_info) {
        if (i.tex) glDeleteTextures(1, &i.tex);
        i = SaveSlotInfo{};
    }
}

uint32_t upload(const std::vector<uint8_t>& rgba, int w, int h) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    return tex;
}

}  // namespace

bool save_slot_write_thumbnail(uint16_t game_index, int slot) {
    if (!g_cpu || !g_game) return false;
    const std::string path = thumb_path(game_index, slot);
    if (path.empty()) return false;

    // The game screen exactly as it renders, off-screen at the full 1280x720
    // (unrotated, so the card shows it the right way up), then box-shrunk.
    constexpr int kW = 1280, kH = 720;
    GLuint fbo = 0, rb = 0;
    glGenFramebuffers(1, &fbo);
    glGenRenderbuffers(1, &rb);
    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kW, kH);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    std::vector<uint8_t> full;
    if (ok) {
        const ScreenRotation rot = g_screen_rotation;
        g_screen_rotation = ScreenRotation::Deg0;
        switch_screen_load_textures();  // no-op unless this game's aren't loaded yet
        switch_screen_render();
        g_screen_rotation = rot;
        full.resize((size_t)kW * kH * 4);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, kW, kH, GL_RGBA, GL_UNSIGNED_BYTE, full.data());
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(1, &rb);
    glDeleteFramebuffers(1, &fbo);
    if (!ok) {
        YOKOI_LOG("save_slots: off-screen framebuffer incomplete, no thumbnail");
        return false;
    }

    // 5x5 box average; GL rows run bottom-up, the file is top-down.
    std::vector<uint8_t> small((size_t)kThumbW * kThumbH * 4);
    for (int y = 0; y < kThumbH; ++y) {
        for (int x = 0; x < kThumbW; ++x) {
            unsigned acc[3] = {0, 0, 0};
            for (int dy = 0; dy < 5; ++dy) {
                const int sy = kH - 1 - (y * 5 + dy);
                const uint8_t* p = &full[((size_t)sy * kW + (size_t)x * 5) * 4];
                for (int dx = 0; dx < 5; ++dx, p += 4) {
                    acc[0] += p[0]; acc[1] += p[1]; acc[2] += p[2];
                }
            }
            uint8_t* o = &small[((size_t)y * kThumbW + x) * 4];
            o[0] = (uint8_t)(acc[0] / 25); o[1] = (uint8_t)(acc[1] / 25); o[2] = (uint8_t)(acc[2] / 25); o[3] = 255;
        }
    }

    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    ThumbHeader hd{kThumbMagic, (uint16_t)kThumbW, (uint16_t)kThumbH, (int64_t)time(nullptr)};
    ok = fwrite(&hd, sizeof(hd), 1, f) == 1 && fwrite(small.data(), small.size(), 1, f) == 1;
    fclose(f);
    if (!ok) remove(path.c_str());
    return ok;
}

void save_slot_delete_thumbnail(uint16_t game_index, int slot) {
    const std::string path = thumb_path(game_index, slot);
    if (!path.empty()) remove(path.c_str());
}

void save_slots_refresh(uint16_t game_index) {
    free_info();
    for (int s = 0; s <= kNumSaveSlots; ++s) {
        SaveSlotInfo& info = g_info[s];
        info.exists = save_state_slot_exists(game_index, s);
        if (!info.exists) continue;
        struct stat st;
        if (stat(get_save_path_slot(game_index, s), &st) == 0) info.saved_at = (int64_t)st.st_mtime;

        const std::string path = thumb_path(game_index, s);
        FILE* f = fopen(path.c_str(), "rb");
        if (!f) continue;
        ThumbHeader hd{};
        if (fread(&hd, sizeof(hd), 1, f) == 1 && hd.magic == kThumbMagic && hd.w > 0 && hd.h > 0 && hd.w <= 1280 &&
            hd.h <= 720) {
            std::vector<uint8_t> px((size_t)hd.w * hd.h * 4);
            if (fread(px.data(), px.size(), 1, f) == 1) {
                info.tex = upload(px, hd.w, hd.h);
                if (hd.saved_at > 0) info.saved_at = hd.saved_at;
            }
        }
        fclose(f);
    }
}

const SaveSlotInfo& save_slot_info(int slot) {
    static const SaveSlotInfo kNone;
    if (slot < 0 || slot > kNumSaveSlots) return kNone;
    return g_info[slot];
}

void save_slots_free() { free_info(); }

const char* save_slot_time_text(int64_t t) {
    static char buf[32];
    buf[0] = '\0';
    if (t <= 0) return buf;
    const time_t tt = (time_t)t;
    const tm* lt = localtime(&tt);
    if (lt) strftime(buf, sizeof(buf), "%d/%m/%Y  %H:%M", lt);
    return buf;
}
