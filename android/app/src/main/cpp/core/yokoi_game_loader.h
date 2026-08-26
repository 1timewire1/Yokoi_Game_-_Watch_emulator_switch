#pragma once

#include <cstdint>

// Android-only: loads a game ROM by index, initializes CPU/input/segments, warms up,
// publishes an initial segment snapshot, and refreshes runtime state.
void yokoi_load_game_by_index_and_init(uint8_t idx);

// Android-only helper for initial game selection.
uint8_t yokoi_get_default_game_index_for_android();

// Android-only helper for default menu choice (load state vs new).
int yokoi_get_default_menu_load_choice_for_game(uint8_t idx);
