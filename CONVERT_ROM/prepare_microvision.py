"""Prepares the Milton Bradley Microvision cartridges for convert_switch.py,
run on the TMS1100 core (source/BrickCores/TMS1100.*).

Each cartridge ROM (a 2048-byte TMS1100 dump, loose .bin or inside a .zip,
any file name) is found in rom/roms/ and rom/microvision/ and identified by
its sha1 against microvision_carts.json (MAME's microvision software list,
CC0). For each one this writes rom/decompress/<set>/:
  <set>.brk        ".brk" container for the "TMS1100" core: clock = the
                   cartridge's oscillator / 6, the console wiring (the same
                   for every cartridge, found by running Pinball), the paddle
                   if the cartridge has one, the keys it uses ("mv.butmask")
                   and their bindings, named after the cartridge's keypad
                   overlay
  <set>_segs.svg   the 16x16 LCD dots, placed in the overlay's screen window
                   (dot r,c is display byte r*2 + c//8, bit c%8: "<b>.<bit>.0")
  Background.png   the cartridge's screen overlay (microvision_overlays/,
                   MAME artwork), its window filled with the LCD's grey
and rom/console/<set>.png - the photo of the console with that cartridge
(microvision_consoles/<set>.png) on the white card. Prints the
games_path_rgds.py entries.

Usage:  py prepare_microvision.py [set ...]     (default: every one found)
"""
import hashlib
import io
import json
import re
import struct
import sys
import zipfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
ROM_DIRS = [HERE / "rom" / "roms", HERE / "rom" / "microvision"]
DECOMPRESS = HERE / "rom" / "decompress"
CONSOLE = HERE / "rom" / "console"
OVERLAYS = HERE / "microvision_overlays"
PHOTOS = HERE / "microvision_consoles"
CARTS = HERE / "microvision_carts.json"

SCREEN_HEIGHT = 1080
MAKER = "Milton Bradley Microvision"
LCD_GREY = (186, 192, 176)

# The console's wiring (TMS1100 pins), the same for every cartridge.
WIRING = {
    "mv.lcd_lp": "R6",      # HLCD 0488 latch pulse
    "mv.lcd_dc": "R7",      # data clock
    "mv.lcd_data": "O0",    # O0-O3
    "mv.speaker": "R0",     # piezo, between R0 and R1
}
# Paddle: R2 starts the RC charge, read on K8. Pinball's paddle spans the
# screen between 700 and 1300 us.
PADDLE = "R2,8,700,1300"

