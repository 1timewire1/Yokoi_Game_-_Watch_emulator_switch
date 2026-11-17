"""Redraws the dot-matrix dots of the Brick Game 9999-in-1 style games in an
already-built pack: the 10x20 play grid and the 4x4 NEXT grid.

The segment atlas these games get from the importer (split_brick_svg.py
rasterising BrickEmuPy's CC0 faces) has dots that came out uneven - a pixel
or two off in size, edges missing, NEXT's small dots mangled. This finds the
dots and draws each one again, clean and identical within its grid:
  filled  E23PLUSMARKII96IN1, E332IN1 (their LCDs have solid dots; the app's
          pause menu > Screen > Pixel Style can draw them "Classic")
  framed  E888IN1 (its art has a frame with a square in the middle)

A dot is a squarish segment with a same-size neighbour one grid step away
(across or down), in a group of at least 12 such neighbours - the play grid
or NEXT's. Lone icons of about the same size aren't in such a group.

The atlas is stored V-flipped (switch_screen.cpp's kUvFlipV): a segment's
rows are image rows H - (pos_tex_y + size_tex_y) .. H - pos_tex_y.

Usage:
  python fix_brick_dots.py <pack.ykp> <out.ykp> [--preview preview.png]
"""
import argparse
import shutil
import struct
import sys
import tempfile
from io import BytesIO
from pathlib import Path

from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import unpack_ykp as up  # noqa: E402
import inject_pack_assets as inj  # noqa: E402

# All three draw their dots the same way, and it isn't a filled square: in
# BrickEmuPy's faces the <svg> is fill="none", so each dot is <rect id="out">
# - an 8x8 square with a 1-unit black stroke and no fill - plus <rect
# id="in">, a 4.5x4.5 filled square centred in it. Over the stroke that's a
# 9-unit box: 1 of frame, 1.25 of gap, 4.5 of centre square, and the grid's
# pitch is 9.8. The importer filled the outer rect in, which is why 96 in 1
# and E-33 came out as solid blocks in the pack (the E-88's came out hollow
# but uneven, NEXT's worst of all).
GAMES = {"E23PLUSMARKII96IN1": "framed", "E332IN1": "framed", "E888IN1": "framed"}


def ink(atlas, s):
    """Share of the segment's box its art covers: ~0.8 for a solid dot,
    ~0.6 for a framed one, well under that for an icon (a star, a note)."""
    W, H = atlas.size
    x0, w, h = s["pos_tex_x"], s["size_tex_x"], s["size_tex_y"]
    y0 = H - (s["pos_tex_y"] + h)
    a = atlas.crop((x0, y0, x0 + w, y0 + h)).getchannel("A")
    return sum(1 for v in a.getdata() if v >= 128) / float(w * h)


def find_dot_groups(segs, atlas, min_ink=0.5):
    """Lists of segment indices, one per grid of dots."""
    sq = [i for i, s in enumerate(segs)
          if s["size_tex_x"] >= 8 and s["size_tex_y"] >= 8 and abs(s["size_tex_x"] - s["size_tex_y"]) <= 3
          and ink(atlas, s) >= min_ink]
    cx = lambda s: s["pos_scr_x"] + s["size_tex_x"] / 2
    cy = lambda s: s["pos_scr_y"] + s["size_tex_y"] / 2
    adj = {i: [] for i in sq}
    for a in sq:
        sa = segs[a]
        d = (sa["size_tex_x"] + sa["size_tex_y"]) / 2
        for b in sq:
            if a == b:
                continue
            sb = segs[b]
            if sa["screen"] != sb["screen"] or abs(sb["size_tex_x"] - sa["size_tex_x"]) > 3 or abs(sb["size_tex_y"] - sa["size_tex_y"]) > 3:
                continue
            dx, dy = abs(cx(sb) - cx(sa)), abs(cy(sb) - cy(sa))
            if (0.9 * d <= dx <= 1.35 * d and dy <= 0.25 * d) or (0.9 * d <= dy <= 1.35 * d and dx <= 0.25 * d):
                adj[a].append(b)
    seen, groups = set(), []
    for i in sq:
        if i in seen or not adj[i]:
            continue
        comp, stack = [], [i]
        seen.add(i)
        while stack:
            k = stack.pop()
            comp.append(k)
            for n in adj[k]:
                if n not in seen:
                    seen.add(n)
                    stack.append(n)
        if len(comp) >= 12:
            groups.append(comp)
    return groups


