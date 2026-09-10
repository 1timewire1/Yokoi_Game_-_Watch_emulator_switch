# Unpacks a .ykp rompack back into a human-inspectable folder tree.
#
# This is the inverse of write_rom_pack_v1() in convert_3ds.py, so the
# constants/layout below mirror that function (and inject_pack_assets.py,
# which parses the same header/game-table/file-table for patching). See the
# comment block at the top of inject_pack_assets.py for the full byte layout.
#
# What CAN be recovered, per game entry:
#   - display_name, ref, date, manufacturer
#   - raw ROM bytes, raw melody bytes (if present)
#   - the segment/background/console texture referenced by name (pulled from
#     the pack's shared file table)
#   - the numeric segment records + segment_info/background_info/console_info
#     arrays (atlas layout data - kept as raw ints since the packer doesn't
#     preserve field-by-field labels for these)
#
# What CANNOT be recovered, because the pack format never stores it:
#   - the original MAME artwork/rom folder name (`folder_name` /
#     `games_path` dict key) - only `ref` survives, so games are grouped by
#     ref instead
#   - which target profile / resolution the pack was built for beyond what's
#     visible in the actual texture pixel dimensions
#
# Every file the pack embeds under a shared filename (segment_*.png,
# background_*.png, console_*.png, plus UI textures like texte_3ds.png and
# any bundled-but-unreferenced art e.g. gfx2x_open_source's David & John
# PNGs - see write_rom_pack_v1's "open_source_dir" block) is extracted once
# into gfx/, matching convert_3ds.py's own shared destination_graphique_file
# convention, rather than duplicated per game.

import argparse
import json
import struct
from pathlib import Path

MAGIC = 0x31504B59  # 'YKP1'
HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
GAME_ENTRY_FIELDS = 25
FILE_ENTRY_SIZE = 16
SEGMENT_STRUCT = "<BBBiiHHHHBB"
SEGMENT_SIZE = struct.calcsize(SEGMENT_STRUCT)

MANUFACTURER_NAMES = {
    0: "Nintendo",
    1: "Tronica",
    2: "Elektronika",
    3: "Tiger",
    4: "David and John",
}

# Field order matches the struct.pack("<" + "I"*25, ...) call in
# write_rom_pack_v1 exactly - each (offset, length) pair, then manufacturer_id.
GAME_ENTRY_FIELD_NAMES = (
    "name", "ref", "date", "rom", "melody",
    "path_segment", "segments", "segment_info",
    "path_background", "background_info",
    "path_console", "console_info",
)


def parse_header(buf: bytes) -> dict:
    if len(buf) < HEADER_SIZE:
        raise ValueError("File too small to be a .ykp pack")
    fields = struct.unpack("<IIIIIIIII", buf[:HEADER_SIZE])
    magic, format_version, platform, content_version, game_count, file_count, games_offset, files_offset, data_offset = fields
    if magic != MAGIC:
        raise ValueError(f"Not a YKP1 pack (magic=0x{magic:08x}, expected 0x{MAGIC:08x})")
    if games_offset != HEADER_SIZE:
        raise ValueError(f"Unexpected games_offset {games_offset} (expected {HEADER_SIZE}) - pack layout doesn't match write_rom_pack_v1")
    if files_offset != games_offset + game_count * GAME_ENTRY_SIZE:
        raise ValueError("games region size doesn't match game_count - pack layout mismatch")
    if data_offset != files_offset + file_count * FILE_ENTRY_SIZE:
        raise ValueError("files region size doesn't match file_count - pack layout mismatch")
    return {
        "format_version": format_version,
        "platform": platform,
        "content_version": content_version,
        "game_count": game_count,
        "file_count": file_count,
        "games_offset": games_offset,
        "files_offset": files_offset,
        "data_offset": data_offset,
    }


def read_string(buf: bytes, off: int, length: int) -> str:
    if length == 0:
        return ""
    return buf[off:off + length].decode("utf-8")


def read_u16_list(buf: bytes, off: int, count: int) -> list[int]:
    if count == 0:
        return []
    return list(struct.unpack("<" + "H" * count, buf[off:off + count * 2]))


