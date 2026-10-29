"""Generate ``games_path`` entries directly from ``default.lay`` files."""


from __future__ import annotations

import re
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple, Iterable

import numpy as np
from PIL import Image

try:
    # When imported as part of the 'source' package
    from source.games_path_utils import GameEntry, write_games_path
    from source.manufacturer_ids import MANUFACTURER_NINTENDO, MANUFACTURER_TRONICA, MANUFACTURER_ELEKTRONIKA, MANUFACTURER_TIGER, MANUFACTURER_DAVID_AND_JOHN, MANUFACTURER_MCDONALDS, MANUFACTURER_BURGER_BARD, MANUFACTURER_KONAMI, MANUFACTURER_NELSONIC, MANUFACTURER_VTECH, MANUFACTURER_BANDAI, MANUFACTURER_EPOCH, MANUFACTURER_NIKKO, MANUFACTURER_TANDY, MANUFACTURER_BY_NAME
    from source.target_profiles import get_target
except ImportError:
    # When run directly from the 'source' directory
    from games_path_utils import GameEntry, write_games_path
    from manufacturer_ids import MANUFACTURER_NINTENDO, MANUFACTURER_TRONICA, MANUFACTURER_ELEKTRONIKA, MANUFACTURER_TIGER, MANUFACTURER_DAVID_AND_JOHN, MANUFACTURER_MCDONALDS, MANUFACTURER_BURGER_BARD, MANUFACTURER_KONAMI, MANUFACTURER_NELSONIC, MANUFACTURER_VTECH, MANUFACTURER_BANDAI, MANUFACTURER_EPOCH, MANUFACTURER_NIKKO, MANUFACTURER_TANDY, MANUFACTURER_BY_NAME
    from target_profiles import get_target

# Preferred background views ordered by desirability. Boolean marks multi-screen views.
VIEW_PRIORITY: Sequence[Tuple[str, bool]] = (
    ("Background Only (No Frame)", False),
    ("Backgrounds Only (No Frame)", True),
    ("Background Only (No Shadow)", False),
    ("Backgrounds Only (No Shadow)", True),
    ("Background Only (No Reflection)", False),
    ("Backgrounds Only (No Reflection)", True),
    ("Background Only", False),
    ("Backgrounds Only", True),
    ("Version 1 - Background Only (No Shadow)", False),
    ("Version 1 - Background Only (No Shadow)", True),
    ("External Layout", False), # Tiger games
)

# set default img when console img is not set for a game
default_img_console = r'.\rom\default_console.png'


try:
    from source.games_path_utils import (
        INDENT_KEY_FIRST,
        INDENT_KEY_OTHER,
        INDENT_FIELD,
        INDENT_CONT,
        INDENT_FOOTER,
    )
except ImportError:
    from games_path_utils import (
        INDENT_KEY_FIRST,
        INDENT_KEY_OTHER,
        INDENT_FIELD,
        INDENT_CONT,
        INDENT_FOOTER,
    )

MODEL_PATTERN = re.compile(r"^[A-Za-z]{2,4}[-_]?\d{2,3}[A-Za-z0-9_-]*$")
DISPLAY_NAME_PATTERNS = (
    re.compile(r"<!--\s*Game\s*&\s*Watch:\s*([^-]+?)\s*-->", re.IGNORECASE),
    re.compile(r"<!--\s*([^:-]+?)\s*-->")
)

ROM_PRIORITY = ("*.program", "*.bin", "*.rom", "*.prg", "*.hex")

# Pixel-value std-dev below which a resolved background candidate is
# treated as "basically a flat tint, not real art" - see
# _image_detail_score/_pick_best_background. Calibrated against the
# "hydef"-style layered artwork's actual flat screen-tint overlay, which
# consistently scores ~6.1 (not near-0 - it's a soft gradient, not a
# perfectly solid fill) versus ~17-110+ for the real decorative/photo
# backgrounds in the same sets.
_FLAT_BACKGROUND_SCORE_THRESHOLD = 8.0


@dataclass(frozen=True)
class GameMetadata:
    model: str
    title: str
    display_title: str
    release_date: Optional[str]
    manufacturer: int
    clone_of: Optional[str] = None


@dataclass(frozen=True)
class Bounds:
    x: float
    y: float
    width: float
    height: float

    def as_int_tuple(self) -> Tuple[int, int, int, int]:
        return (
            int(round(self.x)),
            int(round(self.y)),
            int(round(self.width)),
            int(round(self.height)),
        )


@dataclass(frozen=True)
class SurfaceData:
    screen_bounds: Bounds
    background_bounds: Optional[Bounds]
    background_file: Optional[str]


# Series prefixes dropped from display names ("Game & Watch: Fire" -> "Fire").
# Only these - a colon elsewhere is part of the real title ("Batman: The
# Animated Series", "Top Gun: Second Mission") and must be kept.
_STRIPPED_TITLE_PREFIXES = ("Game & Watch:", "Micro Vs. System:")


def _strip_title_prefix(title: str) -> str:
    cleaned = title.strip()
    for prefix in _STRIPPED_TITLE_PREFIXES:
        if cleaned.startswith(prefix):
            return cleaned[len(prefix):].strip()
    return cleaned


def _normalize_release_date(raw: str) -> Optional[str]:
    value = raw.strip()
    if not value:
        return None
    if value.count(".") == 2:
        parts = value.split(".")
        if all(part.isdigit() for part in parts):
            year, month, day = parts
            return f"{year.zfill(4)}-{month.zfill(2)}-{day.zfill(2)}"
    return value


