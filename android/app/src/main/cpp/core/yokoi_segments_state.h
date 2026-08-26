#pragma once

#include <cstdint>

class SM5XX;

// Publishes the latest segment on/off snapshot for rendering.
// Same behavior as the prior in-file implementation.
void update_segments_from_cpu(SM5XX* cpu);
