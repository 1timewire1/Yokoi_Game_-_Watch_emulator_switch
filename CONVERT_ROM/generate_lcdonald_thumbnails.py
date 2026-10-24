"""Builds the menu thumbnail ("console art") for every McDonald's (LCDonald)
entry, straight from LCDonald's own MAME layout files.

Each game's <game>.lay defines a "Front" view: the photographed handheld
(<game>_front.jpg, canvas-sized), with the LCD background print
(game_bg_crop) laid over the screen area and the game's segment SVG
(<game>_0.svg, or _1.svg for ESPN prints) rendered on top of that in the
<screen> bounds - exactly how LCDonald itself draws a game. This script
reproduces that composite with every segment lit, then:

  1. trims the photo's empty margins down to the console itself, and
  2. pads the result out to the pack's 4:3 console-art shape with a flat
     colour sampled from the photo's own backdrop

so tall/narrow consoles are never cropped (convert_switch.py's
visual_console_data() would otherwise centre-crop anything that isn't 4:3).
The padded PNGs written to tmp/lcd_thumbs/<ref>.png are what
generate_lcdonald_pack.py hands to visual_console_data().

Run from CONVERT_ROM/:  C:/Python314/python.exe generate_lcdonald_thumbnails.py
(needs PIL/numpy and Inkscape at INKSCAPE_PATH, same as the segment
extraction scripts).
"""
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import numpy as np
from lxml import etree
from PIL import Image, ImageFilter

from lcdonald_burger_table import BURGER_GAMES

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parent
GAME_ASSETS = REPO_ROOT / "reference" / "LCDonald-master" / "GameAssets"
OUT_DIR = SCRIPT_DIR / "tmp" / "lcd_thumbs"
BURGER_ASSETS = REPO_ROOT / "reference" / "LCDonald-master" / "GameAssetsBurger"
# Burger art groups that aren't part of a game's own "all lit" screen: the
# WINNER banner (only shown in victory frames) and the two score groups (only
# score-1 is ever shown, as a rare hidden flash) - hidden before rendering so
# the thumbnail shows just the game's own elements.
BURGER_HIDDEN_GROUPS = ("win", "score-0", "score-1")
INKSCAPE_PATH = r"C:\Program Files\Inkscape\bin\inkscape.exe"

# ref -> (asset folder, .lay view name). Bases that also exist as an
# International print use "Front (US)" (the US release is the base entry;
# the plain "Front" view is the international unit, see VARIANTS in
# generate_lcdonald_pack.py). Where a game ships an open/closed pair the
# open view is used, since that's the one with the screen showing.
THUMB_VIEWS = {
    "SAGAME": ("sagame", "Front"),
    "TSKYPATROL": ("tskypatrol", "Front"),
    "KSOCCER": ("ksoccer", "Front (US)"),
    "KSOCCER_INTL": ("ksoccer", "Front"),
    "SGRINDER": ("sgrinder", "Front (US)"),
    "SGRINDER_INTL": ("sgrinder", "Front"),
    "SSPEEDWAY": ("sspeedway", "Front (US)"),
    "SSPEEDWAY_INTL": ("sspeedway", "Front"),
    "SSPEEDWAY_ESPN": ("sspeedway", "Front (ESPN)"),
    "ABANANA": ("abanana", "Front"),
    "SXTREMEBOARD": ("sxtremeboard", "Front (US)"),
    "SXTREMEBOARD_INTL": ("sxtremeboard", "Front"),
    "TSOCCER": ("tsoccer", "Front"),
    "TSOCCER_ESPN": ("tsoccer", "Front Open (ESPN)"),
    "KBASEBALL": ("kbaseball", "Front"),
    "KBASEBALL_ESPN": ("kbaseball", "Front (ESPN)"),
    "SHOCKEY": ("shockey", "Front"),
    "SHOCKEY_RARE": ("shockey", "Front (Rare)"),
    "ARVOLLEYBALL": ("arvolleyball", "Front"),
    "CFLOWER": ("cflower", "Front Open"),
    "BFISHING": ("bfishing", "Front"),
    "SSKATEBOARD": ("sskateboard", "Front Open"),
    "SSKATEBOARD_ESPN": ("sskateboard", "Front Open (ESPN)"),
    "TSKYADVENTURE": ("tskyadventure", "Front Open"),
    "KTREASURE": ("ktreasure", "Front"),
    "SBASKETBALL": ("sbasketball", "Front"),
    "SBASKETBALL_ESPN": ("sbasketball", "Front (ESPN)"),
    "ARTENNIS": ("artennis", "Front"),
    "ARTENNIS_ESPN": ("artennis", "Front (ESPN)"),
    "CFLOWER2": ("cflower2", "Front Open"),
    "BGIANTEGG": ("bgiantegg", "Front Open"),
}

