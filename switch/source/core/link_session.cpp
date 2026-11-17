#include "link_session.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>

#include <switch.h>

#include "BrickCores/BrickCore.h"
#include "E0C6200/E0C6S46.h"
#include "SM5XX/SM5XX.h"
#include "std/debug_log.h"
#include "std/load_file.h"
#include "std/savestate.h"
#include "virtual_i_o/rzone_io.h"
#include "virtual_i_o/virtual_input.h"

#include "audio/switch_sound.h"
#include "input/brick_clock_setup.h"
#include "input/e0c6200_clock_setup.h"
#include "input/brick_input_map.h"
#include "input/control_remap.h"
#include "input/motion_steps.h"
#include "audio/step_sound.h"
#include "input/switch_input.h"
#include "input/time_control.h"
#include "switch_menu.h"
#include "game_controls.h"
#include "render/switch_i18n.h"
#include "render/switch_screen.h"
#include "render/switch_ui.h"
#include "pet_time.h"
#include "switch_game_loader.h"
#include "switch_runtime_state.h"
#include "switch_segments_state.h"
#include "switch_settings.h"

namespace {

// Everything core/ keeps for "the loaded game" - see link_session.h.
struct GameCtx {
    std::unique_ptr<SM5XX> cpu;
    std::unique_ptr<RZoneIO> rzone_io;
    const GW_rom* game = nullptr;
    std::unique_ptr<Virtual_Input> input;
    uint16_t game_index = 0;
    std::vector<Segment> segments;
    std::vector<uint8_t> seg_on, seg_raw_prev;
    uint16_t segment_info[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    bool double_in_one_screen = false;
    uint8_t nb_screen = 1;
    bool split_two_screens_to_panels = false;
    float canvas_w = 1.0f, canvas_h = 1.0f;
    float screen_off_x[3] = {0, 0, 0}, screen_off_y[3] = {0, 0, 0};
    float top_canvas_w = 1.0f, top_canvas_h = 1.0f, bottom_canvas_w = 0.0f, bottom_canvas_h = 0.0f;
    float combined_canvas_w = 1.0f, combined_canvas_h = 1.0f;
    float top_off_x = 0.0f, bottom_off_x = 0.0f, bottom_off_y = 0.0f;
    uint32_t rate_accu = 0;
    int time_set_grace_counter = 0, hour_reassert_grace_frames = 0;
    uint32_t texture_generation = 0;
    SwitchScreenGameState screen;
};

// Swaps the globals with `o`: afterwards the globals hold what `o` held and
// `o` holds what the globals held.
void swap_with_globals(GameCtx& o) {
    std::swap(g_cpu, o.cpu);
    std::swap(g_rzone_io, o.rzone_io);
    std::swap(g_game, o.game);
    std::swap(g_input, o.input);
    std::swap(g_game_index, o.game_index);
    std::swap(g_segments, o.segments);
    std::swap(g_seg_on, o.seg_on);
    std::swap(g_seg_raw_prev, o.seg_raw_prev);
    for (int i = 0; i < 10; i++) std::swap(g_segment_info[i], o.segment_info[i]);
    std::swap(g_double_in_one_screen, o.double_in_one_screen);
    std::swap(g_nb_screen, o.nb_screen);
    std::swap(g_split_two_screens_to_panels, o.split_two_screens_to_panels);
    std::swap(g_canvas_w, o.canvas_w);
    std::swap(g_canvas_h, o.canvas_h);
    for (int i = 0; i < 3; i++) {
        std::swap(g_screen_off_x[i], o.screen_off_x[i]);
        std::swap(g_screen_off_y[i], o.screen_off_y[i]);
    }
    std::swap(g_top_canvas_w, o.top_canvas_w);
    std::swap(g_top_canvas_h, o.top_canvas_h);
    std::swap(g_bottom_canvas_w, o.bottom_canvas_w);
    std::swap(g_bottom_canvas_h, o.bottom_canvas_h);
    std::swap(g_combined_canvas_w, o.combined_canvas_w);
    std::swap(g_combined_canvas_h, o.combined_canvas_h);
    std::swap(g_top_off_x, o.top_off_x);
    std::swap(g_bottom_off_x, o.bottom_off_x);
    std::swap(g_bottom_off_y, o.bottom_off_y);
    std::swap(g_rate_accu, o.rate_accu);
    std::swap(g_time_set_grace_counter, o.time_set_grace_counter);
    std::swap(g_hour_reassert_grace_frames, o.hour_reassert_grace_frames);
    std::swap(g_texture_generation, o.texture_generation);
    switch_screen_swap_game_state(o.screen);
}

// Which device each player is - the Digital Monster's wire link joins three
// kinds of them.
enum class Dev : uint8_t { None, Digimon, DTector, DPower, TamaV3, TamaGo, TamaFriends };

Dev dev_of(const std::string& ref) {
    if (ref == "DIGIMON" || ref == "DIGIMONV2" || ref == "DIGIMONV3" || ref == "DIGIMONV4" || ref == "DIGIMONES" ||
        ref == "DIGIMONJA") {
        return Dev::Digimon;
    }
    if (ref == "DTECTORDIGIVICEUSV2") return Dev::DTector;
    if (ref == "DPOWERDIGIVICEEU101") return Dev::DPower;
    if (ref == "TAMAGOTCHIV3US" || ref == "TAMAGOTCHIV3EU") return Dev::TamaV3;
    if (ref == "TMGTAMAGO") return Dev::TamaGo;
    if (ref.compare(0, 10, "TMGFRIENDS") == 0) return Dev::TamaFriends;
    return Dev::None;
}

// The wire link's pin on the GPL191X ones, found in their ROMs' link code
// (the same send routine on both: ~60 ms low, then 16 bits): port CD bit 2
// on the D-Tector, CD bit 0 on the D-Power. The Digital Monster's is P20
// (E0C6S46::link_drive()).
void wire_config(SM5XX* cpu, Dev d) {
    if (!cpu || cpu->get_cpu_type_id() != CPU_TYPE_BRICK) return;
    BrickCore* b = static_cast<BrickCore*>(cpu);
    if (d == Dev::DTector) b->wire_link_config("CD", 0x04);
    else if (d == Dev::DPower) b->wire_link_config("CD", 0x01);
}
int wire_drive(SM5XX* cpu) {
    if (!cpu) return 1;
    if (cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) return static_cast<E0C6S46*>(cpu)->link_drive();
    if (cpu->get_cpu_type_id() == CPU_TYPE_BRICK) return static_cast<BrickCore*>(cpu)->wire_link_drive();
    return 1;
}
// level -1: unconnected again (the E0C6S46's pin just goes back high).
void wire_set(SM5XX* cpu, int level) {
    if (!cpu) return;
    if (cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) static_cast<E0C6S46*>(cpu)->set_link_level(level < 0 ? 1 : level);
    else if (cpu->get_cpu_type_id() == CPU_TYPE_BRICK) static_cast<BrickCore*>(cpu)->wire_link_set(level);
}

GameCtx g_other;               // the second game while linked (swapped in only briefly)
Dev g_dev[2] = {Dev::None, Dev::None};
bool g_fn_last[2][8] = {};     // each function's last press (Brick cores only take changes)
int g_motion_player = 0;       // whose step button the controller's shake presses
bool g_active = false;
bool g_loading = false;        // link_session_start() is using the loader
LinkKind g_kind = LinkKind::None;
bool g_same_game = false;      // the second one is a copy of the first: never saved
int g_sound_player = 0;        // whose sound plays: 0 player 1, 1 player 2
bool g_last_tx = false;        // for the "Transmitting" label
int g_tx_frames = 0;

bool snapshot(SM5XX* cpu, std::vector<uint8_t>& out) {
    char* buf = nullptr;
    size_t len = 0;
    FILE* f = open_memstream(&buf, &len);
    if (!f) return false;
    const bool ok = cpu->save_state(f);
    fclose(f);
    if (ok && buf && len) out.assign(buf, buf + len);
    free(buf);
    return ok && !out.empty();
}

bool restore(SM5XX* cpu, std::vector<uint8_t>& in) {
    FILE* f = fmemopen(in.data(), in.size(), "rb");
    if (!f) return false;
    const bool ok = cpu->load_state(f);
    fclose(f);
    return ok;
}

// Each device's functions: its buttons first, then its special ones. `slot`
// picks the player's default button (kSlotKeys); `button` is the .brick
// button (BrickCore::set_button()), or for the Digital Monster (E0C6S46)
// "K2" / "K1" / "K0" (K line, active low) and "RES" (the reset pin).
struct Fn {
    const char* name;  // control row label (switch_tr())
    int slot;
    const char* button;
    bool special;
};
constexpr Fn kDigimonFns[] = {
    {"A - Select", 0, "K2", false}, {"B - Decide / Confirm", 1, "K1", false}, {"C - Cancel", 2, "K0", false},
    {"Reset Button", 5, "RES", true},
};
constexpr Fn kDTectorFns[] = {
    {"Button A", 0, "btnA", false},     {"Button B", 1, "btnB", false}, {"Button C", 2, "btnC", false},
    {"Sensor", 3, "btnSensor", true},   {"Step", 4, "btnStep", true},   {"Wave", 5, "btnWave", true},
};
constexpr Fn kDPowerFns[] = {
    {"Left", 0, "btnLeft", false},  {"Down", 1, "btnDown", false},  {"Right", 2, "btnRight", false},
    {"Up", 3, "btnUp", false},      {"Step", 4, "btnStep", true},   {"Card", 5, "btnCard", true},
    {"Reset Button", 6, "btnReset", true},
};
constexpr Fn kTamaFns[] = {
    {"Left Button", 0, "btnLeft", false}, {"Middle Button", 1, "btnCenter", false}, {"Right Button", 2, "btnRight", false},
};
// Each player's default buttons by slot: player 1 the left Joy-Con, player 2
// the right one (ZL / ZR stay free for the pause menu).
constexpr int kSlotKeys[2][7] = {{RB_Left, RB_Down, RB_Right, RB_Up, RB_L, RB_Minus, RB_L3},
                                 {RB_Y, RB_X, RB_A, RB_B, RB_R, RB_Plus, RB_R3}};

void fns_of(Dev d, const Fn*& fns, int& n) {
    switch (d) {
        case Dev::Digimon: fns = kDigimonFns; n = (int)(sizeof(kDigimonFns) / sizeof(Fn)); break;
        case Dev::DTector: fns = kDTectorFns; n = (int)(sizeof(kDTectorFns) / sizeof(Fn)); break;
        case Dev::DPower: fns = kDPowerFns; n = (int)(sizeof(kDPowerFns) / sizeof(Fn)); break;
        case Dev::TamaV3:
        case Dev::TamaGo:
        case Dev::TamaFriends: fns = kTamaFns; n = (int)(sizeof(kTamaFns) / sizeof(Fn)); break;
        default: fns = nullptr; n = 0; break;
    }
}
bool has_step(Dev d) { return d == Dev::DTector || d == Dev::DPower; }

// Presses / releases one function on its device.
void press_fn(SM5XX* cpu, Dev dev, const Fn& f, bool pressed, bool& last) {
    if (!cpu) return;
    const std::string b = f.button;
    if (cpu->get_cpu_type_id() == CPU_TYPE_E0C6S46) {
        E0C6S46* e = static_cast<E0C6S46*>(cpu);
        if (b == "RES") e->set_reset_line(pressed);
        else if (b.size() == 2 && b[0] == 'K') e->set_k_line(b[1] - '0', !pressed);  // active low
    } else if (cpu->get_cpu_type_id() == CPU_TYPE_BRICK) {
        BrickCore* c = static_cast<BrickCore*>(cpu);
        if (pressed == last) return;
        if (pressed && b == "btnStep") {
            // One press = one step on the D-Tector, whose pedometer counts
            // every 3rd pulse (as in input/brick_input_map.cpp), with the
            // device's step sound.
            for (int i = c->step_pulses(); i > 1; i--) {
                c->set_button(b, true);
                c->set_button(b, false);
            }
            step_sound_play(dev == Dev::DTector ? StepSound::DTector : StepSound::DPower);
        }
        c->set_button(b, pressed);
    }
    last = pressed;
}

}  // namespace

LinkKind link_kind_of(const std::string& ref) {
    switch (dev_of(ref)) {
        case Dev::Digimon:
        case Dev::DTector:
        case Dev::DPower: return LinkKind::Digimon;  // all on the two-prong wire
        case Dev::TamaV3: return LinkKind::TamaV3;
        case Dev::TamaGo: return LinkKind::TamaGo;
        case Dev::TamaFriends: return LinkKind::TamaFriends;
        default: return LinkKind::None;
    }
}

std::vector<LinkCandidate> link_candidates() {
    std::vector<LinkCandidate> out;
    if (!g_game) return out;
    const LinkKind kind = link_kind_of(g_game->ref);
    if (kind == LinkKind::None) return out;
    out.push_back({g_game_index, g_game->name, true, true});
    const size_t n = get_nb_name();
    for (size_t i = 0; i < n; i++) {
        if (i == g_game_index) continue;
        const std::string ref = get_ref((uint16_t)i);
        if (link_kind_of(ref) != kind) continue;
        out.push_back({(uint16_t)i, get_name((uint16_t)i), false, save_state_exists((uint16_t)i)});
    }
    return out;
}

bool link_session_active() { return g_active; }

bool link_session_start(const LinkCandidate& c) {
    if (!g_game || !g_cpu || g_active) return false;
    const LinkKind kind = link_kind_of(g_game->ref);
    if (kind == LinkKind::None) return false;

    std::vector<uint8_t> copy;
    if (c.same_game && !snapshot(g_cpu.get(), copy)) return false;
    const std::string first_ref = g_game->ref;

    YOKOI_LOG("link_session: connecting '%s' with game %u%s", first_ref.c_str(), (unsigned)c.index,
              c.same_game ? " (copy)" : "");
    g_other = GameCtx();
    swap_with_globals(g_other);  // globals: empty, g_other: the first game
    g_loading = true;
    switch_load_game_by_index_and_init(c.index);
    g_loading = false;
    bool ok = g_cpu && g_game && link_kind_of(g_game->ref) == kind;
    const Dev second_dev = ok ? dev_of(g_game->ref) : Dev::None;
    if (ok) {
        if (c.same_game) {
            ok = restore(g_cpu.get(), copy);
        } else if (c.has_save) {
            load_game_state(g_cpu.get(), c.index);  // a failed load just leaves it fresh
        }
        if (ok) {
            g_cpu->segments_state_are_update = true;
            update_segments_from_cpu(g_cpu.get());
        }
    }
    swap_with_globals(g_other);  // globals: the first game again, g_other: the second

    // The loader re-armed these for the second game; they belong to the first.
    e0c6200_clock_setup_begin(first_ref, false);
    brick_clock_setup_begin(false);
    pet_time_on_game_loaded();
    switch_sound_reconfigure(g_cpu.get());

    if (!ok) {
        YOKOI_LOG("link_session: second game failed to load");
        switch_screen_free_game_state(g_other.screen);
        g_other = GameCtx();
        return false;
    }
    g_active = true;
    g_kind = kind;
    g_dev[0] = dev_of(first_ref);
    g_dev[1] = second_dev;
    for (auto& p : g_fn_last)
        for (bool& l : p) l = false;
    if (!has_step(g_dev[g_motion_player]) && has_step(g_dev[1 - g_motion_player])) g_motion_player = 1 - g_motion_player;
    if (kind == LinkKind::Digimon) {
        wire_config(g_cpu.get(), g_dev[0]);
        wire_config(g_other.cpu.get(), g_dev[1]);
        wire_set(g_cpu.get(), 1);  // the line idles high
        wire_set(g_other.cpu.get(), 1);
    }
    if (kind == LinkKind::TamaGo || kind == LinkKind::TamaFriends) {
        // The core joins them itself (Tamago::link_pair), both run by the first.
        BrickCore* a = static_cast<BrickCore*>(g_cpu.get());
        if (!a->link_pair(static_cast<BrickCore*>(g_other.cpu.get()))) YOKOI_LOG("link_session: tamago pair refused");
    }
    g_same_game = c.same_game;
    g_tx_frames = 0;
    return true;
}

void link_session_stop() {
    if (!g_active || g_loading) return;
    if ((g_kind == LinkKind::TamaGo || g_kind == LinkKind::TamaFriends) && g_cpu && g_cpu->get_cpu_type_id() == CPU_TYPE_BRICK)
        static_cast<BrickCore*>(g_cpu.get())->link_unpair();
    swap_with_globals(g_other);  // globals: the second game
    if (!g_same_game && g_cpu && !g_debug_mode) {
        save_game_state(g_cpu.get(), g_game_index);
    }
    SwitchScreenGameState textures;
    switch_screen_swap_game_state(textures);  // takes its textures off the globals
    switch_screen_free_game_state(textures);
    swap_with_globals(g_other);  // globals: the first game, g_other: the second
    g_other = GameCtx();         // frees the second CPU
    g_active = false;
    // Release the first game's line, in case the other end held it low.
    if (g_kind == LinkKind::Digimon) wire_set(g_cpu.get(), -1);
    if (g_kind == LinkKind::TamaV3 && g_cpu && g_cpu->get_cpu_type_id() == CPU_TYPE_BRICK)
        static_cast<BrickCore*>(g_cpu.get())->link_set_rx(false);
    g_kind = LinkKind::None;
    g_dev[0] = g_dev[1] = Dev::None;
    YOKOI_LOG("link_session: disconnected");
}

// The Sound Switch Button (switch_settings.h): its HidNpadButton bit / its
// RemapButton, from the hotkey choice 0 Off, 1 ZL, 2 ZR, 3 L3, 4 R3.
static uint64_t link_sound_button_mask() {
    static const uint64_t k[5] = {0, HidNpadButton_ZL, HidNpadButton_ZR, HidNpadButton_StickL, HidNpadButton_StickR};
    return g_link_sound_button < 5 ? k[g_link_sound_button] : 0;
}
static int link_sound_button_rb() {
    static const int k[5] = {-1, RB_ZL, RB_ZR, RB_L3, RB_R3};
    return g_link_sound_button < 5 ? k[g_link_sound_button] : -1;
}

// A physical button one of the players' functions is on now.
static bool link_button_in_use(int rb) {
    for (int pl = 0; pl < 2; pl++) {
        const Fn* fns;
        int n;
        fns_of(g_dev[pl], fns, n);
        for (int i = 0; i < n; i++)
            if (control_remap_source(kLinkControlsRef, kSlotKeys[pl][fns[i].slot]) == rb) return true;
    }
    return false;
}

// Each player's three buttons, as "logical" RemapButtons that Controls
// can move to other physical ones (control_remap_source(kLinkControlsRef, ...)).
void link_session_apply_input() {
    if (!g_active) return;
    const uint64_t held = switch_input_raw_held();
    // Settings > Controller Features > Pedometer: a shake presses the chosen
    // player's step button.
    motion_steps_update(has_step(g_dev[g_motion_player]));
    const bool shake = brick_input_take_motion_frame();
    for (int pl = 0; pl < 2; pl++) {
        SM5XX* cpu = pl == 0 ? g_cpu.get() : g_other.cpu.get();
        const Fn* fns;
        int n;
        fns_of(g_dev[pl], fns, n);
        for (int i = 0; i < n && i < 8; i++) {
            const int phys = control_remap_source(kLinkControlsRef, kSlotKeys[pl][fns[i].slot]);
            bool pressed = (held & remap_button_mask(phys)) != 0;
            if (shake && pl == g_motion_player && std::string(fns[i].button) == "btnStep") pressed = true;
            press_fn(cpu, g_dev[pl], fns[i], pressed, g_fn_last[pl][i]);
        }
    }

    // Touching a device's half of the screen makes it the one you hear.
    static bool s_touching = false;
    HidTouchScreenState ts = {0};
    if (hidGetTouchScreenStates(&ts, 1) && ts.count > 0) {
        // Rotated, the devices are stacked: player 1's half (the top of the
        // rotated screen) is the panel's right half at 90, its left at 270.
        const bool left = ts.touches[0].x < 640;
        if (!s_touching) g_sound_player = (g_screen_rotation == ScreenRotation::Deg90) ? (left ? 1 : 0) : (left ? 0 : 1);
        s_touching = true;
    } else {
        s_touching = false;
    }

    // Docked there's no touch screen: the Sound Switch Button (pause menu)
    // makes you hear the other device. On release, and only if nothing else
    // was pressed meanwhile - so as part of the ZL + ZR pause combo it
    // doesn't flip the sound on its way. Not while a player's function sits
    // on that button (pause menu > Controls).
    static bool s_held = false, s_other = false;
    const uint64_t sw = link_sound_button_mask();
    const bool down = sw && (held & sw);
    if (down && (held & ~sw)) s_other = true;
    if (s_held && !down && !s_other && !link_button_in_use(link_sound_button_rb())) g_sound_player ^= 1;
    if (!down) s_other = false;
    s_held = down;
}

std::vector<ControlRow> link_control_rows() {
    std::vector<ControlRow> rows;
    for (int pl = 0; pl < 2; pl++) {
        const Fn* fns;
        int n;
        fns_of(g_dev[pl], fns, n);
        for (int i = 0; i < n; i++) rows.push_back(ControlRow{fns[i].name, {kSlotKeys[pl][fns[i].slot]}});
    }
    return rows;
}

int link_control_player_rows(int player) {
    const Fn* fns;
    int n;
    fns_of(g_dev[player ? 1 : 0], fns, n);
    return n;
}

bool link_control_row_special(int player, int i) {
    const Fn* fns;
    int n;
    fns_of(g_dev[player ? 1 : 0], fns, n);
    return i >= 0 && i < n && fns[i].special;
}

std::string link_player_name(int player) {
    const GW_rom* g = player ? g_other.game : g_game;
    return g ? g->name : std::string();
}

bool link_motion_choice() { return g_active && (has_step(g_dev[0]) || has_step(g_dev[1])); }
int link_motion_player() { return g_motion_player; }
void link_set_motion_player(int player) {
    g_motion_player = player ? 1 : 0;
}

void link_session_step() {
    if (!g_active || !g_cpu || !g_other.cpu) return;
    g_other.cpu->step();
    if (g_kind == LinkKind::Digimon) {
        // One wire, pulled up: low while either device pulls it low.
        const int line = wire_drive(g_cpu.get()) & wire_drive(g_other.cpu.get());
        wire_set(g_cpu.get(), line);
        wire_set(g_other.cpu.get(), line);
        if (!line) g_last_tx = true;
    } else if (g_kind == LinkKind::TamaV3) {
        BrickCore* a = static_cast<BrickCore*>(g_cpu.get());
        BrickCore* b = static_cast<BrickCore*>(g_other.cpu.get());
        const bool ta = a->link_take_tx(), tb = b->link_take_tx();
        b->link_set_rx(ta);
        a->link_set_rx(tb);
        if (ta || tb) g_last_tx = true;
    }
}

void link_session_after_steps() {
    if (!g_active) return;
    swap_with_globals(g_other);
    update_segments_from_cpu(g_cpu.get());
    swap_with_globals(g_other);
    // "Transmitting" stays up a little after the last activity on the line.
    if (g_last_tx) g_tx_frames = 45;
    else if (g_tx_frames > 0) g_tx_frames--;
    g_last_tx = false;
}

void link_session_render(int viewport_w, int viewport_h) {
    (void)viewport_w;
    (void)viewport_h;
    switch_screen_set_panel(0.5f, 0.0f, true);
    switch_screen_render();
    if (g_active) {
        swap_with_globals(g_other);
        switch_screen_set_panel(0.5f, 0.5f, false);
        switch_screen_render();
        swap_with_globals(g_other);
    }
    switch_screen_set_area(1.0f);

    // Labels over the games, in the games' own orientation: side by side,
    // or (Game Rotation 90/270) player 1 on top, player 2 below.
    const bool stacked = g_screen_rotation != ScreenRotation::Deg0;
    const float LW = stacked ? 720.0f : 1280.0f, LH = stacked ? 1280.0f : 720.0f;
    switch_ui_set_rotation((int)g_screen_rotation);
    switch_ui_begin_frame((int)LW, (int)LH);
    if (stacked) switch_ui_draw_rect(0.0f, LH * 0.5f - 2.0f, LW, 4.0f, 0.1f, 0.1f, 0.1f, 1.0f);
    else switch_ui_draw_rect(638.0f, 0.0f, 4.0f, 720.0f, 0.1f, 0.1f, 0.1f, 1.0f);
    {
        // The device you hear gets the frame a game has in the one-at-a-time
        // list (its maker's colour or the theme accent), round its picture
        // as fitted into its half - see switch_screen_render().
        const int pl = g_sound_player;
        if (pl == 1) swap_with_globals(g_other);
        const float cw = g_combined_canvas_w > 0.0f ? g_combined_canvas_w : 1.0f;
        const float ch = g_combined_canvas_h > 0.0f ? g_combined_canvas_h : 1.0f;
        const uint16_t idx = g_game_index;
        if (pl == 1) swap_with_globals(g_other);
        const float hx = stacked ? 0.0f : (pl ? LW * 0.5f : 0.0f), hy = stacked ? (pl ? LH * 0.5f : 0.0f) : 0.0f;
        const float hw = stacked ? LW : LW * 0.5f, hh = stacked ? LH * 0.5f : LH;
        float w = hw, h = hw * ch / cw;
        if (h > hh) { h = hh; w = hh * cw / ch; }
        static const float kSize[6] = {0.70f, 0.85f, 1.0f, 1.15f, 1.0f, 1.0f};
        const float k = kSize[g_screen_size < 6 ? g_screen_size : 2];
        if (g_screen_size == 4) { w = hw; h = hh; } else { w *= k; h *= k; }
        const float pad = 10.0f;
        const float x0 = std::max(hx, hx + (hw - w) * 0.5f - pad), x1 = std::min(hx + hw, hx + (hw + w) * 0.5f + pad);
        const float y0 = std::max(hy, hy + (hh - h) * 0.5f - pad), y1 = std::min(hy + hh, hy + (hh + h) * 0.5f + pad);
        float fr, fg, fb;
        switch_menu_frame_color(idx, fr, fg, fb);
        switch_ui_draw_rect(x0, y0, x1 - x0, pad, fr, fg, fb, 1.0f);
        switch_ui_draw_rect(x0, y1 - pad, x1 - x0, pad, fr, fg, fb, 1.0f);
        switch_ui_draw_rect(x0, y0, pad, y1 - y0, fr, fg, fb, 1.0f);
        switch_ui_draw_rect(x1 - pad, y0, pad, y1 - y0, fr, fg, fb, 1.0f);
    }
    const float size = 22.0f;
    auto label = [&](float cx, float y, float max_w, const std::string& text) {
        float sz = size;
        while (sz > 13.0f && switch_ui_text_width(text, sz, true) > max_w) sz -= 1.0f;
        const float w = switch_ui_text_width(text, sz, true);
        switch_ui_draw_rect(cx - w * 0.5f - 12.0f, y, w + 24.0f, sz + 14.0f, 0.0f, 0.0f, 0.0f, 0.55f);
        switch_ui_draw_text(cx - w * 0.5f, y + 6.0f, sz, 1.0f, 1.0f, 1.0f, 1.0f, text, true);
    };
    // "(Sound)" marks whose sound plays (pause menu > Sound From).
    const std::string name1 = switch_tr("Player 1") + " - " + (g_game ? g_game->name : std::string()) + (g_sound_player == 0 ? "  (" + switch_tr("Sound") + ")" : std::string());
    const std::string name2 = switch_tr("Player 2") + " - " + (g_other.game ? g_other.game->name : std::string()) + (g_sound_player == 1 ? "  (" + switch_tr("Sound") + ")" : std::string());
    if (stacked) {
        label(LW * 0.5f, 10.0f, LW - 48.0f, name1);
        label(LW * 0.5f, LH * 0.5f + 12.0f, LW - 48.0f, name2);
    } else {
        label(320.0f, 10.0f, 600.0f, name1);
        label(960.0f, 10.0f, 600.0f, name2);
    }
    const std::string state = switch_tr(g_tx_frames > 0 ? "Transmitting..." : "Connected");
    const float w = switch_ui_text_width(state, size, true);
    const float r = g_tx_frames > 0 ? 0.35f : 0.04f, gg = g_tx_frames > 0 ? 0.93f : 0.75f, b = g_tx_frames > 0 ? 0.75f : 1.0f;
    // Bottom centre; stacked, on the line between the two devices.
    const float sy = stacked ? LH * 0.5f - (size + 16.0f) * 0.5f : 664.0f;
    switch_ui_draw_rect(LW * 0.5f - w * 0.5f - 16.0f, sy, w + 32.0f, size + 16.0f, r, gg, b, 0.95f);
    switch_ui_draw_text(LW * 0.5f - w * 0.5f, sy + 7.0f, size, 0.05f, 0.1f, 0.12f, 1.0f, state, true);
    // Each player's buttons, as they are now (pause menu > Controls).
    auto keys_of = [](int pl) {
        const Fn* fns;
        int n;
        fns_of(g_dev[pl], fns, n);
        std::string s;
        for (int i = 0; i < n; i++) {
            if (fns[i].special) continue;
            if (!s.empty()) s += " / ";
            const int phys = control_remap_source(kLinkControlsRef, kSlotKeys[pl][fns[i].slot]);
            s += phys == kNoButton ? std::string("-") : std::string(remap_button_label(phys));
        }
        return s;
    };
    // ...and the button that makes them the one you hear.
    std::string keys1 = keys_of(0);
    std::string keys2 = keys_of(1);
    if (link_sound_button_mask() && !link_button_in_use(link_sound_button_rb()))
        keys1 += std::string("    ") + time_control_button_label(g_link_sound_button) + ": " + switch_tr("Switch sound");
    if (stacked) {
        // Under each device, left - clear of the "Connected" badge.
        switch_ui_draw_text(24.0f, LH * 0.5f - 64.0f, 18.0f, 1.0f, 1.0f, 1.0f, 0.7f, keys1);
        switch_ui_draw_text(24.0f, LH - 44.0f, 18.0f, 1.0f, 1.0f, 1.0f, 0.7f, keys2);
    } else {
        switch_ui_draw_text(24.0f, 676.0f, 18.0f, 1.0f, 1.0f, 1.0f, 0.7f, keys1);
        switch_ui_draw_text(1256.0f - switch_ui_text_width(keys2, 18.0f), 676.0f, 18.0f, 1.0f, 1.0f, 1.0f, 0.7f, keys2);
    }
}

std::string link_session_partner_name() { return (g_active && g_other.game) ? g_other.game->name : std::string(); }

int link_session_sound_player() { return g_sound_player; }
void link_session_set_sound_player(int player) { g_sound_player = player ? 1 : 0; }

SM5XX* link_session_sound_cpu() {
    // Every linkable core ticks at 32768 Hz with no sound divider, so
    // g_sound's setup for player 1 fits player 2's buzzer too, whatever chip.
    if (g_active && g_sound_player == 1 && g_other.cpu) return g_other.cpu.get();
    return g_cpu.get();
}
