"""Builds the Entex Adventure Vision games (core "I8048",
source/BrickCores/AdvVision.*) as a small donor pack, to merge into the main
pack with merge_pack_games.py. The red LED picture (480x40) is drawn by the
app (switch_screen.cpp, refs "EAV..."), so a game has no segment art - one
dummy segment only.

Inputs (CONVERT_ROM/rom/advision/):
  ins8048-11kdp_n.u5, cop411l-kcn_n.u8   the BIOS (MAME set "advision")
  "<Title> (USA, Europe).bin"            the cartridges
  art_<set>.png, art_folder.png          the console pictures (photo with the
                                         game's screen and cartridge)

Manufacturer "Entex Adventure Vision" (44, a folder inside Entex in the menu).

Usage:  py build_advision_pack.py [out.ykp]     (default: advision_donor.ykp)
Then:   py merge_pack_games.py <base.ykp> advision_donor.ykp --out <new.ykp> --replace
"""
import io
import struct
import sys
from pathlib import Path

from PIL import Image

HERE = Path(__file__).resolve().parent
SRC = HERE / "rom" / "advision"

MANUFACTURER_ENTEX_AV = 44
CLOCK = 733_333                        # 11 MHz / 15 (one 8048 cycle)
SCREEN = (1200, 930)                   # the picture, as the real screen (MAME captures 268x208)
CONSOLE_SIZE, CONSOLE_ATLAS = (640, 480), (1024, 512)
BG_ATLAS = (2048, 2048)

# MAME's JOY port (active low in the core; level 1 = pressed).
BUTTONS = [
    ("btn1", "JOY", 0x01, "A"), ("btn2", "JOY", 0x02, "B"),
    ("btn4", "JOY", 0x04, "Y"), ("btn3", "JOY", 0x08, "X"),
    ("btnDown", "JOY", 0x10, "DOWN"), ("btnUp", "JOY", 0x20, "UP"),
    ("btnRight", "JOY", 0x40, "RIGHT"), ("btnLeft", "JOY", 0x80, "LEFT"),
]

GAMES = [  # file title, MAME softlist set, display name
    ("Defender", "defender", "Defender"),
    ("Space Force", "spcforce", "Space Force"),
    ("Super Cobra", "scobra", "Super Cobra"),
    ("Turtles", "turtles", "Turtles"),
]


def container(rom: bytes, snd: bytes, cart: bytes):
    text = "\n".join(f"input.{n}=direct;{port};{mask};1;{keys}" for n, port, mask, keys in BUTTONS).encode()
    out = bytearray(b"BRK1") + bytes([5]) + b"I8048" + struct.pack("<II", CLOCK, len(text)) + text
    blobs = [(b"rom", rom), (b"snd", snd), (b"cart", cart)]
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
             *string(g["path_console"]), *u16s(g["console_info"]), MANUFACTURER_ENTEX_AV]
        entries.append(struct.pack("<" + "I" * 25, *f))
    fentries = []
    for name, blob in sorted(files.items()):
        n = string(name)
        d = chunk(blob)
        fentries.append(struct.pack("<IIII", n[0], n[1], d[0], d[1]))
    header = struct.pack("<IIIIIIIII", 0x31504B59, 3, 2, 5, len(games), len(fentries), games_off, files_off, data_off)
    Path(out_path).write_bytes(header + b"".join(entries) + b"".join(fentries) + bytes(data))


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else HERE / "advision_donor.ykp"
    rom = (SRC / "ins8048-11kdp_n.u5").read_bytes()
    snd = (SRC / "cop411l-kcn_n.u8").read_bytes()
    seg_png = png_bytes(Image.new("RGBA", (32, 32), (255, 255, 255, 0)))
    segs = [(255, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0)]  # word 0: never lit
    games, files = [], {}
    for title, s, name in GAMES:
        key = f"eav_{s}"
        console, console_info = console_texture(SRC / f"art_{s}.png")
        files[f"segment_{key}.png"] = seg_png
        files[f"console_{key}.png"] = png_bytes(console)
        games.append({
            "display_name": name, "ref": "EAV" + s.upper(), "date": "1982-01-01",
            "rom": container(rom, snd, (SRC / f"{title} (USA, Europe).bin").read_bytes()),
            "path_segment": f"gfx/segment_{key}.png", "segments": segs,
            "segment_info": [32, 32, 1, 0, SCREEN[0], SCREEN[1]],
            "path_background": "", "background_info": [BG_ATLAS[0], BG_ATLAS[1], 0, 0, SCREEN[0], SCREEN[1], 0, 0, 0],
            "path_console": f"gfx/console_{key}.png", "console_info": console_info,
        })
        print(f"{title} -> EAV{s.upper()}")
    # The folder's picture in the menu (Entex > Adventure Vision): the console off.
    files["console_eav_folder.png"] = png_bytes(console_texture(SRC / "art_folder.png")[0])
    write_pack(games, files, out)
    print(f"OK -> {out} ({len(games)} games)")


if __name__ == "__main__":
    main()
