# For games with no photographed console/bezel art in their MAME artwork set
# (only a raw background/LCD layer - see the "Background Only"-only .lay
# views for vinnpukh/tgaiden), key that raw capture into the LCD-screen
# region of rom/default_console.png instead of using the flat placeholder
# as-is, so at least the game picker shows something distinctive per game.
#
# Screen region was found by flood-filling from the placeholder's screen
# center color out to its bezel edge (confirmed against a hand-marked
# reference crop): (335, 247)-(666, 460), 331x213px.

import argparse
from pathlib import Path
from PIL import Image

SCREEN_BOX = (335, 247, 666, 460)  # (left, top, right, bottom)


def composite(placeholder_path: Path, raw_capture_path: Path, out_path: Path) -> None:
    base = Image.open(placeholder_path).convert("RGBA")
    capture = Image.open(raw_capture_path).convert("RGBA")

    left, top, right, bottom = SCREEN_BOX
    box_w, box_h = right - left, bottom - top
    target_ratio = box_w / box_h

    cw, ch = capture.size
    capture_ratio = cw / ch
    if capture_ratio > target_ratio:
        new_w = int(ch * target_ratio)
        x0 = (cw - new_w) // 2
        capture = capture.crop((x0, 0, x0 + new_w, ch))
    else:
        new_h = int(cw / target_ratio)
        y0 = (ch - new_h) // 2
        capture = capture.crop((0, y0, cw, y0 + new_h))

    capture = capture.resize((box_w, box_h), Image.LANCZOS)

    out = base.copy()
    out.paste(capture, (left, top))
    out.convert("RGB").save(out_path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("raw_capture", type=Path, help="Raw MAME screenshot (e.g. snap_out/vinnpukh.png)")
    parser.add_argument("out", type=Path, help="Output console.png path")
    parser.add_argument(
        "--placeholder",
        type=Path,
        default=Path(__file__).resolve().parent / "rom" / "default_console.png",
    )
    args = parser.parse_args()

    composite(args.placeholder, args.raw_capture, args.out)
    print(f"Wrote {args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
