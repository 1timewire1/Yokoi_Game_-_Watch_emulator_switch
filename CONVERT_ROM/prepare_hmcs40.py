"""Prepares MAME's hh_hmcs40 VFD tabletop games for convert_switch.py, run on
the Hitachi HMCS40 core (source/BrickCores/HMCS40.*, ported from MAME).

For each set in rom/roms/<set>.zip (the MAME zip: ROM dump + <set>.svg) this
writes rom/decompress/<set>/:
  <set>.brk          a ".brk" container (source/BrickCores/BrickRom.h) for
                     the "HMCS40" core: clock 100000 (MAME's 400 kHz / 4),
                     "hmcs40.game" / "hmcs40.family", the button bindings,
                     and the 4352-byte dump as the "rom" blob
  <set>_segs.svg     the VFD segments under a "segs" layer, titled with the
                     core's display ids and split by colour (see below)
  Background.png     plain black (the VFD glass; MAME's SVGs have no other art)
and rom/console/<set>.png - the photo of the device in hmcs40_consoles/<set>.png
(transparent background) on the white thumbnail card, like the BrickEmuPy
games' faces; without one, the whole screen lit on black - and prints the
games_path_rgds.py entry to add.

Segment ids: MAME titles each VFD segment "<grid>.<plate>" (and "<grid>.a"
for parts lit whenever that grid is strobed). The core keeps 5 display bytes
per grid - plate p at byte grid*5 + p//8, bit p%8; ".a" at byte grid*5 + 4,
bit 0 - so the title becomes "<byte>.<bit>.0".

Colours: a VFD's phosphors are coloured, so every segment carries a colour
index (convert_switch.py's color_segment: "<id>_<index>" titles) into the
renderer's palette (switch/source/render/switch_screen.cpp kSegmentColorRgb,
entries 5-36 below). A segment drawn in several colours (Tron's MCP tower,
Donkey Kong's sprites) is split into one segment per colour, all with the
same id. Transforms are flattened onto each piece, since convert_svg.py
exports titled elements on their own.

Usage:  python prepare_hmcs40.py [set ...]        (default: all of them)
"""
import io
import re
import struct
import sys
import zipfile
from pathlib import Path

from lxml import etree
from PIL import Image

HERE = Path(__file__).resolve().parent
ROMS = HERE / "rom" / "roms"
DECOMPRESS = HERE / "rom" / "decompress"
CONSOLE = HERE / "rom" / "console"

SVG_NS = "http://www.w3.org/2000/svg"
INK_NS = "http://www.inkscape.org/namespaces/inkscape"
NS = "{%s}" % SVG_NS
SHAPES = {NS + t for t in ("path", "rect", "circle", "ellipse", "polygon", "polyline", "line", "text")}

# Palette entries added for the VFD games (index -> RGB); keep in sync with
# kSegmentColorRgb in switch/source/render/switch_screen.cpp.
PALETTE = {
    5: (0x80, 0xFF, 0xF8),   # cyan
    6: (0xFF, 0x48, 0x20),   # red-orange
    7: (0x50, 0xFF, 0x30),   # green
    8: (0xE6, 0xE6, 0xE6),   # white
    9: (0xF6, 0x00, 0x00),   # red
    10: (0xB4, 0x9D, 0xFF),  # violet
    # Dig Dug's SVG bakes its colour overlay (orange/yellow rows) into the art
    11: (0xB0, 0xFF, 0xE0), 12: (0xE0, 0xFF, 0xD0), 13: (0xFF, 0x2C, 0x00), 14: (0xFF, 0xF0, 0xC0),
    15: (0xFF, 0x5B, 0x00), 16: (0xFF, 0xC7, 0x5F), 17: (0xFF, 0xE4, 0x5F), 18: (0xFF, 0xD5, 0x5F),
    19: (0x64, 0xF0, 0xF0), 20: (0xFF, 0xFF, 0x48),
    # Zaxxon's segments seen through its mirror (cyan, red, blue at 56%): 31-33
    # Bandai Zaxxon/Pengo/Zackman/Frisky Tom, Q*Bert (21-29); 30 = black, cut-outs
    # drawn over other colours within a segment (Zaxxon)
    21: (0x48, 0x40, 0xFF), 22: (0xFF, 0x94, 0x29), 23: (0xFF, 0x17, 0x00), 24: (0x64, 0xDF, 0xFF),
    25: (0xFF, 0x4E, 0x30), 26: (0xFF, 0x00, 0x00), 27: (0x97, 0x65, 0xFF), 28: (0x00, 0x8F, 0xFF),
    29: (0x00, 0xBB, 0x00), 30: (0x00, 0x00, 0x00),
    31: (0x48, 0x8F, 0x8B), 32: (0x8F, 0x28, 0x12), 33: (0x28, 0x24, 0x8F),
    # VTech Invaders' colour overlay: 34-36
    34: (0x9E, 0xEA, 0xE8), 35: (0x64, 0xFF, 0x71), 36: (0xFF, 0x66, 0x00),
    # Knock-Em Out Boxing's cyan VFD behind its blue window (layout multiply): 37
    37: (0x0D, 0x66, 0xE0),
    # Coleco Frogger's VFD (MELPS4 core): 38-41
    38: (0x00, 0xFF, 0xCA), 39: (0x26, 0xC9, 0xFF), 40: (0x00, 0xB8, 0x4E), 41: (0xFF, 0x68, 0x20),
}

