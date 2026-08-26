# Injects extra named files into an already-built .ykp rompack, without
# rerunning the full convert_3ds.py pipeline.
#
# Written for one specific job - adding gfx2x_open_source/*.png (the "David
# and John" games' art) to a pack that predates that support - but works on
# any file-by-filename addition, since the pack format looks files up by
# name at runtime (gw_pack::get_file_bytes()), completely independent of the
# game table. See write_rom_pack_v1() in convert_3ds.py for the canonical
# writer this mirrors; this script never touches game data, only the file
# table + the data each file's offset points into.
#
# Pack layout (all offsets absolute, little-endian, matches write_rom_pack_v1):
#   PackHeaderV2  (36 bytes / 9x uint32):
#       magic('YKP1'), format_version, platform, content_version,
#       game_count, file_count, games_offset, files_offset, data_offset
#   GameEntryV2 x game_count (100 bytes / 25x uint32 each) at games_offset:
#       12 (offset,length) pairs referencing the data blob, plus manufacturer_id.
#       Offsets are at even indices 0,2,4,...,22; index 24 is manufacturer_id
#       (not an offset). This script shifts every offset field by `delta`
#       when the data blob moves - it never reads/writes the *values* those
#       offsets point to for games, so game data itself is untouched.
#   FileEntryV1 x file_count (16 bytes / 4x uint32 each) at files_offset:
#       name_offset, name_length, data_offset, data_size
#   Data blob at data_offset: every string/blob referenced above, back to back.
#
# Since game_count never changes here, games_offset and files_offset are
# invariant; only file_count (and therefore data_offset) grows. So the whole
# operation is: shift every data-blob-referencing offset in the game table by
# delta = (new_file_count - old_file_count) * 16, then append fresh file
# entries (for filenames not already present) after the existing ones -
# reusing the *entire* old data blob byte-for-byte, just relocated.

import argparse
import shutil
import struct
from pathlib import Path

MAGIC = 0x31504B59  # 'YKP1'
HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
GAME_ENTRY_FIELDS = 25
GAME_ENTRY_OFFSET_INDICES = (0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22)
FILE_ENTRY_SIZE = 16


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


def read_existing_file_entries(buf: bytes, header: dict) -> list[dict]:
    entries = []
    off = header["files_offset"]
    for _ in range(header["file_count"]):
        name_off, name_len, data_off, data_size = struct.unpack("<IIII", buf[off:off + FILE_ENTRY_SIZE])
        name = buf[name_off:name_off + name_len].decode("utf-8")
        entries.append({"name": name, "name_off": name_off, "name_len": name_len, "data_off": data_off, "data_size": data_size})
        off += FILE_ENTRY_SIZE
    return entries


def shift_game_entries(buf: bytes, header: dict, delta: int) -> bytes:
    out = bytearray()
    off = header["games_offset"]
    for _ in range(header["game_count"]):
        vals = list(struct.unpack("<" + "I" * GAME_ENTRY_FIELDS, buf[off:off + GAME_ENTRY_SIZE]))
        for idx in GAME_ENTRY_OFFSET_INDICES:
            if vals[idx] != 0:  # 0 means "absent" (e.g. no melody/background) - never an address here
                vals[idx] += delta
        out.extend(struct.pack("<" + "I" * GAME_ENTRY_FIELDS, *vals))
        off += GAME_ENTRY_SIZE
    return bytes(out)


def inject(pack_path: Path, assets_dir: Path, out_path: Path) -> None:
    buf = pack_path.read_bytes()
    header = parse_header(buf)

    asset_files = sorted(p for p in assets_dir.iterdir() if p.is_file())
    if not asset_files:
        raise RuntimeError(f"No files found in '{assets_dir}'")
    new_names = {p.name for p in asset_files}

    old_entries = read_existing_file_entries(buf, header)
    kept_entries = [e for e in old_entries if e["name"] not in new_names]
    replaced = [e["name"] for e in old_entries if e["name"] in new_names]

    new_file_count = len(kept_entries) + len(asset_files)
    delta = (new_file_count - header["file_count"]) * FILE_ENTRY_SIZE
    new_data_offset = header["data_offset"] + delta

    game_entries_bytes = shift_game_entries(buf, header, delta)

    # Preserve the *entire* old data blob byte-for-byte, just relocated by
    # delta - this is what the shifted game-entry offsets above still point
    # into (game name/ref/date/rom/segments/path strings, none of which are
    # file-table entries and so are otherwise invisible to this script).
    # Kept file entries point into it too, so they only need their offsets
    # shifted, not re-copied. Bytes belonging to *replaced* file entries
    # become harmless dead space (nothing references them anymore) - a few
    # KB of waste is a fair trade for not having to understand or touch any
    # game-table content.
    data = bytearray(buf[header["data_offset"]:])

    def append_chunk(b: bytes) -> int:
        off = new_data_offset + len(data)
        data.extend(b)
        return off

    file_entries = bytearray()
    for e in kept_entries:
        file_entries.extend(struct.pack("<IIII", e["name_off"] + delta, e["name_len"], e["data_off"] + delta, e["data_size"]))

    for p in asset_files:
        name_bytes = p.name.encode("utf-8")
        blob = p.read_bytes()
        n_off = append_chunk(name_bytes)
        d_off = append_chunk(blob)
        file_entries.extend(struct.pack("<IIII", n_off, len(name_bytes), d_off, len(blob)))

    new_header = struct.pack(
        "<IIIIIIIII",
        MAGIC,
        header["format_version"],
        header["platform"],
        header["content_version"],
        header["game_count"],
        new_file_count,
        header["games_offset"],
        header["files_offset"],
        new_data_offset,
    )

    out_path.write_bytes(new_header + game_entries_bytes + bytes(file_entries) + bytes(data))

    added = sorted(new_names - set(replaced))
    print(f"Injected {len(asset_files)} file(s) from '{assets_dir}' into pack:")
    if added:
        print(f"  added:    {', '.join(added)}")
    if replaced:
        print(f"  replaced: {', '.join(sorted(replaced))}")
    print(f"  game_count unchanged ({header['game_count']}), file_count {header['file_count']} -> {new_file_count}")
    print(f"Wrote: {out_path}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pack", type=Path, help="Path to the existing .ykp pack to update")
    parser.add_argument(
        "--assets-dir",
        type=Path,
        default=None,
        help="Directory of files to inject by filename (default: gfx2x_open_source/ next to this script)",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=None,
        help="Output path (default: overwrite the input pack in place, after backing it up to <pack>.bak)",
    )
    args = parser.parse_args()

    pack_path: Path = args.pack
    if not pack_path.is_file():
        parser.error(f"pack not found: {pack_path}")

    assets_dir: Path = args.assets_dir or (Path(__file__).resolve().parent.parent / "gfx2x_open_source")
    if not assets_dir.is_dir():
        parser.error(f"assets dir not found: {assets_dir}")

    in_place = args.out is None
    out_path: Path = args.out or pack_path

    if in_place:
        backup_path = pack_path.with_name(pack_path.name + ".bak")
        shutil.copyfile(pack_path, backup_path)
        print(f"Backed up original pack to: {backup_path}")
        # Read from the untouched backup, write to the real target - avoids
        # reading a file we're simultaneously overwriting.
        inject(backup_path, assets_dir, out_path)
    else:
        inject(pack_path, assets_dir, out_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
