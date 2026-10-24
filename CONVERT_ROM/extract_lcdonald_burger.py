"""Extracts every Burger Bard game's segment art (one PNG per named SVG group)
and emits the C++ segment maps + game-selection glue that use it.

Same approach the McDonald's games used (look each element the game's logic
can light up directly by id in the SVG, clone it with its ancestor transform
chain into a one-element SVG, rasterise with Inkscape) - see
generate_lcdonald_pack.py's header for the whole pipeline. Differences here:

  * The wanted element lists come from CONVERT_ROM/lcdonald_burger_elements.json
    (built by lcdonald_dump_elements.cpp from the actual ported logic, in
    Burger mode, plus Burger's "win"/"score-1" extras), not hand-written.
  * Burger's SVGs are authored at a huge canvas (5000-10000 px), so each
    segment is rasterised to a fixed working width/height (LONG_SIDE) with
    Inkscape's --export-width/--export-height instead of a DPI - the atlas
    packer resizes it to the game's screen size afterwards anyway.

Outputs:
  tmp/burger_segs/<folder>/<col>.<line>.<word>.0.png      staged segment PNGs
  source/LCDonald/SegmentMaps/burger_<folder>_segment_map.h
  source/LCDonald/BurgerSegmentMaps.h      (includes every map above)
  source/LCDonald/BurgerSelectGame.inc     (switch cases for select_game())

Run from CONVERT_ROM/:  C:/Python314/python.exe extract_lcdonald_burger.py [folder ...]
"""
import json
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from lxml import etree

from lcdonald_burger_table import BURGER_GAMES

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
BURGER_ASSETS = REPO_ROOT / "reference" / "LCDonald-master" / "GameAssetsBurger"
OUT_SEGS = SCRIPT_DIR / "tmp" / "burger_segs"
LCD_SRC = REPO_ROOT / "source" / "LCDonald"
INKSCAPE_PATH = r"C:\Program Files\Inkscape\bin\inkscape.exe"
SVG_NS = "http://www.w3.org/2000/svg"
NS = {"svg": SVG_NS}
LONG_SIDE = 1000  # px, working resolution of each exported segment
WORKERS = 6


def ancestors_of(element, root):
    chain = []
    node = element.getparent()
    while node is not None and node is not root:
        if etree.QName(node).localname == "g":
            chain.append(node)
        node = node.getparent()
    chain.reverse()
    return chain


