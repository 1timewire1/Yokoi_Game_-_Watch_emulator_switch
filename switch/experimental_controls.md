# Experimental Tiger games — control reference

Per-game button mappings for the titles gated behind the Switch-only
**Experimental Features** toggle (`MANUFACTURER_TIGER_EXPERIMENTAL`, see
[`../CONVERT_ROM/GNW_LIST.md`](../CONVERT_ROM/GNW_LIST.md)'s "Tiger
(Experimental)" section). Each game gets its own hand-authored
`Virtual_Input` subclass in
[`../source/virtual_i_o/virtual_input.h`](../source/virtual_i_o/virtual_input.h);
this file exists purely as a human-readable index of what each subclass
actually wires up, for testing against real hardware.

## How to read this

Every game maps the same seven Switch inputs (per the base in-game control
scheme documented in [`README.md`](README.md#controls) — D-pad = left G&W
controls, A/B/X/Y = right G&W controls, L/+/− = the fixed setup row):

| Switch input | Role |
|---|---|
| D-Pad Up / Left Stick Up | Left-Up |
| D-Pad Down / Left Stick Down | Left-Down |
| D-Pad Left / Left Stick Left | Left-Left |
| D-Pad Right / Left Stick Right | Left-Right |
| X | Right-Up |
| B | Right-Down |
| Y | Right-Left |
| A | Right-Right |
| + (Plus) | Power On / Start |
| − (Minus) | Game B (see per-game function below) |
| L | Time |

A row is only listed per-game if that physical console button actually
exists and is wired in MAME's `INPUT_PORTS` for that romset — several of
these consoles have fewer than 4 directions on the d-pad, or only one
button on the right side instead of two. Where a function isn't a clean,
identifiable action (rare — most of these are printed on the console
itself), the raw MAME control name is used instead.

---

## Battletoads (`tbtoads`)

Console: 4-way d-pad (all directions) + "ATTACK" rocker + "SELECT" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Attack Left |
| A | Attack Right |
| B | Select |
| + | Start |
| − | Score |

## Karnov (`tkarnov`)

Console: 4-way d-pad + "FIRE-BALL"/"BOOMERANG" rocker + "SHIELD" button.
Note: this romset's MAME port bit layout is genuinely different from every
other game in this batch (Up/Right sit on bit 0 instead of the usual
bit 2/bit 1) — confirmed directly against its own `INPUT_PORTS`, not
assumed from another game.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Fire-Ball |
| A | Boomerang |
| B | Shield |
| + | Start |
| − | Max Score |

## Hook (`thook`)

Console: 4-way d-pad ("Sword Up" / "Sword Down-Dodge") + "SWING" rocker +
"ATTACK/PICK" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Sword Up |
| D-Pad Down | Sword Down / Dodge |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Swing Left |
| A | Swing Right |
| B | Attack / Pick |
| + | Start |
| − | Pause |

## Back to the Future (`tbttf`)

Console: 4-way d-pad ("Repair" / "Brake") + 2 stacked buttons
("Accelerate" over "Rescue" — no left/right pair on the right side of this
console).

| Switch input | Function |
|---|---|
| D-Pad Up | Repair |
| D-Pad Down | Brake |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Accelerate |
| B | Rescue |
| + | Start |
| − | Pause |

## Transformers: Generation 2 (`ttransf2`)

Console: 4-way d-pad + "CALL" button + "TRANSFORM"/"ATTACK" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Call |
| Y | Transform |
| A | Attack |
| + | Start |
| − | Pause |

## Vindicators (`tvindictr`)

Console: 4-way-shaped d-pad, but only Up/Left/Right are actually wired on
this romset (no Down) + "FIRE" button + "GUN" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Fire |
| Y | Gun Left |
| A | Gun Right |
| + | Start |
| − | Max Score |

## Jurassic Park (`tjpark`)

Console: 4-way d-pad ("Jump/Climb" / "Forward") + 2 stacked buttons
("Swing/Power" over "Call" — no left/right pair on the right side).

| Switch input | Function |
|---|---|
| D-Pad Up | Jump / Climb |
| D-Pad Down | Forward |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Swing / Power |
| B | Call |
| + | Start |
| − | Pause |

## Skeleton Warriors: The Dark Crusade (`tskelwarr`)

Console: 4-way d-pad + "ATTACK" rocker + "PICK/CALL" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Attack Left |
| A | Attack Right |
| B | Pick / Call |
| + | Start |
| − | Pause |

## Gargoyles: Night Flight (`tgargnf`) / Superman (`tsuperman`)

Same `Virtual_Input` class — these two romsets share bit-identical MAME
ports. Console: left/right-only rocker (no up/down on this romset) + 2
stacked buttons ("Punch" over "Call").

**Chip quirk**: this SM511 variant doesn't use the first scan line (MAME's
own port comments read S2/S3/S4... instead of the usual S1/S2/S3...), so
every mapped input except Start needed its internal `group` value shifted
by +1 versus the naive translation used for every other game here — this
was the bug that made Superman non-functional in the previous batch until
fixed. Purely an internal implementation detail; doesn't change what's
listed below.

| Switch input | Function |
|---|---|
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Punch |
| B | Call |
| + | Start |
| − | Max Score |

---

## Third batch (27 games)

The remaining genuine Tiger Electronics romsets from the candidate list -
this batch also excluded 8 further candidates that turned out to be
**Tronica** (not Tiger) and one Elektronika bootleg, none of which belong
under this manufacturer id; see the note at the very end of this file.

## The Addams Family (`taddams`)

Console: 4-way d-pad + "TAKE" button + "THROW"/"SELECT" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Take |
| X | Throw |
| Y | Select |
| + | Start |
| − | Pause |

## Home Alone (`thalone`)

Console: 4-way d-pad + "TRAP" rocker + "CLIMB" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Trap Right |
| X | Climb |
| Y | Trap Left |
| + | Start |
| − | Pause |

## Home Alone 2: Lost in New York (`thalone2`)

Console: same shape as Home Alone - 4-way d-pad + "TRAP/FLASH" rocker +
"CLIMB/PICK" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Trap/Flash Right |
| X | Climb/Pick |
| Y | Trap/Flash Left |
| + | Start |
| − | Pause |

## X-Men: Project X (`txmenpx`)

Console: 4-way d-pad ("Pick" on Down) + "JUMP" button + "MOVE" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Pick |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Jump |
| B | Move |
| + | Start |
| − | Pause |

## Wayne's World (`twworld`)

Console: 4-way d-pad ("High Five" on Up) + "HOCKEY" rocker + "SHOOT/CASSANDRA" button.

| Switch input | Function |
|---|---|
| D-Pad Up | High Five |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Hockey Right |
| X | Shoot / Cassandra |
| Y | Hockey Left |
| + | Start |
| − | Pause |

## The Nightmare Before Christmas (`tnmarebc`)

Console: 4-way d-pad ("Jump" on Up) + a single big oval button literally
labeled "ACTION" on the console - any of A/B/X/Y triggers it.

| Switch input | Function |
|---|---|
| D-Pad Up | Jump |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A / B / X / Y | Action |
| + | Start |
| − | Pause |

## Operation: Aliens (`topaliens`)

Console: 4-way d-pad + "ATTACK" button + "PICK" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Attack |
| B | Pick |
| + | Start |
| − | Pause |

## The Shadow (`tshadow`)

Console: 4-way d-pad + "SHADOW" button + "KICK"/"PUNCH" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Punch |
| X | Shadow |
| Y | Kick |
| + | Start |
| − | Pause |

## Space Jam (`tsjam`)

Console: 4-way d-pad + "SHOOT/BLOCK" button + "TUNE/STEAL" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Shoot / Block |
| B | Tune / Steal |
| + | Start |
| − | Pause |

## Street Fighter 2010: The Final Fight (`tsf2010`)

Console: 4-way d-pad + "PUNCH" rocker + "KICK" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Punch Right |
| X | Kick |
| Y | Punch Left |
| + | Start |
| − | Select Planet |

## Swamp Thing (`tswampt`)

Console: 4-way d-pad + "PUNCH/FIRE" button + "THROW" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Punch / Fire |
| B | Throw |
| + | Start |
| − | Pause |

## The Incredible Crash Dummies (`tdummies`)

Console: 4-way d-pad ("Brake"/"Crash" on Left/Right) + "ACCELERATE" button
+ "PICK" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Brake |
| D-Pad Right | Crash |
| X | Accelerate |
| B | Pick |
| + | Start |
| − | Pause |

## Robocop 2 (`trobocop2`) / The Rocketeer (`trockteer`)

Same `Virtual_Input` class - these two romsets share bit-identical MAME
ports (`trockteer` is `PORT_INCLUDE(trobocop2)` with pure renames, no
rewiring). Console: 4-way d-pad ("Rescue" on Left, "Pick" on Down) +
"SHOOT" button + "SHOOT" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Pick |
| D-Pad Left | Rescue |
| D-Pad Right | Move Right |
| A | Shoot Right |
| X | Shoot Up |
| Y | Shoot Left |
| + | Start |
| − | Pause |

## Robocop 3 (`trobocop3`)

Console: same shape as Robocop 2 - 4-way d-pad ("Jump/Fly" on Up, "Rescue"
on Left, "Pick" on Down) + "SHOOT" button + "SHOOT" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Jump / Fly |
| D-Pad Down | Pick |
| D-Pad Left | Rescue |
| D-Pad Right | Move Right |
| A | Shoot Right |
| X | Shoot Up |
| Y | Shoot Left |
| + | Start |
| − | Pause |

## Independence Day (`tinday`)

Console: 4-way d-pad ("Shield" on Up, "Alert" on Down) + "FIRE" button +
"VELOCITY" button. Quirk: this romset's Up is wired to line 0 instead of
the usual line 2 (confirmed directly against MAME's own port definitions -
every other button follows the normal convention).

| Switch input | Function |
|---|---|
| D-Pad Up | Shield |
| D-Pad Down | Alert |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Fire |
| B | Velocity |
| + | Start |
| − | Pause |

## The Flash (`tflash`)

Console: "PUNCH/BRAKE"/"RUN" rocker (no Up/Down at all on this console) +
"TIE" button + "PICK" button. Quirk: Left and Right aren't wired to the
same port on this romset (unlike every other game in this file) - confirmed
directly against MAME's own port definitions, not assumed.

| Switch input | Function |
|---|---|
| D-Pad Left | Punch / Brake |
| D-Pad Right | Run |
| X | Tie |
| B | Pick |
| + | Start |
| − | Pause |

## Pit-Fighter (`tpitfight`)

Console: genuinely two independent 4-way joysticks (left fist / right
fist) rather than a movement d-pad plus face buttons - MAME's own port
definitions define separate `JOYSTICKLEFT_*`/`JOYSTICKRIGHT_*` axes, which
map cleanly onto this emulator's existing PART_LEFT/PART_RIGHT split with
no compromise. Both sides labeled "ATTACK" (up) / "PICK" (down) on the
console.

| Switch input | Function |
|---|---|
| D-Pad Up | Left Fist: Attack High |
| D-Pad Down | Left Fist: Pick |
| D-Pad Left | Left Fist: Attack Low |
| D-Pad Right | Left Fist: (unnamed direction) |
| X | Right Fist: Attack High |
| B | Right Fist: Pick |
| Y | Right Fist: (unnamed direction) |
| A | Right Fist: Attack Low |
| + | Start |
| − | Select |

## Apollo 13 (`tapollo13`)

Console: 4-way d-pad + a 2x2 button grid - "THRUSTER LEFT"/"THRUSTER RIGHT"
on top, "PARACHUTE"/"CAPTURE" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Thruster Left |
| A | Thruster Right |
| Y | Parachute |
| B | Capture |
| + | Start |
| − | Pause |

## Batman Forever: Double Dose of Doom (`tbatfor`)

Console: 4-way d-pad + a 2x2 button grid - "GRAPPLING GUN"/"BATARANG" on
top, "HELP"/"THRUSTER" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Grappling Gun |
| A | Batarang |
| Y | Help |
| B | Thruster |
| + | Start |
| − | Pause |

## Dennis the Menace (`tdennis`)

Console: 4-way d-pad ("Brake" on Down) + a 2x2 button grid - "SLINGSHOT
LEFT"/"SLINGSHOT RIGHT" on top, "PICK LEFT"/"PICK RIGHT" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Brake |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Slingshot Left |
| A | Slingshot Right |
| Y | Pick Left |
| B | Pick Right |
| + | Start |
| − | Pause |

## Judge Dredd (`tjdredd`)

Console: 4-way d-pad + a 2x2 button grid - "KICK"/"PUNCH" on top,
"PICK/CALL"/"FIRE" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Kick |
| A | Punch |
| Y | Pick / Call |
| B | Fire |
| + | Start |
| − | Pause |

## 007: GoldenEye (`tgoldeye`)

Console: 4-way d-pad + a 2x2 button grid - "KICK"/"PUNCH" on top,
"Q"/"FIRE" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Kick |
| A | Punch |
| Y | Q |
| B | Fire |
| + | Start |
| − | Pause |

## Kazaam (`tkazaam`)

Console: 4-way d-pad + a 2x2 button grid - "KICK"/"PUNCH FORWARD" on top,
"PICK/RESCUE"/"PUNCH BACK" on bottom.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Kick |
| A | Punch Forward |
| Y | Pick / Rescue |
| B | Punch Back |
| + | Start |
| − | Pause |

## Batman: The Animated Series (`tbatmana`)

Console: "JUMP"/"FAST" rocker (no Left/Right at all on this console) + a
single big oval button labeled "THROW/ATTACK" - any of A/B/X/Y triggers
it. Chip quirk: same "doesn't use the first scan line" issue as Superman
above (every internal group value is the actual S-line minus 1) - already
accounted for, doesn't affect the mapping below.

| Switch input | Function |
|---|---|
| D-Pad Up | Jump |
| D-Pad Down | Fast |
| A / B / X / Y | Throw / Attack |
| + | Start |
| − | Max Score |

## Mighty Max (`tmigmax`)

Console: left/right-only rocker (no up/down on this romset) + "PICK"
button + "CALL" button. Same S2-start chip quirk as Batman: The Animated
Series above.

| Switch input | Function |
|---|---|
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Pick |
| B | Call |
| + | Start |
| − | Max Score |

---

## Earlier batch (for reference)

The first five Experimental games from the previous batch:

## Batman (`tbatman`)

Console: 4-way d-pad, but only Up/Right/Down are wired on this romset (no
Left) + 2 stacked buttons ("Pick" over "Attack").

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Right | Move Right |
| X | Pick |
| B | Attack |
| + | Start |
| − | Max Score |

## Spider-Man (`tspidman`)

Console: 4-way d-pad + "PUNCH" button + "RESCUE"/"KICK" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Punch (Shoot Up) |
| A | Kick (Shoot Right) |
| Y | Rescue (Shoot Left) |
| + | Start |
| − | Status |

## X-Men (`txmen`)

Console: 4-way d-pad + "ATTACK/PICK" rocker + "PUNCH/CLAWS" button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A | Attack / Pick Right |
| Y | Attack / Pick Left |
| B | Punch / Claws |
| + | Start |
| − | Pause |

## Mortal Kombat (`tmkombat`)

Console: 4-way d-pad + two stacked 2-way rockers ("H PUNCH L" over
"H KICK L"), mapped onto the right-side diamond as Up/Right/Down/Left.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Punch High |
| A | Kick High |
| B | Punch Low |
| Y | Kick Low |
| + | Start |
| − | Select |

---

## Status

All 41 Tiger Electronics romsets identified as having usable local
artwork are now implemented (14 from the first two batches + 27 from this
one) — every game listed in `GNW_LIST.md`'s "Tiger (Experimental)" section
has a working `Virtual_Input` class and can be played with a controller.

**Intentionally excluded from this manufacturer id**: 8 further "t"-prefixed
romsets with local artwork (`trclchick`, `trdivadv`, `trsgkeep`,
`trspacadv`, `trspacmis`, `trspider`, `trthuball`, `trtreisl`) turned out to
be manufactured by **Tronica**, not Tiger Electronics, per their own MAME
`SYST()` entries — and one (`taynyoke`) is a Russian **Elektronika**
bootleg of `gnw_octopus`. None of these belong under
`MANUFACTURER_TIGER_EXPERIMENTAL`; they'd need their own manufacturer
bucket (or folding into an existing one) if ever added.

If `get_input_config()` ever returns `nullptr` for a game (e.g. a future
addition without a class written yet), that game still boots and renders
but ignores all input except **+** (Start).
