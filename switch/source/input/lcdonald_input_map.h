#pragma once

class LCDonald_fake_cpu;

// Feeds Switch input into a running LCDonald_fake_cpu's named-input model
// (see LCDonald_program::get_available_inputs()) - a single generic
// mapping covers all 20 games without needing a per-game table, since
// every game's input set is drawn from the same small vocabulary:
//   - "Left"/"Right" -> D-Pad Left/Right (mirrored by the left stick
//     already, same as every other game - see switch_input_held()).
//   - "Up"/"Down" -> D-Pad Up/Down, same mirroring.
//   - Any other single name (the game's one action button - "Jump",
//     "Volley", "Tennis", "Hockey", "Basket", "Fire", "Reel", ...) -> A.
// See standard_controls.md's McDonald's section for the documented
// per-game control reference this mapping is designed to match exactly.
void apply_lcdonald_input(LCDonald_fake_cpu* cpu);