def read_segments(buf: bytes, off: int, count: int) -> list[dict]:
    segments = []
    for i in range(count):
        chunk = buf[off + i * SEGMENT_SIZE: off + (i + 1) * SEGMENT_SIZE]
        id0, id1, id2, pos_scr_x, pos_scr_y, pos_tex_x, pos_tex_y, size_tex_x, size_tex_y, color_index, screen = \
            struct.unpack(SEGMENT_STRUCT, chunk)
        segments.append({
            "id0": id0, "id1": id1, "id2": id2,
            "pos_scr_x": pos_scr_x, "pos_scr_y": pos_scr_y,
            "pos_tex_x": pos_tex_x, "pos_tex_y": pos_tex_y,
            "size_tex_x": size_tex_x, "size_tex_y": size_tex_y,
            "color_index": color_index, "screen": screen,
        })
    return segments


def read_file_table(buf: bytes, header: dict) -> dict[str, dict]:
    """Returns {filename: {"data_off":, "data_size":}}."""
    files = {}
    off = header["files_offset"]
    for _ in range(header["file_count"]):
        name_off, name_len, data_off, data_size = struct.unpack("<IIII", buf[off:off + FILE_ENTRY_SIZE])
        name = buf[name_off:name_off + name_len].decode("utf-8")
        files[name] = {"data_off": data_off, "data_size": data_size}
        off += FILE_ENTRY_SIZE
    return files


def read_game_entries(buf: bytes, header: dict) -> list[dict]:
    games = []
    off = header["games_offset"]
    for _ in range(header["game_count"]):
        vals = struct.unpack("<" + "I" * GAME_ENTRY_FIELDS, buf[off:off + GAME_ENTRY_SIZE])
        pairs = {}
        for idx, field_name in enumerate(GAME_ENTRY_FIELD_NAMES):
            pairs[field_name] = (vals[idx * 2], vals[idx * 2 + 1])  # (offset/off, length/count)
        manufacturer_id = vals[24]

        name_off, name_len = pairs["name"]
        ref_off, ref_len = pairs["ref"]
        date_off, date_len = pairs["date"]
        rom_off, rom_size = pairs["rom"]
        melody_off, melody_size = pairs["melody"]
        path_segment_off, path_segment_len = pairs["path_segment"]
        segments_off, segments_count = pairs["segments"]
        segment_info_off, segment_info_count = pairs["segment_info"]
        path_background_off, path_background_len = pairs["path_background"]
        background_info_off, background_info_count = pairs["background_info"]
        path_console_off, path_console_len = pairs["path_console"]
        console_info_off, console_info_count = pairs["console_info"]

        games.append({
            "display_name": read_string(buf, name_off, name_len),
            "ref": read_string(buf, ref_off, ref_len),
            "date": read_string(buf, date_off, date_len),
            "manufacturer": manufacturer_id,
            "manufacturer_name": MANUFACTURER_NAMES.get(manufacturer_id, f"unknown({manufacturer_id})"),
            "rom_off": rom_off, "rom_size": rom_size,
            "melody_off": melody_off, "melody_size": melody_size,
            "path_segment": read_string(buf, path_segment_off, path_segment_len),
            "path_background": read_string(buf, path_background_off, path_background_len),
            "path_console": read_string(buf, path_console_off, path_console_len),
            "segments": read_segments(buf, segments_off, segments_count),
            "segment_info": read_u16_list(buf, segment_info_off, segment_info_count),
            "background_info": read_u16_list(buf, background_info_off, background_info_count),
            "console_info": read_u16_list(buf, console_info_off, console_info_count),
        })
        off += GAME_ENTRY_SIZE
    return games


def texture_basename(path: str) -> str:
    """path_segment/path_background/path_console are runtime paths like
    'romfs:/gfx/segment_ac_01.t3x' - the file table stores bare filenames
    (e.g. 'segment_ac_01.png'), so match by stem + known extensions rather
    than the runtime path's own extension (.t3x for 3DS packs has no file-
    table entry at all; only the .png source does, see write_rom_pack_v1).
    """
    if not path:
        return ""
    return path.rsplit("/", 1)[-1]


def resolve_texture_file(path: str, file_table: dict[str, dict]) -> str | None:
    base = texture_basename(path)
    if not base:
        return None
    if base in file_table:
        return base
    stem = base.rsplit(".", 1)[0]
    for candidate in file_table:
        if candidate.rsplit(".", 1)[0] == stem:
            return candidate
    return None


def sanitize(name: str) -> str:
    return "".join(c if (c.isalnum() or c in "-_.") else "_" for c in name) or "unnamed"