# set -> (ROM file in the zip, HMCS family, display name, manufacturer, date,
#         buttons [(name, core button number, Switch keys)])
GAMES = {
    "packmon": ("hd38800a27", 44, "Packri Monster", "Bandai", "1981-01-01",
                [("btnStart", 0, "PLUS|A"), ("btnLeft", 1, "LEFT"), ("btnRight", 2, "RIGHT"),
                 ("btnUp", 3, "UP"), ("btnDown", 4, "DOWN")]),
    "cdkong": ("hd38820a45", 45, "Donkey Kong (Coleco)", "Coleco", "1982-01-01",
               [("btnJump", 0, "A|B"), ("btnUp", 1, "UP"), ("btnRight", 2, "RIGHT"),
                ("btnDown", 3, "DOWN"), ("btnLeft", 4, "LEFT")]),
    "egalaxn2": ("hd38820a13", 45, "Galaxian 2 (Entex)", "Entex Industries", "1981-01-01",
                 [("btnRight", 0, "RIGHT"), ("btnLeft", 1, "LEFT"), ("btnFire", 2, "A|B")]),
    # Pac Man 2: two VFD revisions (Pac-Man cyan / red), the same ROM - the red
    # one's MAME zip holds only its SVG (the ROM comes from the parent, PARENTS).
    # Skill Control starts a game; the SKILL 1/2 slide switch is L.
    "epacman2": ("hd38820a23", 45, "Pac Man 2 (Entex, cyan Pac-Man)", "Entex Industries", "1981-01-01",
                 [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                  ("btnSkillControl", 4, "A|PLUS"), ("btnDemo", 5, "MINUS"), ("btnSkill", 6, "L")]),
    "epacman2r": ("hd38820a23", 45, "Pac Man 2 (Entex, red Pac-Man)", "Entex Industries", "1981-01-01",
                  [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                   ("btnSkillControl", 4, "A|PLUS"), ("btnDemo", 5, "MINUS"), ("btnSkill", 6, "L")]),
    # Super Space Invader 2, the black version (the Select-A-Game cartridge's
    # MCU); MAME: hold fire at boot for the demo.
    "einvader2": ("inv2_hd38800a31", 44, "Super Space Invader 2 (Entex, black version)", "Entex Industries", "1981-01-01",
                  [("btnRight", 0, "RIGHT"), ("btnLeft", 1, "LEFT"), ("btnFire", 2, "A|B")]),
    "tmtron": ("hd38800a88", 44, "Tron (Tomy)", "TOMY Company", "1981-01-01",
               [("btnDown", 0, "DOWN"), ("btnRight", 1, "RIGHT"), ("btnUp", 2, "UP"),
                ("btnLeft", 3, "LEFT"), ("btnFire", 4, "A|B")]),
    "gdigdug": ("hd38820a69", 45, "Dig Dug (Gakken)", "Gakken", "1983-01-01",
                [("btnSelect", 0, "MINUS|X"), ("btnDown", 1, "DOWN"), ("btnRight", 2, "RIGHT"),
                 ("btnUp", 3, "UP"), ("btnLeft", 4, "LEFT"), ("btnFire", 5, "A|B")]),
    "gscobra": ("hd38820a32", 45, "Super Cobra (Gakken, green Version)", "Gakken", "1982-01-01",
                [("btnBomb", 0, "B|Y"), ("btnMissile", 1, "A|X"), ("btnUp", 2, "UP"),
                 ("btnDown", 3, "DOWN"), ("btnLeft", 4, "LEFT"), ("btnRight", 5, "RIGHT")]),
    # Pepper doubles as Start, Attack as Select (labels above the buttons)
    "bbtime": ("hd38820a65", 45, "Burger Time (Bandai)", "Bandai", "1982-01-01",
               [("btnRight", 0, "RIGHT"), ("btnLeft", 1, "LEFT"), ("btnDown", 2, "DOWN"),
                ("btnUp", 3, "UP"), ("btnAttack", 4, "B|MINUS"), ("btnPepper", 5, "A|PLUS")]),
    # Player 1 on the D-pad; player 2 (Head-to-Head) on X/A/B/Y as a second
    # D-pad. The game mode is picked with P1's stick at the title: Left = Ms.
    # Pac-Man, Down = Head-to-Head, Up = demo.
    "cmspacmn": ("hd38820a61", 45, "Ms. Pac-Man (Coleco)", "Coleco", "1982-01-01",
                 [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                  ("btnP2Up", 4, "X"), ("btnP2Right", 5, "A"), ("btnP2Down", 6, "B"), ("btnP2Left", 7, "Y")]),
    "estargte": ("hd38820a42", 45, "Stargate (Entex)", "Entex Industries", "1982-01-01",
                 [("btnInviso", 0, "Y"), ("btnSmartBomb", 1, "X"), ("btnFire", 2, "A"),
                  ("btnChangeDirection", 3, "LEFT|RIGHT"), ("btnUp", 4, "UP"), ("btnDown", 5, "DOWN"),
                  ("btnThrust", 6, "B"), ("btnSpeedDown", 7, "L"), ("btnSpeedUp", 8, "R")]),
    "bfriskyt": ("hd38800a77", 44, "Frisky Tom (Bandai)", "Bandai", "1982-01-01",
                 [("btnAttack", 0, "A|B"), ("btnDown", 1, "DOWN"), ("btnRight", 2, "RIGHT"), ("btnUp", 3, "UP"),
                  ("btnLeft", 4, "LEFT"), ("btnStart", 5, "PLUS|X")]),
    "bzaxxon": ("hd38800b19", 44, "Zaxxon (Bandai)", "Bandai", "1982-01-01",
                [("btnRight", 0, "RIGHT"), ("btnUp", 1, "UP"), ("btnLeft", 2, "LEFT"), ("btnDown", 3, "DOWN"),
                 ("btnFire", 4, "A|B|PLUS"), ("btnSelect", 5, "MINUS|X")]),
    "zackman": ("hd38820a49", 45, "Zackman (Bandai)", "Bandai", "1983-01-01",
                [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                 ("btnFire", 4, "A|B|PLUS")]),
    "bpengo": ("hd38820a63", 45, "Pengo (Bandai)", "Bandai", "1983-01-01",
               [("btnUp", 0, "UP"), ("btnDown", 1, "DOWN"), ("btnLeft", 2, "LEFT"), ("btnRight", 3, "RIGHT"),
                ("btnAttack", 4, "A|B|PLUS")]),
    "alnattck": ("hd38800a25", 44, "Alien Attack (Coleco)", "Coleco", "1981-01-01",
                 [("btnLeft", 0, "LEFT"), ("btnRight", 1, "RIGHT"), ("btnUp", 2, "UP"), ("btnDown", 3, "DOWN"),
                  ("btnMove", 4, "B"), ("btnFire", 5, "A")]),
    # The stick is diagonal: D-pad Left/Up/Right/Down = up-left/up-right/
    # down-right/down-left as in MAME, and Y/X/A/B the same four.
    "pbqbert": ("hd38820a70", 45, "Q*Bert (Parker Brothers)", "Parker Brothers", "1983-01-01",
                [("btnUpLeft", 0, "LEFT|Y"), ("btnUpRight", 1, "UP|X"), ("btnDownRight", 2, "RIGHT|A"),
                 ("btnDownLeft", 3, "DOWN|B")]),
}

