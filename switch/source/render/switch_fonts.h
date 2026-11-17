#pragma once

#include <cstddef>

// The bundled menu fonts (switch_fonts.cpp). Index 0 of the settings'
// "Menu Font" is the system font; bundled font i is setting i + 1.
struct SwitchFontData {
    const char* name;
    const unsigned char* data;
    size_t size;
};
extern const SwitchFontData kSwitchFonts[];
extern const int kSwitchFontCount;