TARGET_RATIO = 4 / 3
MAX_SIDE = 1600          # working size cap for the padded output
TRIM_MARGIN = 0.035      # breathing room around the console, fraction of its larger side
DIFF_THRESHOLD = 30      # per-channel distance from the backdrop that counts as "console"


def _bounds(el):
    b = el.find("bounds").attrib
    return int(b["x"]), int(b["y"]), int(b["width"]), int(b["height"])


def parse_view(folder: str, view_name: str):
    root = ET.parse(GAME_ASSETS / folder / f"{folder}.lay").getroot()
    files = {e.get("name"): e.find("image").get("file") for e in root.findall("element") if e.find("image") is not None}
    for v in root.findall("view"):
        if v.get("name") != view_name:
            continue
        screen = v.find("screen")
        front = bg = None
        for c in v.findall("element"):
            ref = c.get("ref")
            if ref == "button":
                continue
            if ref.startswith("front"):
                front = (files[ref], _bounds(c))
            elif ref.startswith("game_bg"):
                bg = (files[ref], _bounds(c))
        return {
            "screen_index": int(screen.get("index")),
            "screen": _bounds(screen),
            "front": front,
            "bg": bg,
        }
    raise KeyError(f"{folder}: no view named {view_name!r}")


def render_svg(svg_path: Path, w: int, h: int, out_png: Path):
    subprocess.run(
        [INKSCAPE_PATH, str(svg_path), f"--export-filename={out_png}", "-w", str(w), "-h", str(h)],
        check=True, capture_output=True,
    )


def composite(ref: str, folder: str, view_name: str) -> Image.Image:
    spec = parse_view(folder, view_name)
    front_file, (fx, fy, fw, fh) = spec["front"]
    canvas = Image.open(GAME_ASSETS / folder / front_file).convert("RGB").resize((fw, fh), Image.LANCZOS).convert("RGBA")

    if spec["bg"]:
        bg_file, (bx, by, bw, bh) = spec["bg"]
        bg = Image.open(GAME_ASSETS / folder / bg_file).convert("RGBA").resize((bw, bh), Image.LANCZOS)
        canvas.alpha_composite(bg, (bx, by))

    sx, sy, sw, sh = spec["screen"]
    tmp = OUT_DIR / f"_{ref}_svg.png"
    render_svg(GAME_ASSETS / folder / f"{folder}_{spec['screen_index']}.svg", sw, sh, tmp)
    seg = Image.open(tmp).convert("RGBA")
    canvas.alpha_composite(seg, (sx, sy))
    tmp.unlink()
    return canvas.convert("RGB")


LOWRES = 8               # console detection runs on a 1/8-scale copy
FIT_ITERS = 3            # robust backdrop-fit passes (drops pixels the console covers)
FLUSH_MARGIN = 0.015     # fraction of the console's size kept around its tight box
# Per-entry overrides where the edge detector under-reads a low-contrast
# outline: the silver International Sonic's Speedway has a soft left edge
# against the backdrop that the crisp-edge test partly misses.
MARGIN_OVERRIDES = {"SSPEEDWAY_INTL": 0.05}


def _poly_features(xs, ys):
    return np.stack([np.ones_like(xs), xs, ys, xs * xs, xs * ys, ys * ys], axis=-1)


