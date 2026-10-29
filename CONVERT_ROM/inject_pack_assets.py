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
# Since game_count never changes here, games_offset and files_offset stay
# invariant; only file_count (and therefore data_offset) changes. The data
# blob itself is rebuilt from scratch every run by a mark-and-sweep pass
# (compact_data_blob()): every (offset, length) actually referenced by a
# live game-table entry or a kept/new file-table entry gets copied into a
# fresh blob exactly once, and everything else - most importantly, a
# *replaced* file's now-unreferenced old bytes - is dropped rather than
# left as dead weight. An earlier version of this script instead preserved
# the entire old blob byte-for-byte and only ever appended, on the theory
# that "a few KB of dead space per replace is a fair trade" - true for the
# one-time 5-file David-and-John addition this was written for, but re-
# running it repeatedly against the same large, frequently-changing
# LCDonald asset set (each run "replacing" ~150+ PNG/OGG files without
# reclaiming the previous run's copies) compounded that into ~110MB of
# orphaned data across 4 injection passes in one real session - see the
# git history around 2026-09-22 for the incident this rewrite fixes.

import argparse
import shutil
import struct
from pathlib import Path

MAGIC = 0x31504B59  # 'YKP1'
HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
GAME_ENTRY_FIELDS = 25
GAME_ENTRY_OFFSET_INDICES = (0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22)
# Per-field element size in bytes, matching write_rom_pack_v1()'s append_*()
# helpers and gw_pack.cpp's GameEntryDiskV1 reader - NOT uniformly 1. Most
# pairs are (offset, byte_length): name/ref/date/rom/melody/path_segment/
# path_background/path_console. Four are (offset, ELEMENT COUNT) instead:
# segments (SegmentDiskV1, 21 bytes/struct - id0/id1/id2: u8 x3, pos_scr_x/y:
# i32 x2, pos_tex_x/y+size_tex_x/y: u16 x4, color_index/screen: u8 x2,
# #pragma pack(push,1) so no padding) and segment_info/background_info/
# console_info (uint16_t, 2 bytes/element). Treating every pair as already-
# byte-accurate (an earlier version of this script's mark-and-sweep
# compactor did) copies only `count` bytes instead of `count * element_size`
# for these four - truncating the segments/info arrays into garbage while
# leaving their *stored* count untouched, so the renderer thinks there's
# still N segments worth of (corrupted) data. McDonald's/David-and-John
# games are compiled directly into the .nro and never go through this at
# all (only their PNG textures - the plain-byte-length file table entries -
# come from the pack), which is why only every *other* game's graphics
# broke when this bug was live - see the 2026-09-23 incident this fixes.
GAME_ENTRY_ELEMENT_SIZE = {
    0: 1, 2: 1, 4: 1, 6: 1, 8: 1, 10: 1,   # name, ref, date, rom, melody, path_segment
    12: 21,                                 # segments (SegmentDiskV1)
    14: 2,                                  # segment_info (uint16_t)
    16: 1,                                  # path_background
    18: 2,                                  # background_info (uint16_t)
    20: 1,                                  # path_console
    22: 2,                                  # console_info (uint16_t)
}
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


def read_game_ranges(buf: bytes, header: dict) -> list[tuple[int, int]]:
    """Every (offset, byte_length) a game-table entry references into the
    data blob (12 pairs/game - name/ref/date/rom/segment/path strings etc) -
    converting each pair's stored second value through
    GAME_ENTRY_ELEMENT_SIZE first, since 4 of the 12 store an element COUNT
    there, not an already-correct byte length (see that constant's own
    comment)."""
    ranges = []
    off = header["games_offset"]
    for _ in range(header["game_count"]):
        vals = struct.unpack("<" + "I" * GAME_ENTRY_FIELDS, buf[off:off + GAME_ENTRY_SIZE])
        for idx in GAME_ENTRY_OFFSET_INDICES:
            o, count = vals[idx], vals[idx + 1]
            if o != 0:  # 0 means "absent" (e.g. no melody/background) - never a real address
                ranges.append((o, count * GAME_ENTRY_ELEMENT_SIZE[idx]))
        off += GAME_ENTRY_SIZE
    return ranges