# Keypad: key n = row n//3 (top to bottom), column n%3 (left to right);
# column c is strobed by R(10-c), row r reads on K bit 8 >> r.
# Per cartridge: key -> (name, default Switch keys), from the keypad
# overlays. Only the keys the cartridge uses (MAME's butmask) are listed.
KEYS = {
    "pinball":  {3: ("BALLS", "Y"), 4: ("SPEED", "X"), 5: ("PADDLE", "B"), 10: ("GO", "A")},
    "blckbstr": {3: ("BALLS", "Y"), 4: ("SPEED", "X"), 5: ("PADDLE", "B"), 10: ("GO", "A")},
    "sprblock": {3: ("PLUS", "A"), 5: ("DIAMOND", "B")},
    "alienrdr": {3: ("FIRE_LEFT", "Y"), 4: ("FIRE_CENTER", "X"), 5: ("FIRE_RIGHT", "A"),
                 9: ("GAME", "L"), 10: ("SKILL", "R"), 11: ("GO", "B")},
    "baseball": {3: ("PLAYERS", "L"), 4: ("SKILL_LEVEL", "R"), 5: ("KEY_3", "A"),
                 9: ("COUNT", "Y"), 10: ("SCORE", "X"), 11: ("RESET", "B")},
    "bowling":  {0: ("PLAYERS", "L"), 2: ("SPEED", "R"), 4: ("GO", "A"), 10: ("BOWL", "B")},
    "connect4": {3: ("PLAYERS", "L"), 4: ("SKILL", "R"), 5: ("GO", "A"),
                 9: ("CONF", "Y"), 10: ("DROP", "B"), 11: ("RESET", "X")},
    "cosmichu": {0: ("FIRE_LEFT", "ZL"), 1: ("TOP_CENTER", "Y"), 2: ("FIRE_RIGHT", "ZR"),
                 4: ("UP", "UP"), 6: ("LEFT", "LEFT"), 7: ("CENTER", "A"), 8: ("RIGHT", "RIGHT"),
                 10: ("DOWN", "DOWN")},
    "mindbstr": {0: ("1", "ZL"), 1: ("2", "UP"), 2: ("3", "ZR"), 3: ("4", "LEFT"), 4: ("5", "DOWN"),
                 5: ("6", "RIGHT"), 6: ("7", "Y"), 7: ("8", "X"), 8: ("9", "A"),
                 9: ("A_C", "L"), 10: ("GO", "B"), 11: ("GAME", "R")},
    "phsrstrk": {0: ("SIZE", "L"), 1: ("SPEED", "R"), 2: ("TARGETS", "Y"), 4: ("GO", "A"),
                 6: ("LEFT", "LEFT"), 7: ("FIRE", "UP|B"), 8: ("RIGHT", "RIGHT")},
    "vegasslt": {0: ("BANK", "Y"), 1: ("GO", "A"), 2: ("ODDS", "X"),
                 6: ("TAKE", "L"), 7: ("DISPLAY", "R"), 8: ("PULL", "B")},
    "seaduel":  {0: ("MOVE_UP", "UP"), 1: ("MOVE_RIGHT", "RIGHT"), 2: ("GAME", "ZL"),
                 3: ("MOVE_LEFT", "LEFT"), 4: ("MOVE_DOWN", "DOWN"), 5: ("GO", "ZR"),
                 6: ("FIRE_UP", "X"), 7: ("FIRE_RIGHT", "A"), 8: ("SKILL_SCORE", "L"),
                 9: ("FIRE_LEFT", "Y"), 10: ("FIRE_DOWN", "B"), 11: ("CLEAR", "R")},
}
KNOB_KEYS = ("LEFT|ZL", "RIGHT|ZR")

# Colour of each cartridge's keys (from photos of the console with it), for
# the on-screen keypad (switch/source/input/onscreen_keypad.cpp): "mv.keycolor"
# RRGGBB, and "mv.keyborder=1" for keys drawn with a white outline.
KEY_COLORS = {
    "alienrdr": "346A81",
    "baseball": "366F38",
    "blckbstr": "263F91",
    "bowling": "8D1921",
    "connect4": "2D1C7B",
    "cosmichu": "552C5B",
    "mindbstr": "172F70",
    "phsrstrk": "552356",
    "pinball": "A41D35",
    "seaduel": "7AA0C9",
    "sprblock": "16181C",
    "vegasslt": "3A6C2A",
}
KEY_BORDER = {"sprblock"}

# Where the 16x16 LCD sits under each overlay (overlay units): the size and
# place of the screen in real captures of every cartridge (the web's
# Microvision screenshots), carried over onto these overlays by matching
# their windows - the screen fills each window edge to edge.
SCREEN = {
    "alienrdr": (158.0, 180.5, 955.0, 1037.5),
    "baseball": (160.0, 164.5, 958.0, 1043.5),
    "blckbstr": (163.0, 166.6, 944.0, 995.8),
    "bowling": (163.0, 181.7, 950.0, 966.6),
    "connect4": (149.0, 194.0, 964.0, 982.6),
    "cosmichu": (173.0, 169.0, 957.0, 1030.0),
    "mindbstr": (166.0, 177.0, 945.0, 1008.0),
    "phsrstrk": (168.0, 174.6, 950.0, 1018.4),
    "pinball": (172.0, 179.0, 951.0, 1039.0),
    "seaduel": (183.8, 196.8, 903.4, 955.4),
    "sprblock": (179.0, 174.0, 943.0, 995.0),
    "vegasslt": (151.0, 234.2, 970.0, 940.5),
}
# Overlays replaced here: Connect Four's board with one round hole over
# each 2x2 block of dots, as on the real console. Bowling's lane opened
# so the columns next to the red frame show whole, as on the console. Sea
# Duel's window top brought down to the SHIP box, as on the real console,
# so the 16x16 grid has the same LCD margin on all four sides.
OVERLAY_FILE = {"connect4": "connectfour_overlay_round.png",
                "seaduel": "seaduel_overlay_fit.png",
                "bowling": "bowling_overlay_fit.png"}


