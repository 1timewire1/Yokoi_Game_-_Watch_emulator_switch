"""Prepares a raw MAME segment SVG for convert_switch.py.

source/convert_svg.py's extract_group_segs() looks for the segments inside an
Inkscape layer labelled "segs" (the shape of the reworked SVGs most games here
use). Some MAME SVGs - e.g. the Tiger R-Zone ones (rzbatfor/rztoshden/rzindy500)
- instead put every titled segment group straight under the root. This moves
each top-level element that carries a <title> (the col.line.word segment id)
into a new "segs" layer and writes <name>_segs.svg next to the original, which
the game's games_path "Visual" entry then points at.

Usage:  python wrap_svg_segs.py <game.svg> [<game.svg> ...]
"""
import sys
from pathlib import Path

from lxml import etree

SVG = "http://www.w3.org/2000/svg"
INK = "http://www.inkscape.org/namespaces/inkscape"


def wrap(path: Path) -> Path:
    tree = etree.parse(str(path))
    root = tree.getroot()
    segs = etree.Element(f"{{{SVG}}}g", nsmap=root.nsmap)
    segs.set("id", "layer_segs")
    segs.set(f"{{{INK}}}label", "segs")
    segs.set(f"{{{INK}}}groupmode", "layer")
    moved = 0
    for child in list(root):
        if child.find(f"{{{SVG}}}title") is not None:
            root.remove(child)
            segs.append(child)
            moved += 1
    if moved == 0:
        raise SystemExit(f"{path}: no top-level titled segments found (already wrapped?)")
    root.append(segs)
    out = path.with_name(path.stem + "_segs.svg")
    tree.write(str(out), xml_declaration=True, encoding="UTF-8")
    print(f"{path.name}: {moved} segments -> {out.name}")
    return out


if __name__ == "__main__":
    for arg in sys.argv[1:]:
        wrap(Path(arg))
