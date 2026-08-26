#pragma once

// Android-only: transitions from menu to game, optionally loading savestate.
void yokoi_start_game_from_menu(bool load_state);

// Android-only: transitions from game back to menu (saving state if needed).
void yokoi_return_to_menu_from_game();
