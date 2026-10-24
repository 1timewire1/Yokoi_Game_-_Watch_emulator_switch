// Host-side tool (not part of the Switch build): prints, for every Burger Bard
// game, the element names its ported logic can light - the union of
// get_all_game_elements() in Burger mode and the extras Burger adds ("win",
// the hidden "score-1" group). extract_lcdonald_burger.py reads this to know
// which SVG groups to extract into segments.
//
// Build & run from the repo root (mingw g++ with the devkitPro msys2 toolchain):
//   g++ -std=c++17 -Isource/LCDonald -Isource/LCDonald/Games -Isource -Isource/std \
//       CONVERT_ROM/lcdonald_dump_elements.cpp source/LCDonald/LCDonald_program.cpp \
//       source/LCDonald/Games/*.cpp source/std/timer.cpp -o lcdonald_dump_elements.exe
//   lcdonald_dump_elements.exe > CONVERT_ROM/lcdonald_burger_elements.json
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "LCDonald_program.h"
#include "Games/AmyRougeVolleyball.h"
#include "Games/BigFishing.h"
#include "Games/BillyGiantEgg.h"
#include "Games/CreamFlowerCatch2005.h"
#include "Games/KnucklesBaseball.h"
#include "Games/KnucklesTreasureHunt.h"
#include "Games/ShadowBasketball.h"
#include "Games/ShadowGrinder.h"
#include "Games/ShadowHockey.h"
#include "Games/SonicActionGame.h"
#include "Games/SonicExtremeBoarding.h"
#include "Games/SonicSkateboard.h"
#include "Games/SonicSpeedway.h"
#include "Games/TailsSkyAdventure.h"
#include "Games/TailsSkyPatrol.h"
#include "Games/TailsSoccer.h"

namespace {
template <typename T>
void dump(const char* folder, bool& first) {
    T game;
    game.set_burger(folder);
    std::vector<std::string> names = game.get_all_game_elements();
    names.push_back("win");
    names.push_back("score-1");
    printf("%s  \"%s\": [", first ? "" : ",\n", folder);
    first = false;
    std::vector<std::string> seen;
    bool firstName = true;
    for (const std::string& n : names) {
        bool dup = false;
        for (const std::string& s : seen) dup = dup || s == n;
        if (dup) continue;
        seen.push_back(n);
        printf("%s\"%s\"", firstName ? "" : ", ", n.c_str());
        firstName = false;
    }
    printf("]");
}
}  // namespace

int main() {
    bool first = true;
    printf("{\n");
    dump<AmyRougeVolleyball>("volleyball", first);
    dump<BigFishing>("fishing", first);
    dump<BillyGiantEgg>("eggcatch", first);
    dump<CreamFlowerCatch2005>("heartmooncatch", first);
    dump<KnucklesBaseball>("football", first);
    dump<KnucklesTreasureHunt>("treasure", first);
    dump<KnucklesTreasureHunt>("treasure2", first);
    dump<SonicActionGame>("platform", first);
    dump<SonicActionGame>("platform2", first);
    dump<ShadowBasketball>("wbasketball", first);
    dump<ShadowGrinder>("railgrinder", first);
    dump<ShadowHockey>("magicbattle", first);
    dump<SonicSkateboard>("skateboard", first);
    dump<SonicSpeedway>("speedway", first);
    dump<SonicExtremeBoarding>("extremeboard", first);
    dump<TailsSkyAdventure>("targetshoot", first);
    dump<TailsSkyPatrol>("seashell", first);
    dump<TailsSoccer>("kosoccer", first);
    dump<BillyGiantEgg>("eggcatch2", first);
    printf("\n}\n");
    return 0;
}
