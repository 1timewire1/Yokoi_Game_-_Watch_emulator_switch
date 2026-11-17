#include "game_controls.h"
#include "link_session.h"

#include <cctype>
#include <cstdio>
#include <map>
#include <string>

#include "input/control_remap.h"
#include "core/switch_runtime_state.h"
#include "std/GW_ROM.h"
#include "std/platform_paths.h"
#include "std/savestate.h"
#include "SM5XX/SM5XX.h"
#include "BrickCores/BrickCore.h"
#include "LCDonald/LCDonald_fake_cpu.h"

// Only for Virtual_Input and the CONF_* shapes; its get_input_config()
// definition is renamed so it doesn't clash with yokoi_input_mapping.cpp's.
#define get_input_config get_input_config__game_controls_tu
#include "virtual_i_o/virtual_input.h"
#undef get_input_config

namespace {

std::map<std::string, std::string> g_names;  // "REF\tname" / "*\tname" -> new name
bool g_names_loaded = false;

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

void load_names() {
    g_names_loaded = true;
    g_names.clear();
    FILE* f = fopen(config_path("control_names.txt").c_str(), "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        std::string s(line);
        if (s.empty() || s[0] == '#') continue;
        const size_t eq = s.find('=');
        if (eq == std::string::npos) continue;
        std::string left = trim(s.substr(0, eq));
        const std::string right = trim(s.substr(eq + 1));
        const size_t sp = left.find_first_of(" \t");
        if (sp == std::string::npos || right.empty()) continue;
        std::string ref = left.substr(0, sp);
        for (char& c : ref) c = (char)toupper((unsigned char)c);
        g_names[ref + "\t" + trim(left.substr(sp + 1))] = right;
    }
    fclose(f);
}

// "btnStep" -> "Step", "btn_fire" -> "Fire", "reflector" -> "Reflector".
std::string pretty(const std::string& raw) {
    std::string s = raw;
    if (s.size() > 3 && (s.compare(0, 3, "btn") == 0 || s.compare(0, 3, "Btn") == 0)) s = s.substr(3);
    std::string out;
    for (size_t i = 0; i < s.size(); i++) {
        const char c = s[i];
        if (c == '_' || c == '-') { if (!out.empty() && out.back() != ' ') out += ' '; continue; }
        if (i > 0 && isupper((unsigned char)c) && islower((unsigned char)s[i - 1]) && !out.empty() && out.back() != ' ') out += ' ';
        out += c;
    }
    out = trim(out);
    if (!out.empty()) out[0] = (char)toupper((unsigned char)out[0]);
    return out.empty() ? raw : out;
}

int key_from_name(const std::string& k) {
    static const std::map<std::string, int> kKeys = {
        {"UP", RB_Up}, {"DOWN", RB_Down}, {"LEFT", RB_Left}, {"RIGHT", RB_Right},
        {"A", RB_A}, {"B", RB_B}, {"X", RB_X}, {"Y", RB_Y}, {"L", RB_L}, {"R", RB_R},
        {"ZL", RB_ZL}, {"ZR", RB_ZR}, {"PLUS", RB_Plus}, {"MINUS", RB_Minus},
    };
    auto it = kKeys.find(k);
    return it == kKeys.end() ? -1 : it->second;
}

void add(std::vector<ControlRow>& rows, const char* name, std::initializer_list<int> keys) {
    rows.push_back({name, std::vector<int>(keys)});
}

void sm5xx_side(std::vector<ControlRow>& rows, bool left, uint8_t conf, uint8_t other) {
    const int up = left ? RB_Up : RB_X, down = left ? RB_Down : RB_B;
    const int l = left ? RB_Left : RB_Y, r = left ? RB_Right : RB_A;
    switch (conf) {
        case CONF_1_BUTTON_ACTION:
            // yokoi_input_mapping.cpp: with both sides single buttons the
            // pad splits down the middle; otherwise a side takes its group.
            if (other == CONF_1_BUTTON_ACTION) {
                if (left) add(rows, "Left Button", {RB_Up, RB_Down, RB_Left, RB_Y});
                else add(rows, "Right Button", {RB_A, RB_B, RB_X, RB_Right});
            } else {
                if (left) add(rows, "Left Button", {RB_Up, RB_Down, RB_Left, RB_Right});
                else add(rows, "Right Button", {RB_A, RB_B, RB_X, RB_Y});
            }
            break;
        case CONF_2_BUTTON_UPDOWN:
            add(rows, left ? "Left: Up" : "Right: Up", {up});
            add(rows, left ? "Left: Down" : "Right: Down", {down});
            break;
        case CONF_2_BUTTON_LEFTRIGHT:
            add(rows, left ? "Left: Left" : "Right: Left", {l});
            add(rows, left ? "Left: Right" : "Right: Right", {r});
            break;
        case CONF_4_BUTTON_DIRECTION:
            add(rows, left ? "Left: Up" : "Right: Up", {up});
            add(rows, left ? "Left: Down" : "Right: Down", {down});
            add(rows, left ? "Left: Left" : "Right: Left", {l});
            add(rows, left ? "Left: Right" : "Right: Right", {r});
            break;
        default:
            break;
    }
}

void e0c6200_rows(std::vector<ControlRow>& rows, const std::string& ref) {
    // Mirrors input/e0c6200_input_map.cpp's pin maps.
    if (ref == "STACKCH") {
        add(rows, "Left / Level", {RB_Left});
        add(rows, "Down / Start", {RB_Down});
        add(rows, "Right / Height", {RB_Right});
        add(rows, "Rotate / Contrast", {RB_A, RB_Up});
        add(rows, "On / Off", {RB_Plus});
        add(rows, "Pause", {RB_Minus});
        add(rows, "Sound", {RB_R});
    } else if (ref == "ALIENFEV") {
        add(rows, "Mode", {RB_X});
        add(rows, "Select", {RB_Y});
        add(rows, "Sound", {RB_R});
        add(rows, "Handle", {RB_A});
    } else if (ref == "TAMAYASA") {
        add(rows, "Food", {RB_Left});
        add(rows, "Smile", {RB_Up});
        add(rows, "Heart", {RB_Right});
        add(rows, "Clean Up", {RB_L});
        add(rows, "Bulb", {RB_Down});
        add(rows, "Weight", {RB_R});
        add(rows, "Select", {RB_A});
        add(rows, "Cancel", {RB_B});
    } else if (ref.rfind("DIGIMON", 0) == 0) {
        // The Digital Monster's own button names.
        add(rows, "A - Select", {RB_Y, RB_Left});
        add(rows, "B - Decide / Confirm", {RB_A, RB_Up, RB_Down});
        add(rows, "C - Cancel", {RB_B, RB_Right});
        add(rows, "Reset Button", {RB_Minus});
    } else {
        add(rows, "Left Button", {RB_Y, RB_Left});
        add(rows, "Middle Button", {RB_A, RB_Up, RB_Down});
        add(rows, "Right Button", {RB_B, RB_Right});
        if (ref == "TAMAANG" || ref == "TAMAUMI" || ref == "TAMAMORI" || ref == "TAMAGENJIN" || ref == "TAMAOTCH")
            add(rows, "Sensor", {RB_X});
    }
}

void generic_rows(std::vector<ControlRow>& rows) {
    add(rows, "Up", {RB_Up});
    add(rows, "Down", {RB_Down});
    add(rows, "Left", {RB_Left});
    add(rows, "Right", {RB_Right});
    add(rows, "Button A", {RB_A});
    add(rows, "Button B", {RB_B});
    add(rows, "Button X", {RB_X});
    add(rows, "Button Y", {RB_Y});
    add(rows, "Button L", {RB_L});
    add(rows, "Button R", {RB_R});
    add(rows, "Button +", {RB_Plus});
    add(rows, "Button -", {RB_Minus});
}

}  // namespace

