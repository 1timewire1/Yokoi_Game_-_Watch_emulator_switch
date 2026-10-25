"""Splits a BrickEmuPy face SVG (github.com/azya52/BrickEmuPy, CC0) into
the pieces convert_switch.py wants, for games on a core we already have.

A BrickEmuPy face is one SVG of the whole device:
  <g id="body">     housing, buttons, printed inlay ("layout"), screen
                    frame and the LCD glass ("reflector")
  <g id="display">  every LCD segment, id "<VRAM byte>_<bit>" (possibly
                    nested in untitled "col_N" grouping), drawn black
This writes, into <out_dir>:
  <name>_segs.svg   just the segments, cropped to the LCD glass, each titled
                    with our segment id, under an Inkscape "segs" layer
  Background.png    everything else, cropped to the same rectangle - the
                    printed inlay/LCD tint the segments sit on. Not written
                    (and any old one removed) when that's just one flat
                    colour (Alien Fever, Venus Diet, Stack Challenge): those
                    games are left to the app's Background Color setting,
                    like the other games without background art.
  <name>_face.png   (--face) the whole device with the display hidden, for
                    the menu thumbnail (see --thumb in prepare scripts)
and prints the screen size to use as the game's transform_visual.

Segment ids (E0C6200 only so far): BrickEmuPy numbers segments by VRAM
nibble - byte b (0x50 per bank) bit c - which is the same COM/SEG pin the
E0C6S46 LCD driver lights (e0c6s46_device::lcd_driver_cb):
  bank, off = divmod(b, 0x50); COM = bank*8 + (off&1)*4 + c; SEG = off // 2
giving our "COM.SEG.0" (see source/E0C6200/E0C6S46.h). Verified against
MAME's tama/digimon/tamaang SVGs, which use the same id scheme next to
their COM.SEG titles.

Usage:
  python split_brick_svg.py <face.svg> <out_dir> <name> [--height 1080] [--face] [--pad 0.03]
"""
import argparse
import copy
import re
import subprocess
import tempfile
from pathlib import Path

from lxml import etree

SVG_NS = "http://www.w3.org/2000/svg"
INK_NS = "http://www.inkscape.org/namespaces/inkscape"
INKSCAPE = r"C:\Program Files\Inkscape\bin\inkscape.exe"
SEG_ID = re.compile(r"(\d+)_(\d)")
E0C6200_BANK = 0x50
GLASS_IDS = ("reflector", "background", "screenFrame")


def e0c6200_segment_title(byte: int, bit: int) -> str:
    bank, off = divmod(byte, E0C6200_BANK)
    return f"{bank * 8 + (off & 1) * 4 + bit}.{off // 2}.0"


def _by_id(root, ident):
    return next((e for e in root.iter() if e.get("id") == ident), None)


def _query_bboxes(svg_path: Path) -> dict:
    """Visual bounding boxes {id: (x, y, w, h)} of every element, via Inkscape."""
    out = subprocess.run([INKSCAPE, "--query-all", str(svg_path)], capture_output=True, text=True, check=True).stdout
    boxes = {}
    for line in out.splitlines():
        parts = line.split(",")
        if len(parts) == 5:
            boxes[parts[0]] = tuple(float(v) for v in parts[1:5])
    return boxes


def screen_rect(svg_path: Path, root, pad: float = 0.03) -> tuple:
    """The visible LCD: the segments' bounding box plus `pad` (fraction of
    its size) on each side, clipped to the LCD glass (GLASS_IDS, minus
    its stroke - the stroke is the bezel's bevel).

    The glass alone isn't enough: on some faces (Digital Monster) it's a
    group of backdrop art that runs under the bezel, which the body then
    covers with its frame and corner clips - cropping to the glass picks
    those up. The segments always sit inside the visible window.
    """
    boxes = _query_bboxes(svg_path)
    if "display" not in boxes:
        raise SystemExit(f"{svg_path.name}: no 'display' element")
    dx, dy, dw, dh = boxes["display"]
    x0, y0, x1, y1 = dx - dw * pad, dy - dh * pad, dx + dw * (1 + pad), dy + dh * (1 + pad)
    # The glass isn't named consistently across faces (TamagotchiAngel.svg
    # calls it "background"); with none at all, the padded box stands alone.
    glass = next((g for g in GLASS_IDS if g in boxes), None)
    if glass is None:
        return x0, y0, x1 - x0, y1 - y0
    rx, ry, rw, rh = boxes[glass]
    inset = float(_by_id(root, glass).get("stroke-width", 0) or 0) / 2
    rx0, ry0, rx1, ry1 = rx + inset, ry + inset, rx + rw - inset, ry + rh - inset
    x0, y0, x1, y1 = max(x0, rx0), max(y0, ry0), min(x1, rx1), min(y1, ry1)
    return x0, y0, x1 - x0, y1 - y0