def compact_data_blob(buf: bytes, header: dict, live_ranges: list[tuple[int, int]]) -> tuple[bytes, dict[int, int]]:
    """Copies every distinct live (offset, length) range into a fresh,
    tightly-packed blob (deduplicating identical ranges referenced more
    than once), returning that blob plus an old-offset -> new-offset map
    for every range's *start* - the only remapping callers need, since
    every live range is copied verbatim (same bytes, same length, just
    relocated). Zero-length ranges (an empty string field, or one game's
    unused slot sharing a stale/arbitrary offset with something else -
    seen in practice) are skipped entirely: 0 bytes are read from them
    either way, so callers just emit offset 0 for any zero-length field
    rather than looking it up here."""
    new_blob = bytearray()
    remap: dict[int, int] = {}
    seen_lengths: dict[int, int] = {}
    for o, length in live_ranges:
        if length == 0:
            continue
        if o in remap:
            if seen_lengths[o] != length:
                raise ValueError(f"offset {o} referenced with two different non-zero lengths ({seen_lengths[o]} vs {length}) - pack layout assumption broken")
            continue
        remap[o] = len(new_blob)
        seen_lengths[o] = length
        new_blob.extend(buf[o:o + length])
    return bytes(new_blob), remap


def validate_pack_bytes(buf: bytes) -> None:
    """Re-parses freshly-built pack bytes before they ever reach disk, as a
    regression guard for exactly the two bugs an earlier version of this
    script's compactor shipped in one real incident (2026-09-23): a stale
    data_offset producing a header that fails parse_header()'s own
    files_offset/file_count/data_offset consistency check, and treating a
    game-table field's element COUNT as if it were already a byte length
    (silently truncating segments/segment_info/background_info/console_info
    into garbage while every *other* field - and the stored count itself -
    stayed correct, which is what made it hard to spot: nothing crashed,
    textures still loaded, only in-game positions/sizes were wrong for
    every pack-native game). Every game's segment array is fully decoded
    (not just spot-checked) and sanity-ranged; raises on the first problem
    found rather than writing a pack that looks fine until it's on hardware.
    """
    header = parse_header(buf)  # raises on any header/region-size inconsistency

    off = header["games_offset"]
    for i in range(header["game_count"]):
        vals = struct.unpack("<" + "I" * GAME_ENTRY_FIELDS, buf[off:off + GAME_ENTRY_SIZE])
        name = buf[vals[0]:vals[0] + vals[1]].decode("utf-8", errors="replace")
        for idx in GAME_ENTRY_OFFSET_INDICES:
            o, count = vals[idx], vals[idx + 1]
            if o == 0 or count == 0:
                continue
            byte_len = count * GAME_ENTRY_ELEMENT_SIZE[idx]
            if o + byte_len > len(buf):
                raise ValueError(f"game {i} ({name!r}) field at index {idx}: range [{o}:{o + byte_len}] runs past end of pack ({len(buf)} bytes)")
            if idx == 12:  # segments - decode every one, not just a spot check
                seg_bytes = buf[o:o + byte_len]
                for si in range(count):
                    id0, id1, id2, _psx, _psy, ptx, pty, stx, sty, _ci, screen = struct.unpack(
                        "<BBBiiHHHHBB", seg_bytes[si * 21:si * 21 + 21]
                    )
                    # Loose sanity bounds, not exact validation (real atlas/col/line/word
                    # ranges vary per game) - catches gross corruption (byte-truncated
                    # arrays decoding into wildly implausible values), not subtle bugs.
                    # id0 isn't checked: BrickEmuPy-core games use the whole u8 range
                    # (HT943's display is its 256-nibble RAM, id0 = nibble index).
                    if id1 > 63 or screen > 8 or ptx > 8192 or pty > 8192 or stx > 8192 or sty > 8192:
                        raise ValueError(
                            f"game {i} ({name!r}) segment {si}/{count} looks corrupted: "
                            f"id=({id0},{id1},{id2}) pos_tex=({ptx},{pty}) size_tex=({stx},{sty}) screen={screen}"
                        )
        off += GAME_ENTRY_SIZE

    for e in read_existing_file_entries(buf, header):
        if e["data_off"] + e["data_size"] > len(buf):
            raise ValueError(f"file entry {e['name']!r}: range runs past end of pack")


