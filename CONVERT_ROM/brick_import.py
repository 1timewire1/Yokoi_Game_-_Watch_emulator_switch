"""Imports games from BrickEmuPy (github.com/azya52/BrickEmuPy, CC0) for the
cores ported to source/BrickCores/.

BrickEmuPy describes each game with a JSON ".brick" file: the core, its clock,
"mask_options" (chip configuration, plus extra ROM files such as a sound ROM),
the button wiring ("peripherals": direct_input / matrix_input) and a face SVG.
For every game listed in brick_games.json this writes rom/decompress/<set>/:
  <set>.brk             the ROM container the app loads (source/BrickCores/
                        BrickRom.h): core, clock, options as key=value text,
                        input bindings with their Switch keys, and the ROMs
  <set>_segs.svg, Background.png, Overlay.png ...   via split_brick_svg.py
and rom/console/<set>.png (the face on the white thumbnail card, segments as
ghosts until a core can render a real screen), and adds/updates the game's
games_path_rgds.py entry.

Display names, makers and dates aren't in .brick files: they live in
brick_games.json (tracked), which `--init` seeds from BrickEmuPy's README
for any .brick not listed yet. Games whose core isn't ported, or whose
metadata is incomplete, are skipped with a note.

Switch keys are assigned per button by role (assign_keys) - directions on
the D-Pad, primary actions on A/B/X/Y, start-type buttons on +, pause/mode
on -, sound/time/alarm on R/L, reset-type buttons on a + and - chord - and a
game's "keys" override in brick_games.json wins over that.

Usage:
  python brick_import.py <BrickEmuPy dir> --init          (seed new entries)
  python brick_import.py <BrickEmuPy dir> [set ...]        (import)
  python brick_import.py <BrickEmuPy dir> --controls       (print key tables)
"""
import argparse
import difflib
import json
import re
import struct
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import split_brick_svg as sb  # noqa: E402
from source.manufacturer_ids import MANUFACTURER_BY_NAME  # noqa: E402

META = HERE / "brick_games.json"
DECOMPRESS = HERE / "rom" / "decompress"
CONSOLE = HERE / "rom" / "console"
GAMES_PATH = HERE / "games_path_rgds.py"

# BrickEmuPy core names with a C++ port (source/BrickCores/BrickCore.cpp's
# kCores). Kept in sync by hand.
PORTED_CORES = {"GPL191X", "HT943", "M37520", "MSM50XX", "SPL61X", "SPL81408"}


def load_brick(path: Path) -> dict:
    # BrickEmuPy's .brick files are JSON with the odd trailing comma.
    return json.loads(re.sub(r",(\s*[}\]])", r"\1", path.read_text(encoding="utf-8")))


# --- options / container ----------------------------------------------------

def flatten_options(mask: dict, brick_dir: Path):
    """mask_options -> (["key=value", ...], {blob name: bytes}). Paths to
    extra files ("sound_rom_path": "./assets/x.srom") become blobs named
    without the "_path" suffix; dicts flatten to dotted keys; lists to
    comma-separated values."""
    lines, blobs = [], {}

    def walk(prefix, v):
        if isinstance(v, dict):
            for k, sub in v.items():
                walk(f"{prefix}.{k}" if prefix else k, sub)
        elif isinstance(v, (list, tuple)):
            # Whole numbers as ints (as before); fractions kept as written -
            # MSM50XX's sound_freq_div_tbl has dividers like 5.5.
            lines.append(f"{prefix}=" + ",".join(
                str(int(x)) if float(x).is_integer() else repr(float(x)) for x in v))
        elif isinstance(v, bool):
            lines.append(f"{prefix}={int(v)}")
        elif isinstance(v, (int, float)):
            lines.append(f"{prefix}={v}")
        else:
            lines.append(f"{prefix}={v}")

    for key, value in mask.items():
        if key == "rom_path":
            blobs["rom"] = (brick_dir / value).read_bytes()
        elif key.endswith("_path") and isinstance(value, str):
            blobs[key[:-5]] = (brick_dir / value).read_bytes()
        else:
            walk(key, value)
    return lines, blobs


def write_container(path: Path, core: str, clock: int, option_lines, blobs: dict):
    opts = "\n".join(option_lines).encode("utf-8")
    out = bytearray(b"BRK1")
    out += bytes([len(core)]) + core.encode("ascii")
    out += struct.pack("<II", int(clock), len(opts)) + opts
    out += struct.pack("<I", len(blobs))
    for name, data in blobs.items():
        out += bytes([len(name)]) + name.encode("ascii") + struct.pack("<I", len(data)) + data
    path.write_bytes(out)


