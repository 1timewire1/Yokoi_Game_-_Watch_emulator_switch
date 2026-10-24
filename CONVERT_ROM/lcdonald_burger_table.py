"""Single source of truth for the 19 Burger Bard (LCDonald `#if BURGER` build)
entries, shared by extract_lcdonald_burger.py (segment art + segment maps),
generate_lcdonald_pack.py (GW_rom entries / pack assets) and
generate_lcdonald_thumbnails.py (menu thumbnails).

Burger Bard is LCDonald's white-label re-skin of 16 of the 20 McDonald's
games plus three art-only duplicates (platform2 / treasure2 / eggcatch2),
under the fictional "Burger Bard Crystalectrics" brand - the manufacturer
credited on the (fictional) instruction sheets is "Burger Bard". See
GNW_LIST.md's Burger Bard section for the research notes.

Fields: (game_id, folder, cpp_class, ref, display_name, base_short_name,
clone_of_ref). `folder` is the Burger asset folder / LCDonald short name;
`base_short_name` is the McDonald's game whose logic (and, since the Burger
sounds are byte-identical, sound files) it reuses; `clone_of_ref` is the
ultimate-origin McDonald's ref used for the menu's clone tier (following the
same chain GNW_LIST.md's McDonald's table documents).
"""

# Release date used for every entry: 2025-10-20 - the "Final asset updates" /
# "Add \"win\" element to nongeneric victory animations" commits on
# github.com/Difegue/LCDonald (the burger-whitelabel work merged 2025-10-12
# via PR #44, then finalised on 10-20). The upstream repo has no per-game
# dates; this is the best single "initial complete release" stamp.
BURGER_DATE = "2025-10-20"

BURGER_GAMES = [
    (21, "volleyball", "AmyRougeVolleyball", "BB_VOLLEYBALL", "Curly & Frenchie's Volleyball", "arvolleyball", "ARVOLLEYBALL"),
    (22, "fishing", "BigFishing", "BB_FISHING", "Burger Bard Fishing", "bfishing", "BFISHING"),
    (23, "eggcatch", "BillyGiantEgg", "BB_EGGCATCH", "Halmon's Egg Catch", "bgiantegg", "BGIANTEGG"),
    (24, "heartmooncatch", "CreamFlowerCatch2005", "BB_HEARTMOON", "Honey's Heart-Moon Catch", "cflower2", "ABANANA"),
    (25, "football", "KnucklesBaseball", "BB_FOOTBALL", "King Beareon's Football Catch", "kbaseball", "KBASEBALL_ESPN"),
    (26, "treasure", "KnucklesTreasureHunt", "BB_TREASURE", "Bard's Burger Hunt", "ktreasure", "KTREASURE"),
    (27, "treasure2", "KnucklesTreasureHunt", "BB_TREASURE2", "Halmon's Egg Hunt", "ktreasure", "KTREASURE"),
    (28, "platform", "SonicActionGame", "BB_PLATFORM", "Kola Bear's Platform Adventure", "sagame", "SAGAME"),
    (29, "platform2", "SonicActionGame", "BB_PLATFORM2", "Burger Bard's Platform Adventure", "sagame", "SAGAME"),
    (30, "wbasketball", "ShadowBasketball", "BB_WBASKETBALL", "Gloomy Wolf Basketball", "sbasketball", "SHOCKEY"),
    (31, "railgrinder", "ShadowGrinder", "BB_RAILGRINDER", "Vibes Rail Grinder", "sgrinder", "SGRINDER"),
    (32, "magicbattle", "ShadowHockey", "BB_MAGICBATTLE", "Bat Magic's Magic Battle", "shockey", "SHOCKEY"),
    (33, "skateboard", "SonicSkateboard", "BB_SKATEBOARD", "Vibes Skateboarding", "sskateboard", "SXTREMEBOARD"),
    (34, "speedway", "SonicSpeedway", "BB_SPEEDWAY", "Burger Bard Speedway Racing", "sspeedway", "SSPEEDWAY"),
    (35, "extremeboard", "SonicExtremeBoarding", "BB_EXTREMEBOARD", "Vibes Extreme Skateboarding", "sxtremeboard", "SXTREMEBOARD"),
    (36, "targetshoot", "TailsSkyAdventure", "BB_TARGETSHOOT", "Taco Sal's Target Shooting", "tskyadventure", "TSKYADVENTURE"),
    (37, "seashell", "TailsSkyPatrol", "BB_SEASHELL", "Soakey's Shell Catch", "tskypatrol", "TSKYPATROL"),
    (38, "kosoccer", "TailsSoccer", "BB_KOSOCCER", "Kola Bear's Soccer", "tsoccer", "KSOCCER"),
    (39, "eggcatch2", "BillyGiantEgg", "BB_EGGCATCH2", "Barry's Treasure Hunt", "bgiantegg", "BGIANTEGG"),
]

# Elements every Burger game may light beyond its own get_all_game_elements():
# "win" (the WINNER image shown in victory frames) and the hidden "score-1"
# group. "score-0" exists in the art but nothing in the logic ever shows it.
BURGER_EXTRA_ELEMENTS = ["win", "score-1"]


def cpp_name(folder: str) -> str:
    return f"bb_{folder}"
