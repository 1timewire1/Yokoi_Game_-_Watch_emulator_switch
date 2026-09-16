# Unimplemented games reference

Every game from MAME's `hh_sm510.cpp` driver (the SM5A/SM510/SM511/SM512
family this emulator supports, soon SM530 too) that is **not** currently in
`GNW_LIST.md` in any form - not the base roster, not `## Tiger`, not
`## Experimental`. Compiled as a reference for future work, since tracking
this down meant cross-referencing local `rom/artwork`+`rom/roms` against
every `SYST()` line in the driver and reading each one's header comments
individually.

Last refreshed 2026-09-16. After the Konami batch (20 games), `trtreisl`,
`vesarif`, `bassmate`, `naltair`, the new SM530 core (`nsmb3`/`nsmw`/
`nstarfox`), and the 3-layer modifier-key input scheme (`nummunch`/
`uchitari`) all shipped this session, every group that used to fill this
file is gone except two hard technology blockers. What's left: **2
games**, each blocked for a genuinely distinct reason.

---

### `elbaskb` — Elektronika Basketbol (model ИМ-55)

| Romname | Manufacturer | Title | Year |
|---|---|---|---|
| elbaskb | Elektronika | Basketbol | 199? (exact year unknown) |

**Hard blocker: wrong display technology.** MAME's own comment calls this
out directly: it's "a LED game with an LCD driver MCU, that's
unconventional" - 26 individual LEDs plus 4 seven-segment LEDs, not an LCD
segment panel at all. This project's entire rendering pipeline (background
art + LCD segment overlays composited from PNGs) assumes an LCD display;
a real LED matrix game doesn't fit that model regardless of CPU
compatibility. Would need a fundamentally different rendering approach,
not just new data.

### `supkubik` — Elektronika Superkubiki (model ИМ-20)

| Romname | Manufacturer | Title | Year |
|---|---|---|---|
| supkubik | Elektronika | Superkubiki ("Super Cubes") | 199? (exact year unknown) |

A Soviet **Tetris clone** - confirmed via its own MAME input ports, which
are literally labeled Rotate/Drop/Left/Right/Slow down/Mode/Pause (in
Russian). SM510-compatible, no CPU or display-technology blocker. The real
issue: MAME's artwork is flagged `BAD_DUMP` with the comment *"original
LCD scan needed"* - per the PR that added it
([mamedev/mame#16064](https://github.com/mamedev/mame/pull/16064)), the
contributor had PCB access but not the actual LCD panel, and drew the SVG
"from scratch" based on a low-resolution, blurry infrared photo. Even
MAME's own maintainers don't consider this artwork verified. Implementing
it now would mean baking in guessed art with no better source available
than what the original MAME contributor already lacked.

---

## Summary by blocker type

| Blocker | Games | Fix |
|---|---|---|
| Wrong display technology (LED, not LCD) | elbaskb = **1** | Would need a different rendering approach entirely |
| Unverified/hand-drawn MAME artwork | supkubik = **1** | Wait for a real LCD scan to surface upstream in MAME |
