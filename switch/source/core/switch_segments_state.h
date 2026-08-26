#pragma once

class SM5XX;

// Refreshes g_seg_on from the CPU's current segment states, with the same
// single-frame blink protection as the 3DS/Android renderers. Single-threaded
// simplification of yokoi_segments_state.cpp (no snapshot double-buffering -
// nothing else touches g_seg_on concurrently in the Switch build).
void update_segments_from_cpu(SM5XX* cpu);
