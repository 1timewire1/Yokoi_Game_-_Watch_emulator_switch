# Other MAME handheld cores — expansion survey

A probe into whether MAME has sibling drivers to `hh_sm510.cpp` (our
SM5A/SM510/SM511/SM512/SM530 family) that use the same internal
segment-artwork/SVG convention, so they'd likely port to this project's
rendering pipeline (background art + segment-shape overlays composited from
PNGs) with a similar amount of effort to what we've already done. Not an
implementation plan - just findings to come back to when picking the next
core to add. Compiled 2026-09-17 by parsing every `SYST()`/`CONS()` driver
entry in MAME's `src/mame/handheld/hh_*.cpp` files (GitHub `mamedev/mame`,
`master` branch) and each file's own header comment.

**2026-09-24 addendum - the first pass only looked at `hh_*.cpp`.** That
filename filter missed `rzone.cpp` (Tiger R-Zone), which sits in the same
`handheld/` directory without the `hh_` prefix, and MAME has since split its
drivers into per-manufacturer folders (`tiger/`, `bandai/`, `epoch/`, ...). A
second pass searched the *whole* repo for every driver that instantiates the
SVG screen device (`SCREEN_SVG`, what `hh_sm510` uses for its segment art) and
every driver that instantiates a Sharp SM5xx CPU - see "Second pass" at the end
of this file.

**Same author, same convention.** Every driver below (like `hh_sm510.cpp`)
is written by MAME contributor `hap` and follows the same "one MCU family
per file, internal `.lay` + segment SVG per game" architecture - this is
the actual reason the whole "hh_" prefix exists as a category. That
consistency is what makes cross-driver porting a reasonable idea in the
first place, rather than needing to evaluate each one from scratch.

