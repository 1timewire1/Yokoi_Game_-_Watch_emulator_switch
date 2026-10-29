"""Builds one BrickEmuPy game (e.g. ZeldaNelsonic) on its own and merges it
into an existing pack, without touching the repo's generated files.

Steps (all from CONVERT_ROM/, Inkscape required - see external_apps_template.py):
  1. brick_import.py's import_game(): writes rom/decompress/<set>/<set>.brk,
     the split segment SVG / Background / Overlay, and rom/console/<set>.png.
  2. A temporary games_path_rgds.py holding only that game (any existing one
     is backed up and restored afterwards), then
     convert_switch.py --target rgds --game <key> --pack-name <tmp>, with its
     GW_ALL / GW_ROM / gfx outputs redirected into ./tmp/add_brick_game/.
  3. merge_pack_games.py <base pack> <tmp pack> --out <out pack>.

Usage:
  python add_brick_game.py <BrickEmuPy dir> <base.ykp> <out.ykp> [--brick ZeldaNelsonic] [--replace]
"""
import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import brick_import as bi  # noqa: E402


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("brick_dir", type=Path)
    ap.add_argument("base_pack", type=Path)
    ap.add_argument("out_pack", type=Path)
    ap.add_argument("--brick", default="ZeldaNelsonic", help=".brick name (brick_games.json 'brick')")
    ap.add_argument("--replace", action="store_true", help="replace the game if the base pack already has it")
    a = ap.parse_args()

    meta = json.loads(bi.META.read_text(encoding="utf-8"))
    game = next((g for g in meta["games"] if g["brick"] == a.brick or g["set"] == a.brick.lower()), None)
    if not game:
        sys.exit(f"{a.brick} is not in brick_games.json")

    gp = HERE / "games_path_rgds.py"
    # games_path_rgds.py isn't in git, so its backup lives next to it (not in
    # the work dir this script wipes) and an existing one means an earlier
    # run never got to restore it - stop rather than back up the one-game
    # stand-in over the real list.
    backup = HERE / "games_path_rgds.py.add_brick_game.bak"
    if backup.exists():
        sys.exit(f"{backup.name} exists - an earlier run was interrupted. Check it against "
                 f"{gp.name}, restore whichever is the full list, then delete the .bak.")

    work = HERE / "tmp" / "add_brick_game"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    had_gp = gp.exists()
    if had_gp:
        shutil.copy2(gp, backup)

    captured = {}

    def capture(key, block):
        captured["key"], captured["block"] = key, block

    try:
        # 1. container + split art (brick_import writes into rom/decompress, rom/console)
        bi.upsert_games_path = capture
        bi.CONSOLE.mkdir(parents=True, exist_ok=True)
        bi.DECOMPRESS.mkdir(parents=True, exist_ok=True)
        if not bi.import_game(a.brick_dir, game):
            sys.exit("import failed - see the message above")
        key = captured["key"]

        # 2. a games_path with just this game, then build a one-game pack
        block = captured["block"].replace('            , "', '            "', 1)
        gp.write_text("games_path = {\n" + block + "\n}\n", encoding="utf-8")
        tmp_pack = f"add_brick_game_{game['set']}"
        cmd = [sys.executable, "convert_switch.py", "--target", "rgds", "--game", key, "--clean",
               "--pack-name", tmp_pack,
               "--out-gw-all", str(work / "GW_ALL_rgds.h"),
               "--out-gw-rom-dir", str(work / "GW_ROM_RGDS"),
               "--out-gfx", str(work / "gfx2x") + "/"]
        print(">", " ".join(cmd))
        subprocess.run(cmd, cwd=HERE, check=True)

        # 3. merge into the user's pack
        donor = HERE / f"{tmp_pack}.ykp"
        cmd = [sys.executable, "merge_pack_games.py", str(a.base_pack), str(donor), "--out", str(a.out_pack)]
        if a.replace:
            cmd.append("--replace")
        print(">", " ".join(cmd))
        subprocess.run(cmd, cwd=HERE, check=True)
        for p in HERE.glob(f"{tmp_pack}*.ykp"):
            p.unlink()
    finally:
        if had_gp:
            shutil.copy2(backup, gp)
            backup.unlink()
        elif gp.exists():
            gp.unlink()

    print(f"\nOK: {game['name']} added -> {a.out_pack}")


if __name__ == "__main__":
    main()
