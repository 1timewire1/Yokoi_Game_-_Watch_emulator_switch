"""Gives one game in a .ykp pack the same screen framing as another game
that shares its LCD (same segment ids, same layout - just cropped
differently by its face SVG): e.g. Hitorikko Dinosaur framed like
Hitorikko Chicken.

The target's segments are moved by the offset that lines them up with the
model's, and its screen canvas (segment_info / background_info width and
height) is set to the model's, so both show the LCD at the same proportion.
Patches the pack in place (make a copy first if you want one).

Usage: python match_screen_layout.py <pack.ykp> <target ref> <model ref>
       python match_screen_layout.py yokoi_pack_rgds.ykp HITORIKKODINOSAUR HITORIKKOCHICKEN
"""
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import unpack_ykp as u  # noqa: E402


def entry_fields(buf, header, ref):
    off = header["games_offset"]
    for _ in range(header["game_count"]):
        vals = struct.unpack("<" + "I" * u.GAME_ENTRY_FIELDS, buf[off:off + u.GAME_ENTRY_SIZE])
        pairs = {n: (vals[i * 2], vals[i * 2 + 1]) for i, n in enumerate(u.GAME_ENTRY_FIELD_NAMES)}
        if u.read_string(buf, *pairs["ref"]) == ref:
            return pairs
        off += u.GAME_ENTRY_SIZE
    sys.exit(f"{ref} is not in the pack")


def main():
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    pack, target, model = Path(sys.argv[1]), sys.argv[2], sys.argv[3]
    buf = bytearray(pack.read_bytes())
    header = u.parse_header(bytes(buf[:u.HEADER_SIZE]))
    t, m = entry_fields(buf, header, target), entry_fields(buf, header, model)

    tsegs = u.read_segments(buf, *t["segments"])
    msegs = {(s["id0"], s["id1"], s["id2"]): s for s in u.read_segments(buf, *m["segments"])}
    deltas = {(msegs[k]["pos_scr_x"] - s["pos_scr_x"], msegs[k]["pos_scr_y"] - s["pos_scr_y"])
              for s in tsegs for k in [(s["id0"], s["id1"], s["id2"])] if k in msegs}
    if len(deltas) != 1:
        sys.exit(f"the two LCDs don't line up with a single offset ({len(deltas)} different ones) - not the same layout")
    dx, dy = deltas.pop()

    off, count = t["segments"]
    for i in range(count):
        p = off + i * u.SEGMENT_SIZE
        vals = list(struct.unpack(u.SEGMENT_STRUCT, bytes(buf[p:p + u.SEGMENT_SIZE])))
        vals[3] += dx
        vals[4] += dy
        buf[p:p + u.SEGMENT_SIZE] = struct.pack(u.SEGMENT_STRUCT, *vals)

    # Canvas size = entries 4 and 5 of segment_info and background_info.
    for field in ("segment_info", "background_info"):
        moff, mcount = m[field]
        toff, tcount = t[field]
        mi = u.read_u16_list(buf, moff, mcount)
        ti = u.read_u16_list(buf, toff, tcount)
        before = (ti[4], ti[5])
        ti[4], ti[5] = mi[4], mi[5]
        buf[toff:toff + tcount * 2] = struct.pack("<" + "H" * tcount, *ti)
        print(f"{field}: {before[0]}x{before[1]} -> {ti[4]}x{ti[5]}")

    pack.write_bytes(bytes(buf))
    print(f"{target}: {count} segments moved by ({dx}, {dy}), framed like {model}")


if __name__ == "__main__":
    main()
