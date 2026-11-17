#pragma once

// What each button does in the loaded game, for pause menu > Controls:
// one row per function, with the (logical, see input/control_remap.h)
// buttons that trigger it. Built from the game's own input code - the
// SM5xx side shapes, the BrickEmuPy/HMCS40 container bindings, the
// E0C6200 pin maps, LCDonald's named inputs, R-Zone's fixed set - so a new
// game or function shows up here on its own. Names are English (drawn
// through the translation table like the rest of the menu); the user can
// rename any of them in yokoi_config/control_names.txt:
//     <REF> <name as shown in English> = <new name>
//     *     <name as shown in English> = <new name>     (every game)

#include <string>
#include <vector>

struct ControlRow {
    std::string name;
    std::vector<int> keys;  // RemapButton ids; keys[0] is the one Controls changes
};

std::vector<ControlRow> game_control_rows();
// Re-reads control_names.txt (on entering the Controls section).
void game_controls_reload_names();
