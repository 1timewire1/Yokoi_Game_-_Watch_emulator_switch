#pragma once

#include <cstdint>

// Save slots (pause menu Save State / Load State, and the card screen shown
// when entering a game with saves - switch_menu.cpp). Slot 0 is the usual
// automatic save (written on leaving a game), 1..kNumSaveSlots the manual
// ones, kUndoSlot the hidden copy Simulate Time takes first (Undo Simulate).
// The state files themselves are source/std/savestate.h's
// *_slot() functions; this adds each one's thumbnail: a small capture of
// the game screen plus the time it was taken, "<ref>[_slotN].thm" next to
// the .sav.
constexpr int kNumSaveSlots = 3;
constexpr int kUndoSlot = -1;

struct SaveSlotInfo {
    bool exists = false;     // the .sav is there
    int64_t saved_at = 0;    // unix time (thumbnail header, else the .sav's mtime; 0 = unknown)
    uint32_t tex = 0;        // GL texture of the thumbnail, 0 = none
};

// Renders the running game (g_game/g_cpu) off-screen and writes the
// thumbnail for `slot` of game `game_index`. Needs the GL context (call
// from the main thread, like everything else here). Returns false and
// leaves any old thumbnail alone on failure - the slot still works, the
// card just shows no picture.
bool save_slot_write_thumbnail(uint16_t game_index, int slot);
void save_slot_delete_thumbnail(uint16_t game_index, int slot);

// (Re)reads slots 0..kNumSaveSlots of `game_index` - existence, time,
// thumbnail textures - for the card screens. Cheap enough to call each
// time one of them opens; frees the previous game's textures.
void save_slots_refresh(uint16_t game_index);
const SaveSlotInfo& save_slot_info(int slot);  // 0..kNumSaveSlots
void save_slots_free();

// "08/11/2026 21:40" for a card, "" for 0.
const char* save_slot_time_text(int64_t t);
