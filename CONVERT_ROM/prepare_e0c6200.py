"""Prepares MAME's hh_e0c6200 sets (Seiko Epson E0C6S46/S48 keychain games -
Tamagotchi, Digital Monster, ...) for convert_switch.py.

For each set in rom/roms/<set>.zip this writes rom/decompress/<set>/:
  <set>.bin           the program ROM (12288 / 16384 bytes: E0C6S46 / S48)
  <set>_segs.svg      the LCD, with every segment titled "COM.SEG.0"
  Background.png      only where MAME artwork exists (rom/artwork/<set>.zip,
                      or the set it borrows from - see ARTWORK_FROM)

With --brick <BrickEmuPy assets dir>, the LCD and background come from
BrickEmuPy's whole-device face SVGs instead (github.com/azya52/BrickEmuPy,
CC0 - see BRICK_FACES and split_brick_svg.py): every set then gets its real
printed inlay as Background.png, plus <set>_face.png (the device body,
display hidden) for thumbnails. MAME still supplies the ROMs.

Segment ids: MAME's E0C6S46 LCD driver names each dot "COM.SEG" (16 COMs x
up to 51 SEGs - hh_e0c6200.cpp's lcd_segment_w); convert_switch.py wants a
3-part "col.line.word" id, so a ".0" word is appended and the engine reads
(col, line, word) = (COM, SEG, 0) - see source/E0C6200/E0C6S46.h.

alienfev and venusdm have no SVG in MAME - they're plain dot-matrix screens
drawn by e0c6s46_device::screen_update() - so their SVG is generated here
from the same COM/SEG -> pixel mapping MAME uses (venusdm's pixel_callback
reorders it into a 32x20 grid).

Usage:  python prepare_e0c6200.py [--brick <assets dir>] [set ...]   (default: all)
"""
import io
import re
import sys
import zipfile
from pathlib import Path

from lxml import etree
from PIL import Image

HERE = Path(__file__).resolve().parent
ROMS = HERE / "rom" / "roms"
ARTWORK = HERE / "rom" / "artwork"
DECOMPRESS = HERE / "rom" / "decompress"

SVG_NS = "http://www.w3.org/2000/svg"
INK_NS = "http://www.inkscape.org/namespaces/inkscape"

# set -> SVG file inside its zip (None = generated dot matrix)
SETS = {
    "tama": "tama.svg",
    "tamag2": "tama.svg",
    "tamaang": "tamaang.svg",
    "tamamot": "tamamot.svg",
    "digimon": "digimon.svg",
    "digimonv2": "digimon.svg",
    "digimonv3": "digimon.svg",
    "stackch": "stackch.svg",
    "alienfev": None,
    "venusdm": None,
}

# Gen 2 is the same unit with new graphics; only Gen 1's inlay is dumped.
ARTWORK_FROM = {"tamag2": "tama"}

# set -> BrickEmuPy face SVG (its TamagotchiP2.brick reuses the P1 face too)
BRICK_FACES = {
    "tama": "TamagotchiP1.svg",
    "tamag2": "TamagotchiP1.svg",
    "tamaang": "TamagotchiAngel.svg",
    "tamamot": "TamagotchiMothra.svg",
    "digimon": "DigimonV1.svg",
    "digimonv2": "DigimonV2J.svg",
    "digimonv3": "DigimonV3J.svg",
    "alienfev": "AlienFever.svg",
    "venusdm": "nikko.svg",
    "stackch": "RadioShackStackChallenge.svg",
    # BrickEmuPy-only sets (not in MAME): ROM from its assets too, see BRICK_ROMS
    "tamaj": "TamagotchiP1.svg",
    "tamaumi": "TamagotchiUmino.svg",
    "tamamori": "TamagotchiMorino.svg",
    "tamagenjin": "TamagotchiGenjintch.svg",
    "tamayasa": "TamagotchiYasashii.svg",
    "digimones": "DigimonV1ES.svg",
    "digimonv4": "DigimonV4J.svg",
}

# set -> BrickEmuPy ROM, for the sets MAME doesn't have (need --brick)
BRICK_ROMS = {
    "tamaj": "TamagotchiP1J.bin",
    "tamaumi": "TamagotchiUmino.bin",
    "tamamori": "TamagotchiMorino.bin",
    "tamagenjin": "TamagotchiGenjintch.bin",
    "tamayasa": "TamagotchiYasashii.bin",
    "digimones": "DigimonV1ES.bin",
    "digimonv4": "DigimonV4J.bin",
}


def _rom_bytes(zf: zipfile.ZipFile, name: str) -> bytes:
    data = zf.read(f"{name}.bin")
    if len(data) not in (0x3000, 0x4000):
        raise SystemExit(f"{name}.bin: unexpected size {len(data)}")
    return data


def _retitle_svg(svg_bytes: bytes) -> tuple[bytes, int]:
    """Appends '.0' to every 'COM.SEG' segment title.

    Elements titled anything else (stackch has five titled "unused") are
    never driven by MAME, i.e. never visible, so they're dropped.
    """
    root = etree.fromstring(svg_bytes)
    count = 0
    for title in list(root.iter(f"{{{SVG_NS}}}title")):
        text = (title.text or "").strip()
        if re.fullmatch(r"\d+\.\d+", text):
            title.text = text + ".0"
            count += 1
        else:
            seg = title.getparent()
            seg.getparent().remove(seg)
    if root.find(f'.//{{{SVG_NS}}}g[@{{{INK_NS}}}label="segs"]') is None:
        raise SystemExit("no 'segs' layer in SVG")
    return etree.tostring(root, xml_declaration=True, encoding="UTF-8"), count


