# Third-party code and licenses

Yokoi bundles code from other projects. Each keeps its own license, listed
here; ship this file with every binary release (the Switch `.nro`, the 3DS
build...), next to the README.

## MAME (BSD-3-Clause)

Ported or adapted from [MAME](https://github.com/mamedev/mame). Each file
names its MAME source and copyright holders at the top.

| Yokoi file(s) | From MAME | Copyright holders |
|---|---|---|
| `source/E0C6200/` | `src/devices/cpu/e0c6200`, `src/devices/machine/e0c6s46` | hap |
| `source/SM5XX/SM530/` | `src/devices/cpu/sm510` (SM530) | hap |
| `source/BrickCores/HMCS40.*` | `src/devices/cpu/hmcs40`, `src/mame/handheld/hh_hmcs40.cpp` | hap, Sean Riddle |
| `source/BrickCores/MELPS4.*` | `src/devices/cpu/melps4`, `src/mame/handheld/hh_melps4.cpp` | hap, Kevin Horton |
| `source/BrickCores/Cop411.*` | `src/devices/cpu/cop400` | Curt Coder |
| `source/BrickCores/mcs48.*`, `AdvVision.*` | `src/devices/cpu/mcs48`, `src/mame/entex/advision.cpp` | Dan Boris, Mirko Buffoni, Aaron Giles, Chris Kirmse, hap |
| `source/BrickCores/sm8500.*`, `sm85ops.h`, `GameCom.*` | `src/devices/cpu/sm8500`, `src/mame/tiger/gamecom*.cpp` | Wilbert Pol, Robbbert |
| `source/BrickCores/upd7810*`, `mame_shim.h`, `GamePocket.*`, `GameMaster.*` | `src/devices/cpu/upd7810`, `src/mame/epoch/gamepock.cpp`, `src/devices/video/hd44102.cpp`, `src/mame/handheld/gmaster.cpp` | Juergen Buchmueller, Wilbert Pol, hap, Curt Coder |

```
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

MAME's layout files (`CONVERT_ROM/hmcs40_layouts/*.lay`, author hap) and its
Microvision software list (`CONVERT_ROM/microvision_carts.json`) are CC0-1.0.

## LCDonald (GPL-3.0)

`source/LCDonald/` ports the game logic of
[LCDonald](https://github.com/Difegue/LCDonald) by Difegue, licensed under the
GNU General Public License v3 (full text: `licenses/GPL-3.0.txt`). Builds that include this code
(the Switch `.nro`) are distributed under GPLv3 as a whole, with their source in this repository. The McDonald's and Burger Bard games' scans and
sounds (built into the game pack, not part of this repository) are (c) SEGA,
2004-2006, as LCDonald notes.

## BrickEmuPy (CC0-1.0)

The cores in `source/BrickCores/` that run BrickEmuPy games, the games'
`.brick` configurations and face art come from
[BrickEmuPy](https://github.com/azya52/BrickEmuPy) by azya52, released under
CC0 1.0 (no conditions).

## tamago

`source/BrickCores/tamago_*` (the Tamagotchi TamaTown Tama-Go / Tamagotchi
Friends machine) is a port of
[nullableVoidPtr/tamago](https://github.com/nullableVoidPtr/tamago), used with
its author's permission.

## stb (public domain / MIT)

`switch/source/third_party/stb_image.h` and `stb_vorbis.h` by Sean Barrett:
public domain, or MIT at your choice (see the end of each file).

## Fonts (SIL Open Font License 1.1)

The menu fonts built into the Switch app (`switch/source/render/switch_fonts.cpp`):
JetBrains Mono, Lora, Outfit, Pixelify Sans, Silkscreen and Tektur. Their
copyright notices and the license text are in `switch/fonts_licenses/`.
