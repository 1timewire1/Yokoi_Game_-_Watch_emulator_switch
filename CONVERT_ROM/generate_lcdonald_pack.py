"""One-off generator for the 20 LCDonald-ported McDonald's games' GW_rom data
(plus their 10 US/ESPN variant entries), following the exact same "no real
ROM, self-contained checked-in .cpp/.h + PNGs" pattern the 5 David and John
games already use (see source/std/David_And_John_ROM_rgds/*.cpp and
convert_switch.py's gfx2x_open_source bundling comment) rather than routing
through the shared MAME-oriented games_path/convert_switch.py pipeline that
serves the other 166 games - these have no MAME ROM to extract from, and the
segment art needed a custom extraction step (see
source/LCDonald/SegmentMaps/*.h's own generation notes) that the shared
pipeline's cs.extract_group_segs() doesn't support (different SVG shape).

Reuses convert_switch.py's own visual_data_file()/background_data_file()/
visual_console_data()/rom_text()/melody_text() - the exact same atlas-packing
and Segment-array-emitting code every other game in the library goes through
- just fed pre-extracted segment PNGs (from the standalone extraction script,
copied into ./tmp/img/<name>/ below) instead of letting those functions call
cs.extract_group_segs() themselves (which assumes MAME's <g id="layer1">-
style .lay/SVG shape, not LCDonald's).

Also bundles every game's real .ogg sound effects (see bundle_sounds()) for
switch/source/audio/lcdonald_sound.cpp to decode at runtime (stb_vorbis) -
the one part of this pipeline that isn't a CONVERT_ROM function reused
as-is, since none of the other 166 games have real sampled audio to bundle.

Run from CONVERT_ROM/:  C:/Python314/python.exe generate_lcdonald_pack.py
(needs lxml/PIL/numpy/rectpack - same interpreter the earlier segment-
extraction/validation scripts in this session used, not devkitPro's MSYS2
python which lacks these packages).
"""
import os
import shutil
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
os.chdir(SCRIPT_DIR)
sys.path.insert(0, str(SCRIPT_DIR))

import convert_switch as csw  # noqa: E402
from lcdonald_burger_table import BURGER_DATE, BURGER_GAMES  # noqa: E402
from lcdonald_burger_table import cpp_name as burger_cpp_name  # noqa: E402
from source.target_profiles import get_target  # noqa: E402

# Pre-extracted, validated per-segment PNGs (named <col>.<line>.<word>.<screen>.png,
# matching source/LCDonald/SegmentMaps/*.h), checked in under
# lcdonald_segments/: base/<game>/ from each game's <game>_0.svg (incl. a
# hand-synthesized tsoccer ball-41), espn/<game>/ from the ESPN prints'
# separate <game>_1.svg - ESPN variants aren't just a cosmetic background swap,
# 6 of the 20 games ship genuinely different character art under the same
# element names. Originally produced by one-off Inkscape extraction scripts
# (since lost with the session scratchpad they lived in); the PNGs themselves
# are the source of truth now.
SEG_PNG_SRC = SCRIPT_DIR / "lcdonald_segments" / "base"
ESPN_SEG_PNG_SRC = SCRIPT_DIR / "lcdonald_segments" / "espn"
ESPN_REFS = {
    "SSPEEDWAY_ESPN", "TSOCCER_ESPN", "KBASEBALL_ESPN",
    "SSKATEBOARD_ESPN", "SBASKETBALL_ESPN", "ARTENNIS_ESPN",
}
GAME_ASSETS = REPO_ROOT / "reference" / "LCDonald-master" / "GameAssets"
# Burger Bard (LCDonald's `#if BURGER` white-label build) - see
# lcdonald_burger_table.py; segment PNGs come from extract_lcdonald_burger.py.
BURGER_ASSETS = REPO_ROOT / "reference" / "LCDonald-master" / "GameAssetsBurger"
BURGER_SEG_SRC = SCRIPT_DIR / "tmp" / "burger_segs"
BURGER_SCREEN_LONG_SIDE = 720  # px - the standard games' screen sizes run ~300-970 wide
OUT_CPP_DIR = REPO_ROOT / "source" / "std" / "LCDonald_ROM_rgds"
OUT_GFX_DIR = REPO_ROOT / "gfx2x_open_source"
DEFAULT_CONSOLE = str(SCRIPT_DIR / "rom" / "default_console.png")
# Per-entry menu thumbnails from generate_lcdonald_thumbnails.py (run that
# first) - tmp/lcd_thumbs/<ref lowercase>.png, already trimmed and padded to
# 4:3 so visual_console_data() never has to crop anything.
THUMB_DIR = SCRIPT_DIR / "tmp" / "lcd_thumbs"


