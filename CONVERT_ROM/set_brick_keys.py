"""Changes the Switch key binding of one button of a BrickEmuPy-ported game inside a .ykp pack,
without rebuilding anything else (artwork, other games: carried over unchanged). Needs only
Python 3 - no lxml / Pillow / Inkscape.

  python set_brick_keys.py <pack.ykp> --list [--game <text>]
  python set_brick_keys.py <pack.ykp> <out.ykp> --game <text> --key btnStep=*X [--key btnA=A|X ...]

--game matches the game's ref or name (case-insensitive, part of it is enough) and must match one game.
--key is <button>=<keys>, the same text brick_games.json uses ("*X" = turbo X, "A|*X", "LEFT|Y"...).
"""
import argparse
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import inject_pack_assets as ip  # noqa: E402
import merge_pack_games as mp  # noqa: E402


def parse_container(rom: bytes):
    if rom[:4] != b"BRK1":
        return None
    p = 4
    n = rom[p]; core = rom[p + 1:p + 1 + n]; p += 1 + n
    clock, optlen = struct.unpack_from("<II", rom, p); p += 8
    opts = rom[p:p + optlen].decode("utf-8").split("\n"); p += optlen
    (nblobs,) = struct.unpack_from("<I", rom, p); p += 4
    blobs = []
    for _ in range(nblobs):
        n = rom[p]; name = rom[p + 1:p + 1 + n]; p += 1 + n
        (size,) = struct.unpack_from("<I", rom, p); p += 4
        blobs.append((name, rom[p:p + size])); p += size
    if p != len(rom):
        raise RuntimeError("unexpected bytes after the BRK1 container")
    return core, clock, opts, blobs


def build_container(core: bytes, clock: int, opts, blobs):
    o = "\n".join(opts).encode("utf-8")
    out = bytearray(b"BRK1") + bytes([len(core)]) + core + struct.pack("<II", clock, len(o)) + o
    out += struct.pack("<I", len(blobs))
    for name, data in blobs:
        out += bytes([len(name)]) + name + struct.pack("<I", len(data)) + data
    return bytes(out)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("pack", type=Path)
    ap.add_argument("out", type=Path, nargs="?")
    ap.add_argument("--game")
    ap.add_argument("--key", action="append", default=[], metavar="BUTTON=KEYS")
    ap.add_argument("--list", action="store_true")
    a = ap.parse_args()

    buf, header, files, games = mp.load_pack(a.pack)
    recs = [mp.game_record(buf, files, g) for g in games]

    def matches(r):
        q = (a.game or "").lower()
        return q in r["ref"].lower() or q in r["display_name"].lower()

    if a.list:
        for r in recs:
            c = parse_container(r["rom_bytes"])
            if c and (not a.game or matches(r)):
                print(f"{r['ref']}  ({r['display_name']})")
                for line in c[2]:
                    if line.startswith("input."):
                        print("    " + line)
        return 0

    if not (a.out and a.game and a.key):
        ap.error("need <out.ykp>, --game and at least one --key (or use --list)")
    found = [i for i, r in enumerate(recs) if matches(r) and parse_container(r["rom_bytes"])]
    if len(found) != 1:
        print("--game must match exactly one BrickEmuPy game; matches:", [recs[i]["ref"] for i in found] or "none")
        return 1
    r = recs[found[0]]
    core, clock, opts, blobs = parse_container(r["rom_bytes"])
    for spec in a.key:
        btn, _, keys = spec.partition("=")
        if not keys:
            print(f"bad --key {spec!r}: use BUTTON=KEYS")
            return 1
        idx = [i for i, l in enumerate(opts) if l.startswith(f"input.{btn}=")]
        if len(idx) != 1:
            print(f"{r['ref']} has no button {btn!r}; its buttons:", [l.split("=")[0][6:] for l in opts if l.startswith("input.")])
            return 1
        fields = opts[idx[0]].split(";")
        print(f"  {r['ref']}: {btn}  {fields[-1]!r} -> {keys!r}")
        fields[-1] = keys
        opts[idx[0]] = ";".join(fields)
    r["rom_bytes"] = build_container(core, clock, opts, blobs)

    shared = {name: mp.file_bytes(buf, info) for name, info in files.items()}
    out = mp.write_pack(recs, shared, header)
    ip.validate_pack_bytes(out)
    a.out.write_bytes(out)
    print(f"Wrote {a.out}: {len(recs)} games, {len(shared)} files, {len(out):,} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
