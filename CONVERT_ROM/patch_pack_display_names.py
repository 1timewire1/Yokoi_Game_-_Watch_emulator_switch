"""Syncs a pack's game display names to what GNW_LIST.md implies, without a
full rebuild: the title column, minus only the "Game & Watch:" / "Micro Vs.
System:" series prefix (generate_games_path._strip_title_prefix), minus any
"(Tronica)"/"(Elektronika)" suffix - games sit under a heading for their
manufacturer, so it's redundant. Kept: the three deliberate exceptions,
Bassmate Computer (Telko), Altair (Nauchpribor) and Uchitel' arifmetiki
(prototype).

Names live in the shared data blob, so each changed name is appended at the
end of the file and its game record repointed; the old strings become dead
bytes that the next inject_pack_assets.py run compacts away. Doesn't touch
content_version (names only - an older pack still loads fine). Games not in
GNW_LIST.md (McDonald's / Burger Bard are compiled in, not in the pack) are
left alone.

Usage:  python patch_pack_display_names.py <pack.ykp> [--out patched.ykp]
        (default: in place, after copying the original to <pack>.pre_names)
"""
import argparse
import re
import shutil
import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import inject_pack_assets as ip  # noqa: E402
from source.generate_games_path import _strip_title_prefix  # noqa: E402

HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
KEEP_SUFFIX_REFS = {"BASSMATE", "NALTAIR", "UCHITARI"}
SUFFIX_RE = re.compile(r"\s*\((Tronica|Elektronika)\)\s*$")


def norm(ref: str) -> str:
    return ref.replace("-", "_").upper()


def desired_names(gnw_list: Path = SCRIPT_DIR / "GNW_LIST.md") -> dict:
    out = {}
    for line in gnw_list.read_text(encoding="utf-8").splitlines():
        cols = line.split("|")
        if len(cols) > 6 and re.match(r"\s*\d+\s*$", cols[1]) and cols[3].strip().endswith(".zip"):
            ref = norm(cols[2].strip())
            name = _strip_title_prefix(cols[4].strip())
            if ref not in KEEP_SUFFIX_REFS:
                name = SUFFIX_RE.sub("", name)
            out[ref] = name
    return out


def patch(buf: bytearray, names: dict) -> dict:
    header = ip.parse_header(bytes(buf))
    changed = {}
    for i in range(header["game_count"]):
        base = HEADER_SIZE + GAME_ENTRY_SIZE * i
        vals = list(struct.unpack_from("<25I", buf, base))
        name = buf[vals[0]: vals[0] + vals[1]].decode("utf-8")
        ref = buf[vals[2]: vals[2] + vals[3]].decode("utf-8")
        new = names.get(norm(ref), name)
        if new == name:
            continue
        data = new.encode("utf-8")
        vals[0], vals[1] = len(buf), len(data)
        buf.extend(data)
        struct.pack_into("<25I", buf, base, *vals)
        changed[ref] = (name, new)
    return changed


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("pack", type=Path)
    ap.add_argument("--out", type=Path, default=None)
    args = ap.parse_args()

    buf = bytearray(args.pack.read_bytes())
    changed = patch(buf, desired_names())
    ip.validate_pack_bytes(bytes(buf))
    out = args.out
    if out is None:
        backup = args.pack.with_name(args.pack.name + ".pre_names")
        shutil.copyfile(args.pack, backup)
        print(f"Backed up original to: {backup}")
        out = args.pack
    out.write_bytes(bytes(buf))
    for ref, (old, new) in sorted(changed.items()):
        print(f"  {ref}: {old!r} -> {new!r}")
    print(f"Renamed {len(changed)} game(s); wrote {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