def console_source(ref: str) -> str:
    p = THUMB_DIR / f"{ref.lower()}.png"
    if p.exists():
        return str(p)
    print(f"  WARNING: no generated thumbnail for {ref}, using default_console.png")
    return DEFAULT_CONSOLE

# Background alpha tuning for every LCDonald/Burger LCD print. The library
# default (alpha_bright 1.7 / fond_bright 1.35) turns any print that isn't
# dark into mostly-transparent pixels, which is fine for the flat MAME
# backgrounds it was tuned on but strips the detail out of these photographic
# prints (grass, courts, water...). Same fix the Nelsonic/Konami entries in
# games_path_rgds.py use (0.8 / 1.0): protects detail, at the cost of leaving
# little alpha for the Background Color setting to show through. A first
# pass - a few prints (sspeedway, abanana) are still fairly transparent and
# some others are near fully opaque, so per-game tuning is the next step.
LCD_ALPHA_BRIGHT = 0.8
LCD_FOND_BRIGHT = 1.0

profile = get_target("rgds")
csw.resolution_up = list(profile.resolution_up)
csw.resolution_down = list(profile.resolution_down)
csw.demi_resolution_up = list(profile.demi_resolution_up)
csw.console_size = list(profile.console_size)
csw.console_atlas_size = list(profile.console_atlas_size)
csw.size_altas_check = list(profile.atlas_sizes)
csw.background_atlas_sizes = list(profile.background_atlas_sizes)
csw.export_dpi = profile.export_dpi
csw.pad = 1
csw.texture_path_prefix = "gfx/"
csw.texture_path_ext = ".png"
csw.tex3ds_enabled = False
csw.destination_graphique_file = str(OUT_GFX_DIR) + os.sep