def _dot_matrix_svg(width: int, height: int, dots: dict) -> bytes:
    """dots: (x, y) -> 'COM.SEG'. 10-unit pitch, 9-unit squares."""
    pitch, size = 10, 9
    root = etree.Element(f"{{{SVG_NS}}}svg", nsmap={None: SVG_NS, "inkscape": INK_NS})
    root.set("width", str(width * pitch))
    root.set("height", str(height * pitch))
    root.set("viewBox", f"0 0 {width * pitch} {height * pitch}")
    segs = etree.SubElement(root, f"{{{SVG_NS}}}g")
    segs.set("id", "layer_segs")
    segs.set(f"{{{INK_NS}}}label", "segs")
    segs.set(f"{{{INK_NS}}}groupmode", "layer")
    for (x, y), name in sorted(dots.items(), key=lambda kv: (kv[0][1], kv[0][0])):
        rect = etree.SubElement(segs, f"{{{SVG_NS}}}rect")
        rect.set("x", str(x * pitch + (pitch - size) / 2))
        rect.set("y", str(y * pitch + (pitch - size) / 2))
        rect.set("width", str(size))
        rect.set("height", str(size))
        rect.set("style", "fill:#000000;fill-opacity:0.85")
        etree.SubElement(rect, f"{{{SVG_NS}}}title").text = name + ".0"
    return etree.tostring(root, xml_declaration=True, encoding="UTF-8", pretty_print=True)


def _alienfev_dots() -> dict:
    # screen_update(): x = SEG, y = COM on a 39x16 screen (SEG 39 is off-screen)
    return {(seg, com): f"{com}.{seg}" for com in range(16) for seg in range(39)}


def _venusdm_dots() -> dict:
    # venusdm_state::pixel_callback: dx = dy | (dx / 20) << 4, dy = dx % 20
    dots = {}
    for com in range(16):
        for seg in range(40):
            dots[(com | (seg // 20) << 4, seg % 20)] = f"{com}.{seg}"
    return dots


def _svg_size(svg_bytes: bytes) -> tuple[float, float]:
    root = etree.fromstring(svg_bytes)
    vb = root.get("viewBox")
    if vb:
        _, _, w, h = map(float, vb.split())
        return w, h
    return float(root.get("width")), float(root.get("height"))


def _background_from_layout(art_zip: Path, out_png: Path) -> None:
    """Crops the artwork's background image to the screen, per its .lay.

    hh_e0c6200 layouts put <screen> at the view origin with the inlay image
    (blend="multiply") at some offset around it; only the part under the
    screen is visible, so that's what becomes Background.png.
    """
    with zipfile.ZipFile(art_zip) as zf:
        lay = etree.fromstring(zf.read("default.lay"))
        view = lay.find("view")
        screen = view.find("screen/bounds")
        sw, sh = float(screen.get("width")), float(screen.get("height"))
        sx, sy = float(screen.get("x", 0)), float(screen.get("y", 0))
        el = next(e for e in view.findall("element") if e.get("ref") == "bg")
        b = el.find("bounds")
        bx, by = float(b.get("x")), float(b.get("y"))
        bw, bh = float(b.get("width")), float(b.get("height"))
        image_file = next(i.get("file") for i in lay.iter("image"))
        img = Image.open(io.BytesIO(zf.read(image_file))).convert("RGB")
    # layout units -> image pixels
    kx, ky = img.width / bw, img.height / bh
    box = ((sx - bx) * kx, (sy - by) * ky, (sx + sw - bx) * kx, (sy + sh - by) * ky)
    img.crop(tuple(round(v) for v in box)).resize((round(sw), round(sh)), Image.LANCZOS).save(out_png)


def prepare(name: str, brick: Path = None) -> None:
    out_dir = DECOMPRESS / name
    out_dir.mkdir(parents=True, exist_ok=True)
    if name in BRICK_ROMS:
        if brick is None:
            raise SystemExit(f"{name}: BrickEmuPy-only set, needs --brick")
        rom = (brick / BRICK_ROMS[name]).read_bytes()
        if len(rom) not in (0x3000, 0x4000):
            raise SystemExit(f"{BRICK_ROMS[name]}: unexpected size {len(rom)}")
        (out_dir / f"{name}.bin").write_bytes(rom)
    else:
        with zipfile.ZipFile(ROMS / f"{name}.zip") as zf:
            (out_dir / f"{name}.bin").write_bytes(_rom_bytes(zf, name))
    if brick is not None and name in BRICK_FACES:
        import split_brick_svg
        split_brick_svg.split(brick / BRICK_FACES[name], out_dir, name, face_png=True)
        return
    with zipfile.ZipFile(ROMS / f"{name}.zip") as zf:
        svg_name = SETS[name]
        if svg_name:
            svg, count = _retitle_svg(zf.read(svg_name))
        elif name == "alienfev":
            dots = _alienfev_dots()
            svg, count = _dot_matrix_svg(39, 16, dots), len(dots)
        else:
            dots = _venusdm_dots()
            svg, count = _dot_matrix_svg(32, 20, dots), len(dots)
    (out_dir / f"{name}_segs.svg").write_bytes(svg)
    w, h = _svg_size(svg)
    line = f"{name}: {count} segments, svg {w:g}x{h:g}"

    art = ARTWORK / f"{ARTWORK_FROM.get(name, name)}.zip"
    if art.exists():
        _background_from_layout(art, out_dir / "Background.png")
        line += f", Background.png from {art.name}"
    print(line)


if __name__ == "__main__":
    args = sys.argv[1:]
    brick_dir = None
    if args[:1] == ["--brick"]:
        brick_dir, args = Path(args[1]), args[2:]
    for set_name in args or (list(SETS) + (list(BRICK_ROMS) if brick_dir else [])):
        prepare(set_name, brick_dir)
