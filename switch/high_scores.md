# High Scores

The emulator keeps the best score of every game that shows one on its screen,
for each game mode, and shows it in the game list, in the pause menu and,
optionally, over the game itself.

## What the player sees

**Settings > Game List**

| Option | Values | Default | What it does |
|---|---|---|---|
| High Scores | On / Off | On | Master switch. Off hides everything below and stops recording; the app looks exactly as before. |
| Record On Screen | Off / When Beaten / Always | When Beaten | *When Beaten*: a "NEW HIGH SCORE!" banner for 3 s when you beat a record you already had. *Always*: also the current mode's record in a box at the top right for the whole game. |
| Achievement Alerts | During the Game / When the Game Ends / Off | During the Game | When the achievement banner shows (see *Achievements* below). |
| Show Records in List | On / Off | On | The records box beside the game card (one-at-a-time view) and a "High Scores: ..." line in the maker's game list. |
| Clear All Records | action | — | Deletes every game's records (press A twice). |

**Pause menu > Game Session**

- **High Scores** row, only on games that have records: shows them as
  `A 0420 · B 0185` (or `Score 0156` for single-mode games). Press A twice to
  delete this game's records.

The McDonald's / Burger Bard games draw their own score next to the LCD
(`render/lcdonald_hud.cpp`), now in **both modes**: Normal and Endless. The
original LCDonald app only showed it in Endless, but the games count every
catch in both. Each mode keeps its own record (`normal`, `endless`), with the
same settings: *High Scores* off stops recording and hides the "HI" lines;
*Record On Screen* Off hides them, When Beaten shows them only on the start
screen ("HI Normal" and "HI Endless") and Always also during the run ("HI" of
that mode); both of those show the "NEW HIGH SCORE!" banner. The records also
show in the list and the pause menu (`Normal 0035 · Endless 0042`).

### Achievements

Every game with records has six achievements: reaching **25, 50, 100, 200
and 500 points** (x10 on the VFD tabletop games, which score in tens) in any
mode, and **beating your own record** (only once you had one). Each is got
once and kept with the records (`REF.ach`, a bit mask); deleting a game's
high scores deletes them too.

- *During the Game*: an "ACHIEVEMENT UNLOCKED" banner at the bottom for 3 s
  the moment you get one (after the "NEW HIGH SCORE!" banner if both come
  together, never at the same time).
- *When the Game Ends*: no interruptions; the ones you got show one after
  another when the game ends (the clock comes back, you press Game A / Game B
  or the score drops to start a new game; on the McDonald's games, the
  start screen).
- *Off*: no banner.

The pause menu's **Achievements** row lists them (`25 · 50 · 100  (3/6)`), plus
`+2 new` for this session with *When the Game Ends*.

Nothing is recorded while two devices are linked (`core/link_session.h`) or
during fast-forward.

## How it works

The games don't report a score; it is **read off the LCD**, segment by segment.

- **Game & Watch-style games (Sharp SM5xx)** show the score on the same four
  digits as their clock. Each game was calibrated offline: the clock was set
  to known times (`SM5XX::set_time()`) and the segments that lit told which
  segment is which part of each digit. Whatever only the clock lights (AM,
  PM, the colon) marks "the clock is showing", not a score.
- **VFD tabletop games (HMCS40 / MELPS4)** have no clock. Their 7-segment
  digits were found by shape: three horizontal bars stacked over four vertical
  ones, grouped into a number by position.

At run time (`core/score_tracker.cpp`, once a frame):

1. If any clock mark was lit in the last second, the game is in clock mode:
   nothing is read. The colon blinks, hence the one-second hold.
2. Otherwise the four digits are decoded. A frame where a digit isn't a valid
   number (a graphic crossing it, a redraw) is skipped.
3. A value that stays the same for 6 frames (~0.1 s) is offered to
   `high_score_note()` for the current mode. The mode is set by the Game A /
   Game B button the player pressed (`input/yokoi_input_mapping.cpp`); VFD
   games have one mode.
4. Records are written to `yokoi_config/yokoi_highscores.txt` (format
   `REF.mode=score`, modes `a`, `b`, `normal`, `endless`) when they change, at most every
   10 s during a game, when the game ends and when it is unloaded.

## Supported games (69)

**Nintendo Game & Watch (37):** Chef, Donkey Kong, Donkey Kong II, Donkey
Kong Jr. (New Wide Screen, Panorama), Donkey Kong Circus, Egg, Fire (Wide
Screen), Fire Attack, Green House, Helmet (CN-07, CN-17), Life Boat, Lion,
Manhole (Gold, New Wide Screen), Mario Bros., Mario's Bombs Away, Mario's
Cement Factory (New Wide Screen, CM-72, CM-72A), Mario The Juggler, Mickey &
Donald, Mickey Mouse (Wide Screen, Panorama), Octopus, Oil Panic, Parachute,
Pinball, Popeye (Wide Screen, Panorama), Rain Shower, Safe Buster, Snoopy,
Snoopy Tennis, Spitball Sparky, Turtle Bridge.