# (game_id, short_name, ref, display_name, date, width, height)
# width/height are each game's own <screen index="0"> bounds from its .lay
# (the native aspect ratio the segment SVG/background art was authored at -
# see the .lay parsing pass earlier this session), used directly as
# size_visual so nothing gets stretched relative to the source art.
#
# Names/dates sourced from McDonald's own press releases (2026-09 pass -
# see GNW_LIST.md's McDonald's section doc comment for the exact URLs and
# per-game notes): the 2003-06-23 SEGA Happy Meal launch (+ its 2003-06-23
# CORRECTION, which is where "Sonic's Speedway" and "Sonic Action Game"
# came from - both originally announced as "Sonic The Hedgehog ..."), the
# 2004-04-27 ESPN25 promotion, and the 2004-12-02 Sonic/"Search for the
# Silver Game" promotion. No year suffix on any display name any more (was
# "Title (YYYY[, US])" before this pass) - date precision lives in the
# `date` field now instead. Games that turned out to be clones of an
# earlier, differently-named game (ref, "Clone Of" in GNW_LIST.md) are
# still listed here as their own BASE_GAMES row - "clone" only changes
# which menu filter tier a ref falls under (switch_menu.cpp's
# kKnownCloneRefs) and its GNW_LIST.md documentation, not how its own
# assets/logic are generated.
BASE_GAMES = [
    (1, "sagame", "SAGAME", "Sonic Action Game", "2003-06-27", 786, 605),
    (2, "tskypatrol", "TSKYPATROL", "Tails Sky Patrol", "2003-06-27", 643, 500),
    (3, "ksoccer", "KSOCCER", "Knuckles Soccer", "2003-06-27", 498, 632),
    (4, "sgrinder", "SGRINDER", "Shadow Grinder", "2003-06-27", 719, 558),
    (5, "sspeedway", "SSPEEDWAY", "Sonic's Speedway", "2003-06-27", 690, 535),
    (6, "abanana", "ABANANA", "AiAi Banana Catch", "2003-06-27", 614, 714),
    (7, "sxtremeboard", "SXTREMEBOARD", "Sonic Extreme Boarding", "2004-12-03", 481, 361),
    # Clone of KSOCCER (same segments/logic, its own re-skin) - see GNW_LIST.md.
    (8, "tsoccer", "TSOCCER", "Tails Soccer", "2005-03-18", 579, 703),
    # Clone of KBASEBALL_ESPN (see VARIANTS below) - ESPN25: Michael Vick -
    # Football released 2004-04-30, nearly a year before this Sonic-skinned
    # version (2005-03-18), so the ESPN entry is the actual origin here,
    # not the other way around like every other ESPN variant in this file.
    (9, "kbaseball", "KBASEBALL", "Knuckles Baseball", "2005-03-18", 849, 640),
    (10, "shockey", "SHOCKEY", "Shadow Hockey", "2004-12-03", 523, 668),
    (11, "arvolleyball", "ARVOLLEYBALL", "Amy & Rouge Volleyball", "2004-12-03", 561, 718),
    # Clone of ABANANA. LCDonald's own "cflower"/"cflower2" 2004/2005
    # labeling is backwards (confirmed against McDonald's press releases) -
    # this is genuinely the 2005-03-18 release, cflower2 below is the real
    # 2004-12-03 one. Names/dates are swapped here to match reality; the
    # short_name/segment-map/game-logic mapping (which LCDonald folder is
    # which) is untouched, since that's tied to actual asset content, not
    # the date label - remember this swap when thumbnails (LCDonald's own
    # box art) get done, or the wrong box art will end up on the wrong date.
    (12, "cflower", "CFLOWER", "Cream Flower Catch", "2005-03-18", 324, 430),
    (13, "bfishing", "BFISHING", "Big's Fishing", "2004-12-03", 830, 639),
    # Clone of SXTREMEBOARD.
    (14, "sskateboard", "SSKATEBOARD", "Sonic Skateboard", "2005-03-18", 315, 397),
    (15, "tskyadventure", "TSKYADVENTURE", "Tails Sky Adventure", "2004-12-03", 300, 380),
    (16, "ktreasure", "KTREASURE", "Knuckles Treasure Hunt", "2004-12-03", 570, 716),
    # Clone of SHOCKEY.
    (17, "sbasketball", "SBASKETBALL", "Shadow Basketball", "2005-03-18", 536, 677),
    # Clone of ARVOLLEYBALL.
    (18, "artennis", "ARTENNIS", "Amy & Rouge Tennis", "2005-03-18", 474, 597),
    # Clone of ABANANA - see cflower's comment above for the 2004/2005 swap;
    # this is genuinely the 2004-12-03 release.
    (19, "cflower2", "CFLOWER2", "Cream Flower Catch", "2004-12-03", 444, 566),
    (20, "bgiantegg", "BGIANTEGG", "Billy's Giant Egg", "2004-12-03", 351, 436),
]