def _load_game_metadata(script_dir: Path) -> Dict[str, GameMetadata]:
    md_path = script_dir / "GNW_LIST.md"
    try:
        text = md_path.read_text(encoding="utf-8")
    except OSError:
        return {}

    mapping: Dict[str, GameMetadata] = {}
    in_table = False
    current_manufacturer = MANUFACTURER_NINTENDO
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped:
            in_table = False
            continue

        if stripped.startswith("##"):
            heading = stripped.lstrip("#").strip().lower()
            # Heading-driven manufacturer:
            # - Nintendo section headings usually include "Game & Watch"
            # - Tronica / Elektronika / Tiger / Konami / Nelsonic / V-Tech
            #   headings include the manufacturer's name
            # (There used to be a catch-all "Experimental" heading; every game
            # in it now sits under its real manufacturer's heading instead.)
            # Keep the current value for unrelated headings like "Special Editions".
            in_table = False
            # BrickEmuPy-era makers (manufacturer_ids.MANUFACTURER_BY_NAME,
            # id 15 up) - matched first so e.g. "Popy (Bandai)" lands in Popy.
            brick_era = [mid for name, mid in MANUFACTURER_BY_NAME.items()
                         if mid >= 15 and name.lower() in heading]
            if brick_era:
                current_manufacturer = brick_era[0]
            elif "tronica" in heading:
                current_manufacturer = MANUFACTURER_TRONICA
            elif "elektronika" in heading:
                current_manufacturer = MANUFACTURER_ELEKTRONIKA
            elif "game & watch" in heading or "game and watch" in heading:
                current_manufacturer = MANUFACTURER_NINTENDO
            elif "tiger" in heading:
                current_manufacturer = MANUFACTURER_TIGER
            elif "konami" in heading:
                current_manufacturer = MANUFACTURER_KONAMI
            elif "nelsonic" in heading:
                current_manufacturer = MANUFACTURER_NELSONIC
            elif "v-tech" in heading or "vtech" in heading:
                current_manufacturer = MANUFACTURER_VTECH
            elif "bandai" in heading:
                current_manufacturer = MANUFACTURER_BANDAI
            elif "epoch" in heading:
                current_manufacturer = MANUFACTURER_EPOCH
            elif "nikko" in heading:
                current_manufacturer = MANUFACTURER_NIKKO
            elif "tandy" in heading:
                current_manufacturer = MANUFACTURER_TANDY
            elif "mcdonald" in heading:
                current_manufacturer = MANUFACTURER_MCDONALDS
            elif "burger bard" in heading:
                current_manufacturer = MANUFACTURER_BURGER_BARD
            continue

        if stripped.startswith("| No.") and "Filename" in stripped:
            in_table = True
            continue
        if not in_table:
            continue
        if stripped.startswith("|---"):
            continue
        if not stripped.startswith("|"):
            in_table = False
            continue

        columns = [col.strip() for col in stripped.strip("|").split("|")]
        if len(columns) < 5:
            continue

        filename = columns[2]
        if not filename.lower().endswith(".zip"):
            continue

        model = columns[1]
        title = columns[3]
        release_raw = columns[4]

        clone_raw = columns[6] if len(columns) >= 7 else ""
        clone_value = clone_raw.strip()
        if clone_value.lower().endswith(".zip"):
            clone_value = clone_value.rsplit(".", 1)[0]
        clone_of = clone_value.lower() if clone_value else None

        base_name = filename.rsplit(".", 1)[0].lower()
        display_title = _strip_title_prefix(title)
        release_date = _normalize_release_date(release_raw)

        mapping[base_name] = GameMetadata(
            model=model.strip(),
            title=title.strip(),
            display_title=display_title,
            release_date=release_date,
            manufacturer=current_manufacturer,
            clone_of=clone_of,
        )

    return mapping


def _parse_float(value: str) -> Optional[float]:
    try:
        return float(value.strip())
    except (ValueError, AttributeError):
        return None


def _bounds_from_attrs(attrs: Dict[str, str]) -> Optional[Bounds]:
    if not attrs:
        return None

    x_val = _parse_float(attrs.get("x"))
    y_val = _parse_float(attrs.get("y"))
    width_val = _parse_float(attrs.get("width"))
    height_val = _parse_float(attrs.get("height"))

    if width_val is not None and height_val is not None:
        if x_val is None:
            x_val = 0.0
        if y_val is None:
            y_val = 0.0
        return Bounds(x_val, y_val, width_val, height_val)

    left_val = _parse_float(attrs.get("left"))
    right_val = _parse_float(attrs.get("right"))
    top_val = _parse_float(attrs.get("top"))
    bottom_val = _parse_float(attrs.get("bottom"))

    if None not in (left_val, right_val, top_val, bottom_val):
        return Bounds(left_val, top_val, right_val - left_val, bottom_val - top_val)

    return None


def _extract_bounds(node: Optional[ET.Element], root: ET.Element) -> Optional[Bounds]:
    if node is None:
        return None

    bounds_node = node.find("bounds")
    bounds = _bounds_from_attrs(bounds_node.attrib if bounds_node is not None else node.attrib)
    if bounds is not None:
        return bounds

    ref_name = node.get("ref") or node.get("element")
    if ref_name:
        ref_element = root.find(f".//element[@name='{ref_name}']")
        if ref_element is not None:
            ref_bounds_node = ref_element.find("bounds")
            ref_bounds = _bounds_from_attrs(
                ref_bounds_node.attrib if ref_bounds_node is not None else ref_element.attrib
            )
            if ref_bounds is not None:
                return ref_bounds

    return None

