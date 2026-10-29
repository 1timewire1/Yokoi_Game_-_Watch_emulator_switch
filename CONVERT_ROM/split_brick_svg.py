"""Splits a BrickEmuPy face SVG (github.com/azya52/BrickEmuPy, CC0) into the
pieces convert_switch.py wants, for any core.

A BrickEmuPy face is one SVG of the whole device. BrickEmuPy itself draws
just three things from it (brick_widget.py's _draw): the "body" element
(housing, buttons, printed inlay, screen frame, the LCD glass), every
segment - any element whose id is "<display byte>_<bit>" (0-255, 0-7), an
index into the core's display memory - and, on top, an optional "overlay"
element (glare, frame shadows, printed parts that sit over the LCD).
Anything else in the file (a stray page rect, say) is never drawn.

This writes, into <out_dir>:
  <name>_segs.svg   just the segments, cropped to the visible LCD, each titled
                    with our segment id, under an Inkscape "segs" layer
  Background.png    "body" only, cropped to the same rectangle - the printed
                    inlay/LCD tint the segments sit on. Not written (and any
                    old one removed) when that's one flat colour: those games
                    are left to the app's Background Color setting, like the
                    other games without background art.
  Overlay.png       "overlay" only (transparent elsewhere), same rectangle -
                    drawn over the segments (convert_switch.py's 'Overlay',
                    with 'overlay_alpha': 100 since the art carries its own
                    transparency). Removed if the face has no overlay.
  <name>_face.png   (--face) body + overlay, display hidden, full device.
and prints the screen size to use as the game's transform_visual.

Devices with more than one LCD (MULTI_SCREEN_FACES: Animest Space Cobra
side by side, Spica D'Artagnan stacked) are split per screen, at the gaps
between segment clusters along that axis, and screens after the first get a
number: <name>_segs2.svg, Background2.png, Overlay2.png. split() reports the
arrangement ("horizontal" maps to the app's 2_in_one_screen, "vertical" to
its two stacked panels). This is a list, not detection: gap size can't tell
a second LCD from a layout gap (Space Cobra's screens are 7.8% of the
segment extent apart; the single-LCD Digimon V2/V3 faces have 8.3% between
icons and dot matrix, Yasashii 21% - tmp/brick_gap_survey.py).

Segment id schemes (the pack's (col, line, word) triple):
  brick    (byte, bit, 0) - BrickEmuPy's own addressing; the core exposes the
           same display-memory array BrickEmuPy's get_VRAM() returns. Default.
  e0c6200  (COM, SEG, 0) - the E0C6S46 LCD driver's pins, which is how
           source/E0C6200/ (ported from MAME) addresses its LCD: byte b (0x50
           per bank) bit c -> bank, off = divmod(b, 0x50); COM = bank*8 +
           (off&1)*4 + c; SEG = off // 2. Verified against MAME's tama/
           digimon/tamaang SVGs, which use the same ids next to COM.SEG titles.

Usage:
  python split_brick_svg.py <face.svg> <out_dir> <name> [--scheme brick|e0c6200]
                            [--height 1080] [--face] [--pad 0.03]
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
# What each face calls its LCD glass, in preference order (TamagotchiAngel.svg
# says "background"); with none, the padded segment box stands alone.
GLASS_IDS = ("reflector", "background", "screenFrame")
DRAWN_IDS = ("body", "display", "overlay")
# Faces with more than one LCD, by file stem -> arrangement (see the module doc).
MULTI_SCREEN_FACES = {"AnimestSpaceCobra": "horizontal", "SpicaDArtagnan": "vertical"}


def e0c6200_segment_title(byte: int, bit: int) -> str:
    bank, off = divmod(byte, E0C6200_BANK)
    return f"{bank * 8 + (off & 1) * 4 + bit}.{off // 2}.0"


SCHEMES = {
    "brick": lambda byte, bit: f"{byte}.{bit}.0",
    "e0c6200": e0c6200_segment_title,
}


def load_face(path: Path):
    """Parses a face. Some use xlink:href without declaring the namespace
    (MiconKC32.svg) - Qt's renderer shrugs that off, lxml and Inkscape don't."""
    text = Path(path).read_bytes()
    if b"xlink:" in text and b"xmlns:xlink" not in text:
        text = text.replace(b"<svg", b'<svg xmlns:xlink="http://www.w3.org/1999/xlink"', 1)
    return etree.fromstring(text, etree.XMLParser(huge_tree=True))


def segment_elements(root):
    """(element, byte, bit) for every segment, found by id anywhere in the
    document - the same lookup BrickEmuPy does."""
    out = []
    for el in root.iter():
        if not isinstance(el.tag, str):
            continue
        m = SEG_ID.fullmatch(el.get("id") or "")
        if m and int(m[1]) < 256 and int(m[2]) < 8:
            out.append((el, int(m[1]), int(m[2])))
    return out


def _by_id(root, ident):
    return next((e for e in root.iter() if e.get("id") == ident), None)


def _keep_only(root, ids):
    """A copy with only <defs> and the given top-level elements."""
    r = copy.deepcopy(root)
    for child in list(r):
        if not isinstance(child.tag, str):
            continue
        if etree.QName(child).localname != "defs" and child.get("id") not in ids:
            r.remove(child)
    return r


def _query_bboxes(root) -> dict:
    """Visual bounding boxes {id: (x, y, w, h)} of every element, via Inkscape."""
    with tempfile.TemporaryDirectory() as tmp:
        svg = Path(tmp) / "q.svg"
        svg.write_bytes(etree.tostring(root))
        out = subprocess.run([INKSCAPE, "--query-all", str(svg)], capture_output=True, text=True, check=True).stdout
    boxes = {}
    for line in out.splitlines():
        parts = line.split(",")
        if len(parts) == 5:
            boxes[parts[0]] = tuple(float(v) for v in parts[1:5])
    return boxes


def _groups(intervals, gap):
    """Merges (lo, hi, key) intervals closer than `gap` into groups of keys."""
    groups = []
    for lo, hi, key in sorted(intervals):
        if groups and lo <= groups[-1][1] + gap:
            groups[-1][1] = max(groups[-1][1], hi)
            groups[-1][2].append(key)
        else:
            groups.append([lo, hi, [key]])
    return [g[2] for g in groups]


def screen_rects(root, pad: float = 0.03, arrangement: str = "single"):
    """The visible LCD(s): returns (arrangement, [(rect, member keys), ...]).

    Each screen is its segments' bounding box plus `pad` (fraction of its
    size) on each side, clipped to the LCD glass (GLASS_IDS, minus its
    stroke - the stroke is the bezel's bevel). The glass alone isn't enough:
    on some faces (Digital Monster) it's a group of backdrop art that runs
    under the bezel, which the body then covers with its frame and corner
    clips - cropping to the glass picks those up. The segments always sit
    inside the visible window.

    arrangement "horizontal"/"vertical": split into screens wherever the
    segments leave a gap of over 4% of the overall extent along that axis.
    """
    boxes = _query_bboxes(root)
    segs = {(b, c): boxes[el.get("id")] for el, b, c in segment_elements(root) if el.get("id") in boxes}
    if not segs:
        raise SystemExit("face has no segments")
    ux0 = min(x for x, _, _, _ in segs.values())
    uy0 = min(y for _, y, _, _ in segs.values())
    ux1 = max(x + w for x, _, w, _ in segs.values())
    uy1 = max(y + h for _, y, _, h in segs.values())

    if arrangement == "horizontal":
        clusters = _groups([(x, x + w, k) for k, (x, _, w, _) in segs.items()], 0.04 * (ux1 - ux0))
    elif arrangement == "vertical":
        clusters = _groups([(y, y + h, k) for k, (_, y, _, h) in segs.items()], 0.04 * (uy1 - uy0))
    else:
        clusters = [list(segs)]

    glass = next((g for g in GLASS_IDS if g in boxes), None)
    out = []
    for keys in clusters:
        x0 = min(segs[k][0] for k in keys)
        y0 = min(segs[k][1] for k in keys)
        x1 = max(segs[k][0] + segs[k][2] for k in keys)
        y1 = max(segs[k][1] + segs[k][3] for k in keys)
        w, h = x1 - x0, y1 - y0
        x0, y0, x1, y1 = x0 - w * pad, y0 - h * pad, x1 + w * pad, y1 + h * pad
        if glass is not None:
            rx, ry, rw, rh = boxes[glass]
            inset = float(_by_id(root, glass).get("stroke-width", 0) or 0) / 2
            x0, y0 = max(x0, rx + inset), max(y0, ry + inset)
            x1, y1 = min(x1, rx + rw - inset), min(y1, ry + rh - inset)
        out.append(((x0, y0, x1 - x0, y1 - y0), set(keys)))
    return arrangement, out


def segments_svg(root, rect, scheme: str, members=None) -> tuple:
    title_of = SCHEMES[scheme]
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

    seen = set()
    for el, byte, bit in segment_elements(root):
        if members is not None and (byte, bit) not in members:
            continue
        title = title_of(byte, bit)
        if title in seen:
            raise SystemExit(f"duplicate segment {title}")
        seen.add(title)
        seg = copy.deepcopy(el)
        # Flattened straight under the layer - no face puts a transform or
        # style on a segment's ancestors (checked across all of them with
        # CONVERT_ROM/tmp/brick_face_survey.py), so nothing is lost.
        etree.SubElement(seg, f"{{{SVG_NS}}}title").text = title
        layer.append(seg)
    return etree.tostring(out, xml_declaration=True, encoding="UTF-8"), len(seen)


def render(root, out_png: Path, area=None, width=None, height=None):
    with tempfile.TemporaryDirectory() as tmp:
        svg = Path(tmp) / "r.svg"
        svg.write_bytes(etree.tostring(root))
        args = [INKSCAPE, str(svg), "--export-type=png", f"--export-filename={out_png}",
                "--export-background-opacity=0"]
        if area:
            x, y, w, h = area
            args.append(f"--export-area={x:g}:{y:g}:{x + w:g}:{y + h:g}")
        if width:
            args.append(f"--export-width={width}")
        if height:
            args.append(f"--export-height={height}")
        subprocess.run(args, check=True, capture_output=True)


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


def split(face: Path, out_dir: Path, name: str, scheme: str = "brick", height: int = 1080,
          face_png: bool = False, pad: float = 0.03) -> dict:
    """Returns {"arrangement": "single"|"horizontal"|"vertical", "screens":
    [{"width", "height", "count"}...], "background": bool, "overlay": bool}.
    Background/overlay are kept for every screen if any screen has them, so
    the per-screen path lists stay aligned."""
    from PIL import Image

    out_dir.mkdir(parents=True, exist_ok=True)
    root = load_face(face)
    arrangement, screens = screen_rects(root, pad, MULTI_SCREEN_FACES.get(Path(face).stem, "single"))
    body = _keep_only(root, ("body",))
    overlay_only = _keep_only(root, ("overlay",)) if _by_id(root, "overlay") is not None else None

    info = {"arrangement": arrangement, "screens": [], "background": False, "overlay": False}
    for i, (rect, members) in enumerate(screens):
        sfx = "" if i == 0 else str(i + 1)
        svg, count = segments_svg(root, rect, scheme, members)
        (out_dir / f"{name}_segs{sfx}.svg").write_bytes(svg)
        width = round(rect[2] / rect[3] * height)
        info["screens"].append({"width": width, "height": height, "count": count})

        render(body, out_dir / f"Background{sfx}.png", area=rect, width=width, height=height)
        info["background"] |= not is_flat(out_dir / f"Background{sfx}.png")

        if overlay_only is not None:
            overlay = out_dir / f"Overlay{sfx}.png"
            render(overlay_only, overlay, area=rect, width=width, height=height)
            # Some overlays only shade the bezel around the LCD (Yasashii), so
            # nothing of them falls inside the crop - no point shipping that.
            info["overlay"] |= Image.open(overlay).convert("RGBA").getextrema()[3][1] > 0

    # Drop flat backgrounds / empty overlays, and leftovers from earlier runs.
    for i in range(8):
        sfx = "" if i == 0 else str(i + 1)
        stale = i >= len(screens)
        if stale and i > 0 and (out_dir / f"{name}_segs{sfx}.svg").exists():
            (out_dir / f"{name}_segs{sfx}.svg").unlink()
        if (stale or not info["background"]) and (out_dir / f"Background{sfx}.png").exists():
            (out_dir / f"Background{sfx}.png").unlink()
        if (stale or not info["overlay"]) and (out_dir / f"Overlay{sfx}.png").exists():
            (out_dir / f"Overlay{sfx}.png").unlink()

    if face_png:
        render(_keep_only(root, ("body", "overlay")), out_dir / f"{name}_face.png", height=1026)
    sizes = ", ".join(f"[[{sc['width']},0,0],[{sc['height']},0,0]]" for sc in info["screens"])
    total = sum(sc["count"] for sc in info["screens"])
    print(f"{name}: {total} segments, {len(screens)} screen(s) {arrangement} -> transform_visual [{sizes}]"
          + ("" if info["background"] else " (flat: no background)") + (" + overlay" if info["overlay"] else ""))
    return info


def face_thumbnail(face: Path, out_png: Path, lit=None, card=(1368, 1026), fill: float = 0.92,
                   backdrop=(255, 255, 255)):
    """A menu thumbnail: the whole device (body, display, overlay - what
    BrickEmuPy shows) on a `card`-sized backdrop, the segments whose
    "<byte>_<bit>" id is in `lit` shown lit and the rest as faint ghosts
    (BrickEmuPy's own ghost-segment look). `lit` None = all ghosts."""
    from PIL import Image

    root = _keep_only(load_face(face), DRAWN_IDS)
    lit = lit or set()
    for el, byte, bit in segment_elements(root):
        el.set("opacity", "0.88" if f"{byte}_{bit}" in lit else "0.06")
    with tempfile.TemporaryDirectory() as tmp:
        png = Path(tmp) / "f.png"
        render(root, png, height=round(card[1] * fill))
        device = Image.open(png).convert("RGBA")
    scale = min(card[0] * fill / device.width, card[1] * fill / device.height, 1.0)
    if scale < 1.0:
        device = device.resize((round(device.width * scale), round(device.height * scale)), Image.LANCZOS)
    out = Image.new("RGB", card, backdrop)
    out.paste(device, ((card[0] - device.width) // 2, (card[1] - device.height) // 2), device)
    out.save(out_png)


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("face", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("name")
    ap.add_argument("--scheme", choices=sorted(SCHEMES), default="brick")
    ap.add_argument("--height", type=int, default=1080)
    ap.add_argument("--face", dest="face_png", action="store_true")
    ap.add_argument("--pad", type=float, default=0.03, help="margin around the segments, as a fraction")
    a = ap.parse_args()
    split(a.face, a.out_dir, a.name, a.scheme, a.height, a.face_png, a.pad)