# (base_short_name, ref, display_name, date, bg_suffix)
# bg_suffix picks which of the base game's own alternate background prints
# to use ("" = plain _bg_crop, "_espn" = _bg_crop_espn) - purely cosmetic,
# same segment art/logic as the base game (see GNW_LIST.md's McDonald's
# section notes on why these are duplicates/clones, not separate ports).
# ESPN25 names/dates are from the 2004-04-27 press release (all 6 released
# 2004-04-30); International dates are left as a bare year (LCDonald has no
# more precise date for these, and none of the press releases sourced this
# pass call out international-specific release dates either).
VARIANTS = [
    ("ksoccer", "KSOCCER_INTL", "Knuckles Soccer (International)", "2003-01-01", ""),
    ("sgrinder", "SGRINDER_INTL", "Shadow Grinder (International)", "2003-01-01", ""),
    ("sspeedway", "SSPEEDWAY_INTL", "Sonic's Speedway (International)", "2003-01-01", ""),
    ("sspeedway", "SSPEEDWAY_ESPN", "ESPN25: Tony Stewart - Racing", "2004-04-30", "_espn"),
    ("sxtremeboard", "SXTREMEBOARD_INTL", "Sonic Extreme Boarding (International)", "2004-01-01", ""),
    ("tsoccer", "TSOCCER_ESPN", "ESPN25: Mia Hamm - Soccer", "2004-04-30", "_espn"),
    # Released 2004-04-30, ~11 months before Knuckles Baseball's own
    # 2005-03-18 - the actual origin of this pair, not a clone of it (see
    # BASE_GAMES' kbaseball entry above).
    ("kbaseball", "KBASEBALL_ESPN", "ESPN25: Michael Vick - Football", "2004-04-30", "_espn"),
    ("sskateboard", "SSKATEBOARD_ESPN", "ESPN25: Tony Hawk - Skateboarding", "2004-04-30", "_espn"),
    ("sbasketball", "SBASKETBALL_ESPN", "ESPN25: Vince Carter - Basketball", "2004-04-30", "_espn"),
    ("artennis", "ARTENNIS_ESPN", "ESPN25: Venus and Serena Williams - Tennis", "2004-04-30", "_espn"),
    # Box-art-only duplicate ("Front (Rare)"/"Back (Rare)" in shockey.lay,
    # shockey_rare_front.jpg/shockey_rare_back.jpg - no distinct segments or
    # background, same as an International entry) - the silver-body
    # unit from "Search for the Silver Game" (2004-12-02 press release: find
    # one of 25 silver premiums with a matching ticket to redeem for "an
    # ultimate gaming system"). Same release date as the base game.
    ("shockey", "SHOCKEY_RARE", "Shadow Hockey (Search for the Silver Game)", "2004-12-03", ""),
]

GAME_BY_SHORT = {g[1]: g for g in BASE_GAMES}


def cpp_name(short_name: str) -> str:
    return f"lcd_{short_name}"


def find_bg_path(short_name: str, suffix: str = "") -> str:
    d = GAME_ASSETS / short_name
    for ext in (".jpg", ".png"):
        p = d / f"{short_name}_bg_crop{suffix}{ext}"
        if p.exists():
            return str(p)
    if suffix:
        # ESPN variant art missing (shouldn't happen for the 6 listed) -
        # fall back to the plain crop rather than failing the whole run.
        for ext in (".jpg", ".png"):
            p = d / f"{short_name}_bg_crop{ext}"
            if p.exists():
                return str(p)
    # tskyadventure has no _bg_crop at all - use the uncropped _bg directly
    # (see GNW_LIST.md's variant-handling note).
    for ext in (".jpg", ".png"):
        p = d / f"{short_name}_bg{ext}"
        if p.exists():
            return str(p)
    raise FileNotFoundError(f"no background art found for {short_name}{suffix}")


def stage_segment_pngs(short_name: str, dest_name: str, src_root: Path = SEG_PNG_SRC):
    src = src_root / short_name
    dst = SCRIPT_DIR / "tmp" / "img" / dest_name
    if dst.exists():
        shutil.rmtree(dst)
    dst.mkdir(parents=True, exist_ok=True)
    n = 0
    for f in src.glob("*.png"):
        shutil.copy2(f, dst / f.name)
        n += 1
    if n == 0:
        raise RuntimeError(f"no segment PNGs found for {dest_name} in {src}")
    return n


def build_segment_atlas(name: str, width: int, height: int, bg_path: str):
    """Packs whatever's staged at ./tmp/img/<name>/ into a fresh segment
    atlas + Segment[] text - the shared step between a base game and an
    ESPN variant (which needs its own independent atlas, not the base
    game's, since its segment art genuinely differs - see ESPN_SEG_PNG_SRC)."""
    size_visual = [[width, height]]
    text, new_size_screen, seg_info_ints, _records = csw.visual_data_file(
        name, size_visual, [bg_path], False, False, False, False, [], False
    )
    size_background = [[int(s[0]), int(s[1])] for s in new_size_screen]
    bg_text, _background_info_ints = csw.background_data_file(
        name, [bg_path], size_background, alpha_bright=LCD_ALPHA_BRIGHT, fond_bright=LCD_FOND_BRIGHT
    )
    return text, bg_text, seg_info_ints