# Pac-Man rev. 1 (MAME's cpacmanr1 clone) uses its parent's screen: put
# cpacman.svg (from cpacman.zip) into rom/roms/cpacmanr1.zip next to its ROM.
GAMES["cpacmanr1"] = ("hd38820a28", 45, "Pac-Man (rev. 1) (Coleco)", "Coleco", "1981-01-01",
                      [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                       ("btnP2Up", 4, "X"), ("btnP2Right", 5, "A"), ("btnP2Down", 6, "B"), ("btnP2Left", 7, "Y")])
# rev. 2 (MAME's cpacman): the fixed one - the whole maze shows at power-on.
GAMES["cpacman"] = ("hd38820a29", 45, "Pac-Man (rev. 2) (Coleco)", "Coleco", "1981-01-01", GAMES["cpacmanr1"][5])
SVGS = {"cpacmanr1": "cpacman.svg"}

GAMES.update({
    "gckong": ("hd38800b01", 44, "Crazy Kong (Gakken)", "Gakken", "1982-01-01",
               [("btnRight", 0, "RIGHT"), ("btnDown", 1, "DOWN"), ("btnLeft", 2, "LEFT"), ("btnUp", 3, "UP"),
                ("btnJump", 4, "A|B")]),
    "bdoramon": ("hd38800b43", 44, "Dokodemo Dorayaki Doraemon (Bandai)", "Bandai", "1982-01-01",
                 [("btnLeft", 0, "LEFT"), ("btnAttack", 1, "A|B|PLUS"), ("btnUp", 2, "UP"), ("btnRight", 3, "RIGHT"),
                  ("btnDown", 4, "DOWN")]),
    "kingman": ("hd38800b23", 44, "Kingman (Tomy)", "TOMY Company", "1982-01-01",
                [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnDown", 2, "DOWN"), ("btnLeft", 3, "LEFT"),
                 ("btnJump", 4, "A|B|PLUS")]),
    "ghalien": ("hd38800a04", 44, "Heiankyo Alien (Gakken)", "Gakken", "1980-01-01",
                [("btnUp", 0, "UP"), ("btnDown", 1, "DOWN"), ("btnRight", 2, "RIGHT"), ("btnLeft", 3, "LEFT"),
                 ("btnDig", 4, "A"), ("btnBury", 5, "B")]),
    "machiman": ("hd38820a85", 45, "Machine Man (Bandai)", "Bandai", "1984-01-01",
                 [("btnAttack", 0, "A|B|PLUS"), ("btnLeft", 1, "LEFT"), ("btnRight", 2, "RIGHT")]),
    "bambball": ("hd38750a08", 43, "Dribble Away Basketball (Bambino)", "Bambino", "1979-01-01",
                 [("btnDribbleLow", 0, "Y"), ("btnDribbleMedium", 1, "X"), ("btnDribbleHigh", 2, "A"),
                  ("btnShoot", 3, "UP|B"), ("btnRight", 4, "RIGHT"), ("btnLeft", 5, "LEFT"),
                  ("btnStart", 6, "PLUS"), ("btnDisplay", 7, "MINUS")]),
    "vinvader": ("hd38750a45", 43, "Invaders (VTech)", "V-Tech", "1980-01-01",
                 [("btnLeft", 0, "LEFT"), ("btnRight", 1, "RIGHT"), ("btnFire", 2, "A|B")]),
    "eturtles": ("hd38820a43", 45, "Turtles (Entex)", "Entex Industries", "1982-01-01",
                 [("btnUp", 0, "UP"), ("btnDown", 1, "DOWN"), ("btnLeft", 2, "LEFT"), ("btnRight", 3, "RIGHT"),
                  ("btnFire", 4, "A|B|PLUS"), ("btnSpeedDown", 5, "L"), ("btnSpeedUp", 6, "R")]),
    # Punches on X/A/B (high/medium/low, top to bottom like the real buttons),
    # positions on the D-pad (Up = back, Left/Right = normal, Down = duck).
    # Pair Match's 16-way stick on the D-pad (diagonals = two directions), its
    # button on A/B/Plus, its SKILL switch on L (1 -> 2 -> 3 -> 1). 1 player.
    "pairmtch": ("hd38820a88", 45, "Pair Match (Bandai)", "Bandai", "1981-01-01",
                 [("btnRight", 0, "RIGHT"), ("btnDown", 1, "DOWN"), ("btnUp", 2, "UP"), ("btnLeft", 3, "LEFT"),
                  ("btnButton", 4, "A|B|PLUS"), ("btnSkill", 5, "L")]),
    "bmboxing": ("hd38750a07", 43, "Knock-Em Out Boxing (Bambino)", "Bambino", "1979-01-01",
                 [("btnPunchHigh", 0, "X"), ("btnPunchMedium", 1, "A"), ("btnPunchLow", 2, "B"),
                  ("btnNormal", 3, "LEFT|RIGHT"), ("btnBack", 4, "UP"), ("btnDuck", 5, "DOWN"),
                  ("btnStart", 6, "PLUS")]),
})

