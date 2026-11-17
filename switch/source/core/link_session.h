#pragma once

#include <cstdint>
#include <string>
#include <vector>

class SM5XX;

// Local link: a second device running next to the loaded game on the same
// Switch, joined to it by its link cable / infrared port - Digimon battles,
// Tamagotchi Connection V3 visits. Pause menu > Game > "Connect to Another
// Game".
//
// The second game is loaded with the normal loader into its own set of the
// core/ globals (g_cpu, g_game, g_segments, the layout...) plus the screen's
// per-game state (render/switch_screen.h SwitchScreenGameState), and the two
// sets are swapped around the few places that need the second one: loading
// it, its segment update and its render. Both are stepped in lockstep, one
// 32768 Hz tick each, and their ports joined after every tick:
//   Digimon V1-V4 (E0C6S46/S48), D-Tector and D-Power (GPL191X): the
//     two-prong wire, one pin either device can pull low - each reads the
//     AND of what both drive (E0C6S46::link_drive() on P20,
//     BrickCore::wire_link_drive() on the GPL191X's CD2 / CD0). Any two of
//     them can be joined; whether the games understand each other is theirs.
//   Tamagotchi Connection V3 (SPLB32): each one's IR LED carrier lights the
//     other's receiver (BrickCore::link_take_tx() / link_set_rx()).
//   TamaTown Tama-Go, Tamagotchi Friends (TAMAGO): the core joins the two
//     itself and runs both in lock step (BrickCore::link_pair()); a Tama-Go
//     links with a Tama-Go, a Friends with a Friends.
//
// Screen: player 1 (the game that was loaded) on the left half, player 2 on
// the right. Controls, on one controller: player 1 the left Joy-Con's
// Left/Down/Right buttons, player 2 the right Joy-Con's Y/X/A - each device's
// three buttons left to right (the D-Power's fourth, Up: Up / B).

enum class LinkKind : uint8_t { None, Digimon, TamaV3, TamaGo, TamaFriends };

// Which link a game has (by ref), None for the rest.
LinkKind link_kind_of(const std::string& ref);

struct LinkCandidate {
    uint16_t index;      // game index
    std::string name;
    bool same_game;      // the loaded game itself: a copy of its running state
    bool has_save;       // otherwise: its auto save, if any (else a fresh one)
};
// Games the loaded one can connect to (same LinkKind), the loaded one first.
std::vector<LinkCandidate> link_candidates();

bool link_session_active();
// Loads the second game (needs a game loaded). False if it couldn't.
bool link_session_start(const LinkCandidate& c);
// Saves the second game's auto save (not for a copy of the first) and unloads it.
void link_session_stop();

// Per frame, from main.cpp, instead of the normal input mapping.
void link_session_apply_input();
// One tick of the second game plus the port exchange - right after each
// g_cpu->step() of the first one.
void link_session_step();
// After the frame's steps: the second game's segment update.
void link_session_after_steps();
// Draws both games, the divider and the labels (instead of switch_screen_render()).
void link_session_render(int viewport_w, int viewport_h);
// Name of the second game, for the pause menu ("" when not linked).
std::string link_session_partner_name();

// Whose sound plays (pause menu > Game > Sound From): 0 player 1, 1 player 2.
// Kept between links.
int link_session_sound_player();
void link_session_set_sound_player(int player);
// The device whose buzzer g_sound plays this tick (g_cpu when not linked).
// Also switched by touching a device's half of the screen.
SM5XX* link_session_sound_cpu();

// Pause menu > Controls while linked: each player's buttons and their
// device's special functions (sensor, step, wave, card, reset), each one
// changeable there like a game's (stored under kLinkControlsRef in
// controls.cfg). Player 1 on the left Joy-Con (Left / Down / Right / Up, L,
// Minus, L3), player 2 on the right one (Y / X / A / B, R, Plus, R3).
constexpr const char* kLinkControlsRef = "LINK";
struct ControlRow;
// Player 1's rows, then player 2's (link_control_player_rows(0) of them first).
std::vector<ControlRow> link_control_rows();
int link_control_player_rows(int player);
// Whether row `i` of player `player`'s list is a special function.
bool link_control_row_special(int player, int i);
// That player's device name (its game's).
std::string link_player_name(int player);
// "Shaking the controller counts steps for": 0 player 1, 1 player 2 - only
// when a device has a step button (link_motion_choice()).
bool link_motion_choice();
int link_motion_player();
void link_set_motion_player(int player);
