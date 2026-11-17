"""Builds one MAME VFD tabletop set (prepare_hmcs40.py's GAMES: the HMCS40
sets and the MELPS4 ones, e.g. cfrogger) on its own and merges it into an
existing pack - add_brick_game.py's flow for those sets.

Steps (all from CONVERT_ROM/, Inkscape required - see external_apps_template.py):
  1. prepare_hmcs40.prepare(<set>): rom/roms/<set>.zip (the MAME zip: ROM dump
     + <set>.svg) -> rom/decompress/<set>/ (.brk, segs SVG, Background.png)
     and rom/console/<set>.png (from hmcs40_consoles/<set>.png if present).
  2. A temporary games_path_rgds.py holding only that game (any existing one
     is backed up and restored afterwards), then
     convert_switch.py --target rgds --game <key> --pack-name <tmp>.
  3. merge_pack_games.py <base pack> <tmp pack> --out <out pack>.

Usage:
  py add_mame_game.py <set>[,<set>...] <base.ykp> <out.ykp> [--replace]
  e.g. py add_mame_game.py epacman2,epacman2r base.ykp out.ykp
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import prepare_hmcs40 as ph  # noqa: E402
import prepare_microvision as pm  # noqa: E402

# Microvision cartridges (prepare_microvision.py): found by sha1, any file name.
MV_CARTS = pm.load_carts()
MV_ROMS = None


def mv_prepare(name):
    global MV_ROMS
    if MV_ROMS is None:
        MV_ROMS = pm.find_roms(MV_CARTS)
    return pm.prepare(name, MV_CARTS[name], MV_ROMS[name])


def title(name):
    return pm.display_name(MV_CARTS[name]) if name in pm.KEYS else ph.GAMES[name][2]


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("set", help="MAME set name(s), comma-separated, e.g. cfrogger or epacman2,epacman2r")
    ap.add_argument("base_pack", type=Path)
    ap.add_argument("out_pack", type=Path)
    ap.add_argument("--replace", action="store_true", help="replace the game if the base pack already has it")
    a = ap.parse_args()
    sets = [x.strip().lower() for x in a.set.split(",") if x.strip()]
    for name in sets:
        if name in pm.KEYS and name in MV_CARTS:
            global MV_ROMS
            if MV_ROMS is None:
                MV_ROMS = pm.find_roms(MV_CARTS)
            if name not in MV_ROMS:
                sys.exit(f"{name}: no ROM matching MAME's sha1 in rom/roms/ or rom/microvision/")
            continue
        if name not in ph.GAMES:
            sys.exit(f"{name} is not in prepare_hmcs40.py's GAMES or a Microvision cartridge")
        for z in [name] + ([ph.PARENTS[name]] if name in ph.PARENTS else []):
            if not (ph.ROMS / f"{z}.zip").exists():
                sys.exit(f"missing {ph.ROMS / (z + '.zip')}")

    gp = HERE / "games_path_rgds.py"
    backup = HERE / "games_path_rgds.py.add_mame_game.bak"
    if backup.exists():
        sys.exit(f"{backup.name} exists - an earlier run was interrupted. Check it against "
                 f"{gp.name}, restore whichever is the full list, then delete the .bak.")
    work = HERE / "tmp" / "add_mame_game"
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)

    had_gp = gp.exists()
    if had_gp:
        shutil.copy2(gp, backup)
    try:
        donors = []
        for name in sets:
            # 1. container + segment art + console card
            if name in pm.KEYS:
                block, key = mv_prepare(name)
            else:
                block = ph.prepare(name)
                key = ph.games_path_block(name, 1)[1]

            # 2. a games_path with just this game, then a one-game pack
            block = block.replace('            , "', '            "', 1)
            gp.write_text("games_path = {\n" + block + "\n}\n", encoding="utf-8")
            tmp_pack = f"add_mame_game_{name}"
            cmd = [sys.executable, "convert_switch.py", "--target", "rgds", "--game", key, "--clean",
                   "--pack-name", tmp_pack,
                   "--out-gw-all", str(work / "GW_ALL_rgds.h"),
                   "--out-gw-rom-dir", str(work / "GW_ROM_RGDS"),
                   "--out-gfx", str(work / "gfx2x") + "/"]
            print(">", " ".join(cmd))
            subprocess.run(cmd, cwd=HERE, check=True)
            donors.append(HERE / f"{tmp_pack}.ykp")

        # 3. merge them all into the user's pack
        cmd = [sys.executable, "merge_pack_games.py", str(a.base_pack)] + [str(d) for d in donors] + ["--out", str(a.out_pack)]
        if a.replace:
            cmd.append("--replace")
        print(">", " ".join(cmd))
        subprocess.run(cmd, cwd=HERE, check=True)
        for p in HERE.glob("add_mame_game_*.ykp"):
            p.unlink()
    finally:
        if had_gp:
            shutil.copy2(backup, gp)
            backup.unlink()
        elif gp.exists():
            gp.unlink()

    print(f"\nOK: {', '.join(title(n) for n in sets)} added -> {a.out_pack}")


if __name__ == "__main__":
    main()