def generate_base_game(game_id, short_name, ref, display_name, date, width, height,
                       *, name=None, seg_src=SEG_PNG_SRC, bg_path=None, manufacturer="MCDONALDS"):
    """One self-contained GW_rom entry. The keyword args let the Burger Bard
    entries reuse this: their own cpp identifier (`name`), staged segment
    directory (`seg_src`), background file and manufacturer bucket."""
    name = name or cpp_name(short_name)
    print(f"[{game_id:2d}] {name} ({ref}) - staging segments...")
    n_segs = stage_segment_pngs(short_name, name, src_root=seg_src)

    bg_path = bg_path or find_bg_path(short_name)
    text, bg_text, seg_info_ints = build_segment_atlas(name, width, height, bg_path)

    cs_text, console_info_ints = csw.visual_console_data(name, console_source(ref))

    rom_bytes = bytes([0xFF, 0xFF, 0xFE, game_id])
    rom_path = SCRIPT_DIR / "tmp" / f"{name}_rom.bin"
    rom_path.write_bytes(rom_bytes)

    c_file = f"""
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "{name}.h"

"""
    c_file += csw.rom_text(name, str(rom_path))
    c_file += csw.melody_text(name, "")
    c_file += text
    c_file += bg_text
    c_file += cs_text
    c_file += "\n\n"
    c_file += f'''
const GW_rom {name} (
    "{display_name}", "{ref}", "{date}"
    , rom_GW_{name}, size_rom_GW_{name}
    , melody_GW_{name}, size_melody_GW_{name}
    , path_segment_{name}
    , segment_GW_{name}, size_segment_GW_{name}
    , segment_info_{name}
    , path_background_{name}
    , background_info_{name}
    , path_console_{name}
    , console_info_{name}
    , GW_rom::MANUFACTURER_{manufacturer}
);

'''
    (OUT_CPP_DIR / f"{name}.cpp").write_text(c_file, encoding="utf-8")
    # const globals at namespace scope have internal linkage by default in
    # C++ (unlike C) unless given external linkage by a prior extern
    # declaration seen before their definition - irrelevant for a game with
    # no variants (nothing outside its own .cpp ever names these), but the
    # games below DO have US/International/ESPN variant .cpp files that
    # reuse this same segment/background data via their own extern
    # declarations (see generate_variant()), which only resolve at link
    # time if these declarations - included by the .cpp above BEFORE the
    # matching definitions - already established external linkage here.
    h_file = f'\n#pragma once\n#include "GW_ROM.h"\n\nextern const GW_rom {name};\n'
    h_file += (
        f'extern const std::string path_segment_{name};\n'
        f'extern const Segment segment_GW_{name}[];\n'
        f'extern const size_t size_segment_GW_{name};\n'
        f'extern const uint16_t segment_info_{name}[];\n'
        f'extern const std::string path_background_{name};\n'
        f'extern const uint16_t background_info_{name}[];\n\n'
    )
    (OUT_CPP_DIR / f"{name}.h").write_text(h_file, encoding="utf-8")
    rom_path.unlink()
    print(f"      {n_segs} segments, screen {width}x{height}, "
          f"atlas {seg_info_ints[0]}x{seg_info_ints[1]}, bg={os.path.basename(bg_path)}")
    return name