# --- Switch key assignment ---------------------------------------------------

DIRECTIONS = {"btnLeft": "LEFT", "btnRight": "RIGHT", "btnUp": "UP", "btnDown": "DOWN",
              "btnLeftTurn": "LEFT", "btnRightTurn": "RIGHT", "btnLeftDown": "LEFT", "btnRightUp": "RIGHT",
              "btnEarLeft": "L", "btnEarRight": "R"}
# Three-button virtual pets (Left / Center / Right, no Up/Down): also on
# Y / A / B, matching the E0C6200 Tamagotchis (switch/source/input/
# e0c6200_input_map.cpp). "|" = either key works.
PET_KEYS = {"btnLeft": "LEFT|Y", "btnCenter": "A|UP|DOWN", "btnRight": "RIGHT|B"}
CHORDS = ("PLUS+MINUS", "L+R")
ROLE_WORDS = [  # first match wins
    ("chord", ("Reset", "ACL")),
    ("primary", ("Rotate", "Action", "Jump", "Fire", "Shoot", "Attack", "Throw", "Center", "Circle", "Enter")),
    ("start", ("Start", "Game", "OnPause", "OnReset", "Select")),
    ("minus", ("Pause", "Mode", "Exit", "Esc", "Rev", "Off")),
    ("system", ("Sound", "Mute", "Time", "Alarm", "Clock", "Set", "Adjust", "Speed", "Ear")),
]
ROLE_KEYS = {
    "primary": ["A", "B", "X", "Y"],
    "start": ["PLUS", "MINUS", "X", "Y"],
    "minus": ["MINUS", "PLUS", "X", "Y"],
    "system": ["R", "L", "X", "Y"],
    "other": ["A", "B", "X", "Y", "PLUS", "MINUS", "R", "L"],
}
FALLBACK = ["A", "B", "X", "Y", "PLUS", "MINUS", "R", "L", "UP", "DOWN", "LEFT", "RIGHT"]


def role_of(name: str) -> str:
    if name in DIRECTIONS:
        return "direction"
    for role, words in ROLE_WORDS:
        if any(w in name for w in words):
            return role
    if re.fullmatch(r"btn[A-D]", name) or name in ("btnSquare", "btnPass", "btnDrop", "btnNcha", "btnCard", "btnStep"):
        return "primary"
    return "other"


def assign_keys(buttons, overrides=None) -> dict:
    """{button name: Switch key(s)} for one game's buttons (in .brick order).
    Key syntax: "A" one key, "PLUS+MINUS" all held together, "LEFT|Y" either."""
    overrides = overrides or {}
    if set(PET_KEYS) <= set(buttons) and not {"btnUp", "btnDown"} & set(buttons):
        overrides = {**PET_KEYS, **overrides}
    taken, keys = set(), {}
    for name, key in overrides.items():
        if name in buttons:
            keys[name] = key
            if "+" not in key:  # chords don't use up their keys
                taken.update(key.split("|"))
    order = ["direction", "primary", "start", "minus", "system", "other", "chord"]
    for role in order:
        for name in buttons:
            if name in keys or role_of(name) != role:
                continue
            if role == "chord":
                keys[name] = next((c for c in CHORDS if not any(c == k for k in keys.values())), CHORDS[-1])
                continue
            want = [DIRECTIONS[name]] if role == "direction" else ROLE_KEYS[role]
            key = next((k for k in want + FALLBACK if k not in taken), None)
            if key:
                keys[name] = key
                taken.add(key)
    return keys


def input_lines(brick: dict, overrides=None):
    """'input.<button>=...' option lines (BrickCore::configure's format)."""
    p = brick.get("peripherals", {})
    direct, matrix = p.get("direct_input", {}), p.get("matrix_input", {})
    keys = assign_keys(list(direct) + list(matrix), overrides)
    lines = []
    for name, d in direct.items():
        if name in keys:
            lines.append(f"input.{name}=direct;{d['port']};{d['mask']};{d['level']};{keys[name]}")
    for name, m in matrix.items():
        if name in keys:
            lines.append(f"input.{name}=matrix;{m['port']};{m['mask']};{m['level']['port']};{m['level']['mask']};{keys[name]}")
    return lines, keys


# --- metadata ----------------------------------------------------------------