def segments_svg(root, rect) -> tuple[bytes, int]:
    x, y, w, h = rect
    out = etree.Element(f"{{{SVG_NS}}}svg", nsmap={None: SVG_NS, "inkscape": INK_NS})
    out.set("width", f"{w:g}")
    out.set("height", f"{h:g}")
    out.set("viewBox", f"{x:g} {y:g} {w:g} {h:g}")
    defs = next((e for e in root if isinstance(e.tag, str) and etree.QName(e).localname == "defs"), None)
    if defs is not None:
        out.append(copy.deepcopy(defs))
    layer = etree.SubElement(out, f"{{{SVG_NS}}}g")
    layer.set("id", "layer_segs")
    layer.set(f"{{{INK_NS}}}label", "segs")
    layer.set(f"{{{INK_NS}}}groupmode", "layer")

    display = _by_id(root, "display")
    count = 0
    seen = set()
    for el in display.iter():
        m = SEG_ID.fullmatch(el.get("id") or "")
        if not m:
            continue
        title = e0c6200_segment_title(int(m[1]), int(m[2]))
        if title in seen:
            raise SystemExit(f"duplicate segment {title}")
        seen.add(title)
        seg = copy.deepcopy(el)
        # Flattened straight under the layer - the "col_N" wrappers carry no
        # transform/style (checked), so nothing is lost by dropping them.
        etree.SubElement(seg, f"{{{SVG_NS}}}title").text = title
        layer.append(seg)
        count += 1
    return etree.tostring(out, xml_declaration=True, encoding="UTF-8"), count


def _render(root, out_png: Path, area=None, width=None, height=None):
    with tempfile.TemporaryDirectory() as tmp:
        svg = Path(tmp) / "r.svg"
        svg.write_bytes(etree.tostring(root))
        args = [INKSCAPE, str(svg), "--export-type=png", f"--export-filename={out_png}"]
        if area:
            x, y, w, h = area
            args.append(f"--export-area={x:g}:{y:g}:{x + w:g}:{y + h:g}")
        if width:
            args.append(f"--export-width={width}")
        if height:
            args.append(f"--export-height={height}")
        subprocess.run(args, check=True, capture_output=True)


def without_display(root):
    r = copy.deepcopy(root)
    d = _by_id(r, "display")
    d.getparent().remove(d)
    return r


def is_flat(png: Path, tolerance: int = 12, share: float = 0.98) -> bool:
    """True if nearly all of the image (ignoring a 5% border, where the
    glass's bevel shows) is within `tolerance` of its median colour."""
    import numpy as np
    from PIL import Image

    img = np.asarray(Image.open(png).convert("RGB")).astype(int)
    h, w = img.shape[:2]
    inner = img[int(h * 0.05):int(h * 0.95), int(w * 0.05):int(w * 0.95)].reshape(-1, 3)
    dev = np.abs(inner - np.median(inner, axis=0)).sum(axis=1)
    return (dev < tolerance).mean() >= share


def split(face: Path, out_dir: Path, name: str, height: int = 1080, face_png: bool = False,
          pad: float = 0.03) -> tuple:
    out_dir.mkdir(parents=True, exist_ok=True)
    root = etree.parse(str(face)).getroot()
    rect = screen_rect(face, root, pad)
    svg, count = segments_svg(root, rect)
    (out_dir / f"{name}_segs.svg").write_bytes(svg)

    width = round(rect[2] / rect[3] * height)
    background = out_dir / "Background.png"
    _render(without_display(root), background, area=rect, width=width, height=height)
    flat = is_flat(background)
    if flat:
        background.unlink()
    if face_png:
        _render(without_display(root), out_dir / f"{name}_face.png", height=1026)
    print(f"{name}: {count} segments, screen {rect[2]:.0f}x{rect[3]:.0f} at ({rect[0]:.0f},{rect[1]:.0f}) "
          f"-> transform_visual [[[{width},0,0],[{height},0,0]]]" + (" (flat: no background)" if flat else ""))
    return width, height


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("face", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("name")
    ap.add_argument("--height", type=int, default=1080)
    ap.add_argument("--face", dest="face_png", action="store_true")
    ap.add_argument("--pad", type=float, default=0.03, help="margin around the segments, as a fraction")
    a = ap.parse_args()
    split(a.face, a.out_dir, a.name, a.height, a.face_png, a.pad)