def _extract_display_name(text: str) -> Optional[str]:
    for pattern in DISPLAY_NAME_PATTERNS:
        match = pattern.search(text)
        if match:
            val = match.group(1).strip()
            if val:
                return val
    return None


def _collect_element_images(root: ET.Element) -> Dict[str, str]:
    mapping: Dict[str, str] = {}
    for element in root.findall("element"):
        name_attr = element.get("name") or ""
        image_node = element.find("image")
        if image_node is not None and "file" in image_node.attrib:
            mapping[name_attr] = image_node.attrib["file"]
    return mapping


def _is_background_name(name: str) -> bool:
    lowered = name.lower()
    return (
        "background" in lowered
        or "backdrop" in lowered
        or lowered.startswith("bg")
        or lowered.startswith("screen")
    )


def _is_background_name_global(name: str) -> bool:
    """Stricter than _is_background_name, for _pick_best_background's
    whole-layout fallback scan only.

    The loose bg-/screen-prefix rules in _is_background_name are fine when
    scoped to elements immediately adjacent to one specific <screen> tag
    (its original use) - false positives there are rare and low-stakes.
    Applied across *every* declared element in the layout instead, those
    same loose rules produce real false positives (e.g. "Screen-TopNS", a
    screen-glass/overlay asset unrelated to any decorative background, just
    because its name happens to start with "screen") that can crash later
    crop math when the wrong, oddly-sized image gets treated as one.
    """
    lowered = name.lower()
    return "background" in lowered or "backdrop" in lowered


def _collect_adjacent_background_candidates(
    siblings: Sequence[ET.Element],
    position: int,
) -> List[ET.Element]:
    """Collect every background-name-matching element/overlay adjacent to
    the screen at ``position``, scanning outward in both directions until
    hitting another ``screen`` tag (nearest-before elements first, then
    nearest-after). This is the same scan the old single-candidate version
    did; it just doesn't stop at the first hit, since some artwork styles
    place more than one background-ish element around the same screen (see
    _pick_best_background).
    """
    candidates: List[ET.Element] = []

    before = position - 1
    while before >= 0:
        candidate = siblings[before]
        if candidate.tag == "screen":
            break
        if candidate.tag in {"element", "overlay"}:
            name = candidate.get("ref") or candidate.get("element") or ""
            if _is_background_name(name):
                candidates.append(candidate)
        before -= 1

    after = position + 1
    while after < len(siblings):
        candidate = siblings[after]
        if candidate.tag == "screen":
            break
        if candidate.tag in {"element", "overlay"}:
            name = candidate.get("ref") or candidate.get("element") or ""
            if _is_background_name(name):
                candidates.append(candidate)
        after += 1

    return candidates


def _image_detail_score(path: Path) -> float:
    """Rough 'real art vs. flat tint' signal, used to disambiguate multiple
    background-like candidates near the same screen.

    Some MAME artwork styles (e.g. the "hydef"-credited photo-realistic
    layouts) place a small, near-solid-color screen-tint overlay (composited
    with blend="multiply"/alpha) right next to the <screen> tag alongside
    the real decorative backdrop - both can be named/sized similarly, so
    proximity or bounds alone can't tell them apart. A flat tint is,
    definitionally, close to a single solid color, so its pixel standard
    deviation is near zero versus real photographic/decorative art. Missing
    or unreadable files score lowest so a working candidate is always
    preferred over a broken reference.
    """
    try:
        with Image.open(path) as img:
            arr = np.asarray(img.convert("RGB"), dtype=np.float32)
    except Exception:
        return -1.0
    return float(arr.std())


