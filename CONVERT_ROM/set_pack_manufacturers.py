"""Rewrites the stored manufacturer id of some games in a pack, in place in
their 100-byte game records (like patch_pack_manufacturers.py) - no rebuild.

Usage:  python set_pack_manufacturers.py <pack.ykp> REF=Manufacturer [REF=Manufacturer ...]
        Manufacturer = a source/manufacturer_ids.py MANUFACTURER_BY_NAME name,
        e.g.  CDKONG=Coleco "TMTRON=TOMY Company" BIGCAT9IN1=Apollo
"""
import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

import inject_pack_assets as ip  # noqa: E402
from source.manufacturer_ids import MANUFACTURER_BY_NAME  # noqa: E402

HEADER_SIZE = 36
GAME_ENTRY_SIZE = 100
MFR_FIELD = 24


def main() -> int:
    pack = Path(sys.argv[1])
    want = {}
    for arg in sys.argv[2:]:
        ref, name = arg.split("=", 1)
        want[ref.upper()] = MANUFACTURER_BY_NAME[name]
    buf = bytearray(pack.read_bytes())
    header = ip.parse_header(bytes(buf))
    done = set()
    for i in range(header["game_count"]):
        base = HEADER_SIZE + GAME_ENTRY_SIZE * i
        vals = list(struct.unpack_from("<25I", buf, base))
        ref = buf[vals[2]: vals[2] + vals[3]].decode("utf-8").upper()
        if ref in want:
            print(f"{ref}: {vals[MFR_FIELD]} -> {want[ref]}")
            vals[MFR_FIELD] = want[ref]
            struct.pack_into("<25I", buf, base, *vals)
            done.add(ref)
    missing = set(want) - done
    if missing:
        raise SystemExit(f"not in the pack: {sorted(missing)}")
    ip.validate_pack_bytes(bytes(buf))
    pack.write_bytes(bytes(buf))
    print(f"wrote {pack}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