**Elektronika (17):** Nu, pogodi!, Vesyolyy povar, Ataka asteroidov,
Biathlon, Circus, Hockey, Kosmicheskiy polyot, Kosmicheskiy most,
Kot-rybolov, Kvaka-zadavaka, Morskaja ataka, Nochnye vorishki, Okhota,
Razvedchiki kosmosa, Tayny okeana, Vesyolye futbolisty, Vinni-Pukh.

**Tronica (6):** Clever Chicken, Diver's Adventure, Space Mission, Spider,
Thunder Ball, Space Rescue.

**Nelsonic (2):** Star Fox, Super Mario Bros. 3.

**VFD tabletop, one score (7):** Galaxian 2 (Entex), Pac Man 2 cyan and red
(Entex), Pac-Man rev. 1 and rev. 2 (Coleco), Kingman (Tomy), Frogger (Coleco).

Every one was checked by playing at random in a headless build of the core
and reading the score through a game: it starts at 0, goes up, and the
numbers match the screen. Games that never scored in those runs are included
when their digits calibrated cleanly against the clock.

## Not supported (yet), and why

Of the 60 Nintendo Game & Watch games in the pack, 37 have records; the other
23 are the ones in the first four rows of this table (timer, Zelda, two
players or failed calibration). Pocket Pikachu is also listed under Nintendo but has no
score.

| Games | Reason |
|---|---|
| Bomb Sweeper, Gold Cliff, Crab Grab, Super Mario Bros., Squish | The digits also show a timer or countdown during play. |
| Zelda | It has no score: its digits are the hits the enemy still needs (40 = hit it 40 times). A game is won or lost, with no score. |
| Boxing, Donkey Kong Hockey, Donkey Kong 3 | Two-player games: both scores share the digits. |
| Ball, Flagman, Vermin, Climber, Black Jack, Judge, Fire (Silver), Balloon Fight, Tropical Fish, Bassmate, Altair, Super Mario World (Nelsonic) | The clock calibration failed, or the digits don't decode reliably. |
| Tiger and Konami handhelds (~80) | No clock to calibrate against, and their digits are used for time, energy, lives or stage as often as for score. They need a game-by-game review. |
| Dig Dug, Zackman, Donkey Kong (Coleco), Crazy Kong, Ms. Pac-Man | Unclear readings: graphics over a digit, or values that go down. |
| Invaders, Stargate, Q*Bert, Tron, Burger Time, Pengo, Zaxxon, Turtles and other VFD games | Single digits found but not grouped into a number. |
| Virtual pets, watches, calculators | They have no score. |

A game without layout data has no records and shows nothing.

## Files

| File | Contents |
|---|---|
| `switch/source/core/score_tracker.h/.cpp` | Reading the score, the HUD, the records for the menus. |
| `switch/source/core/score_layouts.inc` | Generated per-game data: four digits × seven segments (as `{col, line, word}` segment ids), the digit pattern table, the clock marks, and extra segments that invalidate a frame. |
| `switch/source/core/high_scores.h/.cpp` | Persistent records (`high_score_get/note/flush`, plus `high_score_clear` / `high_score_clear_all`). |
| `switch/source/core/switch_settings.h/.cpp` | `g_records_enabled` (`records`), `g_record_hud` (`recordhud`), `g_records_in_list` (`recordslist`). |
| `switch/source/core/switch_menu.cpp` | Settings rows, the pause menu row, the list display, the "press A again" confirmation. |
| `switch/source/input/yokoi_input_mapping.cpp` | Game A / Game B button → `score_tracker_set_mode()`. |
| `switch/source/main.cpp` | `score_tracker_update()` after each frame's steps; `score_tracker_render()` over the game. |

### Layout format

```cpp
struct ScoreLayout {
    const char* ref;
    SegId digit[4][7];  // thousands .. units, 7 segments each (255 = none)
    uint8_t table[10];  // bit i = digit[..][i] lit, for 0..9
    SegId clock[4];     // AM / PM / colon: lit = the clock is showing
    SegId extra[4];     // lit = not a number this frame
    uint8_t alt[3];     // alternate 6 / 7 / 9 shapes (shape-found layouts)
    uint8_t modes;      // 2 = Game A + Game B, 1 = one score
};
```

For the clock-calibrated games the seven segments are in the order of the
units digit's own segment list, and `table` holds the patterns actually seen
on the clock. For the shape-found VFD games they are in standard `a..g`
order, with the standard 7-segment table plus `alt`.