def extract_segments(svg_path: Path, out_dir: Path, wanted, screen_index=0):
    out_dir.mkdir(parents=True, exist_ok=True)
    root = etree.parse(str(svg_path)).getroot()
    found, missing = [], []
    next_id = 0
    for name in wanted:
        element = root.find(f'.//*[@id="{name}"]', NS)
        if element is None:
            missing.append(name)
            continue
        g_chain = ancestors_of(element, root)
        col, line, word = (next_id // 64) % 8, (next_id // 4) % 16, next_id % 4
        next_id += 1
        base = f"{col}.{line}.{word}.{screen_index}"

        new_svg = etree.Element(root.tag, nsmap=root.nsmap)
        for attr in ("width", "height", "viewBox"):
            if attr in root.attrib:
                new_svg.attrib[attr] = root.attrib[attr]
        parent = new_svg
        for g in g_chain:
            wrapper = etree.SubElement(parent, f"{{{SVG_NS}}}g")
            if "transform" in g.attrib:
                wrapper.attrib["transform"] = g.attrib["transform"]
            parent = wrapper
        clone = etree.SubElement(parent, element.tag)
        clone.attrib.update(element.attrib)
        for child in element:
            clone.append(child)
        svg_tmp = out_dir / f"{base}.svg"
        svg_tmp.write_bytes(etree.tostring(new_svg, pretty_print=True))
        found.append((name, col, line, word, svg_tmp, out_dir / f"{base}.png"))
    return found, missing


def page_size(svg_path: Path):
    root = etree.parse(str(svg_path)).getroot()
    num = lambda v: float(re.match(r"[\d.]+", v).group(0))  # tolerate unit suffixes (px/mm/...)
    return num(root.attrib["width"]), num(root.attrib["height"])


def export_one(args):
    seg_name, svg_path, png_path, flag, size = args
    cmd = [INKSCAPE_PATH, str(svg_path), f"--export-filename={png_path}", f"{flag}={size}"]
    # Parallel Inkscape instances occasionally collide on their D-Bus session
    # ("Gio::DBus::Error") - a plain retry always succeeds.
    for attempt in range(4):
        r = subprocess.run(cmd, capture_output=True, encoding="utf-8", errors="replace")
        if r.returncode == 0 and png_path.exists():
            svg_path.unlink()  # only the PNGs are staged
            return True
    print(f"  FAILED to export {seg_name}: {r.stderr[-200:]}")
    return False


def process(folder: str, wanted):
    svg = BURGER_ASSETS / folder / f"{folder}_0.svg"
    out_dir = OUT_SEGS / folder
    if out_dir.exists():
        shutil.rmtree(out_dir)
    found, missing = extract_segments(svg, out_dir, wanted)
    w, h = page_size(svg)
    flag, size = ("--export-width", LONG_SIDE) if w >= h else ("--export-height", LONG_SIDE)
    jobs = [(n, s, p, flag, size) for (n, _c, _l, _w, s, p) in found]
    with ThreadPoolExecutor(WORKERS) as ex:
        ok = sum(ex.map(export_one, jobs))
    print(f"{folder:15s} wanted={len(wanted)} found={len(found)} exported={ok} missing={missing}")
    return found, missing


def emit_map_header(folder: str, found):
    ident = "Burger" + folder[0].upper() + folder[1:]
    lines = [
        f"// Auto-generated by extract_lcdonald_burger.py from {folder}_0.svg - do not hand-edit.",
        "#pragma once",
        '#include "../LCDonald_fake_cpu.h"',
        "",
        f"static const LCDonaldSegmentMapEntry k{ident}SegmentMap[] = {{",
    ]
    for name, col, line, word, _s, _p in found:
        lines.append(f'    {{{col}, {line}, {word}, "{name}"}},')
    lines.append("};")
    lines.append(f"constexpr size_t k{ident}SegmentMapSize = sizeof(k{ident}SegmentMap) / sizeof(k{ident}SegmentMap[0]);")
    (LCD_SRC / "SegmentMaps" / f"burger_{folder}_segment_map.h").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return ident


def main():
    only = set(sys.argv[1:])
    elements = json.loads((SCRIPT_DIR / "lcdonald_burger_elements.json").read_text(encoding="utf-8"))
    idents = {}
    for game_id, folder, *_ in BURGER_GAMES:
        if only and folder not in only:
            # Still need the identifier for the glue files below.
            idents[folder] = "Burger" + folder[0].upper() + folder[1:]
            continue
        found, _missing = process(folder, elements[folder])
        idents[folder] = emit_map_header(folder, found)

    includes = ["// Auto-generated by extract_lcdonald_burger.py - do not hand-edit.", "#pragma once"]
    cases = ["// Auto-generated by extract_lcdonald_burger.py - do not hand-edit.",
             "// Burger Bard game ids (see lcdonald_burger_table.py); #included inside select_game()'s switch."]
    for game_id, folder, cls, *_ in BURGER_GAMES:
        ident = idents[folder]
        includes.append(f'#include "SegmentMaps/burger_{folder}_segment_map.h"')
        cases += [
            f"case {game_id}:",
            f"    program_ = std::make_unique<{cls}>();",
            f'    program_->set_burger("{folder}");',
            f"    segment_map_ = k{ident}SegmentMap;",
            f"    segment_map_size_ = k{ident}SegmentMapSize;",
            "    break;",
        ]
    (LCD_SRC / "BurgerSegmentMaps.h").write_text("\n".join(includes) + "\n", encoding="utf-8")
    (LCD_SRC / "BurgerSelectGame.inc").write_text("\n".join(cases) + "\n", encoding="utf-8")
    print("wrote BurgerSegmentMaps.h / BurgerSelectGame.inc")


if __name__ == "__main__":
    main()
