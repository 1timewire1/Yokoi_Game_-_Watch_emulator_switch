# Standard games — control reference

Per-game button mappings for every **fully-supported** game (everything
*except* the Experimental Tiger titles — see
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
| MG_8, TG_18 | Shuttle Voyage / Thief in Garden (Tronica) | 2-way (Up/Down) | 1-button | + is labeled "Mode" in code rather than "Start"; this game doesn't wire up − (Game B) or L (Time) at all, only + and the two sides |
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
| BX_301, AK_302, HK_303 | Vs. games sharing one class (e.g. Boxing) | 4-way | 1-button | This class also defines a full Player 2 control set internally, but the Switch build only exposes one controller's worth of input - Player 2's side isn't reachable in this port |
| SA_12 | Space Adventure (Tronica) | 2-way (Up/Down) | 1-button | Right button is "Fire"; + is labeled "Mode" in code rather than "Start"; − (Game B) and L (Time) aren't wired up at all for this game |

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

## Games always exempt from the Panel Gap setting

Two games in the tables above (`BU_201` Spitball Sparky, `UD_202` Crab
Grab) are internally represented as two "screens" purely so their tall
portrait art fits this codebase's shared dual-screen layout pipeline - not
because the real device has two physically separate screens. The Panel
Gap setting (see the main `README.md`) is hardcoded to never apply to
these two, along with three David & John games with the same situation
(`D_AND_J_CLIMBER`, `D_AND_J_RYTHME`, `D_AND_J_FABRIC`), so they always
render as one continuous display regardless of what Panel Gap is set to.