def fit_backdrop(small: np.ndarray) -> np.ndarray:
    """Quadratic-surface model of the photo backdrop, fitted per channel to
    the outer ring of the image (robustly: ring pixels the console reaches
    are dropped and the fit repeated). Photos have vignettes/gradients and
    a flat median colour can't describe them, which used to skew the box."""
    h, w, _ = small.shape
    ys, xs = np.mgrid[0:h, 0:w]
    xs = xs / max(1, w - 1) * 2 - 1
    ys = ys / max(1, h - 1) * 2 - 1
    feats = _poly_features(xs, ys).reshape(-1, 6)
    ring = max(2, min(h, w) // 25)
    ringmask = np.zeros((h, w), bool)
    ringmask[:ring] = ringmask[-ring:] = True
    ringmask[:, :ring] = ringmask[:, -ring:] = True
    use = ringmask.reshape(-1).copy()
    flat = small.reshape(-1, 3).astype(np.float64)
    model = np.zeros_like(flat)
    for _ in range(FIT_ITERS):
        coef, *_ = np.linalg.lstsq(feats[use], flat[use], rcond=None)
        model = feats @ coef
        resid = np.abs(flat - model).max(axis=1)
        use = ringmask.reshape(-1) & (resid < DIFF_THRESHOLD)
        if use.sum() < 6:
            break
    return model.reshape(h, w, 3)


def largest_blob_bbox(mask: np.ndarray):
    h, w = mask.shape
    seen = np.zeros_like(mask, bool)
    best, best_n = None, 0
    for sy, sx in zip(*np.nonzero(mask)):
        if seen[sy, sx]:
            continue
        stack = [(sy, sx)]
        seen[sy, sx] = True
        n, y0, y1, x0, x1 = 0, sy, sy, sx, sx
        while stack:
            y, x = stack.pop()
            n += 1
            y0, y1, x0, x1 = min(y0, y), max(y1, y), min(x0, x), max(x1, x)
            for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
                if 0 <= ny < h and 0 <= nx < w and mask[ny, nx] and not seen[ny, nx]:
                    seen[ny, nx] = True
                    stack.append((ny, nx))
        if n > best_n:
            best, best_n = (x0, y0, x1 + 1, y1 + 1), n
    return best


def _hardware_number(filename: str) -> int:
    m = re.search(r"hardware_(\d+)", filename)
    return int(m.group(1)) if m else 10**6


def burger_view(folder: str):
    """The .lay view whose console photo is hardware_01 (the one wanted for
    thumbnails); for the few folders whose layout never wires hardware_01
    into a view (treasure/treasure2, platform2, speedway, kosoccer,
    eggcatch2), the lowest-numbered console photo that *is* wired up - its
    screen bounds are the only ones the layout gives. Returns
    (front_file, front_bounds, screen_bounds, bg or None, fallback_used)."""
    root = ET.parse(BURGER_ASSETS / folder / f"{folder}.lay").getroot()
    files = {e.get("name"): e.find("image").get("file") for e in root.findall("element") if e.find("image") is not None}
    cands = []
    for v in root.findall("view"):
        screen = v.find("screen")
        if screen is None:
            continue
        front = bg = None
        for c in v.findall("element"):
            ref = c.get("ref")
            f = files.get(ref)
            if ref == "button" or not f:
                continue
            if ref.startswith("game_bg"):
                bg = (f, _bounds(c))
            elif "hardware" in f:
                front = (f, _bounds(c))
        if front:
            cands.append((_hardware_number(front[0]), front, _bounds(screen), bg))
    cands.sort(key=lambda c: c[0])
    number, front, screen, bg = cands[0]
    return front[0], front[1], screen, bg, number != 1


def hide_burger_groups(svg_in: Path, svg_out: Path):
    root = etree.parse(str(svg_in)).getroot()
    for gid in BURGER_HIDDEN_GROUPS:
        for el in root.iterfind(f'.//*[@id="{gid}"]'):
            style = el.get("style", "")
            el.set("style", (style + ";" if style else "") + "display:none")
    svg_out.write_bytes(etree.tostring(root))


def composite_burger(ref: str, folder: str):
    front_file, (fx, fy, fw, fh), (sx, sy, sw, sh), bg, fallback = burger_view(folder)
    canvas = Image.open(BURGER_ASSETS / folder / front_file).convert("RGB").resize((fw, fh), Image.LANCZOS).convert("RGBA")
    if bg:
        bg_file, (bx, by, bw, bh) = bg
        canvas.alpha_composite(Image.open(BURGER_ASSETS / folder / bg_file).convert("RGBA").resize((bw, bh), Image.LANCZOS), (bx, by))
    svg_tmp = OUT_DIR / f"_{ref}_burger.svg"
    hide_burger_groups(BURGER_ASSETS / folder / f"{folder}_0.svg", svg_tmp)
    png_tmp = OUT_DIR / f"_{ref}_svg.png"
    render_svg(svg_tmp, sw, sh, png_tmp)
    canvas.alpha_composite(Image.open(png_tmp).convert("RGBA"), (sx, sy))
    svg_tmp.unlink()
    png_tmp.unlink()
    return canvas.convert("RGB"), front_file, fallback


EDGE_SCALE = 4          # edge detection runs on a 1/4-scale copy
EDGE_THRESHOLD = 14.0    # gradient magnitude (0-255 levels per 1/4-scale px) that counts as a crisp edge
EDGE_DILATE = 9          # odd; joins outline fragments into one blob per console part
MIN_PART_FRACTION = 0.02  # ignore edge blobs smaller than this fraction of the biggest one (dust, grain)


def console_bbox(img: Image.Image):
    """Tight box around the console, found from *crisp edges* rather than
    from how different each pixel is from the backdrop: the soft cast
    shadows around these photos differ from the backdrop plenty (and used
    to be counted as console, dragging the box towards them), but their
    gradients are gentle, while a console's outline is a hard edge - and a
    reflective finish (the silver Shadow Hockey) still has that outline
    even where its body blends into the backdrop."""
    small = img.resize((max(1, img.width // EDGE_SCALE), max(1, img.height // EDGE_SCALE)), Image.BILINEAR)
    small = small.filter(ImageFilter.GaussianBlur(1))
    arr = np.asarray(small).astype(np.float32)
    gy, gx = np.gradient(arr, axis=(0, 1))
    mag = np.hypot(gx, gy).max(axis=2)
    mask = Image.fromarray(((mag > EDGE_THRESHOLD) * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(EDGE_DILATE))
    m = np.asarray(mask) > 0

    # Connected parts (4-neighbour), each with its area and box.
    h, w = m.shape
    seen = np.zeros_like(m, bool)
    parts = []
    for sy, sx in zip(*np.nonzero(m)):
        if seen[sy, sx]:
            continue
        stack = [(sy, sx)]
        seen[sy, sx] = True
        n, y0, y1, x0, x1 = 0, sy, sy, sx, sx
        while stack:
            y, x = stack.pop()
            n += 1
            y0, y1, x0, x1 = min(y0, y), max(y1, y), min(x0, x), max(x1, x)
            for ny, nx in ((y + 1, x), (y - 1, x), (y, x + 1), (y, x - 1)):
                if 0 <= ny < h and 0 <= nx < w and m[ny, nx] and not seen[ny, nx]:
                    seen[ny, nx] = True
                    stack.append((ny, nx))
        parts.append((n, x0, y0, x1 + 1, y1 + 1))
    if not parts:
        return (0, 0, img.width, img.height)
    biggest = max(p[0] for p in parts)
    keep = [p for p in parts if p[0] >= biggest * MIN_PART_FRACTION]
    r = EDGE_DILATE // 2  # undo the dilation's growth
    x0 = min(p[1] for p in keep) + r
    y0 = min(p[2] for p in keep) + r
    x1 = max(p[3] for p in keep) - r
    y1 = max(p[4] for p in keep) - r
    return (x0 * EDGE_SCALE, y0 * EDGE_SCALE, min(img.width, x1 * EDGE_SCALE), min(img.height, y1 * EDGE_SCALE))


def trim_and_pad(img: Image.Image, margin_fraction: float = FLUSH_MARGIN):
    """Returns (thumbnail, bbox). The window is the console's tight box
    (flush on whichever axis limits the fit), widened symmetrically on the
    other axis to 4:3 using the photo's real surroundings; only what falls
    outside the photo itself is synthesised from the blurred backdrop."""
    x0, y0, x1, y1 = console_bbox(img)
    bw, bh = x1 - x0, y1 - y0
    m = int(max(bw, bh) * margin_fraction)
    cx, cy = (x0 + x1) / 2, (y0 + y1) / 2
    cw, ch = bw + 2 * m, bh + 2 * m
    if cw / ch < TARGET_RATIO:
        cw = ch * TARGET_RATIO
    else:
        ch = cw / TARGET_RATIO
    wx0, wy0 = int(round(cx - cw / 2)), int(round(cy - ch / 2))
    wx1, wy1 = wx0 + int(round(cw)), wy0 + int(round(ch))

    # Canvas big enough to hold the window even where it overhangs the photo.
    pl, pt = max(0, -wx0), max(0, -wy0)
    pr, pb = max(0, wx1 - img.width), max(0, wy1 - img.height)
    if pl or pt or pr or pb:
        raw = np.asarray(img).astype(np.int16)
        small = raw[::LOWRES, ::LOWRES]
        # Non-backdrop pixels are swapped for the fitted backdrop before the
        # edge extension so a console touching the photo's edge can't smear.
        bd = fit_backdrop(np.asarray(img.resize((small.shape[1], small.shape[0]), Image.BILINEAR)).astype(np.int16))
        bd_full = np.asarray(Image.fromarray(np.clip(bd, 0, 255).astype(np.uint8)).resize(img.size, Image.BICUBIC))
        base = bd_full.copy()
        base = np.asarray(Image.fromarray(base).filter(ImageFilter.GaussianBlur(20)))
        ext = np.pad(base, ((pt, pb), (pl, pr), (0, 0)), mode="edge")
        canvas = Image.fromarray(ext, "RGB")
        canvas.paste(img, (pl, pt))
    else:
        canvas = img
    out = canvas.crop((wx0 + pl, wy0 + pt, wx1 + pl, wy1 + pt))
    if max(out.size) > MAX_SIDE:
        s = MAX_SIDE / max(out.size)
        out = out.resize((int(out.width * s), int(out.height * s)), Image.LANCZOS)
    return out, (x0, y0, x1, y1)


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    failures = []
    for ref, (folder, view) in THUMB_VIEWS.items():
        try:
            padded, bbox = trim_and_pad(composite(ref, folder, view), MARGIN_OVERRIDES.get(ref, FLUSH_MARGIN))
            padded.save(OUT_DIR / f"{ref.lower()}.png")
            print(f"{ref:20s} {folder}/{view!r:24s} -> {padded.size} bbox={bbox}")
        except Exception as e:  # noqa: BLE001 - report every game, don't stop at the first
            failures.append((ref, e))
            print(f"{ref:20s} FAILED: {e!r}")

    for _id, folder, _cls, ref, _name, _base, _clone in BURGER_GAMES:
        try:
            comp, front_file, fallback = composite_burger(ref, folder)
            padded, bbox = trim_and_pad(comp, MARGIN_OVERRIDES.get(ref, FLUSH_MARGIN))
            padded.save(OUT_DIR / f"{ref.lower()}.png")
            note = "  (hardware_01 has no layout view - lowest wired console used)" if fallback else ""
            print(f"{ref:20s} {folder}/{front_file} -> {padded.size}{note}")
        except Exception as e:  # noqa: BLE001
            failures.append((ref, e))
            print(f"{ref:20s} FAILED: {e!r}")

    # Contact sheet for eyeballing the whole set at once.
    files = sorted(p for p in OUT_DIR.glob("*.png") if not p.name.startswith("_"))
    if files:
        tw, th, cols = 320, 240, 6
        rows = (len(files) + cols - 1) // cols
        sheet = Image.new("RGB", (cols * tw, rows * th), (60, 60, 60))
        for i, f in enumerate(files):
            t = Image.open(f).convert("RGB").resize((tw - 6, th - 6), Image.LANCZOS)
            sheet.paste(t, ((i % cols) * tw + 3, (i // cols) * th + 3))
        sheet.save(OUT_DIR / "_contact_sheet.png")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
