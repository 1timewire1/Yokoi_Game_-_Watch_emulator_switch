"""Gives TAMAOTCH its background and menu icons in an existing pack, without
rebuilding it: its set was packed with segments only (prepare_e0c6200.py's
SEGMENTS_ONLY). Pack textures are stored mirrored left to right (the renderer
flips them back), and their rectangles count y from the bottom, so a stored y
maps to image row (texture height - y - rectangle height).

The photo of the unit's LCD glass goes on a 1024x512 texture, stretched to 523x480: one dot wider than the set's 507, to the right. The game record is repointed at it. tamaotch_icons/icon1-8.png
(screen order, top row then bottom row) replace the 8 icons in the segment
texture. Icon 1's slot is narrower than the art, so it moves to free space.
The new strings go at the end of the file, and inject_pack_assets.py adds the
textures and compacts the old bytes away.

Usage: python set_tamaotch_background.py <base.ykp> <lcd photo> <out.ykp>
"""
import io
import struct
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageOps

sys.path.insert(0, str(Path(__file__).resolve().parent))
import inject_pack_assets as ip  # noqa: E402
import unpack_ykp as u  # noqa: E402

NAME = "background_unknown_tamagotchi_tamaotch.png"
SEGMENTS = "segment_unknown_tamagotchi_tamaotch.png"
ICONS = Path(__file__).resolve().parent / "tamaotch_icons"
X, Y, W, H = 1, 1, 523, 480  # background's image rectangle: one dot wider than the set's 507
TEX_W, TEX_H = 1024, 512
INFO_Y = 31                  # the same rectangle's y as stored (512 - 1 - 480)
PATH_FIELD, INFO_FIELD = 8 * 2, 9 * 2  # (offset, length) pairs in a game entry
SEGMENTS_FIELD = 6 * 2
SEGMENT_SIZE = 21
# Screen order; icon 1 is 78 wide: its slot gets free space and moves left.
SLOTS = [(0, 8), (1, 8), (2, 8), (3, 8), (12, 28), (13, 28), (14, 28), (15, 28)]
ICON1_ROW, ICON1_SCR_X = 1700, 46
ICON_ALPHA = 215 / 255  # as the game's other art
SEGMENT_INFO_FIELD = 7 * 2
NARROW = 13


def widen_screen(buf: bytearray, fields: list):
    """The screen grows to the right by one dot and nothing moves. The right
    dot column, clipped in Gen. 1's art, takes its neighbour's sprite."""
    start, count = fields[SEGMENTS_FIELD], fields[SEGMENTS_FIELD + 1]
    segs = {}
    for at in range(start, start + count * SEGMENT_SIZE, SEGMENT_SIZE):
        segs[(buf[at], buf[at + 1])] = at
    for (com, seg), at in segs.items():
        if struct.unpack_from("<H", buf, at + 15)[0] == NARROW and seg == 20:
            near = segs[(com, 21)]
            buf[at + 11:at + 19] = buf[near + 11:near + 19]
    info = list(struct.unpack_from(f"<{fields[SEGMENT_INFO_FIELD + 1]}H", buf, fields[SEGMENT_INFO_FIELD]))
    info[4] = W
    fields[SEGMENT_INFO_FIELD] = len(buf)
    buf += struct.pack(f"<{len(info)}H", *info)


def add_icons(buf: bytearray, fields: list, tex: Image.Image):
    start, count = fields[SEGMENTS_FIELD], fields[SEGMENTS_FIELD + 1]
    found = 0
    for at in range(start, start + count * SEGMENT_SIZE, SEGMENT_SIZE):
        slot = (buf[at], buf[at + 1])
        if slot not in SLOTS:
            continue
        n = SLOTS.index(slot) + 1
        art = Image.open(ICONS / f"icon{n}.png").convert("RGBA")
        scr_x = struct.unpack_from("<i", buf, at + 3)[0]
        tx, ty, tw, th = struct.unpack_from("<HHHH", buf, at + 11)
        tex.paste((0, 0, 0, 0), (tx, tex.height - ty - th, tx + tw, tex.height - ty))
        if n == 1:
            scr_x, tw, th = ICON1_SCR_X, art.width, art.height
            ty = tex.height - ICON1_ROW - th
            struct.pack_into("<i", buf, at + 3, scr_x)
            struct.pack_into("<HHHH", buf, at + 11, tx, ty, tw, th)
        if art.size != (tw, th):
            sys.exit(f"icon{n}.png is {art.size}, its slot is {(tw, th)}")
        art.putalpha(art.getchannel("A").point(lambda v: round(v * ICON_ALPHA)))
        tex.paste(ImageOps.mirror(art), (tx, tex.height - ty - th))
        found += 1
    if found != len(SLOTS):
        sys.exit(f"Found {found} of {len(SLOTS)} icon slots")


def main():
    base, photo, out = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
    buf = bytearray(base.read_bytes())
    h = u.parse_header(bytes(buf))
    files = u.read_file_table(bytes(buf), h)
    info = files[SEGMENTS]
    segments = Image.open(io.BytesIO(buf[info["data_off"]:info["data_off"] + info["data_size"]])).convert("RGBA")
    for i in range(h["game_count"]):
        at = h["games_offset"] + i * u.GAME_ENTRY_SIZE
        fields = list(struct.unpack_from("<25I", buf, at))
        if bytes(buf[fields[2]:fields[2] + fields[3]]) != b"TAMAOTCH":
            continue
        # A private copy: other games may share the original array.
        size = fields[SEGMENTS_FIELD + 1] * SEGMENT_SIZE
        own = bytes(buf[fields[SEGMENTS_FIELD]:fields[SEGMENTS_FIELD] + size])
        fields[SEGMENTS_FIELD] = len(buf)
        buf += own
        add_icons(buf, fields, segments)
        widen_screen(buf, fields)
        path = f"gfx/{NAME}".encode()
        bg_info = list(struct.unpack_from(f"<{fields[INFO_FIELD + 1]}H", buf, fields[INFO_FIELD]))
        bg_info[0:6] = [TEX_W, TEX_H, X, INFO_Y, W, H]
        fields[PATH_FIELD], fields[PATH_FIELD + 1] = len(buf), len(path)
        buf += path
        fields[INFO_FIELD] = len(buf)
        buf += struct.pack(f"<{len(bg_info)}H", *bg_info)
        struct.pack_into("<25I", buf, at, *fields)
        break
    else:
        sys.exit("TAMAOTCH isn't in this pack")
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        tex = Image.new("RGBA", (TEX_W, TEX_H), (0, 0, 0, 0))
        tex.paste(ImageOps.mirror(Image.open(photo).convert("RGB").resize((W, H), Image.LANCZOS)), (X, Y))
        assets = tmp / "assets"
        assets.mkdir()
        tex.save(assets / NAME)
        segments.save(assets / SEGMENTS)
        patched = tmp / "patched.ykp"
        patched.write_bytes(buf)
        ip.inject(patched, assets, out)
    print("OK:", out)


if __name__ == "__main__":
    main()