def load_carts():
    data = json.loads(CARTS.read_text(encoding="utf-8"))
    return {c["set"]: c for c in data["carts"] if c["cpu"] == "TMS1100" and c.get("sha1")}


def find_roms(carts):
    """{set: rom bytes} for every known cartridge found, whatever its file name."""
    by_sha = {c["sha1"]: s for s, c in carts.items()}
    found = {}

    def offer(data: bytes, where: str):
        s = by_sha.get(hashlib.sha1(data).hexdigest())
        if s and s not in found:
            found[s] = data
        elif len(data) == 2048 and not s:
            print(f"  {where}: a 2048-byte file that matches no cartridge in MAME's list (bad dump?)")

    for d in ROM_DIRS:
        if not d.exists():
            continue
        for p in sorted(d.iterdir()):
            if p.suffix.lower() == ".bin" and p.stat().st_size == 2048:
                offer(p.read_bytes(), p.name)
            elif p.suffix.lower() == ".zip":
                try:
                    with zipfile.ZipFile(p) as z:
                        for n in z.namelist():
                            if z.getinfo(n).file_size == 2048:
                                offer(z.read(n), f"{p.name}:{n}")
                except zipfile.BadZipFile:
                    pass
    return found


def butmask_keys(mame_mask: int) -> int:
    """MAME's butmask (bit col*4 + K bit, set = unused) -> mv.butmask (bit = key)."""
    out = 0
    for col in range(3):
        for kb in range(4):
            if (mame_mask >> (col * 4 + kb)) & 1:
                out |= 1 << ((3 - kb) * 3 + col)
    return out


