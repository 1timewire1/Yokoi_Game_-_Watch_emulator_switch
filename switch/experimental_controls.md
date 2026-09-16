# Experimental games — control reference

Per-game button mappings for the titles gated behind the Switch-only
**Experimental Features** toggle (`MANUFACTURER_EXPERIMENTAL`, see
[`../CONVERT_ROM/GNW_LIST.md`](../CONVERT_ROM/GNW_LIST.md)'s "Experimental"
section). Originally just the Tiger Electronics catalog, this bucket now
also covers a batch of Konami handhelds plus a few Tronica/Elektronika
stragglers and one non-game novelty device - every title's *original*
manufacturer is still recorded in its display name (in parentheses), this
manufacturer id just controls the Switch-side gating. Each game gets its
own hand-authored `Virtual_Input` subclass in
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

## Fourth batch — Konami classics, Tronica, Elektronika, Bassmate (23 games)

Everything that was left over from the "no artwork yet" Konami/stragglers
audit and turned out to be straightforward button-mapping work rather than
a real driver/hardware blocker. Filed under the same `MANUFACTURER_EXPERIMENTAL`
id as the Tiger games above, with each one's real manufacturer recorded in
its display name instead (e.g. "Double Dribble (Konami)").

## Double Dribble (`kdribble`)

Console: 4-way d-pad + single unlabeled round button (MAME defines no
`PORT_NAME` for it — the console art has no label there either).

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Level Select |

## Contra (`kcontra`)

Console: "LEFT"/"RIGHT" rocker (no up/down on this romset) + single
unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Select |

## Top Gun (`ktopgun`)

Same shape as Double Dribble above. Console: 4-way d-pad + single
unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Select |

## Gradius (`kgradius`)

Console: "UP"/"DOWN" rocker (no left/right on this romset) + single
unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| X | Action |
| + | Start |
| − | Select |

## Teenage Mutant Ninja Turtles (`ktmnt`)

Console: genuinely two joysticks (like Pit-Fighter earlier in this file) —
left = full 4-way d-pad; right = an identical-looking cross button, but
only Left/Right are actually wired on this romset (confirmed via
screenshot).

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Second Stick Left |
| A | Second Stick Right |
| + | Start |
| − | Game Select |

## Skate or Die (`kskatedie`)

Console: 4-way-shaped d-pad, but only Left/Up/Right are wired (no Down) +
"UP"/"DOWN" rocker on the right (split across two MAME ports, still one
physical rocker).

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Right Rocker Up |
| B | Right Rocker Down |
| + | Start |
| − | Continue |

## The Adventures of Bayou Billy (`kbilly`)

Console: full 4-way d-pad + "UP"/"DOWN" rocker.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Rocker Up |
| B | Rocker Down |
| + | Start |
| − | Pause |

## Bottom of the Ninth (`kbottom9`)

Console: full 4-way d-pad + single unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Level / Time |

## The Lone Ranger (`kloneran`)

Console: "LEFT"/"RIGHT" rocker (no up/down) + single unlabeled round
button.

| Switch input | Function |
|---|---|
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Bill Elliott's NASCAR Racing (`knascar`) / Chequered Flag (`kchqflag`) / Top Gun: Second Mission (`ktopgun2`)

Same `Virtual_Input` class — bit-for-bit identical port layouts (Chequered
Flag reuses NASCAR Racing's hardware directly per MAME's own `SYST()`
entry; Top Gun: Second Mission is a confirmed identical-port reskin).
Console: "LEFT"/"RIGHT" rocker + "UP"/"DOWN" rocker — two-stick, no
diagonals on either side.

| Switch input | Function |
|---|---|
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Rocker Up |
| B | Rocker Down |
| + | Start |
| − | Pause |

## Blades of Steel (`kblades`)

Console: full 4-way d-pad + single unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Teenage Mutant Ninja Turtles II: Splinter Speaks (`ktmnt2`)

Console: full 4-way d-pad + single unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## NFL Football (`knfl`)

Same shape as Blades of Steel above. Console: full 4-way d-pad + single
unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Star Trek: 25th Anniversary (`kst25`)