def _pick_best_background(
    candidates: List[ET.Element],
    image_map: Dict[str, str],
    folder: Path,
) -> Optional[ET.Element]:
    """Pick the best background candidate.

    Defaults to candidates[0] - the same element the original single-match
    _find_adjacent_background would have returned (nearest-before the
    screen, else nearest-after), with zero extra file I/O. That proximity
    pick is correct for the vast majority of artwork sets and is trusted
    as-is: this does NOT chase whichever adjacent candidate merely *looks*
    most detailed, since a nearby but unrelated element (e.g. a screen-glass
    overlay) can easily out-score the real background on pure pixel
    variance while still being the wrong asset. Only when the default
    pick's own score suggests it's a flat tint (see
    _image_detail_score/_FLAT_BACKGROUND_SCORE_THRESHOLD) do we look for
    something better - first among the other in-view candidates, then
    (in the caller) across the whole layout file.
    """
    if not candidates:
        return None

    resolved: List[Tuple[ET.Element, Optional[str]]] = []
    seen_files: set[str] = set()
    for cand in candidates:
        file_name = _resolve_background_file(cand, image_map)
        if file_name and file_name in seen_files:
            continue
        if file_name:
            seen_files.add(file_name)
        resolved.append((cand, file_name))

    if not resolved:
        return candidates[0]

    def score_of(file_name: Optional[str]) -> float:
        if not file_name:
            return -1.0
        return _image_detail_score(folder / file_name)

    best_cand, best_file = resolved[0]
    best_score = score_of(best_file)

    if best_score < _FLAT_BACKGROUND_SCORE_THRESHOLD and len(resolved) > 1:
        alt_cand, alt_file = max(resolved[1:], key=lambda item: score_of(item[1]))
        alt_score = score_of(alt_file)
        if alt_score > best_score:
            best_cand, best_file, best_score = alt_cand, alt_file, alt_score

    if best_score < _FLAT_BACKGROUND_SCORE_THRESHOLD:
        # Some artwork styles only ever place a flat screen-tint overlay
        # near the <screen> tag in the view VIEW_PRIORITY prefers (the
        # "clean, no frame/shadow" views), with the real decorative art
        # declared elsewhere in the layout under a different element that's
        # simply not used anywhere near this screen in this view. If even
        # our best in-view candidate looks flat, widen the search to every
        # background-named element declared anywhere in the layout file -
        # image_map already covers the whole document, not just this view.
        global_best_file = best_file
        global_best_score = best_score
        for elem_name, file_name in image_map.items():
            if not _is_background_name_global(elem_name) or file_name == best_file:
                continue
            s = score_of(file_name)
            if s > global_best_score:
                global_best_score = s
                global_best_file = file_name
        if global_best_file != best_file:
            # Synthetic element carrying just a file reference -
            # _resolve_background_file() checks node.get("file") once
            # name-based lookup comes up empty, so this is honored. It has
            # no bounds, so _extract_bounds() will fall back to "use the
            # whole image, no crop" for this surface - a safe default for
            # what's already a best-effort recovery path.
            return ET.Element("element", {"file": global_best_file})

    return best_cand


def _resolve_background_file(node: Optional[ET.Element], image_map: Dict[str, str]) -> Optional[str]:
    if node is None:
        return None
    name_attr = node.get("ref") or node.get("element") or node.get("name") or ""
    if name_attr:
        file_name = image_map.get(name_attr)
        if file_name:
            return file_name
    file_attr = node.get("file")
    if file_attr:
        return file_attr
    image_child = node.find("image")
    if image_child is not None and "file" in image_child.attrib:
        return image_child.attrib["file"]
    return None


def _collect_background_candidates(
    siblings: Sequence[ET.Element],
    image_map: Dict[str, str],
) -> List[str]:
    collected: List[str] = []
    seen: set[str] = set()
    for node in siblings:
        if node.tag not in {"element", "overlay"}:
            continue
        name_attr = node.get("ref") or node.get("element") or ""
        if not _is_background_name(name_attr):
            continue
        file_name = image_map.get(name_attr)
        if file_name and file_name not in seen:
            collected.append(file_name)
            seen.add(file_name)
    return collected


def _find_label_overlay(
    siblings: Sequence[ET.Element],
    screen_pos: int,
    background_node: Optional[ET.Element],
    background_bounds: Optional[Bounds],
    image_map: Dict[str, str],
    root: ET.Element,
) -> Optional[Tuple[str, float]]:
    """Detect a translucent static-label overlay layered directly on top of
    the chosen background, for the screen at ``screen_pos``.

    Some artwork sets (Bassmate Computer is the only one in this catalog)
    draw their always-visible printed labels ("SPRG", "WORM", ...) as a
    separate element named with a "TEXT-" prefix, placed immediately
    *after the <screen> tag itself* with <color alpha="..."> (the
    background element for the same screen sits immediately *before* the
    <screen> tag - see default.lay: "Screen-TopNS, screen, TEXT-Top,
    Screen-BottomNS, screen, TEXT-Bottom"), at identical bounds to the
    background, rather than baking the text into the background PNG
    itself or into the LCD segments. Our renderer only ever draws one
    background image per screen, so without this the labels are silently
    dropped entirely and the LCD segments (which are just plain on/off
    indicator boxes on this kind of device, not glyph-shaped) end up
    looking like meaningless solid squares instead of highlighted options.

    This same "element, screen, translucent element" shape is also used by
    plenty of *other* artwork sets for unrelated things - a same-image
    self-tint left at alpha="0.0" as a no-op (e.g. gnw_dkong), or a
    genuine decorative overlay like Oil Panic's "Balcony" or Pinball's
    "Plastics-Top" at alpha 0.5 that's no more a static label than the LCD
    segments are - baking any of those in was tried and broke 30+ other
    games' resolved backgrounds when this was first written, so two
    conditions narrow this to just the real idiom:
    - the overlay element's own ref/name must start with "text" (the one
      naming convention actually used for this purpose in this catalog -
      verified nothing else in the whole decompress/ tree uses it);
    - the chosen background must be the *literal* immediate predecessor of
      this exact <screen> tag (siblings[screen_pos - 1]), not merely
      "some background-ish element somewhere near it" - _pick_best_background
      can walk past nearby decorative elements (Bubbles/Gradient/a repeated
      self-tint) to find the real background further away in the layout,
      and this must not pair that resolved background with an unrelated
      element that only happens to sit next to the <screen> tag.
    """
    if background_bounds is None or background_node is None:
        return None
    if screen_pos - 1 < 0 or siblings[screen_pos - 1] is not background_node:
        return None

    idx = screen_pos + 1
    if idx >= len(siblings):
        return None

    candidate = siblings[idx]
    if candidate.tag not in {"element", "overlay"}:
        return None

    candidate_name = candidate.get("ref") or candidate.get("element") or candidate.get("name") or ""
    if not candidate_name.lower().startswith("text"):
        return None

    color_node = candidate.find("color")
    if color_node is None:
        return None
    alpha = _parse_float(color_node.get("alpha", "1"))
    if alpha is None or alpha >= 0.999:
        return None

    candidate_bounds = _extract_bounds(candidate, root)
    if candidate_bounds is None:
        return None

    def _close(a: float, b: float, tol: float = 2.0) -> bool:
        return abs(a - b) <= tol

    if not (
        _close(candidate_bounds.x, background_bounds.x)
        and _close(candidate_bounds.y, background_bounds.y)
        and _close(candidate_bounds.width, background_bounds.width)
        and _close(candidate_bounds.height, background_bounds.height)
    ):
        return None

    overlay_file = _resolve_background_file(candidate, image_map)
    if not overlay_file:
        return None
    return overlay_file, alpha


