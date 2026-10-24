"""Adds whole games from one or more donor .ykp packs into a base pack, without
rebuilding the base from artwork - e.g. new games built on their own with
`convert_switch.py --target rgds --game <key> --pack-name <tmp>`.

For each donor game whose ref isn't already in the base, its full record
(name/ref/date, ROM, melody, segment array, info arrays) and the texture files
its segment/background/console paths point at are appended. Everything already
in the base is carried over byte for byte. The output keeps the base's header
(format/platform/content_version) and is validated before it's written.

Usage:
  python merge_pack_games.py <base.ykp> <donor.ykp> [<donor.ykp> ...] --out <merged.ykp>
"""
import argparse
import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import inject_pack_assets as ip  # noqa: E402  (validate_pack_bytes)
import unpack_ykp as u  # noqa: E402  (pack parsing)


def load_pack(path: Path):
    buf = path.read_bytes()
    header = u.parse_header(buf)
    return buf, header, u.read_file_table(buf, header), u.read_game_entries(buf, header)


def file_bytes(buf: bytes, info: dict) -> bytes:
    return buf[info["data_off"]: info["data_off"] + info["data_size"]]


def game_record(buf: bytes, file_table: dict, g: dict) -> dict:
    """A self-contained copy of one parsed game, carrying actual bytes."""
    def texture(raw_path: str):
        if not raw_path:
            return None
        name = u.resolve_texture_file(raw_path, file_table)
        if not name:
            raise RuntimeError(f"{g['ref']}: texture {raw_path!r} not in the pack's file table")
        return raw_path, name, file_bytes(buf, file_table[name])

    seg, con = texture(g["path_segment"]), texture(g["path_console"])
    if seg is None or con is None:
        raise RuntimeError(f"{g['ref']}: missing segment or console texture")
    return {
        "display_name": g["display_name"], "ref": g["ref"], "date": g["date"],
        "manufacturer": g["manufacturer"],
        "rom_bytes": buf[g["rom_off"]: g["rom_off"] + g["rom_size"]],
        "melody_bytes": buf[g["melody_off"]: g["melody_off"] + g["melody_size"]] if g["melody_size"] else b"",
        "segments": g["segments"], "segment_info": g["segment_info"],
        "background_info": g["background_info"], "console_info": g["console_info"],
        "path_segment": seg, "path_background": texture(g["path_background"]), "path_console": con,
    }


def write_pack(games: list, shared_files: dict, header_meta: dict) -> bytes:
    """Same binary layout as convert_switch.py's write_rom_pack_v1."""
    games_offset = u.HEADER_SIZE
    files_offset = games_offset + len(games) * u.GAME_ENTRY_SIZE
    data_offset = files_offset + len(shared_files) * u.FILE_ENTRY_SIZE
    data = bytearray()

    def chunk(b: bytes) -> int:
        off = data_offset + len(data)
        data.extend(b)
        return off

    def string(s: str):
        b = (s or "").encode("utf-8")
        return chunk(b), len(b)

    def u16_list(vals):
        if not vals:
            return 0, 0
        return chunk(struct.pack("<" + "H" * len(vals), *[int(v) & 0xFFFF for v in vals])), len(vals)

    def segments(segs):
        if not segs:
            return 0, 0
        out = bytearray()
        for s in segs:
            out.extend(struct.pack(
                u.SEGMENT_STRUCT,
                int(s["id0"]) & 0xFF, int(s["id1"]) & 0xFF, int(s["id2"]) & 0xFF,
                int(s["pos_scr_x"]), int(s["pos_scr_y"]),
                int(s["pos_tex_x"]) & 0xFFFF, int(s["pos_tex_y"]) & 0xFFFF,
                int(s["size_tex_x"]) & 0xFFFF, int(s["size_tex_y"]) & 0xFFFF,
                int(s["color_index"]) & 0xFF, int(s["screen"]) & 0xFF,
            ))
        return chunk(bytes(out)), len(segs)

    entries = []
    for g in games:
        name = string(g["display_name"])
        ref = string(g["ref"])
        date = string(g["date"])
        rom = (chunk(g["rom_bytes"]), len(g["rom_bytes"]))
        melody = (chunk(g["melody_bytes"]), len(g["melody_bytes"])) if g["melody_bytes"] else (0, 0)
        p_seg = string(g["path_segment"][0])
        segs = segments(g["segments"])
        seg_info = u16_list(g["segment_info"])
        p_bg = string(g["path_background"][0] if g["path_background"] else "")
        bg_info = u16_list(g["background_info"])
        p_con = string(g["path_console"][0])
        con_info = u16_list(g["console_info"])
        entries.append(struct.pack(
            "<" + "I" * 25,
            *name, *ref, *date, *rom, *melody, *p_seg, *segs, *seg_info,
            *p_bg, *bg_info, *p_con, *con_info, int(g["manufacturer"]),
        ))

    file_entries = []
    for fname, content in sorted(shared_files.items()):
        n_off, n_len = string(fname)
        file_entries.append(struct.pack("<IIII", n_off, n_len, chunk(content), len(content)))

    header = struct.pack(
        "<IIIIIIIII", u.MAGIC, header_meta["format_version"], header_meta["platform"],
        header_meta["content_version"], len(games), len(shared_files),
        games_offset, files_offset, data_offset,
    )
    return header + b"".join(entries) + b"".join(file_entries) + bytes(data)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("base", type=Path)
    ap.add_argument("donors", type=Path, nargs="+")
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()

    base_buf, base_header, base_files, base_games = load_pack(args.base)
    games = [game_record(base_buf, base_files, g) for g in base_games]
    shared = {name: file_bytes(base_buf, info) for name, info in base_files.items()}
    have = {g["ref"] for g in games}

    for donor in args.donors:
        d_buf, _d_header, d_files, d_games = load_pack(donor)
        for g in d_games:
            if g["ref"] in have:
                print(f"  skip {g['ref']} (already in base)")
                continue
            rec = game_record(d_buf, d_files, g)
            games.append(rec)
            have.add(g["ref"])
            for tex in (rec["path_segment"], rec["path_background"], rec["path_console"]):
                if tex is None:
                    continue
                _path, fname, content = tex
                if fname in shared and shared[fname] != content:
                    raise RuntimeError(f"{g['ref']}: file {fname!r} already in base with different bytes")
                shared[fname] = content
            print(f"  added {g['ref']} ({g['display_name']}) from {donor.name}")

    out = write_pack(games, shared, base_header)
    ip.validate_pack_bytes(out)
    args.out.write_bytes(out)
    print(f"Wrote {args.out}: {len(games)} games, {len(shared)} files, {len(out):,} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