def grid_cells(segs, grp):
    """Each dot's cell centre on screen: the group fitted to one regular grid
    (step = the usual distance between neighbours, origin = the average of
    every dot's offset from it), so dots whose segment box is a pixel off
    still land exactly in line."""
    cx = {i: segs[i]["pos_scr_x"] + segs[i]["size_tex_x"] / 2 for i in grp}
    cy = {i: segs[i]["pos_scr_y"] + segs[i]["size_tex_y"] / 2 for i in grp}
    def step(c):
        vals = sorted(set(round(v) for v in c.values()))
        d = [b - a for a, b in zip(vals, vals[1:]) if b - a > 3]
        d.sort()
        return d[len(d) // 2] if d else 1.0
    sx, sy = step(cx), step(cy)
    # Refine the step over the whole span, then the origin.
    kx = {i: round((cx[i] - min(cx.values())) / sx) for i in grp}
    ky = {i: round((cy[i] - min(cy.values())) / sy) for i in grp}
    if max(kx.values()) > 0:
        sx = (max(cx.values()) - min(cx.values())) / max(kx.values())
    if max(ky.values()) > 0:
        sy = (max(cy.values()) - min(cy.values())) / max(ky.values())
    ox = sum(cx[i] - kx[i] * sx for i in grp) / len(grp)
    oy = sum(cy[i] - ky[i] * sy for i in grp) / len(grp)
    return {i: (ox + kx[i] * sx, oy + ky[i] * sy) for i in grp}


def draw_dot(img, s, box, style):
    """Clears the segment's old atlas rect and draws one clean dot in the
    first box x box pixels of it (the segment's new, smaller rect): a pixel
    of margin all round, like the art's own dots, so it's centred."""
    W, H = img.size
    x0, w, h = s["pos_tex_x"], s["size_tex_x"], s["size_tex_y"]
    d = ImageDraw.Draw(img)
    d.rectangle([x0, H - (s["pos_tex_y"] + h), x0 + w - 1, H - s["pos_tex_y"] - 1], fill=(255, 255, 255, 0))
    y0 = H - (s["pos_tex_y"] + box)
    size = box - 2
    lx, ty = x0 + 1, y0 + 1
    on = (255, 255, 255, 255)
    if style == "filled":
        d.rectangle([lx, ty, lx + size - 1, ty + size - 1], fill=on)
    else:
        # The SVG's 9-unit box: frame 1, gap 1.25, centre square 4.5.
        t = max(1, round(size / 9.0))
        c = max(3, round(size * 4.5 / 9.0))
        if (size - c) % 2:
            c += 1  # keep the centre square centred on whole pixels
        g = (size - c) // 2 - t
        if g < 1:
            g, t = 1, max(1, (size - c) // 2 - 1)
        d.rectangle([lx, ty, lx + size - 1, ty + size - 1], fill=on)
        d.rectangle([lx + t, ty + t, lx + size - 1 - t, ty + size - 1 - t], fill=(255, 255, 255, 0))
        d.rectangle([lx + t + g, ty + t + g, lx + size - 1 - t - g, ty + size - 1 - t - g], fill=on)


def segment_table_offsets(buf, header):
    """{ref (upper case): offset of its first SegmentDiskV1}."""
    out = {}
    off = header["games_offset"]
    names = up.GAME_ENTRY_FIELD_NAMES
    for _ in range(header["game_count"]):
        vals = struct.unpack("<" + "I" * up.GAME_ENTRY_FIELDS, buf[off:off + up.GAME_ENTRY_SIZE])
        pairs = {n: (vals[k * 2], vals[k * 2 + 1]) for k, n in enumerate(names)}
        ro, rl = pairs["ref"]
        out[buf[ro:ro + rl].decode("utf-8").upper()] = pairs["segments"][0]
        off += up.GAME_ENTRY_SIZE
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("pack", type=Path)
    ap.add_argument("out", type=Path)
    ap.add_argument("--preview", type=Path)
    a = ap.parse_args()

    buf = a.pack.read_bytes()
    header = up.parse_header(buf)
    files = up.read_file_table(buf, header)
    games = {g["ref"].upper(): g for g in up.read_game_entries(buf, header)}
    work = Path(tempfile.mkdtemp(prefix="brickdots_"))
    previews = []
    new_boxes = {ref: {} for ref in GAMES}
    for ref, style in GAMES.items():
        g = games.get(ref)
        if not g:
            print(f"{ref}: not in the pack, skipped")
            continue
        name = up.resolve_texture_file(g["path_segment"], files)
        fe = files[name]
        atlas = Image.open(BytesIO(buf[fe["data_off"]:fe["data_off"] + fe["data_size"]])).convert("RGBA")
        before = atlas.copy()
        segs = g["segments"]
        # The E-88's NEXT dots are so broken they have little ink left; its
        # icons are far enough from NEXT not to join the grid anyway. The
        # other two keep the higher bar, so no icon joins their NEXT grid.
        groups = find_dot_groups(segs, atlas, 0.35 if ref == "E888IN1" else 0.5)
        # Solid-dot games: NEXT's dots are the play grid's size on the real
        # LCD, so one size for both; the E-88's NEXT grid is smaller.
        game_box = min(min(segs[i]["size_tex_x"], segs[i]["size_tex_y"]) for grp in groups for i in grp) if groups else 0
        for grp in groups:
            # One box size for the whole grid (the smallest, so each new
            # rect fits inside the old one), each box on its grid cell.
            box = min(min(segs[i]["size_tex_x"], segs[i]["size_tex_y"]) for i in grp)
            if style == "filled":
                box = game_box
            cells = grid_cells(segs, grp)
            for i in grp:
                draw_dot(atlas, segs[i], box, style)
                new_boxes[ref][i] = (round(cells[i][0] - box / 2), round(cells[i][1] - box / 2), box)
            print(f"{ref}: {len(grp)} dots redrawn, {box - 2} px in {box} px boxes, {style}")
        atlas.save(work / name)
        if a.preview:
            previews.append((ref, before, atlas, segs, groups))
    if not any(work.iterdir()):
        sys.exit("nothing to do")
    inj.inject(a.pack, work, a.out)
    shutil.rmtree(work, ignore_errors=True)

    # The dots' segment records: same box size, each on its cell. Patched in
    # place (same record size), after the injection moved the data around.
    out = bytearray(a.out.read_bytes())
    oh = up.parse_header(bytes(out))
    seg_off = segment_table_offsets(bytes(out), oh)
    for ref, boxes in new_boxes.items():
        base = seg_off.get(ref)
        if base is None:
            continue
        for i, (x, y, box) in boxes.items():
            o2 = base + i * up.SEGMENT_SIZE
            f = list(struct.unpack(up.SEGMENT_STRUCT, out[o2:o2 + up.SEGMENT_SIZE]))
            f[3], f[4], f[7], f[8] = x, y, box, box  # pos_scr_x/y, size_tex_x/y
            out[o2:o2 + up.SEGMENT_SIZE] = struct.pack(up.SEGMENT_STRUCT, *f)
    a.out.write_bytes(bytes(out))
    print(f"OK -> {a.out}")

    if a.preview and previews:
        # Every dot of each grid, before and after, side by side, x6.
        rows = []
        for ref, before, after, segs, groups in previews:
            for grp in groups:
                for im in (before, after):
                    W, H = im.size
                    tiles = []
                    for i in grp[:24]:
                        s = segs[i]
                        y0 = H - (s["pos_tex_y"] + s["size_tex_y"])
                        tiles.append(im.crop((s["pos_tex_x"], y0, s["pos_tex_x"] + s["size_tex_x"], y0 + s["size_tex_y"])))
                    tw = max(t.width for t in tiles) + 2
                    th = max(t.height for t in tiles) + 2
                    row = Image.new("RGB", (tw * len(tiles), th), (150, 160, 155))
                    for k, t in enumerate(tiles):
                        black = Image.new("RGB", t.size, (20, 22, 20))
                        row.paste(black, (k * tw, 0), t.getchannel("A"))
                    rows.append(row.resize((row.width * 6, row.height * 6), Image.NEAREST))
        W = max(r.width for r in rows)
        out = Image.new("RGB", (W, sum(r.height + 8 for r in rows)), "white")
        y = 0
        for r in rows:
            out.paste(r, (0, y))
            y += r.height + 8
        out.save(a.preview)
        print(f"preview -> {a.preview}")


if __name__ == "__main__":
    main()