def write_container(path: Path, s: str, c: dict, rom: bytes):
    opts = dict(WIRING)
    for n in range(12):
        # R8 strobes the right-hand column, R10 the left (Pinball: BALLS, on
        # the left, changes the balls digit under BALLS only like this)
        opts[f"mv.key.{n}"] = f"R{10 - n % 3},{8 >> (n // 3)}"
    opts["mv.butmask"] = str(butmask_keys(c.get("butmask") or 0))
    if s in KEY_COLORS:
        opts["mv.keycolor"] = KEY_COLORS[s]
    if s in KEY_BORDER:
        opts["mv.keyborder"] = "1"
    lines = [f"{k}={v}" for k, v in opts.items()]
    if c.get("paddle"):
        lines.append(f"mv.paddle={PADDLE}")
    for n, (name, keys) in sorted(KEYS[s].items()):
        lines.append(f"input.btn{name}=direct;BTN;{n};1;{keys}")
    if c.get("paddle"):
        lines.append(f"input.btnKNOB_LEFT=direct;KNOB;0;1;{KNOB_KEYS[0]}")
        lines.append(f"input.btnKNOB_RIGHT=direct;KNOB;1;1;{KNOB_KEYS[1]}")
    # power off/on, as on the VFD tabletops (no reset key on the console)
    lines.append("input.btnReset=direct;RES;0;1;PLUS+MINUS")
    text = "\n".join(lines).encode("utf-8")
    out = bytearray(b"BRK1") + bytes([7]) + b"TMS1100"
    out += struct.pack("<II", c["clock_osc"] // 6, len(text)) + text
    blobs = [(b"rom", rom)]
    if c.get("pla") == 1:
        # Output PLA type 1: the four LCD data lines (O0-O3) come out in
        # reverse order (checked on Vegas Slots: its score digits only form
        # whole like this). Type 0 passes A straight through - no blob.
        rev = lambda v: int(f"{v & 15:04b}"[::-1], 2)
        blobs.append((b"opla", bytes(rev(i) for i in range(32))))
    out += struct.pack("<I", len(blobs))
    for name, data in blobs:
        out += bytes([len(name)]) + name + struct.pack("<I", len(data)) + data
    path.write_bytes(bytes(out))


def visible_outline(part, box, dot):
    """The part of a dot the overlay's window shows, as an SVG path, or None
    when that part is just a rectangle. Built from the visible runs of each
    pixel row (rows with the same runs merged), so whatever the overlay
    prints over the dot - a frame edge, Connect Four's round holes,
    Baseball's white lines - stays on top of it."""
    x0, y0, x1, y1 = dot
    px = part.load()
    rows = []
    for yy in range(part.height):
        runs, start = [], None
        for xx in range(part.width + 1):
            on = xx < part.width and px[xx, yy]
            if on and start is None:
                start = xx
            elif not on and start is not None:
                runs.append((start, xx))
                start = None
        rows.append(tuple(runs))
    used = [r for r in rows if r]
    if not used:
        return None
    if len(set(used)) == 1 and len(used[0]) == 1:
        return None  # a rectangle: the bbox clip does it
    cx = lambda v: min(max(box[0] + v, x0), x1)
    cy = lambda v: min(max(box[1] + v, y0), y1)
    parts = []
    yy = 0
    while yy < len(rows):
        runs, top = rows[yy], yy
        while yy + 1 < len(rows) and rows[yy + 1] == runs:
            yy += 1
        yy += 1
        ya, yb = cy(top), cy(yy)
        if yb <= ya:
            continue
        for a, b in runs:
            xa, xb = cx(a), cx(b)
            if xb > xa:
                parts.append(f"M{xa:.2f},{ya:.2f} H{xb:.2f} V{yb:.2f} H{xa:.2f} Z")
    return " ".join(parts) or None


def overlay_images(c: dict, out_dir: Path, s: str):
    """Background.png (overlay over the LCD grey) + the dots SVG, in overlay units."""
    ov = c["overlay"]
    w, h = ov["size"]
    sx, sy, sw, sh = SCREEN.get(s, ov["screen"])
    art = Image.open(OVERLAYS / OVERLAY_FILE.get(s, ov["file"])).convert("RGBA").resize((w, h), Image.LANCZOS)
    # the LCD's grey under the whole overlay: nothing black ever shows through
    bg = Image.new("RGBA", (w, h), LCD_GREY + (255,))
    bg.alpha_composite(art)
    width = round(w / h * SCREEN_HEIGHT)
    bg.convert("RGB").resize((width, SCREEN_HEIGHT), Image.LANCZOS).save(out_dir / "Background.png")

    cw, ch = sw / 16, sh / 16
    pad = 0.12
    # On the console the overlay lies on the LCD; here the dots are drawn
    # over it. So each dot is cut to the part of it the overlay's window
    # shows (Vegas Slots' reels scroll through rows its frame hides), and a
    # dot that's (nearly) all under the frame isn't drawn at all.
    alpha = art.getchannel("A").point(lambda v: 255 if v < 128 else 0)  # 255 = window
    rects = []
    hidden = clipped = 0
    for r in range(16):
        for col in range(16):
            x0, y0 = sx + col * cw + cw * pad, sy + r * ch + ch * pad
            x1, y1 = x0 + cw * (1 - 2 * pad), y0 + ch * (1 - 2 * pad)
            box = (max(0, int(x0)), max(0, int(y0)), min(w, int(x1 + 0.999)), min(h, int(y1 + 0.999)))
            if box[2] <= box[0] or box[3] <= box[1]:
                hidden += 1
                continue
            part = alpha.crop(box)
            seen = sum(part.getdata()) / 255 / (part.width * part.height)
            if seen < 0.15:
                hidden += 1
                continue
            title = f'<title>{r * 2 + col // 8}.{col % 8}.0</title>'
            if seen < 0.99:
                clipped += 1
                shape = visible_outline(part, box, (x0, y0, x1, y1))
                if shape:
                    rects.append(f'<g>{title}<path d="{shape}" style="fill:#282c28"/></g>')
                    continue
                bx = part.getbbox()
                x0, y0 = max(x0, box[0] + bx[0]), max(y0, box[1] + bx[1])
                x1, y1 = min(x1, box[0] + bx[2]), min(y1, box[1] + bx[3])
            rects.append(f'<g>{title}'
                         f'<rect x="{x0:.2f}" y="{y0:.2f}" width="{x1 - x0:.2f}" '
                         f'height="{y1 - y0:.2f}" style="fill:#282c28"/></g>')
    svg = (f'<?xml version="1.0" encoding="UTF-8"?>\n'
           f'<svg xmlns="http://www.w3.org/2000/svg" '
           f'xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" '
           f'width="{w}" height="{h}" viewBox="0 0 {w} {h}">'
           f'<g id="layer_segs" inkscape:label="segs" inkscape:groupmode="layer">'
           + "".join(rects) + "</g></svg>")
    (out_dir / f"{s}_segs.svg").write_text(svg, encoding="utf-8")
    if hidden or clipped:
        print(f"  {s}: {hidden} dots under the overlay's frame, {clipped} cut by its edge")
    return width


def console_card(s: str, out_png: Path, card=(1368, 1026), fill: float = 0.92):
    photo = PHOTOS / f"{s}.png"
    if not photo.exists():
        print(f"  {s}: no photo in {PHOTOS.name}/ - plain card")
        Image.new("RGB", card, (255, 255, 255)).save(out_png)
        return
    device = Image.open(photo).convert("RGBA")
    # white-background photos: treat near-white as transparent
    px = device.load()
    for y in range(device.height):
        for x in range(device.width):
            r, g, b, a = px[x, y]
            if r > 240 and g > 240 and b > 240:
                px[x, y] = (255, 255, 255, 0)
    bbox = device.getchannel("A").getbbox()
    if bbox:
        device = device.crop(bbox)
    scale = min(card[0] * fill / device.width, card[1] * fill / device.height)
    device = device.resize((round(device.width * scale), round(device.height * scale)), Image.LANCZOS)
    out = Image.new("RGB", card, (255, 255, 255))
    out.paste(device, ((card[0] - device.width) // 2, (card[1] - device.height) // 2), device)
    out.save(out_png)


def display_name(c: dict) -> str:
    t = c["title"].replace(" (TMS1100 version)", "")
    return {"Cosmic hunter": "Cosmic Hunter"}.get(t, t)


def games_path_block(s: str, c: dict, width: int):
    from source.manufacturer_ids import MANUFACTURER_BY_NAME
    display = display_name(c)
    key = "MV_" + re.sub(r"[^A-Za-z0-9]+", "_", display).strip("_")
    d = s
    return "\n".join([
        f'            , "{key}" :',
        f'                    {{ "ref" : "MV{s.upper()}"',
        f'                    , "manufacturer" : {MANUFACTURER_BY_NAME[MAKER]}',
        f'                    , "display_name" : "{display}"',
        "                    , \"Rom\" : r'.\\rom\\decompress\\" + d + "\\" + d + ".brk'",
        "                    , \"Visual\" : [r'.\\rom\\decompress\\" + d + "\\" + d + "_segs.svg']",
        "                    , \"Background\" : [r'.\\rom\\decompress\\" + d + "\\Background.png']",
        '                    , "alpha_bright" : 1.0',
        '                    , "fond_bright" : 1.0',
        '                    , "shadow" : False',
        "                    , \"console\" : r'.\\rom\\console\\" + d + ".png'",
        f'                    , "transform_visual" : [[[{width},0,0],[{SCREEN_HEIGHT},0,0]]]',
        f'                    , "date" : "{c["year"]}-01-01"',
        "                }",
    ]), key


def prepare(s: str, c: dict, rom: bytes):
    out = DECOMPRESS / s
    out.mkdir(parents=True, exist_ok=True)
    write_container(out / f"{s}.brk", s, c, rom)
    width = overlay_images(c, out, s)
    CONSOLE.mkdir(parents=True, exist_ok=True)
    console_card(s, CONSOLE / f"{s}.png")
    block, key = games_path_block(s, c, width)
    print(f"{s}: {display_name(c)} ({c['year']}), {c['clock_osc'] // 6} Hz, "
          f"paddle {'yes' if c.get('paddle') else 'no'}, {len(KEYS[s])} keys -> {key}")
    return block, key


if __name__ == "__main__":
    sys.path.insert(0, str(HERE))
    carts = load_carts()
    roms = find_roms(carts)
    want = sys.argv[1:] or sorted(roms)
    blocks = []
    for s in want:
        if s not in carts:
            sys.exit(f"{s}: not a TMS1100 cartridge in {CARTS.name}")
        if s not in roms:
            print(f"{s}: ROM not found in {', '.join(d.name for d in ROM_DIRS)}/ - skipped")
            continue
        blocks.append(prepare(s, carts[s], roms[s])[0])
    print("\n# games_path_rgds.py entries:\n" + "\n\n".join(blocks))