def inject(pack_path: Path, assets_dir: Path, out_path: Path, content_version: int | None = None) -> None:
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
    # The file table's size depends on file_count, so the data blob's start
    # position moves whenever file_count changes (adding any new filename,
    # not just growing/shrinking replaced content) - must be recomputed from
    # new_file_count, never reused from the input header's (now-stale) value.
    new_data_offset = header["files_offset"] + new_file_count * FILE_ENTRY_SIZE

    # Mark: every byte range still referenced by something that survives
    # this operation - the whole game table (game_count never changes here)
    # plus every kept file entry's name+data. Anything else (a replaced
    # file's old bytes, or dead space from a prior run that predates this
    # rewrite) is implicitly swept just by not being in this list.
    live_ranges = read_game_ranges(buf, header)
    for e in kept_entries:
        live_ranges.append((e["name_off"], e["name_len"]))
        live_ranges.append((e["data_off"], e["data_size"]))

    new_blob, remap = compact_data_blob(buf, header, live_ranges)

    # Game table: same values, offsets rewritten through the compacted map.
    game_entries_bytes = bytearray()
    off = header["games_offset"]
    for _ in range(header["game_count"]):
        vals = list(struct.unpack("<" + "I" * GAME_ENTRY_FIELDS, buf[off:off + GAME_ENTRY_SIZE]))
        for idx in GAME_ENTRY_OFFSET_INDICES:
            # A zero-length field was never added to remap (see
            # compact_data_blob) - its offset value doesn't matter (0 bytes
            # are read from it either way), so leave it as whatever it was
            # rather than looking it up.
            if vals[idx] != 0 and vals[idx + 1] != 0:
                vals[idx] = remap[vals[idx]] + new_data_offset
        game_entries_bytes.extend(struct.pack("<" + "I" * GAME_ENTRY_FIELDS, *vals))
        off += GAME_ENTRY_SIZE

    def remapped(offset: int, length: int) -> int:
        # See compact_data_blob()'s doc comment - zero-length ranges were
        # never added to remap, and their offset is meaningless either way.
        return remap[offset] + new_data_offset if length != 0 else 0

    # File table: kept entries remapped through the same compacted map,
    # then new/replaced files appended fresh onto the growing blob.
    file_entries = bytearray()
    for e in kept_entries:
        file_entries.extend(struct.pack(
            "<IIII",
            remapped(e["name_off"], e["name_len"]), e["name_len"],
            remapped(e["data_off"], e["data_size"]), e["data_size"],
        ))

    new_blob = bytearray(new_blob)
    for p in asset_files:
        name_bytes = p.name.encode("utf-8")
        blob = p.read_bytes()
        n_off = new_data_offset + len(new_blob)
        new_blob.extend(name_bytes)
        d_off = new_data_offset + len(new_blob)
        new_blob.extend(blob)
        file_entries.extend(struct.pack("<IIII", n_off, len(name_bytes), d_off, len(blob)))

    final_content_version = header["content_version"] if content_version is None else content_version
    new_header = struct.pack(
        "<IIIIIIIII",
        MAGIC,
        header["format_version"],
        header["platform"],
        final_content_version,
        header["game_count"],
        new_file_count,
        header["games_offset"],
        header["files_offset"],
        new_data_offset,
    )

    out_bytes = new_header + bytes(game_entries_bytes) + bytes(file_entries) + bytes(new_blob)
    validate_pack_bytes(out_bytes)
    out_path.write_bytes(out_bytes)

    added = sorted(new_names - set(replaced))
    old_size = len(buf)
    new_size = len(new_header) + len(game_entries_bytes) + len(file_entries) + len(new_blob)
    print(f"Injected {len(asset_files)} file(s) from '{assets_dir}' into pack:")
    if added:
        print(f"  added:    {', '.join(added)}")
    if replaced:
        print(f"  replaced: {', '.join(sorted(replaced))}")
    print(f"  game_count unchanged ({header['game_count']}), file_count {header['file_count']} -> {new_file_count}")
    if final_content_version != header["content_version"]:
        print(f"  content_version {header['content_version']} -> {final_content_version}")
    print(f"  pack size {old_size:,} -> {new_size:,} bytes (compacted - see this file's own header comment)")
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
    parser.add_argument(
        "--content-version",
        type=int,
        default=None,
        help="Also set the pack's content_version field (see CONVERT_ROM/convert_switch.py's "
             "ROMPACK_CONTENT_VERSION comment) - default: leave it as-is",
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
        inject(backup_path, assets_dir, out_path, args.content_version)
    else:
        inject(pack_path, assets_dir, out_path, args.content_version)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