def generate_variant(base_short, ref, display_name, date, bg_suffix):
    game_id, base_short_name, _base_ref, _base_display, _base_date, width, height = GAME_BY_SHORT[base_short]
    base_name = cpp_name(base_short_name)
    variant_short = ref.lower()
    name = cpp_name(variant_short)

    rom_bytes = bytes([0xFF, 0xFF, 0xFE, game_id])
    rom_path = SCRIPT_DIR / "tmp" / f"{name}_rom.bin"
    rom_path.write_bytes(rom_bytes)

    is_espn = ref in ESPN_REFS
    extra_includes = ""

    if is_espn:
        # ESPN variants have their own segment SVG (<game>_1.svg - see
        # ESPN_SEG_PNG_SRC's doc comment above), not just a different
        # background print - pack a fully independent atlas instead of
        # reusing the base game's.
        print(f"[variant] {name} ({ref}) -> own ESPN segment art + background (game_id={game_id})")
        n_segs = stage_segment_pngs(base_short_name, name, src_root=ESPN_SEG_PNG_SRC)
        bg_path = find_bg_path(base_short_name, bg_suffix)
        seg_text, bg_text, seg_info_ints = build_segment_atlas(name, width, height, bg_path)
        print(f"      {n_segs} ESPN segments, atlas {seg_info_ints[0]}x{seg_info_ints[1]}")
        seg_path_expr = f"path_segment_{name}"
        seg_array_expr = f"segment_GW_{name}, size_segment_GW_{name}"
        seg_info_expr = f"segment_info_{name}"
        bg_path_expr = f"path_background_{name}"
        bg_info_expr = f"background_info_{name}"
    else:
        print(f"[variant] {name} ({ref}) -> reuses {base_name}'s art/logic (game_id={game_id})")
        seg_text = ""
        extra_includes = (
            f'extern const std::string path_segment_{base_name};\n'
            f'extern const Segment segment_GW_{base_name}[];\n'
            f'extern const size_t size_segment_GW_{base_name};\n'
            f'extern const uint16_t segment_info_{base_name}[];\n'
        )
        seg_path_expr = f"path_segment_{base_name}"
        seg_array_expr = f"segment_GW_{base_name}, size_segment_GW_{base_name}"
        seg_info_expr = f"segment_info_{base_name}"

        if bg_suffix:
            # International variants have no bg_suffix and hit the else
            # branch below; this path is unused today (every non-ESPN
            # variant is International, bg_suffix="") but kept for a future
            # non-ESPN variant that does ship its own background.
            bg_path = find_bg_path(base_short_name, bg_suffix)
            size_background = [[width, height]]
            bg_text, _bg_info = csw.background_data_file(
                name, [bg_path], size_background, alpha_bright=LCD_ALPHA_BRIGHT, fond_bright=LCD_FOND_BRIGHT
            )
            bg_path_expr = f"path_background_{name}"
            bg_info_expr = f"background_info_{name}"
        else:
            # No distinct print for this variant - reuse the base game's own
            # background symbols directly rather than repacking an identical atlas.
            bg_text = ""
            extra_includes += (
                f'extern const std::string path_background_{base_name};\n'
                f'extern const uint16_t background_info_{base_name}[];\n'
            )
            bg_path_expr = f"path_background_{base_name}"
            bg_info_expr = f"background_info_{base_name}"

    cs_text, _console_info_ints = csw.visual_console_data(name, console_source(ref))

    c_file = f"""
#include <cstdint>
#include <string>
#include <vector>

#include "segment.h"
#include "GW_ROM.h"
#include "{base_name}.h"
#include "{name}.h"

{extra_includes}
"""
    c_file += csw.rom_text(name, str(rom_path))
    c_file += csw.melody_text(name, "")
    c_file += seg_text
    c_file += bg_text
    c_file += cs_text
    c_file += "\n\n"
    c_file += f'''
const GW_rom {name} (
    "{display_name}", "{ref}", "{date}"
    , rom_GW_{name}, size_rom_GW_{name}
    , melody_GW_{name}, size_melody_GW_{name}
    , {seg_path_expr}
    , {seg_array_expr}
    , {seg_info_expr}
    , {bg_path_expr}
    , {bg_info_expr}
    , path_console_{name}
    , console_info_{name}
    , GW_rom::MANUFACTURER_MCDONALDS
);

'''
    (OUT_CPP_DIR / f"{name}.cpp").write_text(c_file, encoding="utf-8")
    (OUT_CPP_DIR / f"{name}.h").write_text(
        f'\n#pragma once\n#include "GW_ROM.h"\n\nextern const GW_rom {name};\n\n',
        encoding="utf-8",
    )
    rom_path.unlink()
    return name


def burger_screen_size(folder: str):
    """(width, height) for a Burger entry's segment/background atlas: the
    aspect ratio of its <screen> in the .lay's "Closeup" view (the LCD at its
    native shape - matches the SVG and background art exactly), scaled to a
    long side comparable to the McDonald's games' own screen sizes."""
    import xml.etree.ElementTree as ET

    root = ET.parse(BURGER_ASSETS / folder / f"{folder}.lay").getroot()
    closeup = next(v for v in root.findall("view") if v.get("name") == "Closeup")
    b = closeup.find("screen").find("bounds").attrib
    w, h = float(b["width"]), float(b["height"])
    scale = BURGER_SCREEN_LONG_SIDE / max(w, h)
    return int(round(w * scale)), int(round(h * scale))


