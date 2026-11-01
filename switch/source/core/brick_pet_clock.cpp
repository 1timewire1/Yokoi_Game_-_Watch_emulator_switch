#include "brick_pet_clock.h"

#include <cctype>

#include "BrickCores/BrickCore.h"
#include "std/debug_log.h"

namespace {

struct BuiltinOption {
    const char* key;
    const char* value;
};

struct BuiltinClock {
    const char* ref;
    BuiltinOption options[10];
};

// CPU addresses in decimal, like brick_import.py writes them.
const BuiltinClock kBuiltinClocks[] = {
    {"APOLLO2IN1",
     {{"clock.sec", "156,157"},          // 0x9C, 0x9D
      {"clock.min", "158,159"},          // 0x9E, 0x9F
      {"clock.hour", "160,161"},         // 0xA0, 0xA1
      {"clock.pm", "162,1,1"},           // 0xA2 bit 0 set = PM
      // Set-time screen: its fixed pixels ("SET" etc.) all lit (display byte, mask).
      {"clock.set_screen_lcd",
       "9,157,10,192,11,136,12,72,13,64,14,132,17,133,19,136,20,64,25,157,26,192,27,171,28,192,29,8,31,119,33,132,34,64,35,218,36,64,39,85,40,1,41,221,42,192,43,137,44,128,45,8,47,85,55,85,60,24,61,1,62,128,63,119"},
      {"clock.unset", "155,0"},          // 0x9B == 0: never set
      {"clock.confirm", "btnExit"},
      {"clock.pet", "1"},
      {nullptr, nullptr}}},
    {"POCKETPIKACHU",
     {{"clock.sec", "87"},               // 0x57, BCD
      {"clock.min", "88"},               // 0x58, BCD
      {"clock.hour", "89"},              // 0x59, BCD 0-11
      {"clock.pm", "90,1,1"},            // 0x5A bit 0 set = PM
      {"clock.bcd", "1"},
      {"clock.hour_zero", "1"},
      {"clock.unset", "125,3"},          // 0x7D == 3: first-boot set screen
      {"clock.confirm", "btnA"},
      {"clock.pet", "1"},
      {nullptr, nullptr}}},
    {"TAMAGOTCHIMINI2017",
     {{"clock.sec", "151"},              // 0x97, half-seconds 0-119
      {"clock.min", "152"},              // 0x98, 0-59
      {"clock.hour", "153"},             // 0x99, 0-23
      {"clock.binary", "1"},
      {"clock.sec_scale", "2"},
      {"clock.unset_zero", "1"},         // boots stopped at 00:00:00
      {"clock.confirm", "btnCenter"},
      {"clock.pet", "1"},
      {nullptr, nullptr}}},
};

// Pack refs are the game's set name upper-cased ("APOLLO2IN1"); compare
// without case so either spelling matches.
bool same_ref(const std::string& a, const char* b) {
    size_t i = 0;
    for (; i < a.size() && b[i]; i++) {
        if (std::toupper((unsigned char)a[i]) != std::toupper((unsigned char)b[i])) return false;
    }
    return i == a.size() && !b[i];
}

}  // namespace

void brick_pet_clock_apply_builtin(BrickCore* core, const std::string& ref) {
    if (!core) return;
    for (const BuiltinClock& c : kBuiltinClocks) {
        if (!same_ref(ref, c.ref)) continue;
        // Only what the container lacks: older imports carried sec/min/hour/pm
        // but not the rest (Pocket Pikachu's bcd/hour_zero/unset/confirm).
        int added = 0;
        for (const BuiltinOption* o = c.options; o->key; o++) {
            if (core->has_option(o->key)) continue;
            core->set_option(o->key, o->value);
            added++;
        }
        YOKOI_LOG("brick_pet_clock: %s - %d built-in clock option(s) added", ref.c_str(), added);
        return;
    }
}