**The one real caveat: LED vs. LCD/VFD.** MAME represents all of these the
same way internally (a layout of segment/lamp output elements, on or off),
but the *real hardware* behind each driver varies - some are genuine LCD or
VFD segment panels (glyph-shaped segments, exactly like Game & Watch - the
best fit), some are 7-segment LED calculator-style digits (same segment
*shape*, different real-world light source - still a good fit), and some
are true LED matrices (round/square individual lamps, not segment glyphs -
a different art style, though still segment-*like* in MAME's data model).
None of this blocks porting; it just changes what the resulting artwork
looks like. Noted per-driver below.

Excluded entirely: **`hh_mediatek_mt6260a.cpp`** (Accutime smart watches) -
an ARM7-driven raster-screen device, not a segment display at all, and
every one of its 27 entries is `MACHINE_NOT_WORKING` regardless. Wrong
display technology *and* not emulated - the same combination that already
rules out `elbaskb` in `UNIMPLEMENTED_GAMES.md`, just for a whole driver
instead of one game.

---

## Summary

13 relevant drivers, **254 unique games** (306 total `SYST()`/`CONS()`
entries minus 52 clones), after excluding `hh_mediatek_mt6260a.cpp`. Of
those 254: **15 are `MACHINE_NOT_WORKING`** (fully broken in MAME itself,
not implementable regardless of our own effort), **13 more** carry some
`MACHINE_IMPERFECT_*` flag (playable with a known rough edge), and **5**
are `MACHINE_MECHANICAL` (real hardware has a physical/mechanical
component MAME can only partially simulate - same category as `lchicken`
already flagged in `hh_cop400.cpp`). The rest (~225, ~89%) are clean.

| Driver | CPU family | Display tech | Unique games | Not working | Imperfect | Mechanical | Notable manufacturers |
|---|---|---|---|---|---|---|---|
| `hh_hmcs40.cpp` | Hitachi HMCS40 | VFD (segment) | 39 | 7 | 1 | 0 | Bandai, Coleco, Entex, Tomy, Gakken, Mattel, Actronics/Hanzawa |
| `hh_tms1k.cpp` | TI TMS1000 | Mixed: LED, VFD, some LCD | 114 | 3 | 7 | 4 | Mattel, Milton Bradley, Parker Brothers, Coleco, Ideal, Tandy, Kenner |
| `hh_ucom4.cpp` | NEC uCOM4 | VFD (segment) | 24 | 0 | 0 | 0 | Bambino, Gakken, Epoch, Tomy |
| `hh_cop400.cpp` | National Semiconductor COP400 | LED (mostly) | 18 | 1 | 2 | 1 | Coleco, Mattel, LJN, Castle Toy |
| `hh_pic16.cpp` | GI PIC 16xx | LED | 14 | 2 | 2 | 0 | Toytronic, Calfax, U.S. Games Corp. |
| `hh_pps41.cpp` | Rockwell PPS-4/1 | Mixed VFD/LED | 12 | 0 | 0 | 0 | Mattel, Rockwell |
| `hh_e0c6200.cpp` | Seiko Epson E0C6200 | LCD (segment, keychain form factor) | 10 | 0 | 0 | 0 | Bandai (Tamagotchi, Digimon), Epoch, Tandy |
| `hh_rw5000.cpp` | Rockwell A/B5000 | Mostly LED 7-segment (calculators) | 10 | 0 | 0 | 0 | Rockwell, Mattel |
| `hh_cops1.cpp` | National Semiconductor MM57 (COPS) | LED | 6 | 0 | 0 | 0 | Mattel, National Semiconductor, Sinclair Radionics |
| `hh_smc1k.cpp` | Suwa Seikosha SMC11xx | LCD (segment) | 2 | 2 | 0 | 0 | Tiger Electronics (pre-SM510 European releases) |
| `hh_melps4.cpp` | Mitsubishi MELPS 4 | VFD (segment) | 2 | 0 | 0 | 0 | Coleco/Konami, Gakken/Konami |
| `hh_mn1400.cpp` | Matsushita MN1400 | Unclear (no VFD/LCD/LED/svg mentions - needs individual review) | 2 | 0 | 0 | 0 | Lakeside, Selchow & Righter, Tomy |
| `hh_ht11xx.cpp` | Holtek HT11xx | LCD (segment) | 1 | 0 | 0 | 0 | E-Star (unlicensed "Brick Game" clone) |

---

## Per-driver notes

### `hh_hmcs40.cpp` — top candidate

Largest *clean* driver relative to its size (39 games, only 7 broken).
Same manufacturers as our existing Tiger/Konami/Elektronika libraries
(Bandai, Coleco, Entex, Gakken, Tomy) plus a few new ones. Heaviest `svg`
mention count of any driver relative to its game count, meaning most
entries already have real segment artwork in MAME, not placeholder/guessed
art. The 7 `MACHINE_NOT_WORKING` entries (`wantgman`, `ktparman`,
`tkjmaru`, `ggdman`, `gdefender`, `puckimon`, `bombman`) are all Bandai or
Gakken titles from 1982-84 - worth checking MAME's own bug tracker before
picking any of these specifically.

### `hh_tms1k.cpp` — biggest opportunity, needs per-game triage

By far the largest driver (114 unique games - almost as many as our entire
current library from `hh_sm510.cpp`), but display tech is a genuine mix
across its ~9-year toy-era span: LED mentions outnumber VFD and LCD
combined. A real next-steps pass here would mean filtering for the VFD/LCD
subset first (best art fit) rather than assuming the whole driver ports
uniformly. Recognizable names throughout: Mattel, Milton Bradley, Parker
Brothers, Coleco, Ideal, Kenner. Only 3 fully broken (`elecbowl`,
`bshipb`, `tithermos`); the 4 `MACHINE_MECHANICAL` entries have a
real physical/motorized component, same caveat category as `lchicken` in
our existing `hh_cop400.cpp` notes above.

### `hh_ucom4.cpp` — clean, VFD, zero broken games

24 unique games, zero `MACHINE_NOT_WORKING` or `MACHINE_IMPERFECT_*` at
all. Bambino, Gakken, Epoch, Tomy - familiar names. Second-best candidate
after `hh_hmcs40.cpp` on cleanliness alone.

### `hh_e0c6200.cpp` — Tamagotchi/Digimon, different form factor

All 10 games are `MACHINE_REQUIRES_ARTWORK` (no built-in background,
matching the "keychain toy" reality - these are round/oval devices, not
rectangular G&W-style units) but otherwise clean. Strong brand recognition
(original Tamagotchi, Digital Monster) if that's a priority, but the round
keychain form factor means our console-art/thumbnail pipeline would need
new handling rather than reusing the existing rectangular-unit assumptions.

### `hh_smc1k.cpp` — Tiger-adjacent, but not ready yet

Only 2 games, and both are `MACHINE_NOT_WORKING` - MAME's own TODO
comment says why: `add svg for: tkkongq, tlluke2` (the segment artwork
itself hasn't been drawn upstream yet). Notable for lineage, not
opportunity: per the driver's header comment, this is the MCU family
behind Tiger's *pre*-SM510 European releases (published through
Orlitronic/Grandstand/Virca rather than Tiger directly) - same publisher
family as a large chunk of our existing Experimental library, just one
generation earlier and not emulated well enough yet. Worth revisiting
later if MAME finishes these.

### `hh_melps4.cpp` — tiny, but Konami-licensed

Only 2 games, both clean: `cfrogger` (Coleco/Konami Frogger) and
`gjungler` (Gakken/Konami Jungler) - more Konami-licensed titles, on a
completely different MCU than the 20 Konami games we already ported this
session.

### The LED-dominant drivers (`hh_cop400`, `hh_cops1`, `hh_pic16`, most of `hh_rw5000`)

Still segment-shaped data in MAME's model (individual LEDs on/off, or
7-segment LED digits for the `hh_rw5000.cpp` calculators), just a
different real-world light source than LCD/VFD glass. Graphically these
would need LED-style art (dots/segments on a dark background) rather than
the ink-on-white-plastic look our existing pipeline assumes for G&W/Tiger -
not a blocker, but a different visual treatment, so probably lower
priority than the VFD/LCD drivers above unless the games themselves are
the draw.

