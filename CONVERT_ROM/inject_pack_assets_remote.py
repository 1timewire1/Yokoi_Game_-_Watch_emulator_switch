# Same job as inject_pack_assets.py - injects extra named files into an
# already-built .ykp rompack, without rerunning the full convert_3ds.py
# pipeline - but pulls the files from the GitHub repo instead of a local
# gfx2x_open_source/ directory, for anyone running this without a clone of
# the repo on hand (e.g. just has an existing pack and wants the David and
# John assets added to it). See inject_pack_assets.py's own header comment
# for the full pack-format writeup this mirrors; this script only differs in
# where the injected files' bytes come from.
#
# Network access only: GitHub's Contents API to list the directory, then
# each file's raw download_url - both plain HTTPS GETs via the standard
# library (no extra pip dependency). Nothing is written to disk except the
# output pack itself.

import argparse
import json
import shutil
import struct
import urllib.error
import urllib.request
from pathlib import Path

MAGIC = 0x31504B59  # 'YKP1'
HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
GAME_ENTRY_FIELDS = 25
GAME_ENTRY_OFFSET_INDICES = (0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22)
FILE_ENTRY_SIZE = 16

DEFAULT_REPO = "1timewire1/Yokoi_Game_-_Watch_emulator_switch"
DEFAULT_BRANCH = "switch-port"
DEFAULT_PATH = "gfx2x_open_source"

# GitHub's API rejects requests with no User-Agent header.
USER_AGENT = "yokoi-inject-pack-assets-remote"


def http_get(url: str) -> bytes:
    req = urllib.request.Request(url, headers={"User-Agent": USER_AGENT})
    try:
        with urllib.request.urlopen(req) as resp:
            return resp.read()
    except urllib.error.HTTPError as e:
        raise RuntimeError(f"GET {url} failed: HTTP {e.code} {e.reason}") from e
    except urllib.error.URLError as e:
        raise RuntimeError(f"GET {url} failed: {e.reason}") from e


def fetch_asset_list(repo: str, branch: str, path: str) -> list[dict]:
    url = f"https://api.github.com/repos/{repo}/contents/{path}?ref={branch}"
    entries = json.loads(http_get(url))
    if isinstance(entries, dict):
        # The API returns a single object instead of a list if `path` is a
        # file rather than a directory.
        raise RuntimeError(f"'{path}' on {repo}@{branch} is not a directory")
    files = [e for e in entries if e.get("type") == "file" and e.get("download_url")]
    if not files:
        raise RuntimeError(f"No files found under '{path}' on {repo}@{branch}")
    return files


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


def inject(pack_path: Path, assets: dict[str, bytes], out_path: Path) -> None:
    buf = pack_path.read_bytes()
    header = parse_header(buf)

    new_names = set(assets.keys())

    old_entries = read_existing_file_entries(buf, header)
    kept_entries = [e for e in old_entries if e["name"] not in new_names]
    replaced = [e["name"] for e in old_entries if e["name"] in new_names]

    new_file_count = len(kept_entries) + len(assets)
    delta = (new_file_count - header["file_count"]) * FILE_ENTRY_SIZE
    new_data_offset = header["data_offset"] + delta

    game_entries_bytes = shift_game_entries(buf, header, delta)

    # Preserve the *entire* old data blob byte-for-byte, just relocated by
    # delta - see inject_pack_assets.py's own header comment for why (same
    # binary format, same reasoning, unchanged here).
    data = bytearray(buf[header["data_offset"]:])

    def append_chunk(b: bytes) -> int:
        off = new_data_offset + len(data)
        data.extend(b)
        return off

    file_entries = bytearray()
    for e in kept_entries:
        file_entries.extend(struct.pack("<IIII", e["name_off"] + delta, e["name_len"], e["data_off"] + delta, e["data_size"]))

    for name in sorted(assets.keys()):
        blob = assets[name]
        name_bytes = name.encode("utf-8")
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
    print(f"Injected {len(assets)} file(s) into pack:")
    if added:
        print(f"  added:    {', '.join(added)}")
    if replaced:
        print(f"  replaced: {', '.join(sorted(replaced))}")
    print(f"  game_count unchanged ({header['game_count']}), file_count {header['file_count']} -> {new_file_count}")
    print(f"Wrote: {out_path}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("pack", type=Path, help="Path to the existing .ykp pack to update")
    parser.add_argument("--repo", default=DEFAULT_REPO, help=f"GitHub 'owner/repo' to pull from (default: {DEFAULT_REPO})")
    parser.add_argument("--branch", default=DEFAULT_BRANCH, help=f"Branch/ref to pull from (default: {DEFAULT_BRANCH})")
    parser.add_argument("--path", default=DEFAULT_PATH, help=f"Directory within the repo to inject (default: {DEFAULT_PATH})")
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

    print(f"Fetching file list from {args.repo}@{args.branch}:{args.path} ...")
    remote_files = fetch_asset_list(args.repo, args.branch, args.path)

    assets: dict[str, bytes] = {}
    for entry in remote_files:
        name = entry["name"]
        print(f"  downloading {name} ({entry.get('size', '?')} bytes)")
        assets[name] = http_get(entry["download_url"])

    in_place = args.out is None
    out_path: Path = args.out or pack_path

    if in_place:
        backup_path = pack_path.with_name(pack_path.name + ".bak")
        shutil.copyfile(pack_path, backup_path)
        print(f"Backed up original pack to: {backup_path}")
        # Read from the untouched backup, write to the real target - avoids
        # reading a file we're simultaneously overwriting.
        inject(backup_path, assets, out_path)
    else:
        inject(pack_path, assets, out_path)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
