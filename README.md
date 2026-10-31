# Yokoi - Game & Watch Emulator for 3DS, Android & Switch

A Game & Watch emulator for 3DS, Android & Nintendo Switch: SM5A, SM510, and SM511/SM512.

# Supported devices

- **Nintendo 3DS/2DS family** (Old/New models) running Homebrew/CFW.
- **Anbernic RGDS (dual screen, Android)**: supported and recommended. 
- **Android (arm64-v8a)** devices (phones/tablets/handhelds).
- **Nintendo Switch** (homebrew) - see [switch/README.md](switch/README.md) for the full write-up.

# Supported games

Supported games are all of the Game & Watch titles, some Tronica, some Elektronika and some Tiger games.

- Full list: [CONVERT_ROM/GNW_LIST.md](/CONVERT_ROM/GNW_LIST.md)
- Compatible Game & Watch models are those equipped with Sharp processors SM5A, SM510, SM511, SM512
	- Refers to Game & Watch models released by Nintendo between 1980 and 1991
- Not compatible with Game & Watch Mini (SM530 processor)
- Notes:
	- Availability depends on which ROMs/artwork you provide when generating your rompack.
	- Some games may still have quirks (see [Known issues](#known-issues)).

# Features (Switch)

- Game/manufacturer select menu, sortable by manufacturer, name, or release date
- Save states, with configurable load behavior (Ask / Auto-Load / Never Load) and a game list that remembers what you were last playing
- Adjustable screen rotation (0° / 90° / 270°), applied to both gameplay and the menu
- Adjustable background color and LCD segment shadow
- Optional console art display for single-screen games
- Left/right analog stick support (left stick mirrors the D-Pad everywhere, right stick mirrors A/B/X/Y in-game)
- Optional experimental support for Tiger, Konami, Nelsonic, V-Tech, David & John, McDonald's and Burger Bard games, plus the Seiko Epson E0C6S46 keychain games (Tamagotchi, Digital Monster, Alien Fever, Venus Diet Monogatari, Stack Challenge) on the Switch, and games on cores ported from [BrickEmuPy](https://github.com/azya52/BrickEmuPy) (so far the Holtek HT943 - brick games, Bandai's Mame Game Galaxian and Tamagotchi, Compile's Puyolin - the Sunplus SPL81408: McDonald's Chicken McNugget Tetris; the OKI MSM50XX: Nelsonic's Zelda and Super Mario Bros. Game Watches and Hartung's Formel 1; the GeneralPlus GPL191X: Bandai's D-Power Digivice and D-Tector; and the Mitsubishi M37520: Nintendo's Pocket Pikachu - the last three cores ported by angel_hp)

Full controls, settings, and technical details: [switch/README.md](switch/README.md)

# Installation

Installation requires both the application and a rompack with the games. 

**Note:** The rompack is **not provided** as it contains original Game & Watch ROMs and graphics. You will need to build your own. Details are in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)

## 3DS

The emulator is provided as `.cia` and `.3dsx` files for 3DS.

### Install (CIA)

Prerequisites:

- A 3DS with CFW (e.g. Luma3DS) so you can install CIAs.
- A title installer such as **FBI**.

Steps:

1. Copy `Yokoi*.cia` to your SD card (any folder).
2. Launch **FBI** on the 3DS.
3. Go to **SD** -> find the `.cia` -> choose **Install and delete CIA** (or Install).
4. Launch **Yokoi** from the HOME Menu.

### Install (3DSX)

Prerequisites:

- The Homebrew Launcher.

Steps:

1. Copy `Yokoi*.3dsx` (or the release `.3dsx`) to `sdmc:/3ds/yokoi/`.
2. Launch the Homebrew Launcher and start **Yokoi**.

### ROM pack 

The rompack is **not provided**, you will need to generate your own from MAME roms.

1. Generate a 3DS rompack as detailed in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
2. Copy your generated pack file to:
	- `sdmc:/3ds/yokoi_pack_3ds.ykp`

If the emulator starts but shows no games or reports a missing pack, double-check the filename and location exactly match the path above.

## Android

The emulator is provided as an `.apk` in the latest releases section.

### Install (APK)

1. Download the latest `Yokoi*.apk` from the releases.
2. Enable installing from unknown sources (wording depends on Android version):
	- Settings -> Security/Privacy -> Install unknown apps -> allow your browser/file manager.
3. Open the APK and install it.
4. Launch **Yokoi**.

### ROM pack (Android)

The rompack is **not provided**, you will need to generate your own from MAME roms.

1. Generate an Android rompack as detailed in the [CONVERT_ROM/README.md](/CONVERT_ROM/README.md)
2. Copy the generated rompack `yokoi_pack_rgds.ykp` somewhere accessible (e.g. `Download/`).
3. Launch the app and use **Import/Update ROM pack** option shown to select the file.

## Switch

The emulator is provided as a `.nro` homebrew file for Switch. Full install steps, controls, settings, and save-state behavior are covered in [switch/README.md](switch/README.md) - the short version:

1. Copy `yokoi_switch.nro` to `sdmc:/switch/yokoi/`.
2. Generate a rompack (same `yokoi_pack_rgds.ykp` format Android uses) as detailed in [CONVERT_ROM/README.md](/CONVERT_ROM/README.md), and copy it to `sdmc:/switch/yokoi/yokoi_pack_rgds.ykp`.
3. Launch **Yokoi** from the Homebrew Launcher.

# Building

See [BUILDING.md](BUILDING.md) for how to build the 3DS and Android applications, or [switch/README.md](switch/README.md) for the Switch build.

For ROM pack / asset generation, see [CONVERT_ROM/README.md](/CONVERT_ROM/README.md).

# Known issues
- Very imperfect sound / strange resonance on Game & Watch SM510 (e.g., Donkey Kong JR Widescreen)
- Bug on some Game & Watch SM5A (sometimes freezes at the end of a game)


# License
Public Domain / Free to use  
No attribution required :)

# Credits
Original Project Code by RetroValou and aminch
Code inspired by MAME, Game & Watch FPGA projects (Adam Gastineau), and official Sharp documentation.  
ROMs and artwork are based on MAME ROMs.  
Thanks to everyone who contributed to MAME, Adam Gastineau, and all those who worked on Game & Watch ROMs and artwork!
