#pragma once

class BrickCore;

// Feeds Switch input to a BrickEmuPy-ported core (source/BrickCores/). Every
// such game carries its own button list in its ROM container: each .brick
// button name with the Switch keys CONVERT_ROM/brick_import.py assigned it
// ("LEFT", "A", "PLUS+MINUS", "LEFT|Y" ... - BrickCore::bindings()). So
// there's no per-game code here: a binding is pressed while any of its "|"
// alternatives has all of its "+" keys held.
// A chord (two or more keys - used for reset-type buttons, so they can't be
// hit by accident) takes priority over single-key bindings sharing one of
// its keys, which stay released while the chord is held. Only changes are
// passed on to the core.
// A '*' in front of an alternative ("A|*ZR") makes it a turbo: held, it
// presses and releases the button 10 times a second (see kTurboFrames).
void apply_brick_input(BrickCore* core);