def _composite_label_overlay(folder: Path, background_file: str, overlay_file: str, alpha: float) -> Optional[str]:
    """Bake a translucent static-label element (see _find_label_overlay)
    into the background image itself, once, caching the result alongside
    the source images so later runs don't redo the work.
    """
    bg_path = folder / background_file
    overlay_path = folder / overlay_file
    if not bg_path.exists() or not overlay_path.exists():
        return None

    out_name = f"{Path(background_file).stem}_labeled.png"
    out_path = folder / out_name

    try:
        if (
            out_path.exists()
            and out_path.stat().st_mtime >= bg_path.stat().st_mtime
            and out_path.stat().st_mtime >= overlay_path.stat().st_mtime
        ):
            return out_name

        bg_img = Image.open(bg_path).convert("RGBA")
        overlay_img = Image.open(overlay_path).convert("RGBA")
        if overlay_img.size != bg_img.size:
            overlay_img = overlay_img.resize(bg_img.size, Image.LANCZOS)

        bg_arr = np.asarray(bg_img, dtype=np.float32)
        overlay_arr = np.asarray(overlay_img, dtype=np.float32)
        src_a = (overlay_arr[..., 3:4] / 255.0) * alpha
        out_rgb = overlay_arr[..., :3] * src_a + bg_arr[..., :3] * (1.0 - src_a)
        out_arr = np.dstack([out_rgb, bg_arr[..., 3:4]]).astype(np.uint8)
        Image.fromarray(out_arr, mode="RGBA").save(out_path)
    except Exception:
        return None

    return out_name


def _extract_view_assets(
    root: ET.Element,
    image_map: Dict[str, str],
    folder: Path,
) -> Tuple[List[str], List[SurfaceData]]:
    for view_name, _ in VIEW_PRIORITY:
        view = root.find(f".//view[@name='{view_name}']")
        if view is None:
            continue

        siblings = list(view)
        screen_positions = [i for i, node in enumerate(siblings) if node.tag == "screen"]
        surfaces: List[SurfaceData] = []
        background_files: List[str] = []

        if screen_positions:
            ordered_ids: List[str] = []
            surface_by_id: Dict[str, SurfaceData] = {}
            for pos in screen_positions:
                screen_node = siblings[pos]
                screen_bounds = _extract_bounds(screen_node, root)
                if screen_bounds is None:
                    continue
                screen_id = screen_node.get("index") or str(len(ordered_ids))
                if screen_id not in surface_by_id:
                    ordered_ids.append(screen_id)
                bg_candidates = _collect_adjacent_background_candidates(siblings, pos)
                background_node = _pick_best_background(bg_candidates, image_map, folder)
                background_bounds = _extract_bounds(background_node, root)
                background_file = _resolve_background_file(background_node, image_map)

                overlay = _find_label_overlay(siblings, pos, background_node, background_bounds, image_map, root)
                if overlay is not None and background_file:
                    overlay_file, overlay_alpha = overlay
                    labeled_file = _composite_label_overlay(folder, background_file, overlay_file, overlay_alpha)
                    if labeled_file:
                        background_file = labeled_file

                surface_by_id[screen_id] = SurfaceData(
                    screen_bounds=screen_bounds,
                    background_bounds=background_bounds,
                    background_file=background_file,
                )

            surfaces = [surface_by_id[idx] for idx in ordered_ids]
            seen_files: set[str] = set()
            for surface in surfaces:
                if surface.background_file and surface.background_file not in seen_files:
                    background_files.append(surface.background_file)
                    seen_files.add(surface.background_file)

            if surfaces or background_files:
                return background_files, surfaces

        fallback_files = _collect_background_candidates(siblings, image_map)
        if fallback_files:
            return fallback_files, surfaces

    return [], []


def _compute_transform(surface: SurfaceData) -> List[List[int]]:
    screen = surface.screen_bounds
    bg = surface.background_bounds

    width = int(round(screen.width))
    height = int(round(screen.height))

    if bg is None:
        return [[width, 0, 0], [height, 0, 0]]

    left_cut = max(0, int(round(bg.x - screen.x)))
    right_cut = max(0, int(round((screen.x + screen.width) - (bg.x + bg.width))))
    top_cut = max(0, int(round(bg.y - screen.y)))
    bottom_cut = max(0, int(round((screen.y + screen.height) - (bg.y + bg.height))))

    # Defensive clamp: on a multi-screen device (3+ screens, e.g. Tronica's
    # tri-screen units) it's possible for a screen to get paired with the
    # wrong background element from the .lay file, producing a cut that
    # exceeds the screen's own width/height - im.transform_img() downstream
    # has no bounds checking and crashes with a negative-size PIL error in
    # that case, silently dropping the whole game from the pack (caught by
    # convert_switch.py's per-game try/except). Falling back to "no crop"
    # here is a graceful degradation (a slightly less precisely-cropped
    # background) rather than losing the game entirely - confirmed against
    # trtreisl (Treasure Island, Tronica), whose left screen hit exactly
    # this mismatch.
    if left_cut + right_cut >= width or top_cut + bottom_cut >= height:
        return [[width, 0, 0], [height, 0, 0]]

    return [[width, left_cut, right_cut], [height, top_cut, bottom_cut]]


