"""Builds the Tiger Game.com games (core "SM8521", source/BrickCores/GameCom.*)
as a small donor pack, to merge into the main pack with merge_pack_games.py.
The LCD (200x160, five shades) is drawn by the app as a picture, so a game
has no segment art - one dummy segment only.

Inputs (CONVERT_ROM/rom/gamecom/):
  gamecom.zip          the BIOS (MAME set "gamecom": gamecom/internal.bin, gamecom/external.bin)
  *.tgc                cartridges, named "Title (year) (...).tgc"
  tiger_gamecom.webp   the console photo, for the console art
  tiger_gamecom_mask.png  the same photo with its background removed (its outline)
  screens/<file>.png   optional: a 200x160 capture, put in the photo's screen

Manufacturer "Tiger Game.com" (41, a folder inside Tiger in the menu).
Each game boots straight into its cartridge (GameCom.cpp taps the menu).

Usage:  py build_gamecom_pack.py [out.ykp]     (default: gamecom_donor.ykp)
Then:   py merge_pack_games.py <base.ykp> gamecom_donor.ykp --out <new.ykp> --replace
"""
import io
import re
import struct
import sys
import zipfile
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter

HERE = Path(__file__).resolve().parent
SRC = HERE / "rom" / "gamecom"

MANUFACTURER_TIGER_GC = 41
CLOCK = 5_529_600                      # MAME gamecom.cpp: 11.0592 MHz XTAL / 2
SCREEN = (1000, 800)                   # 200x160 at 5 units per pixel
CONSOLE_CARD = (1368, 1026)
CONSOLE_SIZE, CONSOLE_ATLAS = (640, 480), (1024, 512)
BG_ATLAS = (2048, 2048)
LCD_WINDOW = (431, 136, 347, 278)      # the screen in tiger_gamecom.webp (1200x675)
CONSOLE_OUTLINE = (100, 64, 1123, 648)  # the console in it
SHADES = [(178, 192, 170), (130, 140, 126), (90, 97, 91), (54, 58, 58), (32, 34, 38)]

# MAME's IN0/IN1/IN2 rows (gamecom.cpp, active low).
BUTTONS = [
    ("btnUp", "IN0", 0x01, "UP"), ("btnDown", "IN0", 0x02, "DOWN"),
    ("btnLeft", "IN0", 0x04, "LEFT"), ("btnRight", "IN0", 0x08, "RIGHT"),
    ("btnMenu", "IN0", 0x10, "MINUS"), ("btnPause", "IN0", 0x20, "PLUS"),
    ("btnSound", "IN0", 0x40, "L"), ("btnA", "IN0", 0x80, "A"),
    ("btnB", "IN1", 0x01, "B"), ("btnC", "IN1", 0x02, "X"),
    ("btnD", "IN2", 0x02, "Y"),
]

# MAME softlist names, for the refs ("TGC" + set).
SETS = {
    "Batman and Robin": "batmanr", "CastleVania - Symphony of the Night": "sotn",
    "Centipede": "centiped", "Duke Nukem 3D": "dukenuke", "Fighters MegaMix": "fightmix",
    "Frogger": "frogger", "Henry": "henry", "Indy 500": "indy500", "Jeopardy!": "jeopardy",
    "Lights Out": "lightout", "Lost World, The - Jurassic Park": "lostwrld", "Monopoly": "monopoly",
    "Mortal Kombat Trilogy": "mktrilgy", "Quiz Wiz - Cyber Trivia": "quizwiz",
    "Resident Evil 2": "re2", "Scrabble": "scrabble", "Sonic Jam": "sonicjam",
    "Tiger Casino": "tcasino", "Tiger Web Link": "weblink", "Wheel of Fortune": "wheelfor",
    "Wheel of Fortune 2": "wheelfo2", "Williams Arcade Classics": "arcadecl",
}
NAMES = {  # display names, where the file's differs
    "Lost World, The - Jurassic Park": "The Lost World: Jurassic Park",
    "CastleVania - Symphony of the Night": "Castlevania: Symphony of the Night (Prototype)",
    "Quiz Wiz - Cyber Trivia": "Quiz Wiz: Cyber Trivia",
    "Fighters MegaMix": "Fighters Megamix",
}


def container(irom: bytes, kernel: bytes, cart: bytes):
    lines = [f"input.{n}=direct;{port};{mask};1;{keys}" for n, port, mask, keys in BUTTONS]
    text = "\n".join(lines).encode()
    out = bytearray(b"BRK1") + bytes([6]) + b"SM8521" + struct.pack("<II", CLOCK, len(text)) + text
    blobs = [(b"rom", irom), (b"kernel", kernel), (b"cart", cart)]
    out += struct.pack("<I", len(blobs))
    for name, data in blobs:
        out += bytes([len(name)]) + name + struct.pack("<I", len(data)) + data
    return bytes(out)


