#pragma once

#include <cstdint>
#include <string>

// App language (Settings > Display > Language). The UI is written in
// English; with Spanish selected, switch_ui_draw_text()/text_width() pass
// every string through switch_tr() first - an exact-match table of the
// menu's own texts plus a few patterns for the composed ones ("Slot 2",
// "12 games", "Saved to Slot 2"...). Anything not in the table (game and
// maker names, numbers) is drawn unchanged.
enum : uint8_t { kLangEnglish = 0, kLangSpanish = 1, kNumLanguages = 2 };
const char* switch_language_name(uint8_t lang);  // in its own language

// The text to show for `en` in the current language (g_language). The
// reference stays valid until the language changes.
const std::string& switch_tr(const std::string& en);
const char* switch_tr_c(const char* en);
