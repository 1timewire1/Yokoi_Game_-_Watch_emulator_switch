"""Swaps TAMAOTCH's 8 menu icons in a pack that already has them, from
tamaotch_icons/icon1-8.png (screen order, top row then bottom row). Only the
segment texture changes: pack textures are stored mirrored left to right, and
their rectangles count y from the bottom. For a pack that doesn't have the
icons yet, use set_tamaotch_background.py.

Usage: python set_tamaotch_icons.py <base.ykp> <out.ykp>
"""
import io
import sys
import tempfile
from pathlib import Path

from PIL import Image, ImageOps

sys.path.insert(0, str(Path(__file__).resolve().parent))
import inject_pack_assets as ip  # noqa: E402
import unpack_ykp as u  # noqa: E402

SEGMENTS = "segment_unknown_tamagotchi_tamaotch.png"
ICONS = Path(__file__).resolve().parent / "tamaotch_icons"
SLOTS = [(0, 8), (1, 8), (2, 8), (3, 8), (12, 28), (13, 28), (14, 28), (15, 28)]
ICON_ALPHA = 215 / 255  # as the game's other art


def main():
    base, out = Path(sys.argv[1]), Path(sys.argv[2])
    pack = base.read_bytes()
    h = u.parse_header(pack)
    info = u.read_file_table(pack, h)[SEGMENTS]
    tex = Image.open(io.BytesIO(pack[info["data_off"]:info["data_off"] + info["data_size"]])).convert("RGBA")
    game = next(g for g in u.read_game_entries(pack, h) if g["ref"] == "TAMAOTCH")
    for n, slot in enumerate(SLOTS, 1):
        s = next(s for s in game["segments"] if (s["id0"], s["id1"]) == slot)
        x, w, th = s["pos_tex_x"], s["size_tex_x"], s["size_tex_y"]
        y = tex.height - s["pos_tex_y"] - th
        art = Image.open(ICONS / f"icon{n}.png").convert("RGBA")
        if art.size != (w, th):
            sys.exit(f"icon{n}.png is {art.size}, its slot is {(w, th)}")
        art.putalpha(art.getchannel("A").point(lambda v: round(v * ICON_ALPHA)))
        tex.paste((0, 0, 0, 0), (x, y, x + w, y + th))
        tex.paste(ImageOps.mirror(art), (x, y))
    with tempfile.TemporaryDirectory() as tmp:
        assets = Path(tmp) / "assets"
        assets.mkdir()
        tex.save(assets / SEGMENTS)
        ip.inject(base, assets, out)
    print("OK:", out)


if __name__ == "__main__":
    main()