def unpack(pack_path: Path, out_dir: Path) -> None:
    buf = pack_path.read_bytes()
    header = parse_header(buf)
    file_table = read_file_table(buf, header)
    games = read_game_entries(buf, header)

    out_dir.mkdir(parents=True, exist_ok=True)
    gfx_dir = out_dir / "gfx"
    games_dir = out_dir / "games"
    gfx_dir.mkdir(exist_ok=True)
    games_dir.mkdir(exist_ok=True)

    print(f"Pack: {pack_path}")
    print(f"  format_version={header['format_version']} platform={header['platform']} "
          f"content_version={header['content_version']} game_count={header['game_count']} "
          f"file_count={header['file_count']}")

    # Extract every shared file once (segment/background/console textures,
    # UI textures, and anything else bundled by filename - e.g. David & John
    # art in a pack built before those games were wired into games_path).
    for name, info in file_table.items():
        data = buf[info["data_off"]: info["data_off"] + info["data_size"]]
        (gfx_dir / sanitize(name)).write_bytes(data)

    referenced_files: set[str] = set()
    used_refs: dict[str, int] = {}
    game_summaries = []

    for game in games:
        ref = game["ref"] or "NO_REF"
        used_refs[ref] = used_refs.get(ref, 0) + 1
        folder_name = ref if used_refs[ref] == 1 else f"{ref}_{used_refs[ref]}"
        game_dir = games_dir / sanitize(folder_name)
        game_dir.mkdir(exist_ok=True)

        if game["rom_size"]:
            (game_dir / "rom.bin").write_bytes(buf[game["rom_off"]: game["rom_off"] + game["rom_size"]])
        if game["melody_size"]:
            (game_dir / "melody.bin").write_bytes(buf[game["melody_off"]: game["melody_off"] + game["melody_size"]])

        textures = {}
        for label in ("path_segment", "path_background", "path_console"):
            raw_path = game[label]
            resolved = resolve_texture_file(raw_path, file_table)
            if resolved:
                referenced_files.add(resolved)
            textures[label] = {"pack_path": raw_path, "gfx_file": resolved}

        metadata = {
            "display_name": game["display_name"],
            "ref": game["ref"],
            "date": game["date"],
            "manufacturer": game["manufacturer"],
            "manufacturer_name": game["manufacturer_name"],
            "rom_size": game["rom_size"],
            "melody_size": game["melody_size"],
            "textures": textures,
            "segments": game["segments"],
            "segment_info": game["segment_info"],
            "background_info": game["background_info"],
            "console_info": game["console_info"],
        }
        (game_dir / "metadata.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")

        game_summaries.append({"folder": folder_name, "display_name": game["display_name"], "ref": game["ref"]})

    orphan_files = sorted(set(file_table.keys()) - referenced_files)
    missing_files = sorted({
        texture_basename(game[label]) or "(empty)"
        for game in games
        for label in ("path_segment", "path_background", "path_console")
        if game[label] and resolve_texture_file(game[label], file_table) is None
    })

    manifest = {
        "source_pack": str(pack_path),
        "header": {k: v for k, v in header.items()},
        "games": game_summaries,
        "gfx_files": sorted(file_table.keys()),
        "gfx_files_not_referenced_by_any_game": orphan_files,
        "textures_referenced_but_missing_from_pack": missing_files,
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")

    print(f"  -> {len(games)} game folder(s) under {games_dir}")
    print(f"  -> {len(file_table)} shared file(s) under {gfx_dir}")
    if orphan_files:
        print(f"  {len(orphan_files)} gfx file(s) bundled but not referenced by any game entry "
              f"(see manifest.json): {', '.join(orphan_files[:10])}" + (" ..." if len(orphan_files) > 10 else ""))
    if missing_files:
        print(f"  WARNING: {len(missing_files)} texture(s) referenced by a game but not found in the pack: "
              f"{', '.join(missing_files)}")
    print(f"Wrote manifest: {out_dir / 'manifest.json'}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pack", type=Path, help="Path to the .ykp pack to unpack")
    parser.add_argument("out_dir", type=Path, help="Destination folder (created if missing)")
    args = parser.parse_args()

    if not args.pack.is_file():
        parser.error(f"pack not found: {args.pack}")

    unpack(args.pack, args.out_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
