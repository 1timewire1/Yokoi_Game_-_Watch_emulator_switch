# Standard games — control reference

Per-game button mappings for every **fully-supported** game (everything
*except* the post-launch Tiger, Konami, Nelsonic and V-Tech titles — see
[`experimental_controls.md`](experimental_controls.md) for those). Read
straight off each game's `Virtual_Input` subclass in
[`../source/virtual_i_o/virtual_input.h`](../source/virtual_i_o/virtual_input.h),
so it stays accurate to the shipped code rather than being a separately
hand-maintained doc.

Unlike the Experimental titles (mostly licensed action games with clearly
labeled physical buttons - "Punch", "Kick", "Jump"...), the vast majority of
these are classic Nintendo/Tronica/Elektronika Game & Watch units, whose
real hardware has generic, unlabeled directional buttons. What a button
*does* is defined by the game itself, not printed on the case - so for
those, this doc describes the physical **shape** of the control (how many
buttons, which directions) rather than a specific function name. Where a
game's own class comments do name a function (mostly the licensed Tiger
titles below), that name is included.

## How to read this

Every game maps onto the same base scheme (see
[`README.md`](README.md#controls)):

| Switch input | Role |
|---|---|
| D-Pad / Left Stick | Left-side control |
| X / A / B / Y (right diamond) | Right-side control (X=up, A=right, B=down, Y=left) |
| + (Plus) | Power On / Start (Game A) |
| − (Minus) | Game B |
| L | Time |

Each side is one of a handful of physical shapes, used throughout the
tables below:

| Shape | What it means on the real console | How Switch inputs map to it |
|---|---|---|
| **1-button** | A single flat button (no separate directions) | See note below - depends on the *other* side |
| **2-way (Up/Down)** | A 2-position rocker | D-Pad Up/Down (left side) or X/B (right side) |
| **2-way (Left/Right)** | A 2-position rocker | D-Pad Left/Right (left side) or Y/A (right side) |
| **4-way** | A full directional d-pad | D-Pad (left side) or X/A/B/Y (right side) |
| **none** | This side has no control at all | — |

**1-button note**: if *both* sides are 1-button (the majority of the
earliest SM5A-era games), the mapping is: **Left** = D-Pad Up, Down, Left,
or **Y**; **Right** = D-Pad Right, or **A**, **B**, or **X**. If only *one*
side is 1-button, that side instead accepts *any* input from the other
side's full set (any D-Pad direction for a 1-button Left, or any of
A/B/X/Y for a 1-button Right) - see `yokoi_input_mapping.cpp` if you want
the exact logic. Either way, pressing any button in that side's group
does the same single thing.

---

## SM5A era (earliest single-chip games)

Every one of these uses only generic 1-button or 2-way-Up/Down shapes -
none has a distinctly labeled function beyond what's printed here.

| Ref(s) | Game | Left | Right |
|---|---|---|---|
| AC_01 | Ball | 1-button | 1-button |
| FL_02 | Flagman | 2-way (Up/Down) | 2-way (Up/Down) |
| MT_03 | Vermin | 1-button | 1-button |
| RC_04 | Fire (first version) / Kosmicheskiy most | 1-button | 1-button |
| IP_05, IP_15 | Judge | 2-way (Up/Down) | 2-way (Up/Down) |
| MH_06 | Manhole | 2-way (Up/Down) | 2-way (Up/Down) |
| CN_07, CN_17 | Helmet | 1-button | 1-button |
| LN_08 | Lion | 2-way (Up/Down) | 2-way (Up/Down) |
| PR_21 | Parachute | 1-button | 1-button |
| OC_22, IM_03 | Octopus / Tayny okeana | 1-button | 1-button |
| PP_23 | Popeye | 1-button | 1-button |
| FP_24, IM_04 | Chef / Vesyolyy povar | 1-button | 1-button |
| MC_25, EG_26, IM_53, IM_19, IM_11, IM_10, IM_50, IM_32, IM_33, IM_51, IM_49, IM_02, IM_16, IM_13, IM_22 | Mickey Mouse and its many Elektronika reskins (Egg, Ataka asteroidov, Biathlon, Circus, Hockey, Kosmicheskiy polyot, Kot-rybolov, Kvaka-zadavaka, Morskaja ataka, Nochnye vorishki, Nu pogodi!, Okhota, Razvedchiki kosmosa, Vesyolye futbolisty) | 2-way (Up/Down) | 2-way (Up/Down) |
| FR_27, IM_09 | Fire (Wide Screen) / Kosmicheskiy most | 1-button | 1-button |
| SM_11, SG_21 | Space Mission / Spider (Tronica) | 1-button | 1-button |
| SK_10 | Super Goal Keeper (Tronica) | 2-way (Up/Down) | 2-way (Up/Down) |
| IM_23 | Autoslalom (Elektronika) | 2-way (Up/Down) | 2-way (Up/Down) |

---

## SM510 era

All generic shapes again except where noted.

| Ref(s) | Game | Left | Right | Notes |
|---|---|---|---|---|
| TL_28 | Turtle Bridge | 1-button | 1-button | |
| ID_29 | Fire Attack | 2-way (Up/Down) | 2-way (Up/Down) | |
| SP_30 | Snoopy Tennis | 1-button | 2-way (Up/Down) | |
| OP_51 | Oil Panic | 1-button | 1-button | |
| DK_52 | Donkey Kong | 4-way | 1-button | |
| DM_53 | Mickey & Donald | 2-way (Up/Down) | 2-way (Left/Right) | |
| GH_54 | Green House | 4-way | 1-button | |
| JR_55 | Donkey Kong II | 4-way | 1-button | |
| MW_56 | Mario Bros | 2-way (Up/Down) | 2-way (Up/Down) | |
| LP_57 | Rain Shower | 4-way | 1-button | |
| TC_58 | Life Boat | 1-button | 1-button | |
| MG_61 | Squish | 2-way (Up/Down) | 2-way (Left/Right) | |
| DJ_101 | Donkey Kong Jr. (Wide Screen) | 4-way | 1-button | |
| ML_102 | Mario's Cement Factory (Wide Screen) | 2-way (Left/Right) | 1-button | |
| NH_103 | Manhole (Wide Screen) | 2-way (Up/Down) | 2-way (Up/Down) | |
| TF_104 | Tropical Fish | 1-button | 1-button | |
| BU_201 | Spitball Sparky | 2-way (Left/Right) | 1-button | True dual-screen game - **Panel Gap is always off for this one**, regardless of the setting (see below) |
| UD_202 | Crab Grab | 2-way (Left/Right) | 2-way (Up/Down) | True dual-screen game - **Panel Gap is always off for this one** (see below) |
| MG_8, TG_18 | Shuttle Voyage / Thief in Garden (Tronica) | 2-way (Up/Down) | 1-button | + is labeled "Mode" in code rather than "Start"; this game doesn't wire up − (Game B) or L (Time) at all, only + and the two sides. Also has a hidden calculator keypad, gated behind Experimental Features - see [below](#hidden-calculator-keypad-experimental-features-only) |
| DA_37 | Diver's Adventure (Tronica) | 1-button | 2-way (Up/Down) | |
| MG_9, FR_23 | Space Rescue / Thunder Ball (Tronica) | 1-button | 1-button | |

### Clever Chicken (`CC_38V`, Tronica)

A genuine oddball, called out separately rather than forced into the
generic shape table above: only Up/Down are wired on either side (Left/
Right do nothing), and they don't map onto simple directions - each
physical button drives a differently-named game input, and one game input
is fed by *two* different physical buttons at once (matching the real
hardware's wiring, confirmed against MAME's `hh_sm510.cpp`, not a bug).

| Switch input | Function |
|---|---|
| D-Pad Up | Game "Down" input |
| D-Pad Down | Game "Left" input |
| X | Also game "Down" input (same signal as D-Pad Up - either one works) |
| B | Game "Right" input |
| + | Start |
| − | Game B (unlabeled in code) |

---

## SM511/SM512 era

| Ref(s) | Game | Left | Right | Notes |
|---|---|---|---|---|
| PB_59 | Pinball | 1-button | 1-button | |
| BJ_60 | Black Jack | 2-way (Up/Down) | 2-way (Up/Down) | |
| BD_62 | Bomb Sweeper | 4-way | none | |
| JB_63 | Safe Buster | 1-button | 1-button | |
| MV_64 | Gold Cliff | 4-way | 1-button | |
| ZL_65 | Zelda | 4-way | 1-button | |
| CM_72, CM_72A | Mario's Cement Factory (Table Top) | 2-way (Left/Right) | 1-button | |
| SM_91 | Snoopy (Table Top) | 2-way (Left/Right) | 1-button | |
| PG_92 | Popeye (Table Top) | 2-way (Left/Right) | 1-button | |
| CJ_93, IM_12 | Donkey Kong Jr. (Panorama) / Vinni-Pukh | 4-way | 1-button | |
| TB_94 | Mario's Bombs Away (Table Top) | 2-way (Left/Right) | 1-button | |
| DC_95, MK_96 | Donkey Kong Circus / Mickey Mouse (Panorama) | 1-button | 1-button | |
| YM_801, YM_105 | Super Mario Bros | 4-way | 1-button | |
| DR_802, DR_106 | Climber (Ice Climber) | 4-way | 1-button | |
| BF_803, BF_107 | Balloon Fight | 4-way | 1-button | |
| MB_108 | Mario The Juggler | 1-button | 1-button | |
| BX_301, AK_302, HK_303 | Vs. games sharing one class (e.g. Boxing) | 4-way | 1-button | This class also defines a full Player 2 control set internally - a second connected controller now drives it, see [below](#player-2-vs-games-only) |
| SA_12 | Space Adventure (Tronica) | 2-way (Up/Down) | 1-button | Right button is "Fire"; + is labeled "Mode" in code rather than "Start"; − (Game B) and L (Time) aren't wired up at all for this game. Also has a hidden calculator keypad, gated behind Experimental Features - see [below](#hidden-calculator-keypad-experimental-features-only) |

---

### Player 2 (Vs. games only)

`BX_301`/`AK_302`/`HK_303` are the only games in this build with a genuine
second player's worth of input in the emulated hardware itself - real "Vs."
units had two complete control sets, one per side. Fully automatic, no
setting: connect a second controller (most simply via the system's own
HOME-menu overlay) while one of these three is loaded and it's in as
Player 2; disconnect it and Player 2's side just goes back to reading
nothing, same as real hardware with nothing plugged into that port.

| Switch input (2nd controller) | Role |
|---|---|
| D-Pad / Left Stick | Player 2's left-side control (4-way) |
| A / B / X / Y | Player 2's right-side control (1-button - any of the four presses it) |

Player 2's controller has no Time/Game A/Game B of its own - those are
shared, system-level buttons only Player 1's controller reads, same as the
one physical Time/Game A/Game B pair real "Vs." hardware has.

**Confirmed working on hardware**: tested with all three games in Game Mode
B and two controllers connected - the second controller drives only Player
2's character, Player 1's stays independent, and every other (non-Vs.)
game correctly ignores the second controller entirely. See
[`README.md`](README.md#two-player-controller-support) for more on this
feature's origin (ported from D.Smile NX, another MAME-based homebrew
emulator on this same libnx Npad API).

---

### Hidden calculator keypad (Experimental Features only)

Real hardware for all three Tronica games above (`MG_8`/`TG_18`/`SA_12`) has
a full calculator keypad - digits, `+ - × ÷`, `=`, a decimal point, `%`,
`ALM`, and `C` - that was never wired into this codebase's normal
`Virtual_Input` scheme, since none of it fits the usual D-pad/action-button
shape these tables describe. Rather than leave it inaccessible, it's now
layered on top of the normal controls above via the same **hold ZL/hold
ZR** modifier trick built for `nummunch`/`uchitari` (see
[`experimental_controls.md`](experimental_controls.md#sixth-batch--modifier-key-calculator-keypads-2-games)),
just additive here instead of replacing anything - the table above still
works exactly as documented at all times.

This is gated behind the **Experimental Features** setting and does
nothing at all when it's off, unlike everything else in this file: unlike
Tiger/Konami/etc., these three already shipped as ordinary, fully-working
library games, and exposing a keypad that was never part of the original
port changes their input behavior in a way players didn't opt into by
picking an Experimental title - Experimental Features is what that opt-in
looks like here instead. See `apply_calculator_modifier_input()` in
[`source/input/modifier_layer_input.h`](source/input/modifier_layer_input.h)
for the implementation.

| Switch input | Hold ZL | Hold ZR |
|---|---|---|
| D-Pad Up | 1 | . |
| D-Pad Down | 2 | = |
| D-Pad Left | 3 | + |
| D-Pad Right | 4 | − |
| A | 5 | × |
| B | 6 | ÷ |
| X | 7 | ALM |
| Y | 8 | % |
| L | 9 | C |
| R | 0 | — |

---

## David and John (original, non-emulated games)

| Ref(s) | Game | Left | Right |
|---|---|---|---|
| D_AND_J_CLIMBER | David the climber | 2-way (Left/Right) | 1-button |
| D_AND_J_AVIGNON | Vacation in Avignon | 2-way (Left/Right) | 1-button |
| D_AND_J_RYTHME | Stop sleeping David! | 2-way (Left/Right) | 1-button |
| D_AND_J_FABRIC | New Job for John | 2-way (Up/Down) | 1-button |
| D_AND_J_ESQUIVE | Swimming assessment | 2-way (Up/Down) | 2-way (Left/Right) |

`D_AND_J_CLIMBER`, `D_AND_J_AVIGNON`, and `D_AND_J_RYTHME` share one class
(identical controls); Fabric and Esquive each have their own.

---

## Tiger (non-Experimental)

Licensed action games - these have real labeled buttons on the actual
console, so functions are named below same as the Experimental doc.

### Sonic The Hedgehog / Sonic The Hedgehog 2 (`TSONIC`, `TSONIC2`)

Console: 4-way d-pad + single action button.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X / A / B / Y (any) | Action |
| + | Start |
| − | Pause |

### Street Fighter II / Altered Beast / Double Dragon II (`TSFIGHT2`, `TALTBEAST`, `TDDRAGON2`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| A | Kick Right |
| Y | Kick Left |
| X / B | Punch / Special Move |
| + | Start |
| − | Game |

### Double Dragon / Mega Man 3 / Ninja Gaiden / Ninja Gaiden III / Strider (`TDDRAGON`, `TMEGAMAN3`, `TGAIDEN`, `TGAIDEN3`, `TSTRIDER`)

Console: 4-way d-pad + Up/Down pair on the right.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X | Punch / Pick / Action |
| B | Kick / Action |
| + | Start |
| − | Status / Pause |

### Castlevania II - Simon's Quest (`TSIMQUEST`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X | Jump Up |
| A | Jump Forward |
| B / Y | Sword |
| + | Start |
| − | Max Score |

### Double Dragon III - The Rosetta Stone (`TDDRAGON3`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X | Throw |
| A | Punch |
| Y / B | Kick |
| + | Start |
| − | Max Score |

### Super Double Dragon (`TSDDRAGON`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X | Special Technique |
| A | Kick Right |
| Y | Kick Left |
| B | Punch |
| + | Start |
| − | Max Score |

### Golden Axe (`TGOLDNAXE`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X / B | Magic |
| A | Attack Right |
| Y | Attack Left |
| + | Start |
| − | Max Score |

### Gauntlet (`TGAUNT`)

Console: 4-way d-pad + 4-way right diamond.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X | Key |
| A | Attack |
| Y | Bomb |
| B | Pick |
| + | Start |
| − | Max Score |

### MC Hammer - U Can't Touch This (`TMCHAMMER`)

Console: two independent 4-way controls (arm moves / leg moves), rather than the usual "movement + attack" split.

| Switch input | Function |
|---|---|
| D-Pad Up | Arm Up |
| D-Pad Right | Arm Right |
| D-Pad Left | Arm Left |
| D-Pad Down | Arm Down |
| X | Leg Up / Jump |
| A | Leg Footwork Right |
| Y | Leg Footwork Left |
| B | Leg Down |
| + | Start |
| − | Max Score |

### Space Harrier II (`TSHARR2`)

Console: 4-way d-pad + single action button.

| Switch input | Function |
|---|---|
| D-Pad Up/Right/Left/Down | Move |
| X / A / B / Y (any) | Action (unlabeled in code - own comment just says "key") |
| + | Start |
| − | Max Score |

---

### R-Zone: Batman Forever / Battle Arena Toshinden / Indy 500 (`RZBATFOR`, `RZTOSHDEN`, `RZINDY500`)

Tiger's 1995-96 R-Zone cartridge system (MAME's `rzone.cpp`): the same
SM510/SM512 LCD games as the handhelds, but the console's 14 buttons are read
through a 74165 shift register on the cartridge's R/S/K pins instead of a
button matrix, so they use their own input path
(`source/virtual_i_o/rzone_io.h`, `switch/source/input/rzone_input_map.cpp`)
rather than a `Virtual_Input` class. Every R-Zone game has the same controls:

| Switch input | Function |
|---|---|
| D-Pad / left stick | Joystick |
| A / B / X / Y | Buttons 1 / 2 / 3 / 4 (A / B / C / D on the console) |
| + | Start |
| − | Pause |
| L | Select |
| R | Sound (volume) |

Power On / Power Off aren't mapped - the games run from boot without them.
No background or console artwork exists for these yet: they render on the
plain background colour, and the thumbnail is the game's own LCD art (red on
black, after the HeadGear's red-on-black screen). For that look in-game, set
Game Background Color to **Black** and Segment Color to **Red**.

## McDonald's (LCDonald ports)

Twenty games ported from the [LCDonald](https://github.com/Difegue/LCDonald)
project (McDonald's Sonic-branded Happy Meal LCD games, 2003-2005) - logic
hand-simulated in C# there rather than emulated from a MAME ROM dump (no
real MCU exists to dump), ported to this project's `LCDonald_fake_cpu`
(see [`../source/LCDonald/`](../source/LCDonald/)). Their own manufacturer
bucket (`MANUFACTURER_MCDONALDS`) - hidden, like Tiger/Konami/etc., until
the **Experimental Games** setting is on. Controls don't fit this doc's usual
shape/side convention (these are named single-purpose buttons, not a
generic directional G&W layout), so they get their own table instead.

Every game declares its own small set of named inputs
(`LCDonald_program::get_available_inputs()`); the Switch mapping is one
uniform rule covering all twenty rather than a per-game table
(`switch/source/input/lcdonald_input_map.cpp`):

| Declared input | Switch input |
|---|---|
| "Left" / "Right" | D-Pad Left/Right (mirrored by the left stick, same as everywhere else) |
| "Up" / "Down" | D-Pad Up/Down (same mirroring) |
| Anything else (the game's one action button) | **A** |

| Ref | Title | Movement | Action (A) |
|---|---|---|---|
| `sagame` | Sonic Action Game | — | Jump |
| `tskypatrol` | Tails Sky Patrol | Up/Down | — |
| `ksoccer` | Knuckles Soccer | Left/Right | — |
| `sgrinder` | Shadow Grinder | — | Jump |
| `sspeedway` | Sonic's Speedway | Left/Right | — |
| `abanana` | AiAi Banana Catch | Left/Right | — |
| `sxtremeboard` | Sonic Extreme Boarding | Left/Right | — |
| `tsoccer` | Tails Soccer | Left/Right | — |
| `kbaseball` | Knuckles Baseball | Up/Down | — |
| `shockey` | Shadow Hockey | Left/Right | Shoot a puck |
| `arvolleyball` | Amy & Rouge Volleyball | Left/Right | Hit the ball |
| `cflower` | Cream Flower Catch | Left/Right | — |
| `bfishing` | Big's Fishing | Up/Down (rod) | Reel in fish |
| `sskateboard` | Sonic Skateboard | Left/Right | — |
| `tskyadventure` | Tails Sky Adventure | Left/Right | Shoot a projectile |
| `ktreasure` | Knuckles Treasure Hunt | Left/Right | — |
| `sbasketball` | Shadow Basketball | Left/Right | Make a basket |
| `artennis` | Amy & Rouge Tennis | Left/Right | Hit the ball |
| `cflower2` | Cream Flower Catch | Left/Right | — |
| `bgiantegg` | Billy's Giant Egg | Left/Right | — |

**Variant entries** share identical controls with their base game listed
above (same `LCDonald_program` subclass, just different display metadata
and/or art - see `GNW_LIST.md`'s McDonald's section for the full research
behind which entry is primary vs duplicate/clone and why):
- **International duplicates** (`ksoccer_intl`, `sgrinder_intl`,
  `sspeedway_intl`, `sxtremeboard_intl`) and the silver-body
  "Search for the Silver Game" contest duplicate (`shockey_rare`) differ in box art only, reusing their
  base game's segment/background art directly.
- **ESPN25 clones** (`sspeedway_espn`, `tsoccer_espn`, `kbaseball_espn`,
  `sskateboard_espn`, `sbasketball_espn`, `artennis_espn`) ship a genuinely
  different segment SVG under the same element names (different character
  art, same game logic - see `generate_lcdonald_pack.py`'s `ESPN_SEG_PNG_SRC`
  doc comment) plus their own in-unit background print, so they get their
  own independent segment atlas rather than reusing the base game's.
  `kbaseball_espn` is the one exception in the menu's clone/duplicate
  tiering - it released before `kbaseball` itself, so it's the *origin*
  game there, not the clone (same controls either way).
- **Clone-tier base games** (`tsoccer`, `cflower`, `cflower2`,
  `sskateboard`, `sbasketball`, `artennis`) are re-skins of an earlier,
  differently-named McDonald's game (Knuckles Soccer, AiAi Banana Catch,
  Sonic Extreme Boarding, Shadow Hockey, and Amy & Rouge Volleyball
  respectively) - full, independently-ported games with their own row
  above, just categorized as clones for the menu's filter, not a
  cosmetic-only variant of anything on this list.

**Starting a game: Plus = Normal, Minus = Endless.** A McDonald's game
loads onto its all-segments-lit screen and waits, like a Game & Watch before
you pick Game A or Game B; the bottom-right corner shows the two prompts.
Pressing **+** starts the normal game (`start_game(false)`); pressing **-**
starts **Endless mode** (`start_game(true)`), the same game with no levels or
win, which draws a running score in an italic seven-segment counter in the
top-right corner (`render/lcdonald_hud.cpp`) - a plain `000` readout, like
LCDonald's own. The score is the number of `increment_score()` calls the
game makes, exactly as the original counts its `Scored` event. Each game's
game-over/victory routine plays its closing animation and then calls
`LCDonald_program::stop()` (deferred until the animation has finished - see
that function), which lands back on the all-lit screen with the final score
still showing; **+** or **-** there starts a fresh run. Both buttons are
gated on the game being stopped (`is_stopped()`) so a stray press mid-run
can't reset it, and neither is otherwise used during gameplay (these games
aren't in `virtual_input.h`'s `get_input_config()`, so the normal Game A/B
bindings never apply). High scores are **not stored** - a possible later
feature.

**Sound.** LCDonald's sound effects are real `.ogg` files bundled in the
pack (`sound_lcd_*`), decoded with stb_vorbis on first use and played
through their own libnx audrv voices (`switch/source/audio/lcdonald_sound.cpp`),
so overlapping effects mix instead of cutting each other off.

### Burger Bard (LCDonald's white label)

Under its own **BURGER BARD** heading in the menu: LCDonald's `#if BURGER`
re-skin of 16 of the McDonald's games (plus three art-only duplicates -
`platform2`, `treasure2`, `eggcatch2`; the last is password-locked in the
original app and simply unlocked here) as a fictional brand,
"Burger Bard Crystalectrics" - the manufacturer named on its instruction
sheets. Each is the same ported game logic as its McDonald's counterpart, so
**controls are identical** to that game's row in the table above, and the
start flow is the same too (**+** normal, **-** Endless, score in the
top-right corner). All 19 are marked clones (of the McDonald's game they
re-skin, following the same "ultimate origin" chain), so "Hide All Clones"
hides the whole group.

| Ref folder | Title | Movement | Action (A) |
|---|---|---|---|
| `volleyball` | Curly & Frenchie's Volleyball | Left/Right | Hit the ball |
| `fishing` | Burger Bard Fishing | Up/Down (rod) | Reel in fish |
| `eggcatch` | Halmon's Egg Catch | Left/Right | — |
| `heartmooncatch` | Honey's Heart-Moon Catch | Left/Right | — |
| `football` | King Beareon's Football Catch | Up/Down | — |
| `treasure` | Bard's Burger Hunt | Left/Right | — |
| `treasure2` | Halmon's Egg Hunt | Left/Right | — |
| `platform` | Kola Bear's Platform Adventure | — | Jump |
| `platform2` | Burger Bard's Platform Adventure | — | Jump |
| `wbasketball` | Gloomy Wolf Basketball | Left/Right | Make a basket |
| `railgrinder` | Vibes Rail Grinder | — | Jump |
| `magicbattle` | Bat Magic's Magic Battle | Left/Right | Shoot a puck |
| `skateboard` | Vibes Skateboarding | Left/Right | — |
| `speedway` | Burger Bard Speedway Racing | Left/Right | — |
| `extremeboard` | Vibes Extreme Skateboarding | Left/Right | — |
| `targetshoot` | Taco Sal's Target Shooting | Left/Right | Shoot a projectile |
| `seashell` | Soakey's Shell Catch | Up/Down | — |
| `kosoccer` | Kola Bear's Soccer | Left/Right | — |
| `eggcatch2` | Barry's Treasure Hunt | Left/Right | — |

Burger-only behaviour, from LCDonald's own `#if BURGER` code, is switched on
by `LCDonald_program::set_burger()`: a "WINNER" image in victory animations;
Volleyball's hands group (and the bat staying out until the next update);
Extreme Boarding moving the hang-position fix into the main cycle;
Kola Bear's Soccer's longer miss blink; Egg Catch's extra "common" group and
egg blinks; and a rare hidden flash of the `score-1` group (a 1-in-9 chance
per update, once per game, for about 50ms) - a tiny easter egg. See
`GNW_LIST.md`'s Burger Bard section for the research notes.

## Games always exempt from the Panel Gap setting

Two games in the tables above (`BU_201` Spitball Sparky, `UD_202` Crab
Grab) are internally represented as two "screens" purely so their tall
portrait art fits this codebase's shared dual-screen layout pipeline - not
because the real device has two physically separate screens. The Panel
Gap setting (see the main `README.md`) is hardcoded to never apply to
these two, along with three David & John games with the same situation
(`D_AND_J_CLIMBER`, `D_AND_J_RYTHME`, `D_AND_J_FABRIC`), so they always
render as one continuous display regardless of what Panel Gap is set to.