def burger_bg_path(folder: str) -> str:
    """The LCD background print, named by the .lay's game_bg element (a few
    folders reuse another folder's file name - e.g. kosoccer's ksoccer_bg.jpg,
    treasure2's treasure_bg.jpg)."""
    import xml.etree.ElementTree as ET

    root = ET.parse(BURGER_ASSETS / folder / f"{folder}.lay").getroot()
    for e in root.findall("element"):
        if e.get("name") == "game_bg":
            return str(BURGER_ASSETS / folder / e.find("image").get("file"))
    raise FileNotFoundError(f"no game_bg element in {folder}.lay")


def generate_burger_game(game_id, folder, _cls, ref, display_name, _base_short, _clone_of):
    width, height = burger_screen_size(folder)
    return generate_base_game(
        game_id, folder, ref, display_name, BURGER_DATE, width, height,
        name=burger_cpp_name(folder), seg_src=BURGER_SEG_SRC, bg_path=burger_bg_path(folder),
        manufacturer="BURGER_BARD",
    )


def bundle_burger_sounds():
    """Burger's sound files are byte-identical to their base McDonald's game's
    (already bundled as sound_lcd_<base>_*/sound_lcd_common_*), so only clips
    that differ - or that the base doesn't have - are added, as
    "sound_lcd_burger_<folder>_<file>" (preferred by lcdonald_sound.cpp when
    present). At time of writing that's just eggcatch's egg_hatch.ogg."""
    import filecmp

    n = 0
    for _id, folder, _cls, _ref, _name, base_short, _clone in BURGER_GAMES:
        for f in sorted((BURGER_ASSETS / folder).glob("*.ogg")):
            base = GAME_ASSETS / base_short / f.name
            if base.exists() and filecmp.cmp(f, base, shallow=False):
                continue
            shutil.copy2(f, OUT_GFX_DIR / f"sound_lcd_burger_{folder}_{f.name}")
            n += 1
            print(f"  Burger override sound: {folder}/{f.name}")
    print(f"Bundled {n} Burger-specific .ogg override(s)")


def bundle_sounds():
    """Copies every game's own .ogg sound effects, plus the shared common/
    set, into gfx2x_open_source/ under the naming convention
    lcdonald_sound.cpp's resolve_pack_name() expects: "sound_lcd_common_
    <file>" for "../common/<file>" cues, "sound_lcd_<short_name>_<file>" for
    a game's own bare "<file>" cues (namespaced since several games reuse
    the same bare filename - e.g. "hit.ogg" - for a completely different
    clip). Variant refs (lcd_sspeedway_espn etc.) need no entries of their
    own here: they run the same LCDonald_program subclass as their base
    game, so short_name() - and therefore sound resolution - is identical.
    """
    n = 0
    common_dir = GAME_ASSETS / "common"
    for f in sorted(common_dir.glob("*.ogg")):
        shutil.copy2(f, OUT_GFX_DIR / f"sound_lcd_common_{f.name}")
        n += 1

    for _game_id, short_name, *_rest in BASE_GAMES:
        game_dir = GAME_ASSETS / short_name
        for f in sorted(game_dir.glob("*.ogg")):
            shutil.copy2(f, OUT_GFX_DIR / f"sound_lcd_{short_name}_{f.name}")
            n += 1

    print(f"\nBundled {n} .ogg sound file(s) into {OUT_GFX_DIR}")


def main():
    OUT_CPP_DIR.mkdir(parents=True, exist_ok=True)
    OUT_GFX_DIR.mkdir(parents=True, exist_ok=True)
    (SCRIPT_DIR / "tmp" / "img").mkdir(parents=True, exist_ok=True)

    all_names = []
    for game in BASE_GAMES:
        all_names.append(generate_base_game(*game))
    for variant in VARIANTS:
        all_names.append(generate_variant(*variant))
    for burger_game in BURGER_GAMES:
        all_names.append(generate_burger_game(*burger_game))

    bundle_sounds()
    bundle_burger_sounds()

    print(f"\nDone: {len(all_names)} GW_rom entries written to {OUT_CPP_DIR}")
    print("cpp identifiers:")
    for n in all_names:
        print(f"  {n}")


if __name__ == "__main__":
    main()