def _compute_transforms(surfaces: Sequence[SurfaceData]) -> List[List[List[int]]]:
    return [_compute_transform(surface) for surface in surfaces]


def _detect_multiscreen_config(
    surfaces: Sequence[SurfaceData],
    left_right_size: Tuple[int, int],
    top_bottom_size: Tuple[int, int],
) -> Tuple[List[List[int]], bool]:
    """Detect multi-screen orientation and return size_visual and two_in_one_screen flag.
    
    Returns:
        Tuple of (size_visual, two_in_one_screen) where:
        - size_visual is [[w1, h1], [w2, h2]] for multi-screen games, [] otherwise
        - two_in_one_screen is True for left/right orientation, False otherwise
    """
    if len(surfaces) >= 3:
        # 3+ screen devices (e.g. Tronica's tri-screen units, confirmed
        # against trtreisl/Treasure Island) don't fit the uniform-size
        # left/right or top/bottom model below, which assumes exactly two
        # same-sized panels - their screens are typically different sizes
        # (a larger center display flanked by two smaller wings), so each
        # gets its own size, proportioned relative to the others rather
        # than a forced uniform one.
        #
        # The .lay file's own coordinate units are arbitrary and often in
        # the thousands (trtreisl's widest screen is ~1975 units) - passing
        # that straight through as a *pixel* target broke segment atlas
        # packing (every one of this game's 134 segments rendered at a
        # proportionally huge size, together exceeding every candidate
        # atlas bin). Rescale so the single largest screen dimension lands
        # at the same order of magnitude the 2-screen sizes below use,
        # preserving every screen's own aspect ratio and their relative
        # size to each other - only the overall scale changes.
        target_max = max(left_right_size[0], left_right_size[1], top_bottom_size[0], top_bottom_size[1])
        native_max = max(max(s.screen_bounds.width, s.screen_bounds.height) for s in surfaces)
        scale = (target_max / native_max) if native_max > 0 else 1.0
        return [
            [max(1, int(round(s.screen_bounds.width * scale))), max(1, int(round(s.screen_bounds.height * scale)))]
            for s in surfaces
        ], False

    if len(surfaces) != 2:
        # 0 or 1 screens - unchanged from before, convert_switch.py's own
        # [resolution_up, resolution_down] default covers the single-screen
        # case fine (only index 0 of it is ever read).
        return [], False

    screen1 = surfaces[0].screen_bounds
    screen2 = surfaces[1].screen_bounds

    # Calculate centers
    center1_x = screen1.x + screen1.width / 2
    center1_y = screen1.y + screen1.height / 2
    center2_x = screen2.x + screen2.width / 2
    center2_y = screen2.y + screen2.height / 2

    # Determine orientation based on relative positions
    dx = abs(center2_x - center1_x)
    dy = abs(center2_y - center1_y)

    # If screens are primarily arranged horizontally (left/right)
    if dx > dy:
        # Left/right configuration
        w, h = left_right_size
        return [[w, h], [w, h]], True
    else:
        # Top/bottom configuration
        w, h = top_bottom_size
        return [[w, h], [w, h]], False


def _find_rom_in_folder(folder: Path) -> Optional[Path]:
    for pattern in ROM_PRIORITY:
        matches = sorted(folder.glob(pattern))
        if matches:
            return matches[0]

    bare: List[Path] = []
    for item in folder.iterdir():
        if (
            item.is_file()
            and item.suffix == ""
            and item.name.lower() not in {"default", "default.lay", ".gitkeep"}
        ):
            bare.append(item)

    if bare:
        return sorted(bare)[0]
    return None


def _resolve_rom_file(
    name: str,
    folder: Path,
    fallback_folder: Optional[Path],
    folder_map: Dict[str, Path],
    clone_of_map: Dict[str, str],
) -> Optional[Path]:
    rom_path = _find_rom_in_folder(folder)
    if rom_path is not None:
        return rom_path

    if fallback_folder is not None:
        rom_path = _find_rom_in_folder(fallback_folder)
        if rom_path is not None:
            return rom_path

    for fallback_name in _rom_fallback_candidates(name, clone_of_map):
        candidate_folder = folder_map.get(fallback_name)
        if candidate_folder is None:
            continue
        rom_path = _find_rom_in_folder(candidate_folder)
        if rom_path is not None:
            return rom_path

    return None


