# Microvision (TMS1100 core)

Support for the Milton Bradley Microvision (1979): the cartridge holds the
CPU, a Texas Instruments TMS1100 with its own program and output PLA; the
console has the 16x16 LCD (Hughes HLCD 0488 driver), a 12-key keypad, a
rotary paddle and a piezo.

Status: the core is written and self-tested; no cartridge has been imported
yet. The ROMs are not included.

## Cartridges

From MAME's `microvision` software list (CC0), kept in
`CONVERT_ROM/microvision_carts.json` with each cartridge's clock, output PLA
type, paddle and key mask; the importer identifies a ROM by its sha1.

| Cartridge | Year | Oscillator | PLA | Paddle | Chip |
|---|---|---|---|---|---|
| Block Buster | 1979 | 500 kHz | 0 | yes | MP3450A |
| Connect Four | 1979 | 500 kHz | 1 | yes | MP3481 |
| Bowling | 1979 | 500 kHz | 1 | no | MP3475 |
| Star Trek: Phaser Strike | 1979 | 550 kHz | 0 | no | MP3454 |
| Pinball | 1979 | 550 kHz | 0 | yes | MP3455 |
| Vegas Slots | 1979 | 500 kHz | 1 | no | MP3474 |
| Mindbuster | 1979 | 550 kHz | 0 | no | MP3457 |
| Baseball | 1980 | 500 kHz | 0 | yes | MP3479 |
| Sea Duel | 1980 | 350 kHz | 1 | no | MP3496 |
| Alien Raiders | 1981 | 500 kHz | 1 | yes | M34009 |
| Cosmic Hunter | 1981 | 500 kHz | 1 | no | M34017 |
| Super Blockbuster | 1982 | 500 kHz | 1 | yes | M34047 |

The TMS1100's instruction rate is the oscillator / 6 (about 83 kHz at
500 kHz). The output PLA is one of two types shared by the cartridges, not a
per-cartridge dump. Connect Four also had an Intel 8021 version, and the
first runs of Block Buster and Bowling were 8021s (not dumped); the 8021
isn't supported. The list also has homebrew cartridges (TMS1100: Tetris,
Bomber, Space Invaders; 8021: MicroBird, Tetris).

## The core

`source/BrickCores/TMS1100.h/.cpp`, registered in `BrickCore.cpp` as core
`"TMS1100"`. It is Yokoi's own implementation, written from the chip's
documented behaviour.

- **CPU:** the full TMS1100 instruction set, the 6-bit shift-register
  program counter, 2 chapters x 16 pages x 64 words, one-level subroutine
  with page exchange, status latch, output PLA (O = PLA[status latch, A]),
  R0-R10, K1-K8.
- **LCD:** HLCD 0488 protocol. Latch pulse resets the nibble address; data is
  written while the data clock is low; its rising edge moves to the next
  nibble; latch pulse and data clock both high transfer 4 nibbles of rows
  and 4 of columns. Dots are integrated over 1/60 s frames because the games
  scan the rows.
- **Keypad, paddle, piezo:** keys are a strobe pin to K bits; the paddle is an
  RC charge timed from its strobe pin going high; the piezo is one pin.

## Container (`.brk`)

- Core `TMS1100`; clock = instruction cycles per second (oscillator / 6).
- Blobs: `rom` (2048 bytes, linear: chapter*1024 + page*64 + pc), `opla`
  (32-byte lookup table).
- Options, a pin being `R0`-`R10` or `O0`-`O7`:
  `mv.lcd_lp`, `mv.lcd_dc`, `mv.lcd_data` (first of 4), `mv.speaker`,
  `mv.key.<0-11>=<pin>,<k bits>`, `mv.butmask=<mask>` (keys the cartridge
  leaves out: they never press),
  `mv.paddle=<pin>,<k bits>,<min us>,<max us>`.

The console's wiring (LCD, keypad, paddle, piezo pins) is the same for every
cartridge; it and the two PLA tables are confirmed with the first imported
ROM, by running it and watching which pins carry the LCD protocol.

## To do

- Importer (CONVERT_ROM): ROM + PLA -> `.brk`, wiring per cartridge.
- Controls on the Switch: keypad on the buttons, paddle on the stick/touch.
- Overlay art for the screen and the keypad of each cartridge.

## Tests

`_xfer/mvtest`: `tasm.py` (a small assembler that lays code out in the PC's
own order), `run.cpp` (headless runner) and `tests.py` (22 checks:
arithmetic and status, bits, branches, call/return across pages and
chapters, R/O and the PLA, keypad, paddle timing, the LCD protocol).