# MAME hh_melps4 sets, on the Mitsubishi MELPS 4 core (source/BrickCores/
# MELPS4.*, M58846): same tuple, with "melps4" as the family. 600 kHz / 6 =
# 100000 instruction cycles a second, a 4096-byte dump. Frogger's RESET button
# (which starts a game) is Plus; its Skill 1/2 switch is L.
MELPS4_GAMES = {
    "cfrogger": ("m58846-701p", "melps4", "Frogger (Coleco)", "Coleco", "1982-01-01",
                 [("btnUp", 0, "UP"), ("btnRight", 1, "RIGHT"), ("btnLeft", 2, "LEFT"), ("btnDown", 3, "DOWN"),
                  ("btnSkill", 4, "L")]),
}
GAMES.update(MELPS4_GAMES)
RESET_KEYS = {"cfrogger": "PLUS"}

# Sets whose MAME oscillator isn't 400 kHz: instruction cycles per second (/4)
CLOCKS = {"bzaxxon": 112500, "vinvader": 75000, "einvader2": 112500}

# set -> sub-CPU ROM in the zip, stored as the container's "sound_rom" blob
SOUND_ROMS = {"estargte": "cop411l-kec_n", "eturtles": "cop411l-ked_n", "pairmtch": "hd38820a89"}

CLOCK_HZ = 100000
SCREEN_HEIGHT = 1080


# --- container ---------------------------------------------------------------

def write_container(path: Path, name: str, rom: bytes, sound_rom: bytes = None):
    _, family, _, _, _, buttons = GAMES[name]
    melps4 = family == "melps4"
    expected = 4096 if melps4 else 2176 if family == 43 else 4352
    if len(rom) != expected:
        raise SystemExit(f"{name}: ROM is {len(rom)} bytes, expected {expected}")
    opts = [f"melps4.game={name}"] if melps4 else [f"hmcs40.game={name}", f"hmcs40.family={family}"]
    opts += [f"input.{b}=direct;BTN;{n};1;{keys}" for b, n, keys in buttons]
    # Power off/on (BrickCore's "RES" pin: held = reset, running again on
    # release) - these units have no reset button of their own, and some
    # (Alien Attack) only start a new game from power-on.
    opts.append(f"input.btnReset=direct;RES;0;1;{RESET_KEYS.get(name, 'PLUS+MINUS')}")
    text = "\n".join(opts).encode("utf-8")
    core = b"MELPS4" if melps4 else b"HMCS40"
    out = bytearray(b"BRK1") + bytes([len(core)]) + core
    out += struct.pack("<II", CLOCKS.get(name, CLOCK_HZ), len(text)) + text
    blobs = [(b"rom", rom)] + ([(b"sound_rom", sound_rom)] if sound_rom else [])
    out += struct.pack("<I", len(blobs))
    for bname, data in blobs:
        out += bytes([len(bname)]) + bname + struct.pack("<I", len(data)) + data
    path.write_bytes(bytes(out))


# --- SVG ---------------------------------------------------------------------

def parse_transform(s: str):
    """SVG transform attribute -> affine (a, b, c, d, e, f)."""
    m = (1.0, 0.0, 0.0, 1.0, 0.0, 0.0)
    for fn, args in re.findall(r"(\w+)\s*\(([^)]*)\)", s or ""):
        v = [float(x) for x in re.findall(r"[-+]?(?:\d*\.\d+|\d+\.?)(?:[eE][-+]?\d+)?", args)]
        if fn == "matrix" and len(v) == 6:
            t = tuple(v)
        elif fn == "translate":
            t = (1, 0, 0, 1, v[0], v[1] if len(v) > 1 else 0)
        elif fn == "scale":
            t = (v[0], 0, 0, v[1] if len(v) > 1 else v[0], 0, 0)
        elif fn == "rotate":
            import math
            r = math.radians(v[0])
            t = (math.cos(r), math.sin(r), -math.sin(r), math.cos(r), 0, 0)
            if len(v) == 3:
                t = mul(mul((1, 0, 0, 1, v[1], v[2]), t), (1, 0, 0, 1, -v[1], -v[2]))
        else:
            raise SystemExit(f"unsupported transform {fn}")
        m = mul(m, t)
    return m