def _find_melody_file(
    name: str,
    folder: Path,
    fallback: Optional[Path],
    folder_map: Dict[str, Path],
    clone_of_map: Dict[str, str],
    rom_path: Optional[Path] = None,
) -> Optional[Path]:
    candidates: List[Path] = [folder]
    if fallback is not None:
        candidates.append(fallback)

    if rom_path is not None:
        rom_folder = rom_path.parent
        if rom_folder not in candidates:
            candidates.append(rom_folder)

    for fallback_name in _rom_fallback_candidates(name, clone_of_map):
        candidate_folder = folder_map.get(fallback_name)
        if candidate_folder is not None and candidate_folder not in candidates:
            candidates.append(candidate_folder)

    for candidate in candidates:
        matches = sorted(candidate.glob("*.melody"))
        if matches:
            return matches[0]

    return None


def _visual_sort_key(path: Path) -> Tuple[int, str]:
    lowered = path.name.lower()
    if any(token in lowered for token in ("top", "upper", "left")):
        priority = 0
    elif any(token in lowered for token in ("bottom", "lower", "right")):
        priority = 2
    else:
        priority = 1
    return priority, lowered


def _find_visual_files(folder: Path, fallback: Optional[Path]) -> List[Path]:
    visuals = sorted(folder.glob("*.svg"), key=_visual_sort_key)
    if visuals:
        return visuals
    if fallback is not None:
        return sorted(fallback.glob("*.svg"), key=_visual_sort_key)
    return []


def _resolve_console_path(folder: Path) -> Path:
    return folder / f"{folder.name}.png"


def _sanitize_key(display_name: str, fallback: str) -> str:
    base = re.sub(r"[^0-9A-Za-z]+", "_", display_name).strip("_")
    if not base:
        base = re.sub(r"[^0-9A-Za-z]+", "_", fallback).strip("_")
    if base and base[0].isdigit():
        base = f"_{base}"
    return base or fallback


def _rom_fallback_candidates(name: str, clone_of_map: Dict[str, str]) -> List[str]:
    candidates: List[str] = []

    if len(name) > 1:
        trimmed = name[:-1]
        if len(trimmed) >= 3:
            candidates.append(trimmed)

    # Clone mapping is authored in GNW_LIST.md (Clone Of column).
    clone_of = clone_of_map.get(name.lower())
    if clone_of:
        candidates.append(clone_of)

    return candidates


def _parse_layout(lay_path: Path) -> Tuple[Optional[str], Optional[str], List[str], List[SurfaceData]]:
    try:
        text = lay_path.read_text(encoding="utf-8", errors="ignore")
    except OSError:
        return None, None, [], []

    display_name = _extract_display_name(text)

    try:
        root = ET.fromstring(text)
    except ET.ParseError:
        return display_name, None, [], []

    image_map = _collect_element_images(root)
    background_files, surfaces = _extract_view_assets(root, image_map, lay_path.parent)

    ref_candidate: Optional[str] = None
    for element in root.findall("element"):
        name_attr = element.get("name") or ""
        if ref_candidate is None and MODEL_PATTERN.match(name_attr):
            ref_candidate = name_attr.strip()

    return display_name, ref_candidate, background_files, surfaces

def _iter_artwork_folders(root: Path, metadata_map: Dict[str, GameMetadata]) -> Iterable[Tuple[str, Path]]:
    """Yield (name, path) for every artwork folder that exists in GNW_LIST.md.

    This used to hardcode prefix checks (gnw_/tr/ti). Now that we load authoritative
    game keys from GNW_LIST.md at runtime (including additional manufacturers),
    filter by that map instead.
    """
    for entry in sorted(root.iterdir()):
        if not entry.is_dir():
            continue
        name = entry.name
        lowered = name.lower()
        if lowered in metadata_map:
            yield name, entry


def _resolve_default_lay(
    name: str,
    folder: Path,
    folder_map: Dict[str, Path],
    clone_of_map: Dict[str, str],
) -> Tuple[Optional[Path], str]:
    """Check for default.lay in folder, falling back to related folders.

    Returns a tuple (layout_path, source_name) where:
      * layout_path: path to default.lay when found, otherwise None.
      * source_name: 'self' when the file lives in the folder itself,
        otherwise the fallback folder name that supplied the layout.
    """

    layout_path = folder / "default.lay"
    if layout_path.exists():
        return layout_path, "self"

    # Reuse ROM fallback candidates so special clone mappings (e.g., tigarden -> trshutvoy)
    # can also supply the layout and backgrounds.
    for fallback_name in _rom_fallback_candidates(name, clone_of_map):
        fallback_folder = folder_map.get(fallback_name)
        if not fallback_folder:
            continue
        fallback_path = fallback_folder / "default.lay"
        if fallback_path.exists():
            return fallback_path, fallback_name

    return None, ""