### `hh_mn1400.cpp` and `hh_ht11xx.cpp` — too small to categorize confidently

2 and 1 games respectively; neither file's header comment states a display
technology and neither had enough `VFD`/`LCD`/`LED`/`svg` keyword hits to
infer one automatically. Would need to open each `.lay`/game class
directly to know what these actually look like before deciding if they're
worth the effort at that scale.

---

## Not a blocker list — just triage priority

Unlike `UNIMPLEMENTED_GAMES.md` (specific games we've ruled out), nothing
here is disqualified. Rough priority order if/when this gets picked back
up: **`hh_hmcs40.cpp`** (cleanest large driver, familiar manufacturers) →
**`hh_ucom4.cpp`** (clean, smaller) → **`hh_tms1k.cpp`** filtered to its
VFD/LCD subset (biggest raw opportunity, but needs the filtering pass
first) → the LED-dominant and small/uncertain drivers after that, roughly
in the order listed above.

---

## Second pass (2026-09-24): whole-repo search

Method: searched all of `mamedev/mame` for (a) every file using `SCREEN_SVG`
(the device that renders a game's segment SVG - ~50 files) and (b) every
`SM500/SM510/SM511/SM512/SM530/SM590/SM5A(config, ...)` CPU instantiation, then
read each unfamiliar handheld driver's header/`SYST()` entries.

### Drop-in on cores we already have: `rzone.cpp` (Tiger R-Zone)

The one driver outside `hh_sm510.cpp` that instantiates our SM5xx family (the
only other hit is the NES R.O.B. controller, which isn't a game). Three games,
all clean (no `NOT_WORKING`/`IMPERFECT` flags), with internal SVG art
(`rzone.lh` layout, SVGs ~1368x1080 to 1425x1080):

| Romname | Title | Year | CPU |
|---|---|---|---|
| `rzbatfor` | Batman Forever | 1995 | SM512 (+ melody ROM) |
| `rztoshden` | Battle Arena Toshinden | 1996 | SM510 |
| `rzindy500` | Indy 500 | 1996 | SM510 |

Real R-Zone had far more titles - MAME only has these three dumped. Caveats:

- **Input**: the D-pad, four action buttons and power/pause/select go through a
  74165 shift register, so the input layer needs a small wrapper that emulates
  the serial read instead of the usual direct K-line mapping.
- **Look**: the HeadGear model is red segments on black, a different visual
  treatment from the ink-on-LCD games (MAME's own TODO notes a "SuperScreen"
  model would need its SVG colours inverted - not emulated).

Best next step by a wide margin: no new CPU work, same segment pipeline.

**Done (2026-10-23):** all three ported - see `standard_controls.md`'s R-Zone
section. The core gained an optional `SM5XXPortDevice` hook (R/S writes, K
read, speaker) that `source/virtual_i_o/rzone_io.h` implements; MAME's raw
SVGs needed their segments wrapped in a `segs` layer (`wrap_svg_segs.py` ->
`<game>_segs.svg`), and
the games were built with `convert_switch.py --game` and merged into the pack
with `merge_pack_games.py`. Still open: background/console artwork, the LED,
and saving the shift-register state in save states.

### Also SVG-driven, but each needs a new CPU core

Everything else in the search that is a handheld game is already covered in the
first pass above, with one addition:

- **`dbridgec.cpp`** - Diamond Bridge Computer (Micro-Concepts, 1987). A real LCD
  *segment* game (`dbridgec.svg` + `dbridgec.lh`) on a **Hitachi HD44860**, the
  LCD-driver member of the same 4-bit MCU family MAME's `hmcs40` core emulates
  for `hh_hmcs40.cpp` - so it would come along nearly for free once that core
  exists. One game, no sound, a comms port for multiplayer that we'd skip.

Updated priority: **`rzone.cpp`** (existing cores) -> **`hh_hmcs40.cpp`**
(+ `dbridgec`) -> **`hh_ucom4.cpp`** -> **`hh_e0c6200.cpp`** -> the rest as
listed above.

### Uses the SVG device, but a poor fit for this project

- **Electronic chess computers** (~35 files: Saitek, Novag, Excalibur, Newcrest,
  Hegener+Glaser, Mattel `chess.cpp`, Chessking `pmicrodx`, `tryom/omar`). Real
  LCD/LED segment displays, but every one is built around a physical board with
  piece sensors (we'd need a virtual chessboard UI), and most run 6502/Z80/H8
  CPUs - a large amount of new core work for a very different product.
- **Music keyboards** (Yamaha `ympsr*`/`mulcd`, Casio `ctk551`/`wk1800`/`cz230s`):
  LCD art via SVG, but not games and driven by heavy sound-synthesis CPUs.
- **`pensebem.cpp`** (TecToy Pense Bem, 2017): an 8x8 LED matrix on an ATmega -
  wrong display type, and `MACHINE_NOT_WORKING`.

### Other `handheld/` drivers checked and ruled out

| Driver | What it is | Why not |
|---|---|---|
| `gw35th.cpp` | Nintendo Game & Watch 35th Anniversary (SMB / Zelda, 2020) | ARM9 + raster LCD; skeleton driver, `NOT_WORKING` |
| `gmaster.cpp`, `epoch/gamepock.cpp` | Hartung Game Master / Epoch Game Pocket Computer | Dot-matrix raster LCD, not segments |
| `mylife.cpp` | Giochi Preziosi "My Life" | Raster LCD, screen output unimplemented |
| `wildfire.cpp` | Parker Brothers Wildfire (1979) | LED-matrix pinball, needs an external overlay |
| `lk3000.cpp` | Lexicon LK-3000 (1979) | 16-segment LED displays on a pocket computer |