def mul(p, q):
    a, b, c, d, e, f = p
    A, B, C, D, E, F = q
    return (a * A + c * B, b * A + d * B, a * C + c * D, b * C + d * D, a * E + c * F + e, b * E + d * F + f)


def style_dict(el) -> dict:
    out = {}
    for k in ("fill", "stroke", "fill-opacity", "stroke-opacity", "opacity", "stroke-width"):
        if el.get(k) is not None:
            out[k] = el.get(k)
    for part in (el.get("style") or "").split(";"):
        if ":" in part:
            k, v = part.split(":", 1)
            out[k.strip()] = v.strip()
    return out


def hex_rgb(v):
    m = re.fullmatch(r"#([0-9a-fA-F]{6})", (v or "").strip())
    if m:
        return tuple(int(m.group(1)[i:i + 2], 16) for i in (0, 2, 4))
    m = re.fullmatch(r"#([0-9a-fA-F]{3})", (v or "").strip())
    if m:
        return tuple(int(ch * 2, 16) for ch in m.group(1))
    return None


def color_index(rgb) -> int:
    return min(PALETTE, key=lambda i: sum((a - b) ** 2 for a, b in zip(PALETTE[i], rgb)))


def core_id(title: str):
    m = re.fullmatch(r"(\d+)\.(\d+|a)", title.strip())
    if not m:
        return None
    g = int(m.group(1))
    if m.group(2) == "a":
        return f"{g * 5 + 4}.0.0"
    p = int(m.group(2))
    return f"{g * 5 + p // 8}.{p % 8}.0"


def build_segs_svg(svg_bytes: bytes, tint=None):
    root = etree.fromstring(svg_bytes)
    out = etree.Element(NS + "svg", nsmap={None: SVG_NS, "inkscape": INK_NS})
    for k in ("width", "height", "viewBox"):
        if root.get(k):
            out.set(k, root.get(k))
    layer = etree.SubElement(out, NS + "g")
    layer.set("id", "layer_segs")
    layer.set(f"{{{INK_NS}}}label", "segs")
    layer.set(f"{{{INK_NS}}}groupmode", "layer")

    # A translucent black untitled rect is drawn over the segments before it in
    # the document: Zaxxon's one-way mirror, through which the "under" layer is
    # seen at 56% brightness. Those segments get their colour dimmed to match.
    order = {e: i for i, e in enumerate(root.iter())}
    titled = {t.getparent() for t in root.iter(NS + "title")}
    dimmers = []
    for e in root.iter(NS + "rect"):
        if e in titled or any(a in titled for a in e.iterancestors()):
            continue
        st = style_dict(e)
        op = float(st.get("fill-opacity", 1)) * float(st.get("opacity", 1))
        if hex_rgb(st.get("fill")) == (0, 0, 0) and 0 < op < 1:
            dimmers.append((order[e], 1 - op))

    count = 0
    groups = {}  # (id, colour) -> pieces; one segment each, even when MAME
    #              repeats a title over several elements (cdkong's "9.a")
    for title in root.iter(NS + "title"):
        sid = core_id(title.text or "")
        if sid is None:
            continue
        seg = title.getparent()
        count += 1
        leaves = [seg] if seg.tag in SHAPES else [e for e in seg.iter() if e.tag in SHAPES]
        for leaf in leaves:
            chain = list(reversed(list(leaf.iterancestors())))  # root ... parent
            m = (1.0, 0.0, 0.0, 1.0, 0.0, 0.0)
            style = {}
            for anc in chain:
                m = mul(m, parse_transform(anc.get("transform")))
                style.update(style_dict(anc))
            style.update(style_dict(leaf))
            fill, stroke = style.get("fill"), style.get("stroke")
            rgb = hex_rgb(fill) or hex_rgb(stroke)
            if rgb is None or style.get("display") == "none":
                continue
            copy = etree.Element(leaf.tag)
            for k, v in leaf.attrib.items():
                copy.set(k, v)
            for child in leaf:
                if child.tag != NS + "title":
                    copy.append(etree.fromstring(etree.tostring(child)))
            copy.set("style", ";".join(f"{k}:{v}" for k, v in style.items() if k not in ("opacity",)))
            for k in ("fill", "stroke"):
                copy.attrib.pop(k, None)
            wrap = etree.Element(NS + "g")
            wrap.set("transform", "matrix(%s)" % ",".join(f"{x:.6g}" for x in m))
            wrap.append(copy)
            if tint:
                rgb = tuple(round(c * t) for c, t in zip(rgb, tint))
            for at, keep in dimmers:
                if order[leaf] < at:
                    rgb = tuple(round(c * keep) for c in rgb)
            groups.setdefault((sid, color_index(rgb)), []).append(wrap)
    for (sid, ci), wraps in groups.items():
        g = etree.SubElement(layer, NS + "g")
        etree.SubElement(g, NS + "title").text = f"{sid}_{ci}"
        for w in wraps:
            g.append(w)
    pieces = len(groups)
    return etree.tostring(out, xml_declaration=True, encoding="UTF-8"), count, pieces


# Sets with no SVG screen at all: MAME draws their whole display from the
# internal layout (Pair Match's grid of cyan cards with red centres). The
# layout's lit states become an SVG here: a "segr" card centre is cut out of
# the cyan card around it (its black frame), the static black stripes are cut
# out of the bars, and the "1"-"4" text elements stay text.
LAYOUT_SCREENS = {"pairmtch"}