def generate_games_path(target_name: str | None = None) -> bool:
    if not target_name:
        target_name = "3ds"
    profile = get_target(target_name)

    script_dir = Path(__file__).resolve().parent.parent
    metadata_map = _load_game_metadata(script_dir)
    clone_of_map: Dict[str, str] = {
        name: (meta.clone_of or "")
        for name, meta in metadata_map.items()
        if (meta.clone_of or "").strip()
    }
    rom_root = script_dir / "rom" / "decompress"
    if not rom_root.exists():
        print("rom/ directory not found", file=sys.stderr)
        raise SystemExit(1)

    folder_map = {name: path for name, path in _iter_artwork_folders(rom_root, metadata_map)}

    entries: List[GameEntry] = []
    skipped: List[Tuple[str, str]] = []
    key_counts: Dict[str, int] = {}
    missing_console: List[Path] = []

    for name, folder in folder_map.items():
        metadata = metadata_map.get(name.lower())

        if metadata is None:
            skipped.append((name, "unsupported game: not in GNW_LIST.md"))
            continue

        layout_path, source = _resolve_default_lay(name, folder, folder_map, clone_of_map)
        fallback_folder = folder_map.get(source) if source not in {"", "self"} else None

        if layout_path is None:
            trimmed_name = name[:-1] if len(name) > 1 else ""
            trimmed_folder = folder_map.get(trimmed_name)
            if trimmed_folder is not None:
                layout_path = trimmed_folder / "default.lay"
                if layout_path.exists():
                    fallback_folder = trimmed_folder
            if layout_path is None or not layout_path.exists():
                skipped.append((name, "missing default.lay"))
                continue

        display_name, model_ref, background_files, surfaces = _parse_layout(layout_path)

        rom_path = _resolve_rom_file(name, folder, fallback_folder, folder_map, clone_of_map)
        if rom_path is None:
            skipped.append((name, "no ROM candidate"))
            continue

        visuals = _find_visual_files(folder, fallback_folder)
        if not visuals:
            skipped.append((name, "no SVG visuals"))
            continue

        resolved_backgrounds: List[Path] = []
        for file_name in background_files:
            primary_candidate = folder / file_name
            if primary_candidate.exists():
                resolved_backgrounds.append(primary_candidate)
                continue
            if fallback_folder is not None:
                fallback_candidate = fallback_folder / file_name
                if fallback_candidate.exists():
                    resolved_backgrounds.append(fallback_candidate)
        background_paths = resolved_backgrounds

        transform_data = _compute_transforms(surfaces) if surfaces else []
        if transform_data and len(transform_data) < len(visuals):
            transform_data = []

        size_visual_data, two_in_one_screen = _detect_multiscreen_config(
            surfaces,
            profile.multiscreen_left_right_size,
            profile.multiscreen_top_bottom_size,
        )

        console_path = _resolve_console_path(folder)
        if not console_path.exists():
            missing_console.append(console_path)
            console_path = Path(default_img_console)

        melody_path = _find_melody_file(
            name,
            folder,
            fallback_folder,
            folder_map,
            clone_of_map,
            rom_path=rom_path,
        )

        # `GNW_LIST.md` is the source of truth for model/ref.
        ref_value = (metadata.model or "").strip().lower()
        if not ref_value:
            raise ValueError(f"Missing model for {name} in GNW_LIST.md")

        display_source = metadata.display_title
        key_candidate = _sanitize_key(display_source or name, name)
        index = key_counts.get(key_candidate, 0)
        key_counts[key_candidate] = index + 1
        key = key_candidate if index == 0 else f"{key_candidate}_{index + 1}"

        display_label = display_source or key_candidate.replace("_", " ")
        date_value = metadata.release_date

        # Manufacturer is authored via GNW_LIST section headings and copied from metadata.
        manufacturer = int(metadata.manufacturer)

        # Determine if mask should be True (Panorama games and specific models)
        needs_mask = (
            "panorama" in display_label.lower() or
            "table top" in display_label.lower() or
            (metadata and metadata.model.upper() in {"MK-96", "TB-94"})
        )
        print(ref_value)
        # Ball, Fire (2 versions), DK-JR (wide screen)
        background_in_front = ref_value.lower() in ("ac-01", "mt-03", "rc-04", "mh-06", "cn-01", "ln-08"
                                                    "pr-21", "oc-22", "pp-23", "fp-24", "mc-25", "eg-26"
                                                    "fr-27", "sp-30", "dj-101", "ml-102", "nh-103"
                                                    , "gh-54", "jr-55", "mw-56", "mg-61"
                                                    , "ak-302", "hk-303")
        
        active_cam = "crystal" in display_label.lower()

        entry = GameEntry(
            folder_name=name,
            key=key,
            display_name=display_label,
            ref=ref_value,
            manufacturer=manufacturer,
            rom_path=rom_path,
            visual_paths=visuals,
            background_paths=background_paths,
            console_path=console_path,
            melody_path=melody_path,
            date=date_value,
            transform_visual=transform_data,
            size_visual=size_visual_data,
            two_in_one_screen=two_in_one_screen,
            mask=needs_mask,
            shadow=True,
            background_in_front=background_in_front,
            active_cam=active_cam
        )
        entries.append(entry)

    entries.sort(key=lambda item: item.key.lower())

    destination_name = f"games_path_{profile.name}.py"
    destination = script_dir / destination_name
    write_games_path(entries, script_dir, destination)

    print(f"Generated {len(entries)} games. Output -> {destination}")

    has_issues = False
    if skipped:
        has_issues = True
        print("\nSkipped folders:")
        for name, reason in skipped:
            print(f"  - {name}: {reason}")
    if missing_console:
        has_issues = True
        print("\nMissing console images:")
        for path in sorted({p.resolve() for p in missing_console}):
            print(f"  - {path}")

    # Return False if any folders were skipped or console images are missing,
    # True otherwise.
    return not has_issues


if __name__ == "__main__":  # pragma: no cover - script entry point
    # Optional: pass --target rgds (defaults to '3ds').
    # Keep this lightweight (no argparse) to preserve original script behavior.
    arg_target = None
    if len(sys.argv) >= 3 and sys.argv[1] == "--target":
        arg_target = sys.argv[2]
    success = generate_games_path(arg_target)