Console: "LEFT"/"RIGHT" rocker with a combined "UP/DOWN" position (MAME's
own port ties Up and Down to the exact same physical line — commented in
MAME's source as electronically the same button) + single unlabeled round
button.

| Switch input | Function |
|---|---|
| D-Pad Up / D-Pad Down | Up/Down (shared line) |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Teenage Mutant Ninja Turtles 3: Shredder's Last Stand (`ktmnt3`)

Console: full 4-way d-pad + single unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Teenage Mutant Ninja Turtles: Basketball (`ktmntbb`)

**Not the same port layout as TMNT 3 above**, despite the similar name and
model line — confirmed directly against MAME's own port definitions after
the game was reported unable to start under an earlier shared class. All
four directions live on a single port here, and Start/Pause live on a
different port/line entirely. Console: full 4-way d-pad + single
unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Bucky O'Hare (`kbucky`)

Console: "UP"/"DOWN" rocker (no left/right on this romset) + single
unlabeled round button (shares a MAME port with Up).

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| X | Action |
| + | Start |
| − | Pause |

## Garfield (`kgarfld`)

Console: full 4-way d-pad + single unlabeled round button.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| X | Action |
| + | Start |
| − | Pause |

## Treasure Island (Tronica) (`trtreisl`)

Console: full 4-way d-pad + single button labeled "Start/Jump/Pick" (any
of A/B/X/Y triggers it). "Game A"/"Game B" map directly by name. A
clock-only "Hour" function shares the same physical line as
Start/Jump/Pick in MAME's port definition but isn't modeled here, same as
every other clock/ACL function in this file. This is the tri-screen game
whose console thumbnail was recently swapped to a real photo of the
physical device.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| A / B / X / Y | Start / Jump / Pick |
| + | Game A |
| − | Game B |

## Vesolaya arifmetika (Elektronika) (`vesarif`)

**Note the button roles here don't follow the usual "+ = Start" pattern**
— this device has separate "Game" and "Start" buttons and both are
mapped, just not to the buttons you'd expect from every other game in this
file. Console: up/down-only rockers on both sides (no left/right at all) —
a two-stick, vertical-only layout.

| Switch input | Function |
|---|---|
| D-Pad Up | Left Stick Up |
| D-Pad Down | Left Stick Down |
| X | Right Stick Up |
| B | Right Stick Down |
| + | Game |
| − | Start |
| L | Time |

## Bassmate Computer (Telko / Nintendo) (`bassmate`)

**Not a game** — a real fishing-computer novelty device (produced for
Telko by Nintendo, also sold as KMV/Probe 2000) with a keypad of 9 distinct
labeled function buttons (Compute, Wind, Cover, Time of Day, Water Temp F,
Water Clarity, Structure, Water Depth(ft), Season) plus Alarm, nothing
resembling a d-pad. There's no physically-meaningful way to preserve this
on the Left/Right diamond layout, so the 8 non-Compute/Alarm buttons are
assigned to the 8 available direction slots in the same order MAME itself
lists their port bits in — see
[`standard_controls.md`](standard_controls.md) for the general convention
this follows. Its printed option labels (SPRG/SUM/FALL/... etc.) are drawn
by a separate always-on overlay layer in the original artwork that this
pipeline didn't composite into the background at first, which is what
caused the reported "solid black squares instead of labeled options" bug —
now fixed by baking that overlay into the background image during
conversion.

| Switch input | Function |
|---|---|
| + | Compute |
| − | Alarm |
| D-Pad Up | Wind |
| D-Pad Right | Cover |
| D-Pad Left | Time of Day |
| D-Pad Down | Water Temp F |
| X | Water Clarity |
| A | Structure |
| Y | Water Depth (ft) |
| B | Season |

---

## Fifth batch — Nelsonic Game Watches, SM530 (3 games)

The first games on the new SM530 core (see `source/SM5XX/SM530/`). Unlike
every other core in this codebase, SM530's K input is a single
non-multiplexed 8-bit register (read via KTA/KETA as two nibbles) instead
of 8 multiplexed 4-bit groups, so every `input_set()` call in these three
classes passes `not_multiplex=true` — bit position is fixed by MAME's own
`IN.0` port layout (`hh_sm510.cpp`'s `gamewatch`/`nstarfox` input ports),
not chosen freely like the multiplexed cores above.

## Star Fox (`nstarfox`)

Console: 4-way d-pad + two unlabeled round buttons (laser / bomb). Unlike
Super Mario Bros. 3 / Super Mario World below, both buttons are wired as
real-time gameplay actions here (MAME's `nstarfox` input port relabels the
same two bits from `mode`/`set` to `laser`/`bomb`), so there's no Game A/B
row for this one.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| Y | Laser |
| A | Bomb |
| + | Start |

## Super Mario Bros. 3 (`nsmb3`) / Super Mario World (`nsmw`)

Console: 4-way d-pad + `mode` / `set` buttons (cycle watch mode, adjust
time) — no other action buttons on either unit.

| Switch input | Function |
|---|---|
| D-Pad Up | Move Up |
| D-Pad Down | Move Down |
| D-Pad Left | Move Left |
| D-Pad Right | Move Right |
| + | Mode |
| − | Set |

---

## Sixth batch — modifier-key calculator keypads (2 games)

`nummunch` and `uchitari` are calculator-style keypads with ~23-26 distinct
physical buttons - roughly double what the usual 12-input layout (D-pad x4,
A/B/X/Y, L/R, +/-) covers. Rather than leave them unported, both use a new
3-layer scheme instead of the normal `Virtual_Input`/`PART_*` path: holding
**ZL** or **ZR** re-purposes the same 12 inputs as a second and third set of
buttons (36 total), selected every frame in `main.cpp` and applied via
`apply_modifier_layer_input()` (see
[`source/input/modifier_layer_input.h`](source/input/modifier_layer_input.h)).
Neither game has a `Virtual_Input` subclass or a `GNW_LIST.md`-driven
control table entry above - they're wired directly by (group, line) in
`main.cpp`'s `kNummunchLayout`/`kUchitariLayout` tables instead, matching
MAME's own `hh_sm510.cpp` input ports 1:1. Not ideal control-wise (some
buttons need two key-presses to reach), but enough to actually play and
test both on real hardware, which was the point - `ZL`+`ZR` held together
is still the existing pause-menu combo, treated as "no modifier" (falls
back to the base layer) if it's ever read here rather than caught by the
pause check first.

Both layers follow the same shape on each game: the base layer (no
modifier) covers primary functions/navigation, holding **ZL** always maps
to digits 0-9 in the same D-pad/face-button/L/R positions, and holding
**ZR** always maps to the four math operators.

The same ZL/ZR trick was later reused *additively* - layered on top of a
game's existing, unmodified controls rather than replacing them - to
expose a real but previously-unmapped calculator keypad on three ordinary
(non-Experimental) library games: `MG_8`/`TG_18`/`SA_12`. Gated behind
Experimental Features specifically because those three already work as
normal games without it; see
[`standard_controls.md`](standard_controls.md#hidden-calculator-keypad-experimental-features-only)
for that mapping and `apply_calculator_modifier_input()` in this same
`modifier_layer_input.h`.

## Electronic Number Muncher (`nummunch`)

Console: 4-way d-pad (Left/Right only - no Up/Down on this device),
On/Off/Calc.Clear/`=`, Game/Count/Quiz mode buttons, four "Choose" icon
buttons (+/-/×/÷), a 0-9 numeric keypad, and a second +/-/×/÷ operator row
on the keypad itself.

| Switch input | Base (no modifier) | Hold ZL | Hold ZR |
|---|---|---|---|
| D-Pad Up | Game | 1 | + |
| D-Pad Down | Quiz | 2 | − |
| D-Pad Left | Left | 3 | × |
| D-Pad Right | Right | 4 | ÷ |
| A | Count | 5 | — |
| B | = | 6 | — |
| X | Calc. / Clear | 7 | — |
| Y | OFF | 8 | — |
| L | Choose + | 9 | — |
| R | Choose − | 0 | — |
| + | Choose × | — | — |
| − | Choose ÷ | — | — |

## Uchitel' arifmetiki (`uchitari`)

Console: no physical unit was ever recovered (see
`CONVERT_ROM/UNIMPLEMENTED_GAMES.md`'s correction on this game's actual
emulation status vs. its provenance) - button layout below is read
straight from MAME's `hh_sm510.cpp` input ports, not a photographed
console. `Minute`/`"0"` and `Hour`/`"1"` are separate physical buttons
wired to the same line on real hardware (per MAME's own `PORT_CONDITION`
setup), same for `Alarm`/`Exam Level` - each pair below intentionally
shares a Switch input rather than that being a mapping mistake.

| Switch input | Base (no modifier) | Hold ZL | Hold ZR |
|---|---|---|---|
| D-Pad Up | Evaluate Entry | 1 | + |
| D-Pad Down | Start Exam | 2 | − |
| D-Pad Left | Minute (shares "0"'s line) | 3 | × |
| D-Pad Right | Hour (shares "1"'s line) | 4 | ÷ |
| A | Mode | 5 | — |
| B | Clear entry / Correction | 6 | — |
| X | Alarm (shares Exam Level's line) | 7 | — |
| Y | Exam Level (shares Alarm's line) | 8 | — |
| L | Evaluate Division Entries | 9 | — |
| R | — | 0 | — |

---

## Status

All 41 Tiger Electronics romsets identified as having usable local
artwork are implemented (14 from the first two batches + 27 from the
third), plus 23 more from the fourth batch above (Konami classics, one
Tronica straggler, one Elektronika straggler, and the Bassmate novelty
device) — 64 games total, matching `GNW_LIST.md`'s "Experimental" section
row count as of the fourth batch. Every game listed there has a working
`Virtual_Input` class and can be played with a controller. Further
additions since then aren't reflected in that count: `naltair` (SM5A,
documented directly in `virtual_input.h` instead of here since it reuses
`MC_25`'s button layout verbatim), the fifth batch above (SM530, 3 games),
and the sixth batch above (2 games, modifier-key scheme instead of
`Virtual_Input`) — `GNW_LIST.md`'s "Experimental" section now has 70 rows
total.

**Intentionally excluded from this manufacturer id**: 8 further "t"-prefixed
romsets with local artwork (`trclchick`, `trdivadv`, `trsgkeep`,
`trspacadv`, `trspacmis`, `trspider`, `trthuball`) turned out to
be manufactured by **Tronica**, not Tiger Electronics, per their own MAME
`SYST()` entries — and one (`taynyoke`) is a Russian **Elektronika**
bootleg of `gnw_octopus`. These don't get their own manufacturer id; they
render under Nintendo/Tronica/Elektronika's existing ids like every other
non-experimental game (`trtreisl` is the one Tronica straggler that *did*
get folded into this batch instead, since its driver needed real
button-mapping work like the rest of this file).

If `get_input_config()` ever returns `nullptr` for a game (e.g. a future
addition without a class written yet), that game still boots and renders
but ignores all input except **+** (Start).