void game_controls_reload_names() { load_names(); }

std::vector<ControlRow> game_control_rows() {
    std::vector<ControlRow> rows;
    if (!g_cpu || !g_game) return rows;
    // Two linked devices (core/link_session.h): the two players' buttons.
    if (link_session_active()) return link_control_rows();
    const std::string ref = g_game->ref;
    const uint8_t type = g_cpu->get_cpu_type_id();

    if (type == CPU_TYPE_BRICK) {
        const BrickCore* core = static_cast<const BrickCore*>(g_cpu.get());
        for (const BrickCore::Binding& b : core->bindings()) {
            ControlRow row{pretty(b.name), {}};
            std::string key;
            for (size_t i = 0; i <= b.switch_keys.size(); i++) {
                const char c = i < b.switch_keys.size() ? b.switch_keys[i] : '|';
                if (c == '*') continue;
                if (c != '+' && c != '|') { key += c; continue; }
                const int k = key_from_name(key);
                key.clear();
                bool dup = false;
                for (int e : row.keys) dup |= (e == k);
                if (k >= 0 && !dup) row.keys.push_back(k);
            }
            if (!row.keys.empty()) rows.push_back(row);
        }
    } else if (type == CPU_TYPE_E0C6S46) {
        e0c6200_rows(rows, ref);
    } else if (g_game->is_lcdonald_game()) {
        LCDonald_fake_cpu* cpu = static_cast<LCDonald_fake_cpu*>(g_cpu.get());
        bool action = false;
        for (const std::string& n : cpu->program()->get_available_inputs()) {
            if (n == "Left") add(rows, "Left", {RB_Left});
            else if (n == "Right") add(rows, "Right", {RB_Right});
            else if (n == "Up") add(rows, "Up", {RB_Up});
            else if (n == "Down") add(rows, "Down", {RB_Down});
            else if (!action) { rows.push_back({n, {RB_A}}); action = true; }
        }
        add(rows, "Game A", {RB_Plus});
        add(rows, "Game B", {RB_Minus});
    } else if (g_rzone_io) {
        add(rows, "Up", {RB_Up});
        add(rows, "Down", {RB_Down});
        add(rows, "Left", {RB_Left});
        add(rows, "Right", {RB_Right});
        add(rows, "Button 1", {RB_A});
        add(rows, "Button 2", {RB_B});
        add(rows, "Button 3", {RB_X});
        add(rows, "Button 4", {RB_Y});
        add(rows, "Start", {RB_Plus});
        add(rows, "Pause", {RB_Minus});
        add(rows, "Select", {RB_L});
        add(rows, "Sound", {RB_R});
    } else if (g_input) {
        sm5xx_side(rows, true, g_input->left_configuration, g_input->right_configuration);
        sm5xx_side(rows, false, g_input->right_configuration, g_input->left_configuration);
        add(rows, "Game A", {RB_Plus});
        add(rows, "Game B", {RB_Minus});
        add(rows, "Clock (Time)", {RB_L});
    } else {
        // NUMMUNCH / UCHITARI's calculator keypads and anything else
        // without a standard scheme: every button, by its own name.
        generic_rows(rows);
    }

    if (!g_names_loaded) load_names();
    if (!g_names.empty()) {
        for (ControlRow& r : rows) {
            auto it = g_names.find(ref + "\t" + r.name);
            if (it == g_names.end()) it = g_names.find("*\t" + r.name);
            if (it != g_names.end()) r.name = it->second;
        }
    }
    return rows;
}