# The layout's text digits as outlines (DejaVu Sans Bold, 2048 units/em, cap
# height 1493): text doesn't survive the per-segment PNG export.
GLYPHS = {
    "1": "M240 266H580V1231L231 1159V1421L578 1493H944V266H1284V0H240Z",
    "2": "M590 283H1247V0H162V283L707 764Q780 830 815.0 893.0Q850 956 850 1024Q850 1129 779.5 1193.0"
         "Q709 1257 592 1257Q502 1257 395.0 1218.5Q288 1180 166 1104V1432Q296 1475 423.0 1497.5Q550 1520 672 1520"
         "Q940 1520 1088.5 1402.0Q1237 1284 1237 1073Q1237 951 1174.0 845.5Q1111 740 909 563Z",
    "3": "M954 805Q1105 766 1183.5 669.5Q1262 573 1262 424Q1262 202 1092.0 86.5Q922 -29 596 -29Q481 -29 365.5 -10.5"
         "Q250 8 137 45V342Q245 288 351.5 260.5Q458 233 561 233Q714 233 795.5 286.0Q877 339 877 438Q877 540 793.5 592.5"
         "Q710 645 547 645H393V893H555Q700 893 771.0 938.5Q842 984 842 1077Q842 1163 773.0 1210.0Q704 1257 578 1257"
         "Q485 1257 390.0 1236.0Q295 1215 201 1174V1456Q315 1488 427.0 1504.0Q539 1520 647 1520Q938 1520 1082.5 1424.5"
         "Q1227 1329 1227 1137Q1227 1006 1158.0 922.5Q1089 839 954 805Z",
    "4": "M754 1176 332 551H754ZM690 1493H1118V551H1331V272H1118V0H754V272H92V602Z",
}


def svg_from_layout(name: str) -> bytes:
    root = etree.parse(str(LAYOUTS / f"{name}.lay")).getroot()
    lit = {}
    for el in root.findall("element"):
        for sh in el:
            if sh.tag in ("rect", "text") and sh.get("state") == "1":
                c = _lay_color(sh)
                lit[el.get("name")] = (sh.tag, "#%02x%02x%02x" % tuple(round(255 * v) for v in c[:3]),
                                       sh.get("string", ""))
    view = root.find("view")
    vx, vy, vw, vh = _lay_bounds(view)
    items = [(it.get("name"), it.get("ref"), _lay_bounds(it)) for it in view.iter("element")]
    blacks = [b for n, r, b in items if r == "static_black" and b != (vx, vy, vw, vh)]
    centres = [b for n, r, b in items if r == "segr"]

    def inter(a, b):
        x0, y0 = max(a[0], b[0]), max(a[1], b[1])
        x1, y1 = min(a[0] + a[2], b[0] + b[2]), min(a[1] + a[3], b[1] + b[3])
        return (x0, y0, x1 - x0, y1 - y0) if x1 > x0 and y1 > y0 else None

    def rect_d(b):
        x, y, w, h = b[0] - vx, b[1] - vy, b[2], b[3]
        return f"M {x:.3f},{y:.3f} H {x + w:.3f} V {y + h:.3f} H {x:.3f} Z "

    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{vw}" height="{vh}" viewBox="0 0 {vw} {vh}">']
    for n, ref, b in items:
        if not n or ref not in lit:
            continue
        kind, col, text = lit[ref]
        out.append(f"<g><title>{n}</title>")
        if kind == "text":  # the glyph 70% of the box high, centred
            k = 0.7 * b[3] / 1493
            tx = b[0] - vx + b[2] / 2 - 1425 * k / 2
            ty = b[1] - vy + b[3] / 2 + 1493 * k / 2
            out.append(f'<path transform="matrix({k:.6f},0,0,{-k:.6f},{tx:.3f},{ty:.3f})" '
                       f'style="fill:{col}" d="{GLYPHS[text]}"/>')
        else:
            if ref == "segr":  # the lit centre: 10% inside its black frame
                b = (b[0] + 0.1 * b[2], b[1] + 0.1 * b[3], 0.8 * b[2], 0.8 * b[3])
                holes = []
            else:
                holes = [h for h in (inter(b, c) for c in centres + blacks) if h]
            d = rect_d(b) + "".join(rect_d(h) for h in holes)
            out.append(f'<path fill-rule="evenodd" style="fill:{col}" d="{d}"/>')
        out.append("</g>")
    out.append("</svg>")
    return "\n".join(out).encode()


def svg_size(svg_bytes: bytes):
    root = etree.fromstring(svg_bytes)
    vb = root.get("viewBox")
    if vb:
        _, _, w, h = map(float, vb.split())
        return w, h
    return float(root.get("width")), float(root.get("height"))