def console_texture(shot):
    """The photo with the game's capture in its screen, centred on a white
    card, 640x480, mirrored, in a 1024x512 atlas (as build_gamepock_pack.py)."""
    photo = Image.open(SRC / "tiger_gamecom.webp").convert("RGBA")
    # The console cut out (tiger_gamecom_mask.png, the same photo with its
    # background removed - its alpha, scaled up): outside it, white.
    cut = SRC / "tiger_gamecom_mask.png"
    if cut.exists():
        mask = Image.open(cut).convert("RGBA").split()[-1].resize(photo.size, Image.LANCZOS)
    else:
        mask = Image.new("L", photo.size, 0)
        ImageDraw.Draw(mask).rounded_rectangle(CONSOLE_OUTLINE, radius=65, fill=255)
        mask = mask.filter(ImageFilter.GaussianBlur(1.2))
    photo = Image.composite(photo, Image.new("RGBA", photo.size, (255, 255, 255, 255)), mask)
    if shot is not None:
        x, y, w, h = LCD_WINDOW
        photo.paste(shot.convert("RGBA").resize((w, h), Image.NEAREST), (x, y))
    card = Image.new("RGBA", CONSOLE_CARD, (255, 255, 255, 255))
    s = min(CONSOLE_CARD[0] * 0.96 / photo.width, CONSOLE_CARD[1] * 0.96 / photo.height)
    p = photo.resize((round(photo.width * s), round(photo.height * s)), Image.LANCZOS)
    card.alpha_composite(p, ((CONSOLE_CARD[0] - p.width) // 2, (CONSOLE_CARD[1] - p.height) // 2))
    img = card.transpose(Image.FLIP_LEFT_RIGHT).resize(CONSOLE_SIZE, Image.LANCZOS)
    atlas = Image.new("RGBA", CONSOLE_ATLAS, (0, 0, 0, 0))
    atlas.paste(img, (0, 0))
    info = [CONSOLE_ATLAS[0], CONSOLE_ATLAS[1], 0, CONSOLE_ATLAS[1] - CONSOLE_SIZE[1], CONSOLE_SIZE[0], CONSOLE_SIZE[1]]
    return atlas, info


def png_bytes(img):
    b = io.BytesIO()
    img.save(b, "PNG", optimize=True)
    return b.getvalue()


def write_pack(games, files, out_path):
    """The .ykp layout of convert_switch.py's write_rom_pack_v1()."""
    header_size, entry_size, file_size = 36, 100, 16
    games_off = header_size
    files_off = games_off + len(games) * entry_size
    data_off = files_off + len(files) * file_size
    data = bytearray()

    def chunk(b):
        off = data_off + len(data)
        data.extend(b)
        return off, len(b)

    def string(s):
        return chunk(s.encode("utf-8"))

    def u16s(v):
        off, _ = chunk(struct.pack("<" + "H" * len(v), *v)) if v else (0, 0)
        return off, len(v)

    entries = []
    for g in games:
        seg_blob = b"".join(struct.pack("<BBBiiHHHHBB", *s) for s in g["segments"])
        f = [*string(g["display_name"]), *string(g["ref"]), *string(g["date"]), *chunk(g["rom"]), 0, 0,
             *string(g["path_segment"]), chunk(seg_blob)[0], len(g["segments"]), *u16s(g["segment_info"]),
             *string(g["path_background"]), *u16s(g["background_info"]),
             *string(g["path_console"]), *u16s(g["console_info"]), MANUFACTURER_TIGER_GC]
        entries.append(struct.pack("<" + "I" * 25, *f))
    fentries = []
    for name, blob in sorted(files.items()):
        n = string(name)
        d = chunk(blob)
        fentries.append(struct.pack("<IIII", n[0], n[1], d[0], d[1]))
    header = struct.pack("<IIIIIIIII", 0x31504B59, 3, 2, 5, len(games), len(fentries), games_off, files_off, data_off)
    Path(out_path).write_bytes(header + b"".join(entries) + b"".join(fentries) + bytes(data))


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "gamecom_donor.ykp"
    with zipfile.ZipFile(SRC / "gamecom.zip") as z:
        irom = z.read("gamecom/internal.bin")
        kernel = z.read("gamecom/external.bin")
    # One transparent 1x1 segment, never lit (GameCom::get_segments_state).
    seg_atlas = Image.new("RGBA", (32, 32), (255, 255, 255, 0))
    seg_png = png_bytes(seg_atlas)
    segs = [(255, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0)]  # word 0: never lit
    games, files, seen = [], {}, set()
    for p in sorted(SRC.glob("*.tgc")):
        m = re.match(r"(.+?) \((\d{4}|Prototype)\)", p.stem)
        title = m.group(1) if m else p.stem
        year = m.group(2) if m and m.group(2).isdigit() else "1997"
        s = SETS.get(title, re.sub(r"[^a-z0-9]", "", title.lower())[:8])
        if s in seen:
            print(f"{p.name}: duplicate of {s} - skipped")
            continue
        seen.add(s)
        key = f"tgc_{s}"
        shot_path = SRC / "screens" / f"{s}.png"
        shot = Image.open(shot_path) if shot_path.exists() else None
        console, console_info = console_texture(shot)
        files[f"segment_{key}.png"] = seg_png
        files[f"console_{key}.png"] = png_bytes(console)
        games.append({
            "display_name": NAMES.get(title, title), "ref": "TGC" + s.upper(), "date": f"{year}-01-01",
            "rom": container(irom, kernel, p.read_bytes()),
            "path_segment": f"gfx/segment_{key}.png", "segments": segs,
            "segment_info": [32, 32, 1, 0, SCREEN[0], SCREEN[1]],
            "path_background": "", "background_info": [BG_ATLAS[0], BG_ATLAS[1], 0, 0, SCREEN[0], SCREEN[1], 0, 0, 0],
            "path_console": f"gfx/console_{key}.png", "console_info": console_info,
        })
        print(f"{p.name} -> TGC{s.upper()} ({year})")
    # The folder's picture in the menu (Tiger > Game.com): the console, screen blank.
    files["console_tgc_folder.png"] = png_bytes(console_texture(None)[0])
    write_pack(games, files, out)
    print(f"OK -> {out} ({len(games)} games)")


if __name__ == "__main__":
    main()
