"""Builds the Hartung Game Master games (core "UPD78C11",
source/BrickCores/GameMaster.*) as a small donor pack, to merge into the
main pack with merge_pack_games.py - the same way as build_gamepock_pack.py.
The LCD is 61x64 identical square dots, drawn here directly.

Inputs (CONVERT_ROM/rom/gamemaster/):
  gmaster.zip          the BIOS (MAME set "gmaster", 4096 bytes)
  <set>.zip            the 18 cartridges (MAME softlist names)
  consoles/<set>.png   the console art already finished (label + screen)
  consoles/folder.png  the folder's picture (blank screen)

Manufacturer "Hartung Game Master" (a folder inside Hartung in the menu).

Usage:  py build_gamemaster_pack.py [out.ykp]   (default: gamemaster_donor.ykp)
Then:   py merge_pack_games.py <base.ykp> gamemaster_donor.ykp --out <new.ykp> --replace
"""
import io
import struct
import sys
import zipfile
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SRC = HERE / "rom" / "gamemaster"

MANUFACTURER_HARTUNG_GM = 40
CLOCK = 2_000_000                     # MAME gmaster.cpp; x2 is an option (Settings > Game Master Speed)
W, H = 61, 64                         # LCD dots
PITCH, DOT = 18, 16                   # screen pixels per dot, lit square size
SCREEN = (W * PITCH, H * PITCH)       # 1098 x 1152
CONSOLE_CARD = (1368, 1026)           # like the Microvision consoles
CONSOLE_SIZE, CONSOLE_ATLAS = (640, 480), (1024, 512)
BG_ATLAS = (2048, 2048)               # no background art: only its size matters

# MAME's input port (gmaster.cpp, PA, active high in GameMaster.cpp).
BUTTONS = [
    ("btnRight", "JOY", 1, "RIGHT"), ("btnLeft", "JOY", 2, "LEFT"),
    ("btnDown", "JOY", 4, "DOWN"), ("btnUp", "JOY", 8, "UP"),
    ("btnB", "JOY", 16, "B"), ("btnA", "JOY", 32, "A"),
    ("btnSelect", "JOY", 64, "MINUS"), ("btnStart", "JOY", 128, "PLUS"),
]

GAMES = [  # set, display name, year (MAME hash/gmaster.xml; "199?" -> 1990)
    ("bublboy", "Bubble Boy", 1990), ("carracing", "Car Racing (Super F-1)", 1990),
    ("cgalaxy", "Continental Galaxy", 1990), ("dungeon", "Dungeon Adventure", 1990),
    ("fallbloc", "Falling Block", 1990), ("finitezn", "Finite Zone", 1990),
    ("gobang", "Go Bang", 1990), ("hspace", "Hyper Space", 1990),
    ("kungfu", "Kung Fu", 1990), ("moveit", "Move It", 1992),
    ("pinball", "Pin Ball", 1990), ("scastle", "Space Castle", 1990),
    ("soccer", "Soccer (3 on 3)", 1990), ("spaceinv", "Space Invader", 1992),
    ("swarrior", "Space Warrior", 1990), ("tankwar", "Tank War", 1990),
    ("tennis", "Tennis", 1990), ("uchamp", "Urban Champion", 1990),
]


def read_zip(names):
    for n in names:
        p = SRC / f"{n}.zip"
        if p.exists():
            with zipfile.ZipFile(p) as z:
                bins = [i for i in z.namelist() if not i.endswith("/")]
                return z.read(bins[0])
    return None


def container(bios: bytes, cart):
    lines = [f"input.{n}=direct;{port};{mask};1;{keys}" for n, port, mask, keys in BUTTONS]
    text = "\n".join(lines).encode()
    out = bytearray(b"BRK1") + bytes([8]) + b"UPD78C11" + struct.pack("<II", CLOCK, len(text)) + text
    blobs = [(b"rom", bios)] + ([(b"cart", cart)] if cart else [])
    out += struct.pack("<I", len(blobs))
    for name, data in blobs:
        out += bytes([len(name)]) + name + struct.pack("<I", len(data)) + data
    return bytes(out)


def segment_atlas():
    """One lit-dot tile all 3904 segments share (pos_tex is per segment, so
    they can all point at the same one), with a transparent pixel round it."""
    # 8 px of transparent margin, so the tile stays aligned through the
    # mipmap levels the app builds for it (switch_screen.cpp).
    atlas = Image.new("RGBA", (32, 32), (255, 255, 255, 0))
    atlas.paste(Image.new("RGBA", (DOT, DOT), (255, 255, 255, 255)), (8, 8))
    pos_tex = (8, 32 - 8 - DOT)  # bottom-up y, as convert_switch.py's atlases
    return atlas, pos_tex


