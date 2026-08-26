from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Tuple


@dataclass(frozen=True)
class TargetProfile:
    """Output profile for generating PNGs/metadata from original assets."""

    name: str

    # Default size_visual used when a game doesn’t specify it.
    resolution_up: Tuple[int, int]
    resolution_down: Tuple[int, int]
    demi_resolution_up: Tuple[int, int]

    # Console screenshot output size and atlas size.
    console_size: Tuple[int, int]
    console_atlas_size: Tuple[int, int]

    # SVG segment rasterization DPI for Inkscape exports.
    export_dpi: int

    # Candidate atlas sizes for segment packing.
    atlas_sizes: Tuple[int, ...]

    # Background atlas max size (backgrounds are packed differently).
    background_atlas_sizes: Tuple[int, ...]

    # Multiply per-game size_visual (and thus generated assets/coords).
    size_scale: int

    # Multi-screen *logical* LCD panel sizes used for generating size_visual
    # in games_path and related post-processing.
    #
    # NOTE: These are per-target, because the Android/RGDS build uses a
    # different logical coordinate system than the original 3DS output.
    multiscreen_left_right_size: Tuple[int, int]
    multiscreen_top_bottom_size: Tuple[int, int]


TARGETS: Dict[str, TargetProfile] = {
    "3ds": TargetProfile(
        name="3ds",
        resolution_up=(400, 240),
        resolution_down=(320, 240),
        demi_resolution_up=(200, 240),
        console_size=(320, 240),
        console_atlas_size=(512, 256),
        export_dpi=50,
        atlas_sizes=(32, 64, 128, 256, 512, 1024),
        background_atlas_sizes=(128, 256, 512, 1024),
        size_scale=1,
        multiscreen_left_right_size=(200, 240),
        multiscreen_top_bottom_size=(320, 240),
    ),
    # RG DS / 640x480-class target:
    # - Sizes here are already the intended RGDS logical sizes (2x vs 3DS).
    # - Therefore size_scale stays 1 to avoid double-scaling.
    "rgds": TargetProfile(
        name="rgds",
        resolution_up=(640, 480),
        resolution_down=(640, 480),
        demi_resolution_up=(320, 480),
        console_size=(640, 480),
        console_atlas_size=(1024, 512),
        export_dpi=100,
        atlas_sizes=(32, 64, 128, 256, 512, 1024, 2048),
        background_atlas_sizes=(128, 256, 512, 1024, 2048),
        size_scale=1,
        multiscreen_left_right_size=(320, 480),
        multiscreen_top_bottom_size=(640, 480),
    ),
}


def get_target(name: str) -> TargetProfile:
    try:
        return TARGETS[name]
    except KeyError:
        raise KeyError(f"Unknown target '{name}'. Valid targets: {', '.join(sorted(TARGETS.keys()))}")
