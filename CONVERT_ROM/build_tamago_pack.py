"""Builds the Tamagotchi TamaTown Tama-Go and Tamagotchi Friends (Europe)
(core "TAMAGO", source/BrickCores/Tamago.*) as a small donor pack, to merge
into the main pack with merge_pack_games.py. The LCD and its icons are drawn
by the app (switch_screen.cpp, refs "TMG..."), so a game has no segment art -
one dummy segment only.

Inputs (CONVERT_ROM/rom/tamago/):
  tamago.bin                    TamaTown Tama-Go ROM (640 KB)
  Tamagotchi_Friends_EU.bin     Tamagotchi Friends, European ROM (768 KB)
  figures/(NN)_Tama-Go_<name>.bin   the Gotchi figure dumps, in number order
                                    (fig01.. in the Tama-Go's container;
                                    Settings > Gotchi Figure lists them so)
  art_tamago.png, art_friends.png   the console pictures

Manufacturer Bandai (11), no folder.

Usage:  py build_tamago_pack.py [out.ykp]     (default: tamago_donor.ykp)
Then:   py merge_pack_games.py <base.ykp> tamago_donor.ykp --out <new.ykp> --replace
"""
import io
import struct
import sys
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SRC = HERE / "rom" / "tamago"

MANUFACTURER_BANDAI = 11
CLOCK = 4_000_000
SCREEN = (1152, 1096)                  # the core's picture, 288x274, x4
CONSOLE_SIZE, CONSOLE_ATLAS = (640, 480), (1024, 512)
BG_ATLAS = (2048, 2048)

# Left / middle / right: Y / B / A, the three face buttons in a row as on the
# egg (and Left / Up-Down / Right on the d-pad).
BUTTONS = [
    ("btnLeft", "KEYS", 0x01, "LEFT|Y"),
    ("btnCenter", "KEYS", 0x02, "B|UP|DOWN"),
    ("btnRight", "KEYS", 0x04, "RIGHT|A"),
]

GAMES = [  # rom, ref, display name, date, art, with figures
    ("tamago.bin", "TMGTAMAGO", "TamaTown Tama-Go", "2010-08-01", "art_tamago.png", True),
    ("Tamagotchi_Friends_EU.bin", "TMGFRIENDSEU", "Tamagotchi Friends (Europe)", "2013-12-26", "art_friends.png", False),
]


def clock_options(friends: bool):
    """The game's clock in work RAM (the port's gameclock.c), plain bytes: for
    Pet Time (clock.pet), which writes the time and catches the pet up on the
    time away. The first-boot "what time is it" request is answered by the
    core itself (Tamago::end_frame)."""
    return ["clock.binary=1", f"clock.sec={0x136 if friends else 0x396:#x}", "clock.min=0x307", "clock.hour=0x306", "clock.pet=1"]


def container(rom: bytes, figures, friends: bool):
    opts = [f"input.{n}=direct;{port};{mask};1;{keys}" for n, port, mask, keys in BUTTONS] + clock_options(friends)
    text = "\n".join(opts).encode()
    out = bytearray(b"BRK1") + bytes([6]) + b"TAMAGO" + struct.pack("<II", CLOCK, len(text)) + text
    blobs = [(b"rom", rom)] + [(f"fig{i + 1:02d}".encode(), f) for i, f in enumerate(figures)]
    out += struct.pack("<I", len(blobs))
    for name, data in blobs:
        out += bytes([len(name)]) + name + struct.pack("<I", len(data)) + data
    return bytes(out)


def console_texture(path):
    """The picture centred on a white card (0.92 fill), 640x480, mirrored,
    at (0,0) of a 1024x512 atlas."""
    photo = Image.open(path).convert("RGBA")
    card = Image.new("RGBA", CONSOLE_SIZE, (255, 255, 255, 255))
    s = min(CONSOLE_SIZE[0] * 0.92 / photo.width, CONSOLE_SIZE[1] * 0.92 / photo.height)
    p = photo.resize((round(photo.width * s), round(photo.height * s)), Image.LANCZOS)
    card.alpha_composite(p, ((CONSOLE_SIZE[0] - p.width) // 2, (CONSOLE_SIZE[1] - p.height) // 2))
    atlas = Image.new("RGBA", CONSOLE_ATLAS, (0, 0, 0, 0))
    atlas.paste(card.transpose(Image.FLIP_LEFT_RIGHT), (0, 0))
    return atlas, [CONSOLE_ATLAS[0], CONSOLE_ATLAS[1], 0, CONSOLE_ATLAS[1] - CONSOLE_SIZE[1], CONSOLE_SIZE[0], CONSOLE_SIZE[1]]


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
             *string(g["path_console"]), *u16s(g["console_info"]), MANUFACTURER_BANDAI]
        entries.append(struct.pack("<" + "I" * 25, *f))
    fentries = []
    for name, blob in sorted(files.items()):
        n = string(name)
        d = chunk(blob)
        fentries.append(struct.pack("<IIII", n[0], n[1], d[0], d[1]))
    header = struct.pack("<IIIIIIIII", 0x31504B59, 3, 2, 5, len(games), len(fentries), games_off, files_off, data_off)
    Path(out_path).write_bytes(header + b"".join(entries) + b"".join(fentries) + bytes(data))


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "tamago_donor.ykp"
    figures = [f.read_bytes() for f in sorted((SRC / "figures").glob("*.bin"))]
    seg_png = png_bytes(Image.new("RGBA", (32, 32), (255, 255, 255, 0)))
    segs = [(255, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0)]  # word 0: never lit
    games, files = [], {}
    for rom, ref, name, date, art, with_figs in GAMES:
        key = ref.lower()
        console, console_info = console_texture(SRC / art)
        files[f"segment_{key}.png"] = seg_png
        files[f"console_{key}.png"] = png_bytes(console)
        games.append({
            "display_name": name, "ref": ref, "date": date,
            "rom": container((SRC / rom).read_bytes(), figures if with_figs else [], not with_figs),
            "path_segment": f"gfx/segment_{key}.png", "segments": segs,
            "segment_info": [32, 32, 1, 0, SCREEN[0], SCREEN[1]],
            "path_background": "", "background_info": [BG_ATLAS[0], BG_ATLAS[1], 0, 0, SCREEN[0], SCREEN[1], 0, 0, 0],
            "path_console": f"gfx/console_{key}.png", "console_info": console_info,
        })
        print(f"{name} -> {ref}" + (f" ({len(figures)} figures)" if with_figs else ""))
    write_pack(games, files, out)
    print(f"OK -> {out} ({len(games)} games)")


if __name__ == "__main__":
    main()