def readme_titles(brick_dir: Path):
    titles = []
    for line in (brick_dir / "README.md").read_text(encoding="utf-8").splitlines():
        m = re.match(r"\s+\* (.+)", line)
        if m:
            titles.append(re.sub(r"\s*\((?:distributed without ROM|High probability[^)]*)\)", "", m.group(1)).strip())
    return titles


def guess_title(stem: str, titles):
    """The README line sharing the most of the file name's words/numbers
    (each camel-case word or digit run of the stem, found anywhere in the
    title), ties broken by overall similarity. Only a seed for brick_games.json
    - every pick gets reviewed there."""
    tokens = [t.lower() for t in re.findall(r"[A-Z][a-z]+|[A-Z]+(?![a-z])|[a-z]+|\d+", stem)]

    def score(title):
        flat = re.sub(r"[^a-z0-9]", "", title.lower())
        hits = sum(1 for t in tokens if t in flat)
        return hits, difflib.SequenceMatcher(None, " ".join(tokens), title.lower()).ratio()

    return max(titles, key=score)


def set_id(stem: str) -> str:
    return re.sub(r"[^a-z0-9]", "", stem.lower())


def init_metadata(brick_dir: Path):
    meta = json.loads(META.read_text(encoding="utf-8")) if META.exists() else {"games": []}
    known = {g["brick"] for g in meta["games"]}
    titles = readme_titles(brick_dir)
    added = 0
    for b in sorted((brick_dir / "assets").glob("*.brick")):
        cfg = load_brick(b)
        if b.stem in known or cfg["core"] == "E0C6200":  # E0C6200 games go through prepare_e0c6200.py
            continue
        title = guess_title(b.stem, titles)
        year = re.search(r"\b(19\d\d|20\d\d)\b", title)
        meta["games"].append({
            "brick": b.stem, "set": set_id(b.stem), "core": cfg["core"],
            "name": None, "readme": title, "manufacturer": None,
            "date": f"{year.group(1)}-01-01" if year else None, "keys": {},
        })
        added += 1
    meta["games"].sort(key=lambda g: (g["core"], g["brick"]))
    META.write_text(json.dumps(meta, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"{added} new entries in {META.name} - fill in name/manufacturer/date")


# --- games_path ---------------------------------------------------------------

def games_path_block(key: str, game: dict, info: dict) -> str:
    d, ref = game["set"], game["set"]
    screens = info["screens"]
    sfx = ["" if i == 0 else str(i + 1) for i in range(len(screens))]
    lines = [
        f'            , "{key}" :',
        f'                    {{ "ref" : "{ref}"',
        f'                    , "manufacturer" : {MANUFACTURER_BY_NAME[game["manufacturer"]]}',
        f'                    , "display_name" : "{game["name"]}"',
        "                    , \"Rom\" : r'.\\rom\\decompress\\" + d + "\\" + d + ".brk'",
        "                    , \"Visual\" : [" + ", ".join(
            "r'.\\rom\\decompress\\" + d + "\\" + d + f"_segs{s}.svg'" for s in sfx) + "]",
    ]
    if info["background"]:
        lines += ["                    , \"Background\" : [" + ", ".join(
                      "r'.\\rom\\decompress\\" + d + f"\\Background{s}.png'" for s in sfx) + "]",
                  '                    , "alpha_bright" : 0.38',
                  '                    , "fond_bright" : 1.0']
    if info["overlay"]:
        lines += ["                    , \"Overlay\" : [" + ", ".join(
                      "r'.\\rom\\decompress\\" + d + f"\\Overlay{s}.png'" for s in sfx) + "]",
                  '                    , "overlay_alpha" : 100']
    if info["arrangement"] == "horizontal":
        lines.append('                    , "2_in_one_screen" : True')
    lines += [
        "                    , \"console\" : r'.\\rom\\console\\" + d + ".png'",
        '                    , "transform_visual" : [' + ", ".join(
            f"[[{sc['width']},0,0],[{sc['height']},0,0]]" for sc in screens) + "]",
        f'                    , "date" : "{game["date"]}"',
        "                }",
    ]
    return "\n".join(lines)


def upsert_games_path(key: str, block: str):
    s = GAMES_PATH.read_text(encoding="utf-8")
    marker = f'            , "{key}" :'
    if marker in s:
        start = s.index(marker)
        end = s.index("\n                }", start) + len("\n                }")
        s = s[:start] + block + s[end:]
    else:
        idx = s.rstrip().rfind("}")
        s = s[:idx].rstrip() + "\n\n" + block + "\n}\n"
    GAMES_PATH.write_text(s, encoding="utf-8", newline="")


# --- main -----------------------------------------------------------------------

def clock_lines(clock) -> list:
    """brick_games.json "clock" (RAM cells of a real-time clock, found with
    tmp/review/brick_clockprobe.exe) -> "clock.*" container options, read by
    BrickCore::set_clock(): sec/min = [units, tens], hour = [units, tens] or
    [one binary cell], pm = [addr, mask, 1 if set means PM else 0], start =
    the .brick button that leaves a boot test screen, bcd = 1 for one BCD
    byte per field, hour_zero = 1 for 12 o'clock stored as 0, unset =
    [addr, value] while the clock awaits its first setting and confirm = the
    button that sets it (all optional - see BrickCore.h)."""
    if not clock:
        return []
    lines = [f"clock.{k}=" + ",".join(str(int(v)) for v in clock[k])
             for k in ("sec", "min", "hour", "pm", "unset") if k in clock]
    lines += [f"clock.{k}={int(clock[k])}" for k in ("bcd", "hour_zero") if clock.get(k)]
    lines += [f"clock.{k}={clock[k]}" for k in ("start", "confirm") if clock.get(k)]
    return lines


def import_game(brick_dir: Path, game: dict) -> bool:
    if game.get("exclude"):
        print(f"skip {game['brick']}: excluded - {game['exclude']}")
        return False
    cfg = load_brick(brick_dir / "assets" / f"{game['brick']}.brick")
    if cfg["core"] not in PORTED_CORES:
        print(f"skip {game['brick']}: core {cfg['core']} not ported yet")
        return False
    missing = [k for k in ("name", "manufacturer", "date") if not game.get(k)]
    if missing:
        print(f"skip {game['brick']}: brick_games.json is missing {', '.join(missing)}")
        return False
    if game["manufacturer"] not in MANUFACTURER_BY_NAME:
        print(f"skip {game['brick']}: unknown manufacturer {game['manufacturer']!r} "
              "(add it to source/manufacturer_ids.py and GW_ROM.h)")
        return False
    d = DECOMPRESS / game["set"]
    d.mkdir(parents=True, exist_ok=True)
    options, blobs = flatten_options(cfg["mask_options"], brick_dir)
    options += clock_lines(game.get("clock"))
    inputs, _ = input_lines(cfg, game.get("keys"))
    write_container(d / f"{game['set']}.brk", cfg["core"], cfg["clock"], options + inputs, blobs)
    face = brick_dir / cfg["face_path"]
    # "screen_pad" (brick_games.json, optional): margin around the segments as
    # a fraction, clipped to the LCD glass - a large value (1.0) crops to the
    # whole glass, for faces whose dot matrix sits in a bigger window (D-Power
    # Digivice: 2:1 matrix in a ~1.25:1 glass), so it isn't blown up to fill
    # the screen.
    info = sb.split(face, d, game["set"], pad=float(game.get("screen_pad", 0.03)))
    sb.face_thumbnail(face, CONSOLE / f"{game['set']}.png")
    key = re.sub(r"[^A-Za-z0-9]+", "_", game["name"]).strip("_")
    upsert_games_path(key, games_path_block(key, game, info))
    print(f"{game['brick']} -> {game['set']} ({key})")
    return True


def print_controls(brick_dir: Path):
    meta = json.loads(META.read_text(encoding="utf-8")) if META.exists() else {"games": []}
    overrides = {g["brick"]: g.get("keys") for g in meta["games"]}
    for b in sorted((brick_dir / "assets").glob("*.brick")):
        cfg = load_brick(b)
        if cfg["core"] == "E0C6200":
            continue
        _, keys = input_lines(cfg, overrides.get(b.stem))
        print(f"{b.stem:28s} " + "  ".join(f"{k[3:]}={v}" for k, v in keys.items()))


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("brick_dir", type=Path)
    ap.add_argument("sets", nargs="*")
    ap.add_argument("--init", action="store_true")
    ap.add_argument("--controls", action="store_true")
    a = ap.parse_args()
    if a.init:
        init_metadata(a.brick_dir)
    elif a.controls:
        print_controls(a.brick_dir)
    else:
        meta = json.loads(META.read_text(encoding="utf-8"))
        for game in meta["games"]:
            if not a.sets or game["set"] in a.sets or game["brick"] in a.sets:
                import_game(a.brick_dir, game)