def render_console(svg_bytes: bytes, out_png: Path, w: int, h: int):
    """The whole screen lit, on black, letterboxed into the 4:3 console card.
    Uses split_brick_svg's Inkscape call."""
    import split_brick_svg as sb
    root = etree.fromstring(svg_bytes)
    root.insert(0, etree.Element(NS + "rect", x="-100000", y="-100000", width="200000", height="200000",
                                 style="fill:#000000"))
    card_w, card_h = 1368, 1026
    scale = min(card_w * 0.92 / w, card_h * 0.92 / h)
    tmp = out_png.with_suffix(".tmp.png")
    sb.render(root, tmp, width=round(w * scale), height=round(h * scale))
    shot = Image.open(tmp).convert("RGB")
    tmp.unlink()
    card = Image.new("RGB", (card_w, card_h), (0, 0, 0))
    card.paste(shot, ((card_w - shot.width) // 2, (card_h - shot.height) // 2))
    card.save(out_png)


def console_from_photo(photo: Path, out_png: Path, card=(1368, 1026), fill: float = 0.92):
    """split_brick_svg.face_thumbnail's layout for a device photo."""
    device = Image.open(photo).convert("RGBA")
    bbox = device.getchannel("A").getbbox()
    if bbox:
        device = device.crop(bbox)
    scale = min(card[0] * fill / device.width, card[1] * fill / device.height)
    device = device.resize((round(device.width * scale), round(device.height * scale)), Image.LANCZOS)
    out = Image.new("RGB", card, (255, 255, 255))
    out.paste(device, ((card[0] - device.width) // 2, (card[1] - device.height) // 2), device)
    out.save(out_png)


def games_path_block(name: str, width: int) -> str:
    _, _, display, maker, date, _ = GAMES[name]
    key = re.sub(r"[^A-Za-z0-9]+", "_", display).strip("_")
    from source.manufacturer_ids import MANUFACTURER_BY_NAME
    d = name
    return "\n".join([
        f'            , "{key}" :',
        f'                    {{ "ref" : "{name}"',
        f'                    , "manufacturer" : {MANUFACTURER_BY_NAME[maker]}',
        f'                    , "display_name" : "{display}"',
        "                    , \"Rom\" : r'.\\rom\\decompress\\" + d + "\\" + d + ".brk'",
        "                    , \"Visual\" : [r'.\\rom\\decompress\\" + d + "\\" + d + "_segs.svg']",
        "                    , \"Background\" : [r'.\\rom\\decompress\\" + d + "\\Background.png']",
        '                    , "alpha_bright" : 1.0',
        '                    , "fond_bright" : 1.0',
        '                    , "shadow" : False',
        '                    , "color_segment" : True',
        "                    , \"console\" : r'.\\rom\\console\\" + d + ".png'",
        f'                    , "transform_visual" : [[[{width},0,0],[{SCREEN_HEIGHT},0,0]]]',
        f'                    , "date" : "{date}"',
        "                }",
    ]), key


# --- MAME layout (bezel / overlay art printed around and over the VFD) -------
#
# Some sets draw part of the playfield on the bezel rather than on the VFD:
# Crazy Kong's side ladders, Packri Monster's maze walls, Dribble Away
# Basketball's hoops. MAME draws those from the set's internal layout
# (src/mame/layout/<set>.lay, copied into hmcs40_layouts/); this paints its
# rect / disk / text elements into Background.png and widens the screen to the
# layout's view, so they sit where MAME puts them. A full-screen "multiply"
# element is a colour filter over the VFD (Basketball's green overlay): it
# tints the segment colours instead.

LAYOUTS = HERE / "hmcs40_layouts"


def _lay_color(node, default=(1.0, 1.0, 1.0, 1.0)):
    c = node.find("color") if node is not None else None
    if c is None:
        return default
    return tuple(float(c.get(k, d)) for k, d in zip(("red", "green", "blue", "alpha"), default))


def _lay_bounds(node):
    b = node.find("bounds") if node is not None else None
    if b is None:
        return None
    if b.get("left") is not None:
        x0, y0 = float(b.get("left")), float(b.get("top"))
        return x0, y0, float(b.get("right")) - x0, float(b.get("bottom")) - y0
    return float(b.get("x", 0)), float(b.get("y", 0)), float(b.get("width", 1)), float(b.get("height", 1))


def load_layout(name: str, scr_w: float = 1.0, scr_h: float = 1.0):
    """-> (screen bounds, view bounds, [(element shapes, bounds, alpha, flipx, blend)]) or None."""
    path = LAYOUTS / f"{name}.lay"
    if not path.exists():
        return None
    # ~scr0width~ / ~scr0height~: the screen's own size (the SVG's)
    text = path.read_bytes().replace(b"~scr0width~", str(scr_w).encode()).replace(b"~scr0height~", str(scr_h).encode())
    root = etree.fromstring(text)
    elements = {}
    for el in root.findall("element"):
        shapes = []
        for sh in el:
            if sh.tag in ("rect", "disk", "text"):
                shapes.append((sh.tag, _lay_bounds(sh), _lay_color(sh), sh.get("string", "")))
        elements[el.get("name")] = shapes
    view = root.find("view")
    screen = _lay_bounds(view.find("screen"))
    items = []
    for it in view.iter("element"):
        orient = it.find("orientation")
        items.append((elements[it.get("ref")], _lay_bounds(it), _lay_color(it)[3],
                      orient is not None and orient.get("flipx") == "yes", it.get("blend", "alpha")))
    vb = _lay_bounds(view)
    if vb is None:  # the union of everything in the view
        boxes = [screen] + [b for _, b, _, _, _ in items]
        x0, y0 = min(b[0] for b in boxes), min(b[1] for b in boxes)
        x1, y1 = max(b[0] + b[2] for b in boxes), max(b[1] + b[3] for b in boxes)
        vb = (x0, y0, x1 - x0, y1 - y0)
    return screen, vb, items


def layout_tint(layout):
    """A multiply element covering the screen: its colour, else None."""
    if not layout:
        return None
    screen, _, items = layout
    for shapes, b, _, _, blend in items:
        if blend == "multiply" and shapes and b == screen:
            return shapes[0][2][:3]
    return None


def render_layout(layout, out_png: Path, height: int) -> int:
    """Paints the layout's view (black, its art on top) -> width in pixels."""
    from PIL import ImageDraw, ImageFont, ImageChops
    _, (vx, vy, vw, vh), items = layout
    k = height / vh
    width = round(vw * k)
    img = Image.new("RGB", (width, height), (0, 0, 0))
    for shapes, (bx, by, bw, bh), alpha, flipx, blend in items:
        if blend == "multiply":
            continue
        # element space = the union of its shapes' bounds (shapes without bounds fill it)
        sb = [b for _, b, _, _ in shapes if b] or [(0, 0, 1, 1)]
        ex0, ey0 = min(b[0] for b in sb), min(b[1] for b in sb)
        ex1, ey1 = max(b[0] + b[2] for b in sb), max(b[1] + b[3] for b in sb)
        layer = Image.new("RGB", (width, height), (0, 0, 0))
        mask = Image.new("L", (width, height), 0)
        d, m = ImageDraw.Draw(layer), ImageDraw.Draw(mask)
        for kind, b, (r, g, bl, a), text in shapes:
            sx, sy, sw, sh = b or (ex0, ey0, ex1 - ex0, ey1 - ey0)
            fx0 = (sx - ex0) / (ex1 - ex0)
            fx1 = (sx + sw - ex0) / (ex1 - ex0)
            if flipx:
                fx0, fx1 = 1 - fx1, 1 - fx0
            x0 = (bx - vx + fx0 * bw) * k
            x1 = (bx - vx + fx1 * bw) * k
            y0 = (by - vy + (sy - ey0) / (ey1 - ey0) * bh) * k
            y1 = (by - vy + (sy + sh - ey0) / (ey1 - ey0) * bh) * k
            col = tuple(round(255 * v) for v in (r, g, bl))
            box = [x0, y0, max(x0, x1 - 1), max(y0, y1 - 1)]
            if kind == "rect":
                d.rectangle(box, fill=col)
                m.rectangle(box, fill=round(255 * a * alpha))
            elif kind == "disk":
                d.ellipse(box, fill=col)
                m.ellipse(box, fill=round(255 * a * alpha))
            else:
                font = ImageFont.load_default(size=max(8, round((y1 - y0) * 0.9)))
                tw = d.textlength(text, font=font)
                pos = ((x0 + x1 - tw) / 2, y0)
                d.text(pos, text, fill=col, font=font)
                m.text(pos, text, fill=round(255 * a * alpha), font=font)
        if blend == "add":
            img = ImageChops.add(img, Image.composite(layer, Image.new("RGB", img.size), mask))
        else:
            img = Image.composite(layer, img, mask)
    img.save(out_png)
    return width


def widen_viewbox(segs: bytes, layout, svg_w: float, svg_h: float) -> bytes:
    """The segs SVG's viewBox grown from the screen to the layout's view."""
    (sx, sy, sw, sh), (vx, vy, vw, vh), _ = layout
    kx, ky = svg_w / sw, svg_h / sh
    root = etree.fromstring(segs)
    vb = [(vx - sx) * kx, (vy - sy) * ky, vw * kx, vh * ky]
    root.set("viewBox", " ".join(f"{v:.4f}" for v in vb))
    root.set("width", f"{vb[2]:.4f}")
    root.set("height", f"{vb[3]:.4f}")
    return etree.tostring(root, xml_declaration=True, encoding="UTF-8")


# MAME clones whose zip has no ROM of its own: set -> parent set
PARENTS = {"epacman2r": "epacman2"}


def prepare(name: str) -> str:
    rom_file = GAMES[name][0]
    with zipfile.ZipFile(ROMS / f"{name}.zip") as zf:
        if rom_file in zf.namelist() or name not in PARENTS:
            rom = zf.read(rom_file)
        else:
            with zipfile.ZipFile(ROMS / f"{PARENTS[name]}.zip") as pz:
                rom = pz.read(rom_file)
        sound_rom = zf.read(SOUND_ROMS[name]) if name in SOUND_ROMS else None
        svg = svg_from_layout(name) if name in LAYOUT_SCREENS else zf.read(SVGS.get(name, f"{name}.svg"))
    out = DECOMPRESS / name
    out.mkdir(parents=True, exist_ok=True)
    write_container(out / f"{name}.brk", name, rom, sound_rom)
    w, h = svg_size(svg)
    layout = None if name in LAYOUT_SCREENS else load_layout(name, w, h)
    segs, count, pieces = build_segs_svg(svg, tint=layout_tint(layout))
    if layout:
        segs = widen_viewbox(segs, layout, w, h)
        width = render_layout(layout, out / "Background.png", SCREEN_HEIGHT)
    else:
        width = round(w / h * SCREEN_HEIGHT)
        Image.new("RGB", (width, SCREEN_HEIGHT), (0, 0, 0)).save(out / "Background.png")
    (out / f"{name}_segs.svg").write_bytes(segs)
    CONSOLE.mkdir(parents=True, exist_ok=True)
    photo = HERE / "hmcs40_consoles" / f"{name}.png"
    if photo.exists():
        console_from_photo(photo, CONSOLE / f"{name}.png")
    else:
        render_console(segs, CONSOLE / f"{name}.png", w, h)
    block, key = games_path_block(name, width)
    print(f"{name}: {count} segments ({pieces} with colour splits), screen {width}x{SCREEN_HEIGHT} -> {key}")
    return block


if __name__ == "__main__":
    sys.path.insert(0, str(HERE))
    blocks = [prepare(n) for n in (sys.argv[1:] or list(GAMES))]
    print("\n# games_path_rgds.py entries:\n" + "\n\n".join(blocks))