def segments(pos_tex):
    off = (PITCH - DOT) // 2
    return [{"id0": x, "id1": y, "id2": 0,
             "pos_scr_x": x * PITCH + off, "pos_scr_y": y * PITCH + off,
             "pos_tex_x": pos_tex[0], "pos_tex_y": pos_tex[1], "size_tex_x": DOT, "size_tex_y": DOT,
             "color_index": 0, "screen": 0}
            for y in range(H) for x in range(W)]


def console_texture(s):
    """convert_switch.py's visual_console_data() on a card like Microvision's:
    the photo centred on white, 640x480, mirrored, in a 1024x512 atlas. The
    game's capture (screens/<set>.png), if any, goes in the LCD window."""
    finished = SRC / "consoles" / f"{s}.png"  # the photo with its own label and screen
    photo = Image.open(finished).convert("RGBA")
    card = Image.new("RGBA", CONSOLE_CARD, (255, 255, 255, 255))
    s = min(CONSOLE_CARD[0] * 0.92 / photo.width, CONSOLE_CARD[1] * 0.92 / photo.height)
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
        seg_blob = b"".join(struct.pack("<BBBiiHHHHBB", s["id0"], s["id1"], s["id2"], s["pos_scr_x"], s["pos_scr_y"],
                                        s["pos_tex_x"], s["pos_tex_y"], s["size_tex_x"], s["size_tex_y"],
                                        s["color_index"], s["screen"]) for s in g["segments"])
        f = [*string(g["display_name"]), *string(g["ref"]), *string(g["date"]), *chunk(g["rom"]), 0, 0,
             *string(g["path_segment"]), chunk(seg_blob)[0], len(g["segments"]), *u16s(g["segment_info"]),
             *string(g["path_background"]), *u16s(g["background_info"]),
             *string(g["path_console"]), *u16s(g["console_info"]), MANUFACTURER_HARTUNG_GM]
        entries.append(struct.pack("<" + "I" * 25, *f))
    fentries = []
    for name, blob in sorted(files.items()):
        n = string(name)
        d = chunk(blob)
        fentries.append(struct.pack("<IIII", n[0], n[1], d[0], d[1]))
    header = struct.pack("<IIIIIIIII", 0x31504B59, 3, 2, 5, len(games), len(fentries), games_off, files_off, data_off)
    Path(out_path).write_bytes(header + b"".join(entries) + b"".join(fentries) + bytes(data))


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "gamemaster_donor.ykp"
    bios = read_zip(["gmaster"])
    if not bios or len(bios) != 4096:
        sys.exit(f"BIOS not found: {SRC / 'gmaster.zip'} (4096 bytes)")
    atlas, pos_tex = segment_atlas()
    segs = segments(pos_tex)
    seg_png = png_bytes(atlas)
    games, files = [], {}
    for s, name, year in GAMES:
        cart = read_zip([s])
        if cart is None:
            print(f"{s}: cartridge not found in {SRC} - skipped")
            continue
        key = f"hgm_{s}"
        files[f"segment_{key}.png"] = seg_png
        console, console_info = console_texture(s)
        files[f"console_{key}.png"] = png_bytes(console)
        games.append({
            "display_name": name, "ref": "HGM" + s.upper(), "date": f"{year}-01-01", "rom": container(bios, cart),
            "path_segment": f"gfx/segment_{key}.png", "segments": segs,
            "segment_info": [32, 32, 1, 0, SCREEN[0], SCREEN[1]],
            "path_background": "", "background_info": [BG_ATLAS[0], BG_ATLAS[1], 0, 0, SCREEN[0], SCREEN[1], 0, 0, 0],
            "path_console": f"gfx/console_{key}.png", "console_info": console_info,
        })
        print(f"{s}: {name} ({year}) -> HGM{s.upper()}, cartridge {len(cart)} bytes")
    # The folder's picture in the menu (Hartung > Game Master): switch_menu.cpp's folder_art().
    if (SRC / "consoles" / "folder.png").exists():
        files["console_hgm_folder.png"] = png_bytes(console_texture("folder")[0])
    write_pack(games, files, out)
    print(f"OK -> {out} ({len(games)} games)")


if __name__ == "__main__":
    main()
