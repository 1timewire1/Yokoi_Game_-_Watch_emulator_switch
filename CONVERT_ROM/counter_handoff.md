# BrickEmuPy curation: counter-handoff

Reply to the BrickEmuPy curation handoff, covering all 70 `.brick` configs: the 49 in `brick_games.json` and the 21 already-packed E0C6200 games. Each config gets the eight requested categories (name, manufacturer, date, region/variant, model no., clone of, key overrides, notes/sources), each with a confidence level.

**Companion file:** `brick_games.json`. It is your seed file with `name`, `manufacturer`, `date`, `model`, `clone_of`, `keys`, `notes`, `sources` and a per-field `confidence` object filled in for all 49 entries; the existing keys (`brick`, `set`, `core`, `readme`) are unchanged. An extra top-level `e0c6200_packed` array holds the same fields for the 21 packed games, keyed by `brick` plus `pack_id`. Delete that array if the importer is strict about top-level keys.

## How this was researched

- **First pass:** seven batches of 10 configs, in handoff order. Sources:
  - BrickEmuPy (README, `.brick` configs, git history). The face SVGs were rendered to PNG so the text printed on each unit could be read; most SVGs have their text converted to outlines.
  - MAME's handheld drivers, with ROM SHA1 comparisons where MAME has the same dump.
  - Collector databases: handheldmuseum.com, handheldempire.com, electronicplastic.com.
  - Japanese and English Wikipedia, wikimon.net, the Tamagotchi wiki.
  - Catalogue scans (radioshackcatalogs.com), McDonald's China press material, habr.com.
  - Auction and listing photos: Yahoo! Auctions, Mercari, eBay.
- **Second pass:** the shared web-search budget ran out during the first pass. Three gap-fill agents then re-researched about 30 configs whose name, maker, model or date had been left Low or Unknown.
- Only URLs actually fetched or seen in search results are cited. Some sites blocked fetching (Cloudflare or 403 errors); facts taken from a search-result snippet are marked "snippet" in the per-game text.

### Confidence levels

- **High:** primary evidence (unit or box photo, manufacturer catalogue, text printed on the face, matching MAME ROM hash) or two or more independent reliable sources.
- **Medium:** one good source, or strong consistent secondary evidence.
- **Low:** an inference, one weak source, or a reasoned guess (the reason is given).
- **Unknown:** nothing found.

Dates follow your convention: `YYYY-01-01` when only the year is known, `YYYY-MM-01` when only the month is known.

## Corrections to the original handoff

These are generator bugs and errors in the README lines; the facts were checked against upstream.

1. **`SMBNelsonic` README line is wrong.** It reads "Tamagotchi Angel". Upstream's entry is "Super Mario Bros. Game Watch".
2. **`GA888` README line is wrong.** It repeats the E-88's line. Upstream's entry is "Block Game & Echo Key GA888 (Tetris Jr. clone)".
3. **`ApolloPrince&Princess_ch` / `_en` README line is wrong.** It reads "Apollo 2 in 1 virtual pet", which is `Apollo2in1`'s line. The game is **Prince & Princess**.
4. **`DigimonV1` README line is wrong.** It says "1JA", but this legacy config is the **JB** dump. Upstream commit `c76422b` (2026-03-24) renamed `DigimonV1`→`DigimonV1JB`, `DigimonV2`→`DigimonV2J` and `DigimonV3`→`DigimonV3J`. `DigimonV1`, `DigimonV2` and `DigimonV3` no longer exist upstream.
5. **`Keychain55in1` is really "Pikorin 55" by Gametech.** The Pikorin logo is on the face, and the ROM's SHA1 matches MAME `piko55`.
6. **`nikko` (packed as `VENUSDM`) is Nikko's *Beans Collection: Venus Diet Monogatari* (1997).** Its ROM is byte-identical to MAME `venusdm` (SHA1 `22f6a2ed…`), which is where the pack ID came from.
7. **`MameTamagotch` and `MameGalaxian` are Bandai**, from its 1997 "Mame Game" (マメゲーム) keychain line. They are not unbranded Holtek keychains.
8. **README dates that are wrong:**
   - `GundamGaidenII` is 1990 (box: "BANDAI 1990"), not 1985. The 1985 is a reused case mould.
   - `PocketBoyNo2` is probably 1990, not 1993 (Low).
   - `GakkenSoccer` is probably 1982, not 1981 (Low).
   - For `MiniClassicsSoccer`, "1998/2004" most likely means the year the series licence started (1998) plus this title's own year (2004).
9. **Upstream file problems to handle at import:**
   - `Apollo2in1.brick` points to `Apollo2in1.bin`, but the file is `apollo2in1.bin` (breaks on case-sensitive filesystems).
   - `GundamGaidenII` and `MiniClassicsSoccer` still reference `./assets/test/` paths.
   - `GundamGaidenII` also declares a link peripheral, so the config looks unfinished.
10. **Bigger name changes for the packed E0C6200 games:**
    - `DIGIMONES` "Digital Monster (Spanish)" is most likely the **European English** Ver.1. No Spanish-language unit was found.
    - `TAMAG2`'s real Japanese title is 新種発見!!たまごっち (*Shinshu Hakken!! Tamagotchi*).
    - `TAMAGENJIN` is 原人っちのたまごっち (*Genjintchi no Tamagotchi*), a tie-in to the film *Peking Man*.

**Date confidence across all 70 configs:** High 29, Medium 15, Low 26, Unknown 0.

## Summary: proposed values for every config

Order follows the original handoff. `conf` = confidence of that field. Key overrides are listed in full in the per-game sections and in `brick_games.json`.


### Toshiba 4-bit

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `AnimestSpaceCobra` | Space Cobra Professional (High) | Popy (High) | 1982-01-01 (High) | CO-02 (High) | – | override |
| `DorayakiHouse` | Dorayaki House (High) | Popy (High) | 1983-01-01 (High) | DO-01 (High) | – | override |
| `DrSlumpAraleNchaBycha` | Dr. Slump Arale: Ncha! Bycha (High) | Popy (High) | 1982-01-01 (High) | AR-03 (High) | – | auto |
| `GakkenSoccer` | Soccer (High) | Gakken (High) | 1982-01-01 (Low) | – (Unknown) | – | auto |
| `Gundam3in1` | Gundam Space Combat 3 in 1 (High) | Bandai (High) | 1981-10-01 (Medium) | 16194 (Medium) | – | override |
| `GundamGaidenII` | SD Gundam Gaiden II: Taose! Psycho Golem (High) | Bandai (High) | 1990-01-01 (High) | 29908 (Medium) | – | override |
| `IsogeDoraemon` | Isoge! Doraemon (High) | Popy (High) | 1983-01-01 (High) | DO-02 (High) | – | auto |
| `JumpingBoy` | Jumping Boy (High) | Gakken (High) | 1982-01-01 (Medium) | – (Unknown) | – | override |
| `ParmanDaiPinch` | Parman Dai-Pinch (High) | Popy (High) | 1983-01-01 (High) | PA-01 (High) | – | auto |
| `Pengo` | Pengo (High) | Bandai (High) | 1983-01-01 (Medium) | 0200011 (High) | – | override |
| `PowerFishing` | Power Fishing (High) | Bandai (High) | 1984-01-01 (High) | – (Unknown) | – | override |
| `SpicaDArtagnan` | D'Artagnan (High) | Spica (High) | 1985-01-01 (Low) | SH-002B (Medium) | – | override |
| `TomJerryPrank` | Tom & Jerry Prank (High) | Gakken (High) | 1983-01-01 (High) | 81751 (Medium) | – | override |

### 6502 family (Sunplus)

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `Apollo126in1B0202` | 126 in 1 (B0202) (Medium) | Apollo (High) | 1995-01-01 (Medium) | B0202 (High) | – | auto |
| `Apollo18in1B0302` | 18 in 1 (B0302) (Medium) | Apollo (High) | 1995-01-01 (Medium) | B0302 (High) | – | auto |
| `Apollo2in1` | 2 in 1 (Medium) | Apollo (Medium) | 1997-01-01 (Low) | – (Unknown) | – | override |
| `ApolloPrince&Princess_ch` | Prince & Princess (Chinese) (Medium) | Apollo (Medium) | 1997-01-01 (Low) | – (Unknown) | – | override |
| `ApolloPrince&Princess_en` | Prince & Princess (English) (Medium) | Apollo (Medium) | 1997-01-01 (Low) | – (Unknown) | – | override |
| `BigCat9in1` | Big Cat 9 in 1 (Medium) | Unbranded (Low) | 1998-01-01 (Low) | – (Unknown) | Gyaoppi9in1 | auto |
| `ElfinTwins` | Elfin Twins (High) | Sparkle Crystal (Medium) | 1997-01-01 (Low) | GM-021 (High) | – | override |
| `Gyaoppi9in1` | Super Gyaoppi 9 in 1 (High) | Nature Land (High) | 1997-01-01 (Low) | – (Unknown) | – | override |
| `Tamagotchi_v3_eu` | Tamagotchi Connexion V3 (Europe) (High) | Bandai (High) | 2006-04-01 (Medium) | GPZ06331 (Low) | Tamagotchi_v3_us | auto |
| `Tamagotchi_v3_us` | Tamagotchi Connection V3 (US) (High) | Bandai (High) | 2006-02-01 (Medium) | Asst. #19400 (Medium) | – | auto |
| `TetrisNugget` | Chicken McNugget Tetris (Low) | McDonald's (High) | 2023-05-24 (High) | – (Unknown) | – | override |

### Holtek 4-bit

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `E23PlusMarkII96in1` | Brick Game 96 in 1 (E-23 Plus Mark II) (High) | Brick Game (High) | 1993-01-01 (Low) | E-23 Plus Mark II (High) | – | auto |
| `E88_8in1` | Brick Game 8 in 1 (E-88) (High) | Brick Game (Medium) | 1994-01-01 (Low) | E-88 (High) | – | auto |
| `GA888` | Block Game & Echo Key (High) | Brick Game (High) | 1996-01-01 (Low) | GA888 (High) | – | auto |
| `Keychain55in1` | Pikorin 55 (High) | Gametech (High) | 1996-01-01 (Low) | – (Unknown) | – | auto |
| `KeychainPinBall` | Keychain Pin Ball (High) | Brick Game (Unknown) | 1996-01-01 (Low) | – (Unknown) | – | auto |
| `MameGalaxian` | Galaxian (High) | Bandai (High) | 1997-01-01 (Medium) | – (Unknown) | – | auto |
| `MameTamagotch` | Mame Game de Hakken!! Tamagotchi (High) | Bandai (High) | 1997-01-01 (High) | – (Unknown) | – | auto |
| `MickeyVGS` | Mickey Deluxe Virtual Game (Low) | VGS (Low) | 1997-01-01 (Low) | – (Unknown) | – | override |
| `Puyolin` | Puyolin (High) | Compile (High) | 1997-07-25 (Medium) | – (Unknown) | – | override |
| `SpaceIntruderTK150I` | Space Intruder (High) | Brick Game (Low) | 1998-01-01 (Low) | TK-150I (Medium) | – | auto |

### Samsung 4-bit

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `GA878` | GA-878 (Medium) | Brick Game (Medium) | 2000-01-01 (Low) | GA-878 (High) | – | auto |
| `KotoKunekunetchyo` | Kunekunetchyo (High) | Hiro (High) | 1997-09-01 (Medium) | LCD Keychain Series No.4 (High) | – | auto |
| `MiconKC32` | Micon KC-32 (High) | Brick Game (Medium) | 2000-01-01 (Low) | KC-32 (High) | – | override |
| `PocketPuppy` | Pocket Puppy (High) | Cool Tec (Low) | 1997-01-01 (Low) | – (Unknown) | – | auto |

### Oki 4-bit

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `formel1` | Formel 1 (High) | Hartung (Medium) | 1990-01-01 (Low) | WG 032 (Low) | – | auto |
| `SMBNelsonic` | Super Mario Bros. Game Watch (High) | Nelsonic (High) | 1989-01-01 (High) | – (Unknown) | – | auto |
| `ZeldaNelsonic` | The Legend of Zelda Game Watch (High) | Nelsonic (High) | 1989-10-05 (Medium) | 8321 (Medium) | – | override |

### Sanyo LC57

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `EdlaSpaceRescue` | Space Rescue (Low) | Edla (High) | 1989-01-01 (Low) | – (Unknown) | – | auto |
| `PocketBoyNo2` | Dragon Makyuu no Nazo (High) | Hiro (High) | 1990-01-01 (Low) | Pocket Boy No.2 (Medium) | – | auto |

### 6502 family (Sunplus, runs on GPL191X)

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `DPowerDigiviceEU101` | D-Power Digivice (Europe) (High) | Bandai (High) | 2001-01-01 (Low) | – (Unknown) | – | auto |

### EMC 4-bit

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `E33_2in1` | Brick Game E-33 2 in 1 (High) | Brick Game (Medium) | 1994-01-01 (Low) | E-33 (High) | – | auto |

### NEC uPD750x

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `EpochMonsterPanic` | Monster Panic (High) | Epoch (High) | 1981-01-01 (Medium) | 36520 (Low) | – | auto |

### 6502 family (Sitronix?)

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `MiniClassicsSoccer` | Soccer (Medium) | Stadlbauer (Medium) | 2004-01-01 (Low) | – (Unknown) | – | override |

### 6502 family (Mitsubishi 740)

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `PocketPikachu` | Pocket Pikachu (High) | Nintendo (High) | 1998-03-27 (High) | MPG-001 (Medium) | – | auto |

### 6502 family (Generalplus)

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `TamagotchiMini2017` | Tamagotchi Mini (20th Anniversary) (High) | Bandai (High) | 2017-04-08 (High) | – (Unknown) | – | auto |

### Seiko Epson E0C6200

| `.brick` | Name (conf) | Group (conf) | Date (conf) | Model (conf) | Clone of | Keys |
|---|---|---|---|---|---|---|
| `AlienFever` | Chibi Pachi: Alien Fever (High) | Epoch (High) | 1997-01-01 (Medium) | – (Unknown) | – | auto |
| `DigimonV1` | Digital Monster Ver.1 (Japan, Rev. B) (High) | Bandai (High) | 1997-06-26 (High) | – (Unknown) | DigimonV1JB | auto |
| `DigimonV1ES` | Digital Monster Ver.1 (Europe) (Medium) | Bandai (High) | 1998-01-01 (Low) | – (Unknown) | DigimonV1JA | auto |
| `DigimonV1JA` | Digital Monster Ver.1 (Japan, Rev. A) (Medium) | Bandai (High) | 1997-06-26 (High) | – (Unknown) | – | auto |
| `DigimonV1JB` | Digital Monster Ver.1 (Japan, Rev. B) (Medium) | Bandai (High) | 1997-06-26 (High) | – (Unknown) | DigimonV1JA | auto |
| `DigimonV2` | Digital Monster Ver.2 (High) | Bandai (High) | 1997-12-01 (High) | – (Unknown) | DigimonV2J | auto |
| `DigimonV2J` | Digital Monster Ver.2 (High) | Bandai (High) | 1997-12-01 (High) | – (Unknown) | – | auto |
| `DigimonV3` | Digital Monster Ver.3 (High) | Bandai (High) | 1998-03-01 (High) | – (Unknown) | DigimonV3J | auto |
| `DigimonV3J` | Digital Monster Ver.3 (High) | Bandai (High) | 1998-03-01 (High) | – (Unknown) | – | auto |
| `DigimonV4J` | Digital Monster Ver.4 (High) | Bandai (High) | 1998-05-01 (High) | – (Unknown) | – | auto |
| `nikko` | Venus Diet Monogatari (High) | Nikko (High) | 1997-01-01 (Medium) | – (Unknown) | – | auto |
| `RadioShackStackChallenge` | Stack Challenge (High) | Tandy (High) | 1991-01-01 (High) | 60-2247 (High) | – | auto |
| `TamagotchiAngel` | Tenshitchi no Tamagotchi (High) | Bandai (High) | 1997-08-01 (High) | – (Unknown) | – | auto |
| `TamagotchiGenjintch` | Genjintchi no Tamagotchi (High) | Bandai (High) | 1997-11-01 (Medium) | – (Unknown) | – | auto |
| `TamagotchiMorino` | Mori de Hakken!! Tamagotchi (Medium) | Bandai (High) | 1998-02-01 (High) | – (Unknown) | – | auto |
| `TamagotchiMothra` | Mothra no Tamagotchi (High) | Bandai (High) | 1997-12-01 (High) | – (Unknown) | – | auto |
| `TamagotchiP1` | Tamagotchi (Gen. 1, International) (Medium) | Bandai (High) | 1997-05-01 (High) | 1800 (Low) | TamagotchiP1J | auto |
| `TamagotchiP1J` | Tamagotchi (Gen. 1, Japan) (High) | Bandai (High) | 1996-11-23 (High) | – (Unknown) | – | auto |
| `TamagotchiP2` | Tamagotchi (Gen. 2, Japan) (Medium) | Bandai (High) | 1997-02-01 (High) | – (Unknown) | – | auto |
| `TamagotchiUmino` | Umi de Hakken!! Tamagotchi (Medium) | Bandai (High) | 1998-03-01 (High) | – (Unknown) | – | auto |
| `TamagotchiYasashii` | Yasashii Tamagotchi (High) | Bandai (High) | 1998-10-01 (Low) | – (Unknown) | – | override |

## Proposed manufacturer groups (counts)

| Group | Configs | Existing group? |
|---|---|---|
| Bandai | 28 | yes |
| Brick Game | 8 | **new** |
| Popy | 5 | **new** |
| Apollo | 5 | **new** |
| Gakken | 3 | **new** |
| Hiro | 2 | **new** |
| Nelsonic | 2 | yes |
| Epoch | 2 | yes |
| Spica | 1 | **new** |
| Unbranded | 1 | **new** |
| Sparkle Crystal | 1 | **new** |
| Nature Land | 1 | **new** |
| McDonald's | 1 | yes |
| Gametech | 1 | **new** |
| VGS | 1 | **new** |
| Compile | 1 | **new** |
| Cool Tec | 1 | **new** |
| Hartung | 1 | **new** |
| Edla | 1 | **new** |
| Stadlbauer | 1 | **new** |
| Nintendo | 1 | yes |
| Nikko | 1 | yes |
| Tandy | 1 | yes |

## Open checks from the handoff: answers

| Pack ID | Handoff value | Proposed | Confidence | Basis |
|---|---|---|---|---|
| `TAMAUMI` | 1998-01-01 | **1998-03-01** | High | Japanese Wikipedia, Tamagotchi wiki and Bandai-derived chronologies agree on March 1998 |
| `DIGIMONES` | "Digital Monster (Spanish)", 1997-01-01 | **Digital Monster Ver.1 (Europe)**, **1998-01-01** | Name Medium, date Low | Community tools label this ROM EU/English. The face's blue shell with light-grey trim matches a documented European colour. No Spanish unit found. 1997-01-01 is before the product existed (Japan launch 1997-06-26). 1997-12-05, the first English (North American) release, is an alternative date. |
| `TAMAJ` | 1996-01-01 | **1996-11-23** | High | Japanese launch date per Japanese Wikipedia, the Tamagotchi wiki and Bandai history |
| `STACKCH` | – | **Stack Challenge**, Radio Shack cat. **60-2247**, **1991** | High | "NEW!" in Radio Shack's December 1991 flyer and in the 1992 catalogue (p.163); not in the 1991 catalogue. ROM hash matches MAME `stackch`. |
| all 17 names | chosen without research | see summary table | – | Main changes: Digimon → "Digital Monster Ver.N", Gen 2 region, Angel/Umino Japanese titles, "!!" spelling, Genjintchi title. The Digimon legacy configs are duplicates (Decision 5). |

## Decisions: research-backed recommendations

These are recommendations; the decisions are yours. Each one notes the evidence behind it.

1. **New manufacturer groups.** The research gives these makers (counts are in the table above):
   - **Popy (5):** keep as its own group. The 1982 units carry only the ポピー mark. The 1983 units carry BANDAI on the back label, but their boxes still read "POPY ELECTRONIC(S) GAME / ANIMEST" (Popy merged into Bandai in March 1983). Filing under Bandai is defensible too; if you do, put "Popy" in the notes.
   - **Gakken (3):** Soccer, Jumping Boy (© Konami on the back sticker, but Gakken-branded), Tom & Jerry Prank.
   - **Apollo (5):** two brick games and three pet configs.
   - **Hiro (2):** Kunekunetchyo (Hiro LCD Keychain Series No.4, developed by Koto Laboratory, one of Gunpei Yokoi's last designs) and Pocket Boy No.2. "Koto Laboratory" belongs in the notes rather than a group.
   - **Brick Game (8):** for the generic Chinese and Taiwanese bricks and keychains. E-23, E-88 and E-33 are E-Star (Taiwan), whose 机之宝 logo is on the face; GA888, Keychain Pin Ball, Space Intruder, GA-878 and Micon KC-32 carry no traceable brand. One "Brick Game" group, with the OEM in `notes`, matches how these were sold.
   - **One-title makers:** Spica (Taiwan), Nature Land (Super Gyaoppi; designer "U.I."), Sparkle Crystal (Elfin Twins; the name is in the ROM), Gametech (Pikorin 55), Compile (Puyolin), Hartung (Formel 1), Edla, Stadlbauer (Mini Classics), VGS (Mickey, Low), Cool Tec (Pocket Puppy, Low). Rather than ten one-game groups, consider one **"Other"** group and keep the maker in the name or notes.
   - **Existing groups gain titles:**
     - Bandai: Pengo, Power Fishing, the two Gundams, both Mame Games, Tamagotchi V3, Tamagotchi Mini 2017, D-Power Digivice.
     - Nelsonic: SMB, Zelda.
     - Epoch: Monster Panic, Alien Fever.
     - Nintendo: Pocket Pikachu.
     - Tandy: Stack Challenge.
     - Nikko: Venus Diet Monogatari.
   - **Big Cat 9 in 1** is a reskin of Super Gyaoppi: the first 22 KB, the program code, are byte-identical. So `clone_of: Gyaoppi9in1`, which files it under Nature Land by your clone convention. Its own brand is unknown, so the JSON says "Unbranded".
2. **McDonald's Tetris Nugget.** This is a genuine McDonald's China promotional item:
   - **Launch:** 2023-05-24, for the McNugget 40th anniversary and Children's Day, mainland China only.
   - **Distribution:** 400,000 units, ¥30 with any order.
   - **Licence and branding:** Tetris Company licence, golden arches on the LCD.
   - **Name:** there is no official product name; the press release calls it a McNugget-shaped Tetris machine. "Chicken McNugget Tetris" is proposed.
   - **Recommendation:** it belongs in the **McDonald's** group. Rework `GW_rom::is_lcdonald_family` to key on the core or ROM type instead of the group name. A separate "McDonald's (China)" group is the fallback.
3. **Experimental gating.** This is not something research can settle. Several new configs look unfinished or test-stage upstream, which supports gating at least these:
   - `GundamGaidenII`: test paths and a link peripheral.
   - `MiniClassicsSoccer`: still under `assets/test/`.
   - `TamagotchiMini2017`: only moved out of a test path on 2026-10-12.
   - Tamagotchi V3: flagged by upstream.
   - Gating every new-core game, as with the E0C6200 set, is the consistent choice.
4. **Questionable dumps.** Include Tamagotchi Connection V3 and Tamagotchi Mini 2017 with a dump warning in `notes` (already added), behind Experimental. Also note: the handoff says `DigimonV1JA` and `DigimonV1JB` share one ROM, but upstream treats them as two separate revisions. If your two files are byte-identical, one is probably a mislabelled copy. Re-check before packing both.
5. **Configs sharing a ROM:**
   - **Prince & Princess `_ch`/`_en`:** one entry each. They differ only in a hardware language strap (`port_pullup` PA bit 1), and players see different text. Named "(Chinese)" and "(English)".
   - **Tamagotchi V3 `_us`/`_eu`:** one entry each. They differ in a region strap bit (PD pull-up). Named *Tamagotchi Connection V3 (US)* and *Tamagotchi Connexion V3 (Europe)* (Connexion is the EU spelling), with the EU entry `clone_of` the US one.
   - **Digimon:** `DigimonV1`, `DigimonV2` and `DigimonV3` are pre-rename copies of V1JB, V2J and V3J. Drop them, or keep them only as aliases. Keep V1JA, V1JB, V1ES, V2J, V3J and V4J.
6. **Series names:**
   - **Drop:** "Animest" (a badge on the face; box titles never put it in front of the title), Gakken "LCD Card Game", Bandai "Super Game Digital", "LCD Game Digital" and "Pocket Club P-1".
   - **Keep "Game Watch" for Nelsonic:** "Super Mario Bros. Game Watch" is on the box. "The Legend of Zelda" alone would clash with Nintendo's own Zelda Game & Watch. Both Nelsonic names were harmonised that way in this merge.
   - **Bandai "Mame Game":** the Tamagotchi entry keeps it because it is part of the real title (マメゲームで発見!!たまごっち); Galaxian is just "Galaxian".
   - **Spelling:**
     - Bandai's "…で発見!!" titles use "!!". Mori and Umi both use "!!", and the handoff's "Mori de Hakken! Tamagotchi" example should get the second "!".
     - Digimon uses "Ver.1" with no space, matching Bandai's Japanese styling.

## Still weak after two passes

Configs where the name, maker or date is still Low or Unknown. Each one's section below records what was searched.

| `.brick` | Weak fields |
|---|---|
| `GakkenSoccer` | date (Low) |
| `SpicaDArtagnan` | date (Low) |
| `Apollo2in1` | date (Low) |
| `ApolloPrince&Princess_ch` | date (Low) |
| `ApolloPrince&Princess_en` | date (Low) |
| `BigCat9in1` | manufacturer (Low), date (Low) |
| `ElfinTwins` | date (Low) |
| `Gyaoppi9in1` | date (Low) |
| `TetrisNugget` | name (Low) |
| `E23PlusMarkII96in1` | date (Low) |
| `E88_8in1` | date (Low) |
| `GA888` | date (Low) |
| `Keychain55in1` | date (Low) |
| `KeychainPinBall` | manufacturer (Unknown), date (Low) |
| `MickeyVGS` | name (Low), manufacturer (Low), date (Low) |
| `SpaceIntruderTK150I` | manufacturer (Low), date (Low) |
| `GA878` | date (Low) |
| `MiconKC32` | date (Low) |
| `PocketPuppy` | manufacturer (Low), date (Low) |
| `formel1` | date (Low) |
| `EdlaSpaceRescue` | name (Low), date (Low) |
| `PocketBoyNo2` | date (Low) |
| `DPowerDigiviceEU101` | date (Low) |
| `E33_2in1` | date (Low) |
| `MiniClassicsSoccer` | date (Low) |
| `DigimonV1ES` | date (Low) |
| `TamagotchiYasashii` | date (Low) |

## Per-game details

Each section starts with the final proposed values (after both passes). Below that is the first-pass research table, then any second-pass (gap-fill) update. Where they differ, the second pass and the final values win.

### Toshiba 4-bit

#### `AnimestSpaceCobra` – Space Cobra Professional

**Final:** name **Space Cobra Professional** (High); group **Popy** (High); date **1982-01-01** (High); model **CO-02** (High); clone of **none** (High); keys: `btnRightUp`→`RIGHT|UP`, `btnLeftDown`→`LEFT|DOWN`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Space Cobra Professional**. JP box: スペースコブラ プロフェッショナル. The face prints "PROFESSIONAL" over the screen, a "SPACE COBRA" logo, and "POPY ELECTRONICS GAME". The series badge "ANIMEST" is on the face. A European box seen in HE photos is titled just "PROFESSIONAL" (French/Italian text, Bandai France / Bandai Italia). | High | Face SVG text, HHM box photo, Wikipedia list, EP, HE |
| manufacturer | Popy (ポピー, Bandai subsidiary; merged back into Bandai in 1983). Box: "POPY ELECTRONICS GAME / ANIMEST". Menu group: **Popy**, or Bandai if Popy is folded in | High | Face, box, back mold mark (ポピー logo) |
| date | **1982-01-01** (Japan, first release). The anime started 1982-10-07, so the real release is probably late 1982. Mini-Arcade lists it as Bandai, 1983, probably the later Bandai-era stock or the export release | High (year) | Back mold "PAT.PEND.1982"; HHM, EP, Wikipedia (citing HHM) and HE all say 1982 |
| region / variant | n/a (Japanese unit; face matches the JP Popy version) | Medium | Face art = JP unit |
| model | **CO-02** | High | Molded back label "MODEL NO. CO-02/DC3V" (HE photo). HHM's "Model# 72619" is the JAN barcode fragment on the box (a 1000 0200 72619), not a catalogue number |
| clone_of | none | High | No relative in the list. Sister fold-case game: Golgo 13 "Secret Killer". Separate title: Space Cobra "The Psychogun" |
| keys | Override suggested: `{"btnRightUp": "RIGHT\|UP", "btnLeftDown": "LEFT\|DOWN"}`. The two big keys are printed ▲/▶ (upper) and ◀/▼ (lower); BrickEmuPy also binds both arrows to each. Fire=A, Game=PLUS, Mode=MINUS, Alarm=R match the face labels (GAME, MODE, ALARM, ACL, FIRE) | Medium | Face SVG |
| notes | Fold-up two-screen case (horizontal dual LCD, opens like a travel clock). Back copyright: ©寺沢武一・集英社・C.I.C.・フジテレビ. BrickEmuPy credits: ROM dump and photos by Milan Galcik. | | |

Sources:
- https://www.handheldmuseum.com/Popy/Professional.htm - Popy, 1982, LCD; box front/back photos (JP title, POPY ELECTRONICS GAME ANIMEST, barcode)
- https://www.electronicplastic.com/game/?id=629 - POPY Space Cobra Professional (1982), fold-up 2-display case
- https://handheldempire.com/game.jsp?game=1393 - Popy, Animest "Folding", 1982; photos of the molded back (MODEL NO. CO-02, PAT.PEND.1982) and the European "PROFESSIONAL" box
- https://en.wikipedia.org/wiki/List_of_Cobra_video_games - Popy, 1982 Japan, LCD, スペースコブラ プロフェッショナル
- https://miniarcade.com/bandai/spacecobra.php (search result only) - lists Bandai, 1983 (conflict; not adopted)


#### `DorayakiHouse` – Dorayaki House

**Final:** name **Dorayaki House** (High); group **Popy** (High); date **1983-01-01** (High); model **DO-01** (High); clone of **none** (High); keys: `btnJump`→`UP`, `btnAction`→`A`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Dorayaki House**. JP box: ドラヤキハウス (katakana, not どら焼き) with the ドラえもん logo. The face prints "DORAYAKI HOUSE" and the tagline "'DORAEMON' PLEASANT AND REAL ANIMATION GAME". Series badge "ANIMEST". HHM/HE list it as "Doraemon Dorayaki House". | High | Face SVG, HHM box photo |
| manufacturer | Popy. Box: "POPY ELECTRONICS GAME / ANIMEST"; back label shows the ポピー logo. Group: **Popy** (or Bandai) | High | Box and back label photos |
| date | **1983-01-01** (Japan only) | High (year) | Back label "PAT. PEND. 1983" (HE photo); HE says 1983. HHM says 1982, which conflicts with the label, so not adopted |
| region / variant | n/a (Japan) | High | |
| model | **DO-01** | High | Back label "MODEL NO. DO-01/DC3V 0,00015W" |
| clone_of | none | High | Same "portrait" Animest case as Parman Dai-Pinch, but a different game |
| keys | Override suggested: `{"btnJump": "UP", "btnAction": "A"}`. The face labels the top key of the cross "JUMP UP", and BrickEmuPy binds Jump to W/Up arrow. ACTION is the separate big side key, the natural A. Mode=MINUS stays | Medium | Face SVG + brick hotkeys |
| notes | ©藤子・小学館・テレビ朝日 on the label. Not to be confused with Bandai's "Dokodemo Dorayaki Doraemon" (VFD, MAME `hh_hmcs40` B43, PT-412). | | |

Sources:
- https://www.handheldmuseum.com/Popy/DorayakiHouse.htm - Popy Doraemon Dorayaki House (says 1982); box photo with ドラヤキハウス and POPY ELECTRONICS GAME ANIMEST
- https://handheldempire.com/game.jsp?game=1377 - Popy, Animest "Portrait", Japan, 1983; back-label photo (MODEL NO. DO-01, PAT. PEND. 1983, ポピー)


#### `DrSlumpAraleNchaBycha` – Dr. Slump Arale: Ncha! Bycha

**Final:** name **Dr. Slump Arale: Ncha! Bycha** (High); group **Popy** (High); date **1982-01-01** (High); model **AR-03** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Dr. Slump Arale: Ncha! Bycha** (the handoff's own example). The face prints "dr.SLUMP ARALE" and "NCHA! BYCHA", plus "REAL ANIMATION ANIMEST & WIDEVISION". JP box: Dr.スランプ アラレちゃん "んちゃ!ばいちゃ" 第3巻. HHM lists it as "Dr. Slump Arale Part III Ncha! Bycha". If the series number is wanted, "Dr. Slump Arale Part III: Ncha! Bycha" also works. | High | Face SVG, box photos (HHM, HE) |
| manufacturer | Popy. Box: "POPY ELECTRONICS GAME / ANIMEST"; back mold shows the ポピー logo. Group: **Popy** (or Bandai) | High | Back mold, box |
| date | **1982-01-01** (Japan) | High (year) | Back mold "PAT. PEND. 1982"; HHM, EP and HE all say 1982 |
| region / variant | n/a (Japan). HHM: released in three case colours | Medium | HHM |
| model | **AR-03** | High | Back mold "MODEL NO. AR-03/DC3V 0.0002W" (HE photo); HHM "Model# AR-03" |
| clone_of | none | High | Part III of the Popy Arale trilogy (Part I Hoyoyo Bomber, Part II Gatchan Kazi Kazi); none of those is in the list |
| keys | keep auto (Ncha=A, the ↺/↻ TURN keys = LEFT/RIGHT, Game1/Game2=PLUS/MINUS, Mode=X, Alarm=R) | High | Face labels: NCHA, GAME 1, GAME 2, MODE, ACL, ALARM, ↺TURN, ↻TURN |
| notes | ©鳥山明/集英社・フジテレビ・東映動画. Battery LR43×2 (other Animests use LR44). BrickEmuPy credits the ROM dump to Milan Galcik. | | |

Sources:
- https://www.handheldmuseum.com/Popy/DrSlump3.htm - 1982, Model# AR-03, three colours
- https://www.electronicplastic.com/game/?id=636 - POPY Dr. Slump Ncha Bycha (1982)
- https://handheldempire.com/game.jsp?game=3315 - Popy, Animest "Silver Card", 1982; back-mold and box photos


#### `GakkenSoccer` – Soccer

**Final:** name **Soccer** (High); group **Gakken** (High); date **1982-01-01** (Low); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Soccer**. The face prints "LCD CARD GAME / Soccer / Gakken", so the series prefix is "LCD Card Game". | High | Face SVG, HHM photo |
| manufacturer | Gakken (学研). Group: new **Gakken** group | High | Face |
| date | **1981-01-01** (only explicit source: the BrickEmuPy README) | Low | Not confirmed elsewhere. HHM and Retro Handheld Games give no year. Counter-evidence: Toshiba masks are numbered roughly in release order, and Soccer's T6814 sits between Jumping Boy (T6813, EP says 1982) and Arale Ncha! Bycha (T6815, PAT.PEND 1982). That points to 1982. Worth another search pass |
| region / variant | n/a. The face matches HHM's first (Japanese-style) Soccer. HHM also shows a different-looking Italian-market Soccer ("2 SCREENS (offense/defense) IN 1") whose art does not match this SVG | Medium | HHM photos |
| model | Unknown | Unknown | No box number seen. The Gakken LCD Card Game box for Pyonkichi carries "NO. 81553", so a similar 8xxxx number probably exists |
| clone_of | none | High | Unrelated to `MiniClassicsSoccer` |
| keys | keep auto. The face prints "SHOOT A" and "PASS B", which match Shoot=A and Pass=B; arrows map to the D-pad; TIME/ALARM/ACL buttons are at top right | High | Face SVG |
| notes | The SVG is a "flat LCD (no background)" render. The real unit has a coloured stadium/crowd overlay (visible in HHM photos), not represented. | | |

Sources:
- https://www.handheldmuseum.com/Gakken/Soccer.htm - Gakken Soccer (19xx, LCD); both versions pictured
- https://www.retrohandheldgames.com/gakken - Soccer (LCD), year unknown

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1981-01-01 (Low) | **1982-01-01 (Low-Medium, inferred)** | Two independent indicators point to 1982, and nothing supports the README's 1981. (1) Handheld Empire and RetroHandheldGames file Soccer under Gakken's "Cornerless card" series. Every dated member of that series is from 1982-83: Pooyan 81709 (1982), Moon Patrol 81728 (1982), Tom & Jerry Prank 81751 (1983). The 1981 Gakken cards are the "metal card" series (Trojan Horse 81554, Towering Rescue 81555, Fishing, all 1981). (2) The Toshiba mask T6814 sits between T6813 (Jumping Boy, 1982) and T6815 (Arale, PAT.PEND 1982). Online listings give contradictory years (1980 on one eBay listing, 1983 on another seller page), so I did not use them. No primary dated source was found. |
| model | Unknown | Unknown (probably 817xx) | Cornerless-card numbers run 81709 to 81751, so Soccer is probably 817xx, but I saw no number. The export box back photo (HE) shows only "LCD CARD GAME / Soccer / Pinball / Circus, Gakken Toy Department ... MADE IN JAPAN", with no number and no year. |
| name (JA) | 学研 LCDカードゲーム サッカー? | no change (Unknown) | No Japanese box was found. The HE photos are of the export box ("Soccer / LCD CARD GAME / GAME+DIGITAL QUARTZ WATCH WITH ALARM"). Yahoo! Auctions closed-search titles show only bare "サッカー" items. Italian packages exist as "Calcio" (R.E.G.E. S.r.l. Firenze / Duracell) and German ones as "tacotronic Fussball". |
| notes | | add | Series siblings on the same box back are Pinball and Circus. The unit was also sold in Italy as "Calcio" by R.E.G.E. S.r.l. (Firenze) and in Germany as "tacotronic Fussball" (HE photos). |

Sources:
- https://handheldempire.com/game.jsp?game=853 - Gakken Soccer, "Cornerless card" series, year/model blank; box and back photos (downloaded and read)
- https://www.retrohandheldgames.com/gakken/soccer-862 - Cornerless card, Made in Japan, year unknown
- https://www.retrohandheldgames.com/gakken/pooyan-1217 - Cornerless card, 81709, 1982
- https://www.retrohandheldgames.com/gakken/moon-patrol-1562 - Cornerless card, 81728, 1982
- https://www.retrohandheldgames.com/gakken/trojan-horse-cheval-de-troie-246 - metal card, 81554, 1981
- https://www.retrohandheldgames.com/gakken/towering-rescue-274 - metal card, 81555, 1981


#### `Gundam3in1` – Gundam Space Combat 3 in 1

**Final:** name **Gundam Space Combat 3 in 1** (High); group **Bandai** (High); date **1981-10-01** (Medium); model **16194** (Medium); clone of **none** (High); keys: `btnStart`→`A|PLUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Gundam Space Combat 3 in 1**. The face prints "SUPER GAME DIGITAL / GUNDAM SPACE COMBAT 3in1" and "BANDAI ELECTRONICS". JP box: ガンダムスペースコンバット; JA Wikipedia lists it as "LCDスーパーゲームデジタル GUNDAM SPACE COMBAT3in1". Series prefix "Super Game Digital" (GD logo). The three games are ズゴックVS.ガンダム, ザクVS.ガンダム and 機雷層突破作戦. | High | Face SVG, HHM box photo, JA Wikipedia |
| manufacturer | Bandai (Bandai Electronics). Group: **Bandai** | High | Face, box |
| date | **1981-10-01** (Japan) | Medium | Collector blog: "発売は1981年(昭和56年)10月". HHM also says 1981. EP says 1983 (conflict). The older T6770S core in this batch fits 1981 |
| region / variant | n/a (Japan) | High | |
| model | **16194** | Medium | Number printed on the box top-right in the HHM photo; HHM "Model# 16194" |
| clone_of | none | High | Same Bandai silver 3-in-1 body as Floppy Jump 3 in 1 and Space Shuttle 3 in 1 (EP), which are not in the list |
| keys | Override suggested: `{"btnStart": "A\|PLUS"}`. The big lower-left key is labelled "FIRE SPEED / START", so it is the in-game fire button. Up/Down (UP ACTION DOWN), Time=R (TIME / GAME 1.2.3) and Reset=ACL stay | Medium | Face SVG |
| notes | The README gives no maker or year; the face settles maker and title. Its hint that this might be a third-party unit is wrong: it is genuine Bandai. | | |

Sources:
- https://www.handheldmuseum.com/Bandai/Gundam3in1.htm - Bandai Gundam Space Combat 3-in-1 (1981, Model# 16194), box photo
- https://www.electronicplastic.com/game/?id=42 - BANDAI Gundam Space Combat 3 in 1 (1983)
- https://ameblo.jp/neo-izayoi-onepiece/entry-12223925850.html - release October 1981, "SUPER GAME DIGITAL"
- https://ameblo.jp/showagamer/entry-12498262891.html - the three game names
- https://ja.wikipedia.org/wiki/ガンダムシリーズゲーム作品一覧 - "LCDスーパーゲームデジタル GUNDAM SPACE COMBAT3in1"


#### `GundamGaidenII` – SD Gundam Gaiden II: Taose! Psycho Golem

**Final:** name **SD Gundam Gaiden II: Taose! Psycho Golem** (High); group **Bandai** (High); date **1990-01-01** (High); model **29908** (Medium); clone of **none** (High); keys: `btnStart`→`A|PLUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **SD Gundam Gaiden II: Taose! Psycho Golem** (倒せ!サイコゴーレム). The face prints the SDガンダム logo, "SDガンダム外伝II" and the big subtitle "倒せ!サイコゴーレム". JA Wikipedia lists it as "LSI GAMEポケットクラブ P-1 SDガンダム SDガンダム外伝II 倒せ!サイコゴーレム". Series prefix "Pocket Club P-1". "Taose!" is my romanization; no official English title found. | High | Face SVG, JA Wikipedia list, Yahoo Auctions listing |
| manufacturer | Bandai. Group: **Bandai** | High | JA Wikipedia; listings |
| date | **1990-01-01** (Japan) | Low | No release date found. The README's "1985" is impossible: SD Gundam Gaiden began as a Carddass line in 1989, Psycho Golem is the final boss of arc II "Densetsu no Kyojin", and the Zeon arc ran 1989-1990. The face's ©創通エージェンシー・サンライズ also fits ~1990. P-1 came before the P-1 mini series (Gaiden III/IV), so 1990 (possibly 1989) is the reasoned guess |
| region / variant | n/a (Japan) | High | |
| model | Unknown | Unknown | Not found |
| clone_of | none | High | Sister P-1 titles (Gaiden I "Knight Gundam no Bōken" and others) are not in the list |
| keys | Override suggested: `{"btnStart": "A\|PLUS"}`. The left key is labelled "ATTACK / START". The two right keys (↺ / ↻, "ACTION") map fine to LEFT/RIGHT. ACL stays | Medium | Face SVG |
| notes | **Config warning:** the .brick's `face_path` and `rom_path` point to `./assets/test/...`, a folder that does not exist in the clone (the files sit in `assets/`), so the config looks unfinished. It also declares a `direct_connection` peripheral (link port: INP bit 0 in, OUTP bit 1 out), which suggests the P-1 units could link. Treat as experimental. The README's Toshiba "7704" mask is lower than Power Fishing's 7790 (1984), so the mask number does not help date it here. | | |

Sources:
- https://ja.wikipedia.org/wiki/ガンダムシリーズゲーム作品一覧 - full P-1 / P-1 mini series list (fetched wikitext)
- https://gundam.fandom.com/ja/wiki/ガンダムシリーズのゲーム作品一覧 - same list (wikitext via API)
- https://ja.wikipedia.org/wiki/SDガンダム外伝_ジークジオン編 - Zeon arc 1989-1990, 伝説の巨人 arc
- https://dic.pixiv.net/a/サイコゴーレム (search result) - Psycho Golem = final boss of SD Gundam Gaiden II
- https://auctions.yahoo.co.jp/closedsearch/closedsearch/サイコゴーレム/0 - listing "LSI GAME SDガンダム外伝II 倒せ!サイコゴーレム [ポケットクラブP-1] BANDAI"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1990-01-01 (Low) | **1990-01-01 (High, year)** | Box photo: the front-panel footer reads **"BANDAI 1990 MADE IN JAPAN"**. The seller also says the unit was bought new "1990年当時". |
| README 1985 explained | "impossible" | The unit's moulded back plate reads **"BANDAI 1985 MADE IN JAPAN"** | This comes from a reused 1985 Bandai LSI case mould. The README year was taken from the case, not from the game. |
| model | Unknown | **29908** (Bandai item code) (Medium) | Box back: JAN **4902425 29908 2** (check digit verified) and Bandai code "103015-0029908-4800". The ¥1,800 price sticker fits Bandai's usual long-code format. The unit has no printed model number. Series prefix on the box: "LSI GAME ポケットクラブ P-1". |
| name | (High) | no change | The box confirms "SDガンダム外伝II 倒せ!サイコゴーレム" and "LSI GAME ポケットクラブ P-1". Face labels: ATTACK/START, ACL, ACTION, OFF-SOUND-ON, OFF-POWER-ON, ©創通エージェンシー・サンライズ. |
| notes | | add | P-1 mini titles such as SDガンダム外伝『光の騎士伝説』 are listed as 1991, which is consistent with full-size P-1 = 1990. |

Sources:
- https://auctions.yahoo.co.jp/jp/auction/v1241212455 - box photos: "BANDAI 1990 MADE IN JAPAN", JAN 4902425299082, 103015-0029908-4800; unit back "BANDAI 1985"; seller "bought new in 1990"
- https://auctions.yahoo.co.jp/closedsearch/closedsearch?p=%E3%83%9D%E3%82%B1%E3%83%83%E3%83%88%E3%82%AF%E3%83%A9%E3%83%96+P-1+%E3%82%AC%E3%83%B3%E3%83%80%E3%83%A0 - P-1 / P-1 mini Gundam titles, "【1991年】... 光の騎士伝説 ... P-1ミニ"


#### `IsogeDoraemon` – Isoge! Doraemon

**Final:** name **Isoge! Doraemon** (High); group **Popy** (High); date **1983-01-01** (High); model **DO-02** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Isoge! Doraemon**. JP box: 急げ!ドラえもん (kanji 急げ, not いそげ). The face prints "ISOGE! DORAEMON" plus the "'DORAEMON' PLEASANT AND REAL ANIMATION GAME" tagline. Series badge "ANIMEST". | High | Face SVG, HHM box photo |
| manufacturer | Popy brand, Bandai era. Box: "POPY ELECTRONICS GAME / ANIMEST" with ポピー top-left and a BANDAI logo at the bottom; the manual says BANDAI; the unit back label shows BANDAI. Group: **Popy** (or Bandai) | High | HHM box photo, HE back-label photo |
| date | **1983-01-01** (Japan) | High (year) | Back label "PAT.PEND.1983". HHM and HE say 1982, which conflicts with the label, so not adopted |
| region / variant | n/a (Japan) | High | |
| model | **DO-02** | High | Back label "MODEL NO. DO-02/DC3V 0.00015W" |
| clone_of | none | High | Shares the DO- prefix with `DorayakiHouse` (DO-01), but a different game |
| keys | keep auto (Action=A, the big left key; Up/Down; Mode=MINUS; ACL) | High | Face labels ACTION, UP, DOWN, MODE, ACL |
| notes | HE calls this case style "Animest: bump plastic" (raised Doraemon figure). | | |

Sources:
- https://www.handheldmuseum.com/Popy/IsogeDoraemon.htm - Popy Isoge! Doraemon; boxed photo (急げ!ドラえもん, POPY ELECTRONICS GAME ANIMEST, BANDAI)
- https://handheldempire.com/game.jsp?game=1387 - Popy, Japan; back-label photos (MODEL NO. DO-02, PAT.PEND.1983, BANDAI)


#### `JumpingBoy` – Jumping Boy

**Final:** name **Jumping Boy** (High); group **Gakken** (High); date **1982-01-01** (Medium); model **–** (Unknown); clone of **none** (High); keys: `btnGameA`→`DOWN|PLUS`, `btnGameB`→`UP|MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Jumping Boy** (export/English title, the one on this face). The face prints "Gakken / LCD CARD GAME / JUMPING BOY", so the series prefix is "LCD Card Game". Japanese domestic title: **ぴょん吉 (Pyonkichi)**, with the same layout in a white case. France: Lansay "La Traversée" (LCD Pocket Jeu). | High | Face SVG; HHM and HE photos of all three |
| manufacturer | Gakken; game © Konami (both the Jumping Boy and Pyonkichi back stickers read "Gakken © Konami"). Group: **Gakken**. Konami is the developer and already has a menu group, but the unit is sold as Gakken | High | Unit back-sticker photos (HE) |
| date | **1982-01-01** | Medium | EP gives 1982, and an itch.io page in search results also says 1982. HHM, HE and Retro Handheld Games give no year. The mask T6813 is adjacent to Arale's T6815 (1982). Japanese original vs export order is unknown; Pyonkichi is presumably the domestic first release, same year |
| region / variant | Export (English) version; JP Pyonkichi and FR La Traversée share the game | Medium | Faces differ only in title/brand. A same-ROM claim rests on BrickEmuPy listing all three names |
| model | Unknown for Jumping Boy. Pyonkichi box back: **NO. 81553** | Medium (Pyonkichi no.) | HE photo of the Pyonkichi box back |
| clone_of | none in list. Outside the list: Gakken Pyonkichi, Lansay La Traversée | High | |
| keys | Override suggested: `{"btnGameA": "DOWN\|PLUS", "btnGameB": "UP\|MINUS"}`. The only play buttons are "▼ GAME A" (jump back) and "▲ GAME B" (jump forward), which also start each game (box back: ▼ = start A / jump back, ▲ = start B / jump forward). Auto PLUS/MINUS alone feels wrong for play. Time=R, Alarm=L stay | Medium | Face SVG, Pyonkichi box back |
| notes | Frogger-like: hop logs and turtles to reach rescue boats. Back sticker text: "A great flood has occurred!...". | | |

Sources:
- https://www.handheldmuseum.com/Gakken/JumpingBoy.htm - Jumping Boy, released in France by Lansay as La Traversee; box photo
- https://www.handheldmuseum.com/Gakken/Pyonkichi.htm - Pyonkichi unit and box (ぴょん吉, LCD CARD GAME)
- https://www.handheldmuseum.com/Lansay/LaTraversee.htm - French game licensed from Gakken (same as Jumping Boy)
- https://www.electronicplastic.com/game/?id=293 - GAKKEN Jumping Boy (1982); EP's note claims it was "released outside Japan as Pyonkichi", which is backwards: Pyonkichi is the JP name
- https://handheldempire.com/game.jsp?game=836 - photos of the "Gakken © Konami" back stickers and the Pyonkichi box back (NO. 81553)
- https://itizso.itch.io/gakken-jumping-boy (search result only) - "released by Gakken in 1982"


#### `ParmanDaiPinch` – Parman Dai-Pinch

**Final:** name **Parman Dai-Pinch** (High); group **Popy** (High); date **1983-01-01** (High); model **PA-01** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Parman Dai-Pinch**. The face prints "PARMAN DAI-PINCH" and "'PAR MAN' PLEASANT AND REAL ANIMATION GAME". Series badge "ANIMEST". The Japanese title is probably パーマン大ピンチ, but no Japanese source was found (Unknown). | High (romanized) | Face SVG; HE photos |
| manufacturer | Popy/Bandai: Animest (Popy) series, but the unit back label carries the BANDAI logo. Group: **Popy** (or Bandai) | High | HE back-label photo |
| date | **1983-01-01** (Japan) | High (year) | Back label "PAT.PEND.1983"; the second Parman anime started April 1983 |
| region / variant | n/a (Japan) | High | |
| model | **PA-01** | High | Back label "MODEL NO. PA-01/DC3V 0.00015W" (HE photo) |
| clone_of | none | High | Same "portrait" case as `DorayakiHouse`. Different from Bandai's "Tondeke Parman" (1983, 0309011) and "Kiteyo Parman" (VFD, MAME `hh_hmcs40` B42) |
| keys | keep auto (cross keys UP/DOWN/LEFT/RIGHT, Action=A, Mode=MINUS) | High | Face labels |
| notes | ©藤子・小学館・テレビ朝日. Not on HHM or EP; HE lists it with year "unknown". | | |

Sources:
- https://handheldempire.com/game.jsp?game=1389 - Popy, Animest "Portrait"; photos of the unit and back label (MODEL NO. PA-01, PAT.PEND.1983, BANDAI)
- https://www.handheldmuseum.com/Bandai/TondekeParman.htm - for disambiguation (Tondeke Parman, 1983, 0309011)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| name (JA) | パーマン大ピンチ? (Unknown) | **パーマン大ピンチ!** (with "だい" ruby over 大, and an exclamation mark) (High) | Box photos (front and side) show "パーマン大ピンチ!" with the romanized "PARMAN DAI-PINCH" pattern. The box top reads "POPY ELECTRONIC GAME ANIMEST". The seller's title also writes "パーマン大ピンチ". |
| manufacturer | Popy/Bandai (High) | confirmed | Box: "BANDAI" logo + "ポピー事業部" (Bandai's Popy division). Unit back label: MODEL NO. PA-01 ... PAT.PEND.1983, BANDAI (same as HE). |
| model | PA-01 (High) | PA-01 (High). Add **box computer number 0309010** | Box end flap: "COMPUTER NUMBER 0309010" with the ST mark. This fits Bandai's Tondeke Parman 0309011 in the same numbering. |
| date | 1983 (High) | no change | |

Sources:
- https://auctions.yahoo.co.jp/jp/auction/j1233917683 - lot with Yonezawa "パーマンワールド" and the boxed "パーマン大ピンチ!" (photos: box front/side, COMPUTER NUMBER 0309010, BANDAI ポピー事業部, unit back label MODEL NO. PA-01 / PAT.PEND.1983)
- https://auctions.yahoo.co.jp/closedsearch/closedsearch?p=%E3%83%91%E3%83%BC%E3%83%9E%E3%83%B3+LSI - closed listings; the only "大ピンチ" hit
- (side find, not in list) https://auctions.yahoo.co.jp/jp/auction/e1244741337 - Popy/Bandai "パーマン デチョンパ" P-badge LCD watch (computer number 0302126); a different product


#### `Pengo` – Pengo

**Final:** name **Pengo** (High); group **Bandai** (High); date **1983-01-01** (Medium); model **0200011** (High); clone of **none** (High); keys: `btnStart`→`A|PLUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Pengo** (ペンゴ). The face prints "Pengo™ ペンゴ", the GD logo and "BANDAI ELECTRONICS". Box: "BANDAI ELECTRONICS / GD LCD GAME DIGITAL", so the series prefix is "LCD Game Digital". | High | Face SVG, HHM box photo |
| manufacturer | Bandai (Bandai Electronics), "Made and sold under license from SEGA Enterprises Ltd." Group: **Bandai** | High | Box back |
| date | **1983-01-01** (Japan) | Medium | Sources split: HHM and EP say 1982; Segaretro, Indie Gamer Chick (via search) and BrickEmuPy's commit "add Bandai Pengo 1983" say 1983. 1983 chosen because the arcade game came out in 1982, the Segaretro entry has the JP box scans, and the mask T6941 sits between Parman T6923 and Isoge T6981 (both PAT.PEND 1983). No date is printed on the box |
| region / variant | n/a (Japan; Segaretro: possibly never released outside Japan) | Medium | |
| model | **0200011** | High | Printed on the box top-right (HHM photo); HHM "Model# 0200011" |
| clone_of | none | High | Do NOT confuse with Bandai "FL Pengo" (VFD tabletop, 1983, MAME `bpengo`, Hitachi HD38820 A63). That is a different product |
| keys | Override suggested: `{"btnStart": "A\|PLUS"}`. In the SVG, `btnStart` is the key labelled **ATTACK**; the box back calls it "セレクト&アタックキー" (selects time/game and kicks ice). The joystick does UP/DOWN/LEFT/RIGHT, and per the box also START. Reset = ACL | High | Face SVG element positions + box back |
| notes | ¥4,980 price sticker on the HHM box. The SVG also has a SOUND/OFF switch and a TIME/GAME label that are not emulated as buttons. | | |

Sources:
- https://www.handheldmuseum.com/Bandai/Pengo.htm - Bandai Pengo (1982, LCD, Model# 0200011), box front/back photos
- https://segaretro.org/Pengo_(LCD) - Bandai Electronics, LCD, JP 1983, licensed from Sega (read via `action=raw`)
- https://www.electronicplastic.com/game/?id=46 - BANDAI Pengo LCD (1982); https://www.electronicplastic.com/game/?id=124 - the separate FL Pengo VFD (1983)
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_hmcs40.cpp - MAME `bpengo` = FL Pengo VFD ("licensed from Sega", HD38820 A63)


#### `PowerFishing` – Power Fishing

**Final:** name **Power Fishing** (High); group **Bandai** (High); date **1984-01-01** (High); model **–** (Unknown); clone of **none** (High); keys: `btnRotateCCW`→`DOWN|L`, `btnRotateCW`→`UP|R`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Power Fishing** (JP: パワーフィッシング). No series prefix. | High | The face render prints "POWER FISHING" with a fish logo. Handheld Empire, Electronic Plastic and a Japanese YouTube video title all use the same name. |
| manufacturer | Bandai (group: **Bandai**) | High | Handheld Empire, Electronic Plastic, the todocoleccion listing and the JP YouTube title all say Bandai. No maker logo is drawn on the face SVG. |
| date | 1984-01-01 | High (year) | Handheld Empire, Electronic Plastic, the YouTube title ("made in japan 1984") and the todocoleccion listing all give 1984. The month is unknown. This is a Japanese release (made in Japan). No other regional release was found, though a boxed unit turned up in Spain. |
| region / variant | n/a (Japanese unit) | Medium | Made in Japan. Only one version is known. |
| model | Unknown | Unknown | Handheld Empire's model field is empty, and Electronic Plastic and the listings give none. lsigame.com's Bandai list does not include it. |
| clone_of | none | High | |
| keys | `{"btnRotateCCW": "DOWN|L", "btnRotateCW": "UP|R"}` (optional) | Low | The unit is a gun-grip fishing rod. It has a POWER OFF/ON slide, START, THROW and a side reel crank with CW/CCW arrows on the face. Auto maps the crank to B and X, which is awkward. BrickEmuPy's own hotkeys put CCW on Down and CW on Up, and the override copies that. Keep Throw=A and Start=PLUS. |
| notes | The original casts by a motion/tilt sensor: you swing the unit forward and a bird gets hit if the cast is too strong or weak. You reel by turning the crank. The emulator exposes the cast as a "Throw" button, and the face SVG also draws a THROW button. README chip: Toshiba "7790" (mask). | | |

Sources:
- https://handheldempire.com/game.jsp?game=2649 - Bandai, 1984, Japan, model blank
- http://www.electronicplastic.com/game/?company=&id=93&skip=&filter=&search= - Bandai, 1984, buttons + dial
- https://www.youtube.com/watch?v=4wqZxO_0B1E - JP title パワーフィッシング バンダイ … 1984 (title read via oEmbed), tilt/sensor casting
- https://en.todocoleccion.net/video-games-and-consoles/power-fishing-lsi-game-watch-bandai-1984~x210343931 - "Power fishing LSI game watch Bandai 1984", boxed
- https://junksave.com/item/bandai-power-fishing-handheld-game-original-1980s-new - swing-forward casting (seller claim)


#### `SpicaDArtagnan` – D'Artagnan

**Final:** name **D'Artagnan** (High); group **Spica** (High); date **1985-01-01** (Low); model **SH-002B** (Medium); clone of **none** (Medium); keys: `btnJump`→`UP|A`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **D'Artagnan** | Medium | The BrickEmuPy README and the Retro Handheld Games entry both use it. The face render shows only "SPICA" and the button labels, with no title (it may be on the box or the back). |
| manufacturer | Spica (group: new **Spica**) | High | The face render prints "SPICA" in large letters. RetroHandheldGames lists the publisher as Spica. The country is unknown. |
| date | 1985-01-01 (guess) | Low | No source dates D'Artagnan. A sister unit in the same SH-002 family, Spica UFO SH-002C, is dated 1985 on RetroHandheldGames, so 1985 is an inference. |
| region / variant | n/a | | |
| model | SH-002B | Medium | The README says SH-002B. RetroHandheldGames says "SH-0020", which is probably a misreading. Sister Spica units are SH-002C (UFO, 1985) and SH-002F (UFO), so a letter-suffix scheme is likely and supports SH-002B. |
| clone_of | none | Medium | No reskins were found. |
| keys | `{"btnJump": "UP|A"}` | Medium | The face render shows a left vertical rocker (up/down arrows) labelled JUMP/START, a round button and a right horizontal L/R rocker. BrickEmuPy hotkeys put Jump on W/Up and Down on S/Down, so Jump is the rocker's up. Mapping it to UP too makes it a d-pad. Keep Start=PLUS. |
| notes | Vertical clamshell ("Flip and Clasp" series on RHG) with 2 screens, white case, 2×LR44. The art shows anthropomorphic dog musketeers. They look like the anime *Dogtanian and the Three Muskehounds* (JP *Wanwan Sanjūshi*, 1981-82), but I found no proof that it was licensed (Low, inference). The ROM dump is by Milan Galcik (brick `credits`). Chip: Toshiba T7717S. | | |

Sources:
- https://www.retrohandheldgames.com/spica/dartagnan-2850 - D'Artagnan, Spica, "SH-0020", Flip and Clasp, 2 screens, year unknown
- https://www.retrohandheldgames.com/spica/ufo-182 - Spica UFO SH-002C, 1985
- https://www.retrohandheldgames.com/spica/ufo-183 - Spica UFO SH-002F
- https://www.retrohandheldgames.com/spica - Spica catalogue (4 items, only date 1985)
- https://github.com/azya52/BrickEmuPy/blob/main/README.md - "Spica D'Artagnan SH-002B (Toshiba T7717S)"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| country | Unknown | **Made in Taiwan** (High) | Unit back moulding in the RHG photo: "BATTERY: A76X2 / RATING: DC3V-0.00018W / MADE IN TAIWAN / FABRIQUE A TAIWAN". The French text points to a French-speaking market (France/Canada), but that is inference. |
| name | D'Artagnan (Medium) | **D'Artagnan (High)** | Front lid art prints "D'ARTAGNAN" in red, with a small model number before it. The case prints SPICA. |
| model | SH-002B (Medium) | SH-002B (Medium, conflict noted) | The lid prints "SH-002x" before the title. In the low-res RHG photo the last character reads as "0" or "D", not clearly "B", which explains RHG's "SH-0020". The README's SH-002B (the dumper's unit) is kept. Sibling Spica units are SH-002C (UFO, 1985) and SH-002F, and LastDodo lists "Go Home SH-002 (1985)". |
| date | 1985 (Low) | 1985 (Low) - no change | No dated source. LastDodo (search snippet) dates the sibling "Go Home SH-002" to 1985, which agrees with RHG UFO SH-002C = 1985. Two sibling datings now support the inference. |
| licensing | Dogtanian? (Low) | **Characters are Dogtanian (D'Artacán / ワンワン三銃士) designs** (Medium, visual). Licence: Unknown, probably unlicensed | The lid shows a dog musketeer on a white horse; the case shows a red-haired female dog (Juliette) and other dog heads matching the 1981 BRB / Nippon Animation series. No licence credit appears on the lid, case or back, and a Taiwan-made unit with no © line suggests it was unlicensed. |
| manufacturer | Spica (High) | Spica (High). Origin note added | There is a Singapore "Spica Co. Ltd." (trademark registered by Gan & Beng Electronics, Singapore, 1988), known for NES bootlegs. The bootleg-games wiki says it started in 1986, which is after a 1985 LCD game, so **I could not link it to this Taiwan-made Spica LCD line** (Unknown). |

Sources:
- https://www.retrohandheldgames.com/spica/dartagnan-2850 - photos (downloaded): lid "SH-002? D'ARTAGNAN", back "MADE IN TAIWAN / FABRIQUE A TAIWAN", A76x2
- https://www.retrohandheldgames.com/spica/ufo-182 - SH-002C, 1985
- https://www.lastdodo.com/en/items/6850625-go-home-sh-002 - (search-result title only; page blocked) "Go Home SH-002 (1985)"
- https://bootleggames.fandom.com/wiki/Spica - Singapore Spica (Gan & Beng), from 1986, trademark 1988 (read via the MediaWiki API)


#### `TomJerryPrank` – Tom & Jerry Prank

**Final:** name **Tom & Jerry Prank** (High); group **Gakken** (High); date **1983-01-01** (High); model **81751** (Medium); clone of **none** (High); keys: `btnGameA`→`LEFT|Y`, `btnGameB`→`RIGHT|A`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tom & Jerry Prank**. Series prefix on the unit: "Gakken LCD CARD GAME". | High | The face render prints the "Tom & Jerry" logo with "Prank" below, plus "Gakken LCD CARD GAME". Mini-Arcade, Electronic Plastic and RetroHandheldGames agree on the title. |
| manufacturer | Gakken (group: new **Gakken**) | High | Face render plus 3 databases. Licensed from MGM/UA: a listing quotes the box credit "Gakken MGM UA 1983". |
| date | 1983-01-01 | High (year) | Mini-Arcade, Electronic Plastic, Handheld Empire and RHG all give 1983. The month is unknown. This is the Japanese release. |
| region / variant | n/a (Japanese Gakken unit) | Medium | |
| model | 81751 | Medium | Handheld Empire and RetroHandheldGames both give 81751 (RHG seems to mirror HE data, so it is effectively one source). |
| clone_of | none | High | |
| keys | `{"btnGameA": "LEFT|Y", "btnGameB": "RIGHT|A"}` | Medium | GAME A and GAME B are the two big buttons under the screen. Each has a rotation arrow printed under it, because they turn Tom's arm left and right during play as well as starting the games. BrickEmuPy hotkeys are GameA = A/Left and GameB = D/Right. Auto puts them on PLUS and MINUS, which is unplayable. Time=R, Alarm=L and ACL=PLUS+MINUS are fine. |
| notes | Game A: Tom's rotating arm catches Jerry and must avoid the traps. Game B: a "lucky number" fortune game where you stop 3 digits slot-style. The unit is also a clock with alarm (TIME / ALARM / ACL). Sister units: Tom & Jerry Popper and Tom & Jerry Pinball (Gakken 1983). RHG lists a French Lansay "Tom & Jerry les Pièges et l'Horoscope" in the same series ("traps and horoscope"), which is probably this game distributed by Lansay in France (Low, inference from the title). HE/RHG call the series "Cornerless card", but the unit prints "LCD CARD GAME". The ROM dump is by Milan Galcik. Chip: Toshiba 6909. | | |

Sources:
- https://miniarcade.com/gakken/tomjerryprank.php - Gakken 1983, gameplay, lucky number mode
- https://www.electronicplastic.com/game/?company=&id=300&skip=&filter=lcd&search=year:1983 - Gakken 1983, clock feature
- http://handheldempire.com/game.jsp?game=862 - model 81751, 1983, Game A/B description
- https://www.retrohandheldgames.com/gakken/tom-jerry-prank-305 - model 81751, related Lansay titles
- https://www.echtvintage.nl/product/tom-jerry-prank-lcd-game-gakken-alarm-clock-1983-new-old-stock/ - Gakken 1983 NOS, alarm clock
- https://www.worthpoint.com/worthopedia/vintage-tom-jerry-popper-gakken-lcd-1872461637 - (search snippet) sister title Popper, "Gakken MGM UA 1983" box credit


### 6502 family (Sunplus)

#### `Apollo126in1B0202` – 126 in 1 (B0202)

**Final:** name **126 in 1 (B0202)** (Medium); group **Apollo** (High); date **1995-01-01** (Medium); model **B0202** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **126 in 1 (B0202)**. Full name as described: "Apollo 126 in 1 English Talking (B0202)". The menu would show "126 in 1 (B0202)" or "126 in 1 English Talking". | Medium | README: "Apollo 126 in 1 B0202". The habr reverse-engineering article calls it "Apollo 126 in 1 English Talking (B0202)". The face render shows only the "Apollo" logo and button labels, with no game count. I kept the B-code because Apollo sold many "N in 1" variants (for example 18 in 1 B0311, Russian talking). |
| manufacturer | Apollo (group: new **Apollo**) | High | The face render prints the "Apollo" script logo twice. habr and the README agree. |
| date | 1995-01-01 | Medium | habr: the author's unit "was released in April 1995" (production date of one unit). I used the year only, because that is a unit date rather than a launch date. |
| region / variant | English Talking (speech in English) | Medium | habr. |
| model | B0202 | High | README, habr, and the grayhex/ApolloBrickGames PR. |
| clone_of | none | High | Its own Apollo firmware. It shares speech sample blocks with Apollo18in1B0302 (same family), but the game sets differ. |
| keys | keep auto | High | The face prints LEFT/HEIGHT, RIGHT/SPEED, DOWN/MODE, ROTATE/DIRECTION, ON/OFF, MUTE, START/P, RESET. The auto map (Rotate=A, Start/P=PLUS, On/Off=MINUS, Mute=R, Reset=PLUS+MINUS) is sensible. |
| notes | Black brick. Sunplus SPL02 die marked PA2071-160 under epoxy. 4-bit PCM speech (the voice says "Apollo"). Games include Tetris variants, racing/shooting, a Frogger-like game and a slot machine. Brand context: Apollo is the best-known 1990s brick-game brand. Hard Drop wiki says Apollo's black bricks used custom ROMs with an "APOLLO" title screen and that late-1990s "Appollo" spellings may be fakes. Bojogá (BR) calls Apollo "an American import company" with a 1989 launch, which conflicts with the habr dating and is unverified. Wikipedia's History of Nintendo mentions a Hong Kong "Apollo Ltd." raided in 2000 for Nintendo piracy, but I could not tie it to this brand. | | |

Sources:
- https://habr.com/ru/articles/955488/ - "Apollo 126 in 1 English Talking (B0202)", April 1995 unit, SPL02/PA2071-160, speech
- https://github.com/grayhex/ApolloBrickGames/pull/44 - B0202 = black Apollo unit, 126 positions
- https://github.com/azya52/BrickEmuPy/blob/main/README.md - README entry
- https://www.youtube.com/watch?v=rOtHSD-Q54E - "Обзор тетриса Apollo 126 in 1" (title via oEmbed)
- https://harddrop.com/wiki/Brick_Game - Apollo brand context (search snippet)
- https://bojoga.com.br/acervo/brinquedos-eletronicos/brick-game-132-em-1/ - conflicting "American importer, 1989" claim


#### `Apollo18in1B0302` – 18 in 1 (B0302)

**Final:** name **18 in 1 (B0302)** (Medium); group **Apollo** (High); date **1995-01-01** (Medium); model **B0302** (High); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **18 in 1 (B0302)**. Retail description: "Apollo 18-in-1 English Talking Bricks Game Console B0302". | Medium | README "Apollo 18 in 1 B0302". The habr article covers "18 в 1 B0302". A Pinterest pin title (search result) reads "Retro APOLLO Portable 18-in-1 English Talking Bricks Game Console B0302". The face render shows only the Apollo logo. Keep the B-code: Apollo also sold an "18 in 1 B0311 Russian talking". |
| manufacturer | Apollo (group: **Apollo**) | High | Face render logo, README, habr. |
| date | 1995-01-01 (guess) | Low | Undated. It is the same family as B0202 (1995 unit), with the same case, near-identical pinout and shared speech-sample data, so 1995 is an inference. |
| region / variant | English Talking | Low-Medium | Pinterest title only. |
| model | B0302 | High | README, habr. |
| clone_of | none | Medium | Different chip (SPL03) and game set from B0202. |
| keys | keep auto | High | Same face and labels as B0202. |
| notes | Sunplus SPL03 (inferred by habr from the ~11.5 KB ROM size). The habr author used this unit for a "Bad Apple!!" demo. The habr author suggests that the "02"/"03" in the B-code index the Sunplus chip (SPL02/SPL03). | | |

Sources:
- https://habr.com/ru/articles/955488/ - B0302 hardware, SPL03 inference
- https://in.pinterest.com/pin/histria--822329213205564029/ - (search-result title) "18-in-1 English Talking … B0302" (page body not retrievable)
- https://www.youtube.com/watch?v=Wtnh27Eq7aY - (search result) "Apollo 18 in 1 B0311 Russian Talking", a different variant
- https://emulation.gametechwiki.com/index.php/Brick_Game - (search snippet) lists Apollo 18 in 1 B0302, year unknown

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1995-01-01 (Low) | **1995-01-01 (Medium)** | The habr B0302 main-board photo shows the silkscreen **"NYG-95"** on the PCB, a board revision code that most likely carries the year 1995. Sibling B0202 is a unit dated April 1995. |

Sources:
- https://habr.com/ru/articles/955488/ - B0302 PCB photo (https://habrastorage.org/r/w1560/getpro/habr/upload_files/ad8/98a/43c/ad898a43cf11e82e0c334f8ac6da530f.jpg), silkscreen "NYG-95"


#### `Apollo2in1` – 2 in 1

**Final:** name **2 in 1** (Medium); group **Apollo** (Medium); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Low); keys: `btnMode`→`Y`, `btnExit`→`B`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **2 in 1** (virtual pet). Russian collectors call it "Тамагочи Аполло 2 в 1". | Medium | The face render prints only "2 IN 1" on the roof of a house drawing on an egg shell, with no brand. README: "Apollo 2 in 1 virtual pet". A Russian video review is titled "Обзор на Тамагочи Аполло 2 в 1". |
| manufacturer | Apollo (group: **Apollo**) | Medium | README plus collector usage. No Apollo logo is drawn on the face SVG, so it may be on the back or packaging. |
| date | 1997-01-01 (guess) | Low | Generic late-1990s Tamagotchi-clone era. The virtual-pets wiki says the Apollo pets are "late 1990s". No dated source. |
| region / variant | n/a | | |
| model | Unknown | Unknown | |
| clone_of | none | Low | Possible relation: the virtual-pets wiki says Apollo 8 in 1 ("Magic 8 Pets", egg with house doodle, 4 buttons, MODE leftmost) has a "99 in 1 (sometimes labeled as '2 in 1')" variant. This face is also an egg with a house doodle and 4 buttons, so it may be that variant (Low). Neither of those is in the list. |
| keys | `{"btnMode": "Y", "btnExit": "B"}` (optional) | Low | Face order, left to right: MODE, ENTER, EXIT, REV. Auto puts Mode on MINUS and Exit on PLUS, which is awkward for a pet where Mode cycles the menu all the time. The suggestion gives Y = Mode, A = Enter, B = Exit, X = Rev (all face buttons). |
| notes | **File bug:** `Apollo2in1.brick` points to `./assets/Apollo2in1.bin`, but the file on disk is `apollo2in1.bin` (lowercase). Loading fails on case-sensitive filesystems, so rename it when importing. Icons: food, scale/status, racket (game), nurse (medicine), book (discipline), broom; clock, seal/walk?, sun (weather), bulb (light), ?, and a toilet/link icon. | | |

Sources:
- https://github.com/azya52/BrickEmuPy/blob/main/README.md - "Apollo 2 in 1 virtual pet"
- https://yandex.ru/video/preview/16700142354299867404 - (search result) "Обзор на Тамагочи Аполло 2 в 1"
- https://virtual-pets.fandom.com/wiki/Apollo_8_in_1 - (read via the wiki API) Apollo 8 in 1 egg/house design, "2 in 1"-labelled variant
- https://virtual-pets.fandom.com/wiki/Apollo_3_in_1 - (wiki API) Apollo pets "Late 1990s"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date / model | 1997 (Low) / Unknown | no change | Tried: an EN/RU search for Apollo "2 in 1" box/year, the virtual-pets wiki search API (it has only Apollo 3/8 in 1 pages; the 3 in 1 page says "Late 1990s"), and a scan of the ROM for plain or shifted ASCII credits (none, unlike Elfin Twins). |
| clone_of note | Low | (Low, but corroborated) | A Russian collector list (willianna tumblr) says "Apollo 2 in 1 - the same as 99 in 1". This matches the virtual-pets wiki note that the 99 in 1 is "sometimes labelled 2 in 1" and is the 8 in 1 / Magic 8 Pets family. Neither model is in the list. |

Sources:
- https://willianna.tumblr.com/tamagotchi - "Apollo 2 in 1 ... тот же, что и 99 in 1"


#### `ApolloPrince&Princess_ch` – Prince & Princess (Chinese)

**Final:** name **Prince & Princess (Chinese)** (Medium); group **Apollo** (Medium); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: `btnMode`→`B|MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Prince & Princess (Chinese)**. Russian collectors call it "Тамагочи Аполло Принц и Принцесса". The handoff's README line "Apollo 2 in 1 virtual pet" is wrong: the real README has no separate Prince & Princess entry, and its single "Apollo 2 in 1 virtual pet" line belongs to `Apollo2in1`. | Medium | Several Russian collector videos and wishlists are titled "Tamagotchi Apollo Prince and Princess" (YouTube by SavitarSvit, Rutube review, myWishBoard/mywishlist). A fan site writes "Apollo Prince&Princesse". The face render shows no text except MODE / ◀ / ▶ / ENTER, with a castle drawing. |
| manufacturer | Apollo (group: **Apollo**) | Medium | Collector titles plus the brick filename. No brand on the face render. |
| date | 1997-01-01 (guess) | Low | Collector titles only say "из 90х" (from the 90s). Generic Tamagotchi-clone era. |
| region / variant | **Chinese** language setting | High (mechanism) / Medium (label) | Both configs load the same ROM (`ApolloPrince&Princess.bin`) and differ only in `port_pullup` PA: _ch = 62 (0b111110), _en = 60 (0b111100). The buttons sit on PA2-PA5, so PA1 is a spare input used as a factory language strap (pulled up = Chinese, grounded = English). The language names come from the filenames. |
| model | Unknown | Unknown | |
| clone_of | none | Medium | Same ROM as `_en`. See the en entry. |
| keys | `{"btnMode": "B|MINUS"}` (optional) | Low | Face order: MODE, ◀, ▶, ENTER. Auto (LEFT/RIGHT/A, Mode=MINUS) works. Adding B gives a natural face-button pair (B = Mode, A = Enter). |
| notes | Same-ROM pair: handoff Decision 5. Icons: scale (status), knife and fork (food), spoon/racket (game?), first-aid box (medicine), lips + book (discipline/education); broom, sun/rain (weather), bath, lamp (light), squawking bird (attention). | | |

Sources:
- https://www.youtube.com/watch?v=KA7UQ8pQj3s - "Тамагочи Apollo Prince and Princess из 90х." (title via oEmbed)
- https://rutube.ru/video/d3e1ef5dc3b2a674105dac0ae03f0118/ - "Tamagotchi Apollo Prince and Princess", "legendary 90s tamagotchi from Apollo"
- https://mywishboard.com/wish/tamagoci-apollo-8in1-3in-4794962?lang=ru - (search result) "Тамагочи Apollo 8in1, 3in1, Superpet, Принц и принцесса"
- https://batareikin.wixsite.com/tamagotchifanclub - "Apollo Prince&Princesse" keyword

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date / model / brand | 1997 (Low) / Unknown / Apollo (Medium) | no change | Tried: an RU search ("Принц и Принцесса" год выпуска), an EN search, the virtual-pets wiki API (no page) and a ROM string scan (nothing). The collector list above names it "Apollo Prince and Princess" under Apollo, which supports the brand but adds no date. |

Sources:
- https://willianna.tumblr.com/tamagotchi - lists "Apollo Prince and Princess" among Apollo pets (wanted list)


#### `ApolloPrince&Princess_en` – Prince & Princess (English)

**Final:** name **Prince & Princess (English)** (Medium); group **Apollo** (Medium); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Low); keys: `btnMode`→`B|MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Prince & Princess (English)** | Medium | As for _ch. |
| manufacturer | Apollo (group: **Apollo**) | Medium | As for _ch. |
| date | 1997-01-01 (guess) | Low | As for _ch. |
| region / variant | **English** language setting (PA1 strap grounded, `port_pullup` PA = 60) | High (mechanism) / Medium (label) | Same ROM as _ch. |
| model | Unknown | Unknown | |
| clone_of | none (or `ApolloPrince&Princess_ch` if one entry is to be marked as the variant) | Low | Same ROM, so neither is a reskin of the other. If the menu files variants under one parent, use _ch as the parent, since Apollo pets were Chinese-made. |
| keys | `{"btnMode": "B|MINUS"}` (optional) | Low | As for _ch. |
| notes | Russian-market units, the ones shown in collector videos, would be the English setting. That is an inference and I did not verify it. | | |

Sources: as for `ApolloPrince&Princess_ch`.

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date / model / brand | 1997 (Low) / Unknown / Apollo (Medium) | no change | Tried: an RU search ("Принц и Принцесса" год выпуска), an EN search, the virtual-pets wiki API (no page) and a ROM string scan (nothing). The collector list above names it "Apollo Prince and Princess" under Apollo, which supports the brand but adds no date. |

Sources:
- https://willianna.tumblr.com/tamagotchi - lists "Apollo Prince and Princess" among Apollo pets (wanted list)


#### `BigCat9in1` – Big Cat 9 in 1

**Final:** name **Big Cat 9 in 1** (Medium); group **Unbranded** (Low); date **1998-01-01** (Low); model **–** (Unknown); clone of **Gyaoppi9in1** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Big Cat 9 in 1** (README spelling "Big Cat 9-in-1") | Medium | The face render prints only "9 in1" on a tiger-head shell, with buttons A/B/C/D. A TamaTalk collector thread names it "Big Cat 9 in 1" (search snippet, page 403). |
| manufacturer | Unknown / unbranded (group: **Unbranded**, or file it under the Super Gyaoppi maker U.I.) | Low | No brand on the face. It is a derivative of U.I.'s Super Gyaoppi (see clone_of). |
| date | 1998-01-01 (guess) | Low | It must postdate Super Gyaoppi (Gyaoppi 1997, Super Gyaoppi later). No dated source. |
| region / variant | n/a | | |
| model | Unknown | Unknown | |
| clone_of | **Gyaoppi9in1** | High | ROM comparison: 72 % of bytes are identical at the same offsets, and 310 of 512 64-byte blocks are shared with `Gyaoppi9in1.bin`. The same shifted-text tables appear at matching offsets. The TamaTalk thread describes it as "an oversized version of the Super Gyaoppi". Differences: a 4th button (D) and a different `port_pullup` (222 vs 255). |
| keys | keep auto | Medium | The face has D (top-left), A (bottom-left), C (top-right), B (bottom-right). The auto map A=A, B=B, C=X, D=Y matches the letters. |
| notes | Super Gyaoppi clones are known to add a 10th pet (horse), per the virtual-pets wiki. This could be one of those, but that is unverified. | | |

Sources:
- https://www.tamatalk.com/threads/identification-help-with-lots-of-different-virtual-pets.195025/ - (search snippet; page 403) "Big Cat 9 in 1", oversized Super Gyaoppi
- https://virtual-pets.fandom.com/wiki/Gyaoppi - (wiki API) Gyaoppi by U.I., 1997, Super Gyaoppi 9 pets, clones adding a horse
- Local ROM comparison (BrickEmuPy assets `BigCat9in1.bin` vs `Gyaoppi9in1.bin`)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | Unbranded (Low) | no change. Note: the parent design is Nature Land / U.I. (see Gyaoppi9in1) | Tried: an EN search for "Big Cat 9 in 1" (no hits besides TamaTalk). The TamaTalk thread is behind Cloudflare and the archive.org copy was not reachable. |
| date | 1998 (Low) | no change | |


#### `ElfinTwins` – Elfin Twins

**Final:** name **Elfin Twins** (High); group **Sparkle Crystal** (Medium); date **1997-01-01** (Low); model **GM-021** (High); clone of **none** (High); keys: `btnEsc`→`B|MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Elfin Twins** | High | The face render prints the "ELFIN TWINS" logo and "GM-021". README and the codanric GBA port agree. |
| manufacturer | Sparkle Crystal Development Co., Ltd. (group: new **Sparkle Crystal**, or **Unbranded**) | Medium | The ROM signature (verified locally: ASCII stored +0x20 at 0x7F27) reads "Sparkle Crystal Development Co.,Ltd. WangLaXian 1996.11.16". That names the developer, not necessarily the brand. No brand is printed on the face. Nothing else is known about the company. |
| date | 1997-01-01 (guess). 1996 is also possible. | Low | The ROM build date is 1996-11-16, so release came then or later. The codanric repo says "from 1996", but its only evidence is that signature. 1997 is more likely for retail. |
| region / variant | n/a | | |
| model | GM-021 | High | Printed on the face (render). README agrees. |
| clone_of | none | High | Unique ROM (about 4 % similarity with the other SPLB20 pets). |
| keys | `{"btnEsc": "B|MINUS"}` | Medium | The face prints ESC, ENTER and CLOCK plus a 4-way pad. Auto: Enter=A, Esc=MINUS, Clock=R. ESC is a primary back button, so B is better, and the codanric GBA port also uses ESC=B. Keep Clock=R. Esc+Enter = rescue / new pair of twins (codanric). |
| notes | The pad's Up and Down are chords of the Left/Right/Clock lines (`btnUp` mask 6, `btnDown` mask 12), as on the real pad. A "SCD TEST" name unlocks a test mode (codanric). There is a fan GBA port with a rebuilt manual and disassembly. Sunplus SPLB20. | | |

Sources:
- https://github.com/codanric/elfin-twins-gba - ROM signature, 1996, button roles, reconstructed manual
- https://github.com/codanric/elfin-twins-gba/releases/tag/v1.2 - port release (search result)
- Local ROM scan (`ElfinTwins.bin` offset 0x7F27, bytes −0x20 → "Sparkle Crystal Development Co.,Ltd. WangLaXian 1996.11.16")

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| brand / date | Sparkle Crystal (Medium) / 1997 (Low) | no change | Tried: searches for "Elfin Twins" GM-021, "Elfin Twins" 1997, and "Sparkle Crystal Development" Co Ltd (no company record). The codanric README (fetched raw) says "from 1996" (based only on the ROM signature) and "We couldn't find anything else about the company"; its toy picture is a render with no brand. Keep 1997 Low (ROM signed 1996-11-16). 1996-12 or 1997 are both possible. |

Sources:
- https://raw.githubusercontent.com/codanric/elfin-twins-gba/main/README.md - "small virtual pet from 1996", "couldn't find anything else about the company", SCD TEST mode


#### `Gyaoppi9in1` – Super Gyaoppi 9 in 1

**Final:** name **Super Gyaoppi 9 in 1** (High); group **Nature Land** (High); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: `btnA`→`LEFT|Y`, `btnB`→`A|UP|DOWN`, `btnC`→`RIGHT|B`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Super Gyaoppi 9 in 1**. The face shows "SUPER GYAOPPI" with "9 IN 1" below it. The Japanese spelling is スーパーぎゃおッPi (read "gyaoppi" or "gyaoppī"). Some boxes also say "28 Bit!". | High (face) | Face SVG; JP auction title "スーパーぎゃおッPi 9種のペット"; enpedia and dailyportalz spell it ぎゃおッPi |
| manufacturer | **U.I.**, a Japanese company. Its name is printed on the back of the shell but not on the box, which carries no maker. The unit was made in China. One eBay seller uses "Nature Land" as the brand. Suggested group: new **"U.I."**, or a generic "Unbranded" pets group. | Low | One fan wiki and one review name "U.I.". Japanese sources say the maker is unknown (メーカー不明), and dailyportalz notes the package names no maker. |
| date | 1997-01-01 | Low | BrickEmuPy README says 1997, and the wiki says the Gyaoppi line started in 1997. Super Gyaoppi is the third model (after Gyaoppi and NEW! Gyaoppi II), and the Pixel Petting Zoo review says its unit was "Produced: 1998". **1998 is equally plausible.** |
| region / variant | n/a (Japan-market Tamagotchi knock-off) | Medium | |
| model | none found | Unknown | No model number in any listing or review |
| clone_of | none | Medium | **Related:** the first 0x5800 bytes (22 KB, the program code) of `Gyaoppi9in1.bin` are byte-identical to `BigCat9in1.bin`; the data area after that differs. So Big Cat 9-in-1 is a reskin or derivative built on the same software (see Batch-level findings). The wiki also mentions Super Gyaoppi clones that add a 10th pet (a horse). |
| keys | Three unlabelled buttons that work like a Tamagotchi's: select, confirm, cancel. The auto mapping is A=A, B=B, C=X. **Optional** override to match the Tamagotchi mapping: `{"btnA":"LEFT\|Y","btnB":"A\|UP\|DOWN","btnC":"RIGHT\|B"}` | Low | Face SVG; button roles from Pixel Petting Zoo (menu, confirm, cancel) |
| notes | A Tamagotchi knock-off from the 1997 craze (dailyportalz: "パクリであると問題になった", i.e. criticised as a copy). Nine pets: cat, dog, dinosaur, duck, alien, frog, penguin, bird, panda. It adds a weather-care mechanic. Sold as a capsule-toy (gachapon) prize and in toy shops. | | |

Sources:
- https://virtual-pets.fandom.com/wiki/Gyaoppi - U.I. as maker, 1997, models (Gyaoppi, NEW! Gyaoppi II, Super Gyaoppi with 9 pets), clones with a horse (read via the MediaWiki API)
- http://pixelpettingzoo.blogspot.com/2010/07/super-gyaoppi-9-in-1-review.html - maker U.I., "Produced: 1998", Japan (made in China), pet list
- https://enpedia.org/wiki/%E3%81%8E%E3%82%83%E3%81%8A%E3%83%83Pi - spelling ぎゃおッPi, maker unknown
- https://dailyportalz.jp/kiji/161110198017 - no maker printed on the package; Tamagotchi copy
- https://aucview.aucfan.com/yahoo/g133911470/ - auction titled "スーパーぎゃおッPi 9種のペット" (seen in search results only)
- https://www.ebay.com/itm/297188260909 - listing titled "Nature Land Super Gyaoppi 9-in-1" (search result title only)
- https://www.ebay.com/itm/167662193128 - "Super Gyaoppi 28 Bit! 9 In 1" (search result title only)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | U.I. (Low) | **Brand: Nature Land® (distributor/brand on box). Designer: "U.I. LINGO & SUZUKI". Shell: "U.I. / MADE IN CHINA"** (High) | Box back photo: "DESIGNED BY U.I. LINGO & SUZUKI", the "Nature Land®" logo and "MADE IN CHINA". Back of the unit (another listing): "U.I. MADE IN CHINA". This settles the earlier "U.I." vs "Nature Land" conflict: both appear, in different roles. Suggested menu group: **"Nature Land"** (brand on the box), with U.I. noted as designer. |
| name | Super Gyaoppi 9 in 1 (High) | no change. Box detail added | Box front: "スーパーぎゃおッPi 28ビット!" with "9 in 1". The side panel reads "デジタルペッツ 自然をたいせつに". Box back: "9しゅるいの赤ちゃんをそだててネ!". |
| date | 1997 (Low) | no change (Low; 1997 vs 1998 unresolved) | The box shows no copyright year. Seller descriptions say only "1990年代後半". Tried: Japanese searches (ぎゃおッPi 発売 1997/1998), J-town, nicovideo (403), Rakuten (blocked). |

Sources:
- https://auctions.yahoo.co.jp/jp/auction/n1234862033 - boxed new "スーパーぎゃおッPi 28ビット!"; box back "DESIGNED BY U.I. LINGO & SUZUKI", "Nature Land®", MADE IN CHINA (photos read)
- https://auctions.yahoo.co.jp/jp/auction/u1233196028 - metallic-blue unit; back moulding "U.I. MADE IN CHINA"; "1990年代後半"
- https://j-town.net/2017/03/01239767.html?p=all - original Gyaoppi 1997, no maker printed on that unit


#### `Tamagotchi_v3_eu` – Tamagotchi Connexion V3 (Europe)

**Final:** name **Tamagotchi Connexion V3 (Europe)** (High); group **Bandai** (High); date **2006-04-01** (Medium); model **GPZ06331** (Low); clone of **Tamagotchi_v3_us** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tamagotchi Connexion V3**. The European blister pack shows "TAMAGOTCHI Connexion V3". Proposed menu name: `Tamagotchi Connexion V3 (Europe)`. If the maintainer prefers one spelling across the pair, use `Tamagotchi Connection V3 (Europe)`. | High | EU (Greek/Italian) packaging photo; the franchise wiki says the line was sold as "Tamagotchi Connexion" in Europe |
| manufacturer | Bandai. Distribution varied by country: the photographed Italy/Greece card is by Giochi Preziosi (GP logo), and Bandai S.A. covered other markets. Group: **Bandai** | High | Packaging photo |
| date | 2006-01-01 | Low | No EU-specific month found. A Berlin museum record (museum-digital, via search snippet) dates a "Tamagotchi Connexion Version 3" to 2006 in Germany. Context: the franchise wiki says V4 reached Europe in October 2006, before the US (January 2007), so V3 in Europe sometime in 2006 is consistent. |
| region / variant | Europe. Same ROM as US. Config differs only in PD pull-up (EU 6), which is probably a region strap: D/M date format and grams, by analogy with V1/V2. | Medium | `.brick` diff |
| model | Packaging code **GPZ06331**, specific to Giochi Preziosi in Italy/Greece. Bandai's own European codes were not found. | Low | Readable on the EU packaging photo; the wiki's promo image is named "00 GPZ06331 L.jpg" |
| clone_of | Tamagotchi_v3_us | High | Same ROM `Tamagotchi_v3_us_eu.bin` |
| keys | keep auto | High | Same as US |
| notes | Same dump-quality warning as US. International units shipped with a ball-chain clasp, while US units had a chain-link key ring (search snippet). Card text is "© BANDAI, WIZ 2004-200x" (last digit unreadable). | | |

Sources:
- https://static.wikia.nocookie.net/tamagotchi/images/6/69/ConnectionV3EuropePackaging.png/revision/latest?cb=20230327182837 - EU card "Tamagotchi Connexion V3", GPZ06331, Giochi Preziosi, tamagotchieurope.com
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_(franchise) - "also known as Tamagotchi Connexion in Europe"; V3 2006; V4 October 2006 (EU) / January 2007 (US)
- https://smb.museum-digital.de/pdf/object/99369.pdf?lang=en - Berlin museum object "Tamagotchi Connexion Version 3", 2006, Germany (search snippet only; fetch timed out)
- https://en.wikipedia.org/wiki/Tamagotchi_Connection - "Tamagotchi Connexion outside the Americas"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 2006-01-01 (Low) | **2006-04-01 (Medium)** | Tamagotchi wiki V3 shell list (raw wikitext fetched via the fandom API): the "Europe" table marks every first-wave EU shell (Cow, Tropical Green, Surf Wave, Blue Camo, ...) as "Wave 1 (April 2006)". In the same table the US Wave 1 is February 2006 and Wave 2 is March 2006. A TamaTalk thread from late February 2006 (search snippet only; 403 on fetch) says V3 was already out in the USA, Canada and Australia but "not out in Europe yet ... rumoured March or April". Both point to April 2006 for Europe. No per-country (UK/DE/FR/IT) day-level date was found. The Berlin museum record (2006, Germany) is consistent. |
| notes | - | EU shells carry the vintage-style Tamagotchi logo and a ball chain (shell list "Regional Differences"). | same page |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_3/Shell_list - Europe Wave 1 (April 2006); US Wave 1 Feb 2006, Wave 2 Mar 2006
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_3 - NA release February 2006 (infobox), no EU date
- https://www.tamatalk.com/threads/tamagotchi-connection-version-3.36753/ - Feb 2006 thread: not yet in Europe, March/April rumoured (search snippet only)


#### `Tamagotchi_v3_us` – Tamagotchi Connection V3 (US)

**Final:** name **Tamagotchi Connection V3 (US)** (High); group **Bandai** (High); date **2006-02-01** (Medium); model **Asst. #19400** (Medium); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tamagotchi Connection V3** (US). The box shows "TAMAGOTCHI CONNECTION" with a "V3 Version" badge, and wikis call it "Tamagotchi Connection Version 3". Proposed menu name: `Tamagotchi Connection V3 (US)` | High | US blister-pack photo; US manual |
| manufacturer | Bandai. In the US it was distributed by Bandai America Inc. (Cypress, CA), with "© Bandai, WiZ 2004-2006". Group: **Bandai** | High | US manual on archive.org |
| date | 2006-02-01 | Medium | Tamagotchi wiki infobox: "na release date = February 2006", price $14.99. SoraNews24 and IGDB/NextQuest also say February 2006 (search results). One search snippet gave "February 18, 2006" from an unverified source. |
| region / variant | US / North America. Shares its ROM with `Tamagotchi_v3_eu`. The two configs differ only in `port_pullup` PD (US 2 vs EU 6, i.e. bit 2 / value 4 is pulled up only on EU), which looks like a region-select strap (my inference). TCRF documents that earlier Connection versions differ by region in date format (M/D vs D/M) and weight unit (lb vs g). | Medium | `.brick` diff; TCRF via search snippet |
| model | Box shows "Asst. #19400". Each colour has its own item number in the 194xx range: the pictured yellow box shows "Item #194??" (last digits unreadable), and an eBay listing titles a Pink Ice Cream Cone shell "19435". | Medium | Box photo (assortment number readable); eBay title |
| clone_of | none (parent of `Tamagotchi_v3_eu`) | High | Same ROM |
| keys | keep auto. Left=A (select), Center=B (confirm), Right=C (cancel), the same mapping as TamagotchiP1. | High | Manual uses buttons (A)/(B)/(C) |
| notes | **BrickEmuPy flags this dump as having a "high probability of errors"**, and distributes it without ROM. It is the international counterpart of Japan's Keitai Kaitsuu! Tamagotchi Plus, and the first non-Japan Tamagotchi to link with the TamaTown website. It has 52 characters, runs on one CR2032, and has IR connectivity plus an EEPROM (peripherals beyond basic play). | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Connection_Version_3 - NA release February 2006, $14.99, Keitai counterpart, TamaTown (read via the MediaWiki API)
- https://archive.org/details/tamagotchi-connection-v3-instructions - US manual: "Distributed by Bandai America Incorporated ... TM & © 2006 Bandai", "© Bandai, WiZ 2004-2006", buttons (A)/(B)/(C)
- https://static.wikia.nocookie.net/tamagotchi/images/c/c3/Version_3_box.jpg/revision/latest?cb=20191015141814 - US box photo: "Asst. #19400", "V3 Version", made in China
- https://www.ebay.com/c/1001612912 - "Pink Ice Cream Cone 19435" (search result title)
- https://soranews24.com/2017/05/04/w-t-f-japan-top-5-best-tamagotchi-releases-%e3%80%90weird-top-five%e3%80%91/amp/ - V3 US release February 2006 (search snippet)
- https://www.igdb.com/games/tamagotchi-connection-v3 - debut February 2006 (search snippet)
- https://en.wikipedia.org/wiki/Tamagotchi_Connection - V3 2006; sold as "Connexion" outside the Americas
- https://tcrf.net/Tamagotchi_Connection - regional differences of V1/V2 (search snippets; the page itself returned 403)


#### `TetrisNugget` – Chicken McNugget Tetris

**Final:** name **Chicken McNugget Tetris** (Low); group **McDonald's** (High); date **2023-05-24** (High); model **–** (Unknown); clone of **none** (High); keys: `btnDrop`→`UP`, `btnRotate`→`A|B`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **No official product name.** McDonald's China's press release calls it "麦乐鸡形状的俄罗斯方块游戏机" ("McNugget-shaped Tetris game machine") and "麦乐鸡外形趣味游戏机周边". English press uses "Tetris McNugget" (famouscampaigns), "Chicken McNugget Tetris" (MobileSyrup) and "Chicken Nugget Boy" (RetroDodo). Proposed: `Chicken McNugget Tetris`; alternative `Tetris McNugget`. | Medium (that no name exists) / Low (chosen wording) | Official release; press coverage |
| manufacturer | McDonald's China promotional item, licensed by The Tetris Company and Tetris Holding LLC. The McDonald's arches are printed on the LCD. The OEM/factory is unknown. Group: **McDonald's**, but the LCDonald engine check would need reworking (see Batch-level findings). | High (McDonald's); Unknown (OEM) | Face SVG shows the arches; Gizmodo and MobileSyrup report the license |
| date | 2023-05-24 | High | Official press release dated 2023-05-24 ("5月24日起"). IT之家, Gigazine and RetroDodo all report sales starting 24 May 2023, sold out the same day. |
| region / variant | Mainland China only. Limited to 400,000 units. Customers got it by adding ¥30 to any order, during the McNugget 40th anniversary and Children's Day (六一) promotion (24 May to about 27 June 2023). | High | IT之家, famouscampaigns |
| model | none | Unknown | |
| clone_of | none | High | |
| keys | The face shows a 4-way pad with Drop (hard-drop icon) on top, Left/Right, Down at the bottom, a large round Rotate button, and small mute, power and pause buttons. Auto mapping puts Drop on A and Rotate on B, which is awkward because Drop is physically the D-pad's up button. Proposed: `{"btnDrop":"UP","btnRotate":"A\|B"}` | Medium | Face SVG; RetroDodo: "left, right, down, hard drop (up), and a single rotate action button"; Gigazine: one rotation direction only |
| notes | BrickEmuPy distributes it without ROM. Game: Tetris only, single rotation direction, unlit LCD. The shell is the "boot"-shaped McNugget. | | |

Sources:
- https://www.mcdonalds.com.cn/news/20230524-McNuggets-40-years - official McDonald's China release, 2023-05-24, "麦乐鸡形状的俄罗斯方块游戏机", "靴形" (boot) shape
- https://www.ithome.com/0/695/540.htm - "麦当劳麦乐鸡 40 周年俄罗斯方块掌机", 400,000 units, mainland China only, +¥30, 24 May
- https://gigazine.net/gsc_news/en/20230526-mcdonalds-celebrating-mcnugget-tetris-handheld - sold out 24 May 2023; cross pad, rotate and power buttons; official license notice on the back
- https://retrododo.com/mcdonalds-tetris-handheld/ - 24 May launch, ¥30, button layout
- https://gizmodo.com/tetris-chicken-nugget-mcnugget-handheld-mcdonalds-china-1850475404 - licensed with The Tetris Company and Tetris Holding LLC; arches on the LCD
- https://mobilesyrup.com/2023/05/25/mcdonalds-chicken-mcnugget-tetris/ - partnership with The Tetris Company and Tetris Holding (search snippet)
- https://www.famouscampaigns.com/2023/05/the-tetris-chicken-nugget/ - "Tetris McNugget", 400,000 units


### Holtek 4-bit

#### `E23PlusMarkII96in1` – Brick Game 96 in 1 (E-23 Plus Mark II)

**Final:** name **Brick Game 96 in 1 (E-23 Plus Mark II)** (High); group **Brick Game** (High); date **1993-01-01** (Low); model **E-23 Plus Mark II** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | Printed on the face: "super" (top), "BRICK GAME 96 IN 1" and "E-23 PLUS MARK II". Habr calls it "Super Brick Game E-23 PLUS MARK II 96 IN 1", and MAME calls it "Brick Game 96 in 1 (E-23 Plus Mark II)". Proposed: `Brick Game 96 in 1 (E-23 Plus Mark II)`, following MAME. | High | Face SVG, MAME, habr |
| manufacturer | **E-Star**, a Taiwanese company (habr, MAME). The face also carries a smiley-with-hands logo with the Chinese brand "机之宝". Suggested group: **"Brick Game"** for the generic Chinese and Taiwanese brick games, with E-Star in the notes. Alternatively a new "E-Star" group shared with E-88. | High (E-Star per 2 sources) | habr 773040; MAME `brke23p2` |
| date | 1993-01-01 | Low | MAME `CONS( 1993, brke23p2 ...)` with the comment "some other dieshots have 1996 on them". Habr says the die is one year older than the E-88's (mask 1994). This is a chip/die date, not a confirmed retail date. |
| region / variant | n/a | | |
| model | **E-23 Plus Mark II** (printed "E-23 PLUS MARK II") | High | Face SVG |
| clone_of | none | High | ROM is distinct (8% byte similarity with E-88) |
| keys | keep auto. Face labels: START/PAUSE, ON/OFF, MUTE, LEFT/LEVEL, RIGHT/SPEED, DOWN/MODE, ROTATE/DIRECTION. | High | Face SVG |
| notes | The dumped chip is a Holtek HT-943D0 (HT1190 family), read by visual decap. The ROM SHA1 matches MAME `brke23p2`. Habr article 773040 describes the reverse engineering; it reviews "1 in 9999"-style units and notes that 1990s Taiwan brick games used Holtek 4-bit MCUs. BrickEmuPy's E-88 config reuses this game's sound ROM. | | |

Sources:
- https://habr.com/ru/articles/773040/ - "Super Brick Game E-23 PLUS MARK II 96 IN 1 от тайваньской компании E-Star" (from the Taiwanese company E-Star), HT-943 (HT443A0)
- https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_ht11xx.cpp - "E-Star Brick Game 96 in 1 (E-23 Plus Mark II)", year 1993 (dieshots with 1996 also exist), SHA1 a36213309e6add31f31e4248f02f17de9914a5c1

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1993-01-01 (Low) | 1993-01-01 (Low), **no change** | No retail date found. New context: habr 773040 has a die-photo caption "dies used in Brick Game from 1993, 2003 and 2008", and says this die is "one year older" than the E-88's (1994). That confirms 1993 only as a die/mask year. The Hard Drop wiki says E-Star sold the earliest units "possibly in 1992 or earlier". The E-23 first shipped as a 2-game unit *without* "Plus Mark II", so the 96-in-1 Plus Mark II is a later revision. Copycats appeared c.1993-94, and E-Star's 98/118/154/999-game models followed c.1995-97. A 96-in-1 retail date of 1993-95 is plausible, but 1993 is still only the chip year. arcade-history, planetemu and mdk all just mirror MAME's 1993. |

Sources:
- https://harddrop.com/wiki/Brick_Game - E-Star early models E-23/E-33/E-88 (1/2/8 games); E-23 originally a 2-game version without "Plus Mark II"; copies c.1993-94; later E-Star models c.1995-97
- https://habr.com/ru/articles/773040/ - die caption 1993/2003/2008; Taiwan-made Holtek units early/mid 90s
- https://www.arcade-history.com/?n=brick-game-96-in-1-model-e-23-plus-mark-ii&page=detail&id=384263&o=9 - E-Star, 1993 (MAME-derived)


#### `E88_8in1` – Brick Game 8 in 1 (E-88)

**Final:** name **Brick Game 8 in 1 (E-88)** (High); group **Brick Game** (Medium); date **1994-01-01** (Low); model **E-88** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | Printed on the face: "BRICK GAME 8-IN-1" and "E-88". Proposed: `Brick Game 8 in 1 (E-88)` | High | Face SVG |
| manufacturer | E-Star (Taiwan) is likely: an eBay listing reads "8 in 1 E-88 C E-star", and the face has the same smiley-with-hands logo as the E-23. Group: **"Brick Game"** (or "E-Star") | Low-Medium | One listing title, plus the logo shared with the E-23 |
| date | 1994-01-01 | Low | Habr 767520: the E-88 die is marked HT-943E5 with a 1994 mask date. This is a chip date only. |
| region / variant | n/a | | |
| model | **E-88** | High | Face SVG |
| clone_of | none | High | Distinct ROM |
| keys | keep auto. Face: START/PAUSE, ON/OFF, SOUND, LEFT/HEIGHT, RIGHT/LEVEL, DOWN/MODE, and "UP" printed above ROTATE/DIRECTION. Optional: `{"btnRotate":"A\|UP"}`, since the unit labels the rotate button "UP" too. | Medium | Face SVG |
| notes | Uses the E-23's sound ROM (`sound_rom_path` points to E23PlusMarkII96in1.srom), so the melodies may not be original. Not in MAME. azya's X post announced its addition to BrickEmuPy. eBay sellers also use "Super Brick Game", "Pro-Star" and "E-S88" for similar units. | | |

Sources:
- https://habr.com/ru/articles/767520/ - E-88 die marked HT-943E5, mask 1994
- https://www.ebay.com/itm/397025125785 - listing "Vintage Brick Video Game Handheld 8 in 1 E-88 C E-star" (search result title)
- https://x.com/OneBitOnePixel/status/1803063959936733252 - "A new Brick Game has been added to BrickEmuPy - E-88 8 in 1" (search result)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | Brick Game group, maker E-Star (Low) | Brick Game group, maker **E-Star (Medium)** | Three independent signals: (1) the Hard Drop wiki names "the E-23 and E-33 and E-88" as early E-Star models; (2) several eBay listing titles name E-Star ("8 in 1 E-88 C E-star", other sellers' "E-Stars 8 in 1", per search snippets); (3) the face has the same smiley-with-hands logo as the MAME-attributed E-Star E-23. Variants seen in Russian listings: E-88C, E-S88, E88A "Russian talking". |
| date | 1994-01-01 (Low) | 1994-01-01 (Low), **no change** | Still only the HT-943E5 mask year (habr 767520). No retail date found. The Russian Tetris-museum collector list (ynk.ucoz.ru) lists E-88/E-S88/E 88 variants without years. |

Sources:
- https://harddrop.com/wiki/Brick_Game - E-88 among E-Star early models
- https://www.ebay.com/itm/397025125785 - "8 in 1 E-88 C E-star" (title, search result)
- https://violity.com/en/111429800-igra-brick-game-8-in-1-e-88-c - E-88-C listing (search result)
- https://ynk.ucoz.ru/index/0-57 - Russian collector list: "BRICK GAME 8 in 1(E-88)", "(E-S88)", "(E88А) RUSSIAN TALKING"


#### `GA888` – Block Game & Echo Key

**Final:** name **Block Game & Echo Key** (High); group **Brick Game** (High); date **1996-01-01** (Low); model **GA888** (High); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Block Game & Echo Key** (model GA888). The face shows "GA888" and "BLOCK GAME & ECHO KEY". BrickEmuPy's README entry is "Block Game & Echo Key GA888 (Tetris Jr. clone) (HT-943I0)", so **the handoff's E-88 README line for GA888 is a generator bug.** Proposed: `Block Game & Echo Key` (with model GA888), or `Block Game & Echo Key GA888` to match MAME. | High | Face SVG, BrickEmuPy README, MAME |
| manufacturer | Unbranded and unknown (MAME uses "<unknown>"). Group: **"Brick Game"** | High (that it is unknown) | MAME `ga888` |
| date | 1996-01-01 (placeholder) | Low | MAME year is "199?". Same Holtek HT1130-family chip and the same 8x12 LCD artwork as Gametech's Pikorin 55 (1996). Tetris Jr. (Hiro, BPS-licensed), which this is said to clone, belongs to the same 1996-97 keychain boom. |
| region / variant | n/a | | |
| model | **GA888** | High | Face SVG |
| clone_of | none in the list. Outside the list it is described as a clone of **Tetris Jr.** (BrickEmuPy, and MAME with a question mark). It shares its LCD glass layout with `Keychain55in1` (MAME uses the same SVG for both, CRC 9ab6dd67), but the ROM is different (5% similarity). | Medium | MAME, BrickEmuPy |
| keys | The face has ON/PAUSE (one button), RESET, a 4-way pad (Right also labelled SOUND, Down also labelled MODE, Up has no function per MAME's comment) and START/ROTATE. Auto mapping is fine. Optional for consistency with Keychain55in1's ON/PAUSE: `{"btnPause":"PLUS"}` | Low | Face SVG; MAME comment "the unit also has an up button ... is 'up' connected to anything?" |
| notes | Chip HT-943I0 (Holtek HT1130 family), ROM read by visual decap. SHA1 00b08773e8ee7577377f4d52cd1e6bb657b6f242 matches MAME `ga888`. The "Echo Key" part of the name is unexplained; it may be a key-finder/echo feature. | | |

Sources:
- https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_ht11xx.cpp - "Block Game & Echo Key GA888", "<unknown>", 199?, "clone of Tetris Jr?", HT1130, 8x12 LCD
- https://github.com/azya52/BrickEmuPy - README "Block Game & Echo Key GA888 (Tetris Jr. clone) (HT-943I0)"
- https://www.weblio.jp/content/%E3%83%94%E3%82%B3%E3%83%AA%E3%83%B355 - Tetris Jr. was licensed from BPS and sold by Hiro

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | unknown (High that unknown) | **no change** | Searched EN/RU ("echo key" key-finder, Russian 'брелок тетрис GA888'). The only new hit is a Russian meshok.net listing "Тетрис брелок BLOCK GAME ECHO KEY GA888 Tetris рабочий редкий" (title only; 403 on fetch). That confirms the product name and that it circulated in Russia, but gives no maker. |
| date | 1996-01-01 placeholder (Low) | **no change** (Low) | MAME still has `199?`. The MAME header lists "Pikorin Bros." and "Tetris Mini" as other Gametech keychains possibly on the same HT1130 hardware. |

Sources:
- https://meshok.net/en/item/29988056_%D0%A2%D0%B5%D1%82%D1%80%D0%B8%D1%81_%D0%B1%D1%80%D0%B5%D0%BB%D0%BE%D0%BA_BLOCK_GAME_ECHO_KEY_GA888_Tetris_%D1%80%D0%B0%D0%B1%D0%BE%D1%87%D0%B8%D0%B9_%D1%80%D0%B5%D0%B4%D0%BA%D0%B8%D0%B9 - listing title (search snippet)


#### `Keychain55in1` – Pikorin 55

**Final:** name **Pikorin 55** (High); group **Gametech** (High); date **1996-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Pikorin 55** (ピコリン55). The face logo reads "ピコリン55", and MAME names the same ROM "Pikorin 55 (ピコリン55) - keychain game". The handoff's "Keychain 55 in 1" is a generic name. | High | Face SVG; MAME `piko55` (SHA1 match) |
| manufacturer | **Gametech** (ゲームテック), Japan. Suggested group: new **"Gametech"** | High | MAME; Mercari listing "GAMETECH ... ピコリン55"; weblio |
| date | 1996-01-01 | Low | Pikorin 55 is a retitled Tetrin 55 (Gametech, 1996, winner of the DIME Trend Award that year) with the "Tetrin" screen legend removed. The retitle could be 1996 or 1997; no month found. |
| region / variant | Japan | Medium | |
| model | none found. Original list price was ¥1,430 according to Suruga-ya. | Low | |
| clone_of | none in the list. Outside the list it is the same game as Gametech's **Tetrin 55** (テトリン55), and many unlicensed clones exist. It shares its LCD layout with GA888. | Medium | weblio, middle-edge |
| keys | keep auto. Face: SELECT over the Left button, GAME by the Down button, plus Right, START/ROTATE, ON/PAUSE, SOUND, RESET. | High | Face SVG |
| notes | 55 "games" that are really 4 types: block game, shooting, car race, and Dragon & Ball. Runs on two LR44 cells. Predecessor: Mini Tetrin (8 block-game variants). Chip HT-943I0 (HT1130 family). | | |

Sources:
- https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_ht11xx.cpp - "Pikorin 55 (ピコリン55) - keychain game", Gametech, ROM keychain55in1.bin SHA1 27fe405a8a866bfc6a857af886ed00f64083c2cc
- https://www.weblio.jp/content/%E3%83%94%E3%82%B3%E3%83%AA%E3%83%B355 - Tetrin 55: Gametech, 1996, 55 games (4 real types), LR44 x2; Pikorin 55 is a retitle with identical content
- https://middle-edge-neo.jp/articles/8012/ - same; Mini Tetrin predecessor
- https://en.wikipedia.org/wiki/List_of_Japanese_handheld_electronic_games - "Tetrin 55, Gametech, 1996"
- https://www.mercari.com/jp/items/m84112495907/ - "GAMETECH 携帯液晶ミニゲーム機 ピコリン55" (search snippet)
- https://www.suruga-ya.jp/kaitori/kaitori_detail/148000546 - ピコリン55, list price ¥1,430 (search snippet)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1996-01-01 (Low) | 1996-01-01 (Low), **no change** | Tried JA Wikipedia テトリン55 (Tetrin 55: "1996年", no month; no date for the Pikorin retitle), weblio, Yahoo! Auctions lot c1028294425 (no printed date in the text), a Gametech history search and the 2020 Tetris Mini press (denfaminicogamer: the boom was in 1996, no product names). New context: JA Wikipedia's "テトリス" article (search snippet) says Gametech's devices were unlicensed and similarly named, the Tetris rights holder disputed the trademark, and Gametech **renamed テトリン55 to ピコリン55 and kept making and selling it**. A middle-edge article says the mini-Tetrin boom was already cooling by November 1996. The retitle therefore falls in late 1996 or 1997; neither year can be confirmed. MAME also gives `199?` for piko55. |

Sources:
- https://ja.wikipedia.org/wiki/%E3%83%86%E3%83%88%E3%83%AA%E3%83%B355 - Tetrin 55, Gametech, 1996 (year only)
- https://ja.wikipedia.org/wiki/%E3%83%86%E3%83%88%E3%83%AA%E3%82%B9 - trademark dispute led to the Tetrin 55 → Pikorin 55 rename (search snippet)
- https://auctions.yahoo.co.jp/jp/auction/c1028294425 - "ピコリン５５ テトリン テトリス" lot (no date info)


#### `KeychainPinBall` – Keychain Pin Ball

**Final:** name **Keychain Pin Ball** (High); group **Brick Game** (Unknown); date **1996-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Keychain Pin Ball**, printed in large type on the face | High | Face SVG |
| manufacturer | Unbranded and unknown. Group: **"Brick Game"** (or a generic "Unbranded" group) | Unknown | No brand on the face. Not in MAME. Searches found nothing specific. |
| date | 1996-01-01 (placeholder) | Low | Not found. Same Holtek HT-943I0 die family as GA888 and Pikorin 55, which suggests the mid-to-late-1990s keychain boom. |
| region / variant | n/a | | |
| model | none | Unknown | |
| clone_of | none | Medium | Distinct ROM |
| keys | keep auto. Face: LEFT, RIGHT, RESET, MUTE, PAUSE, OFF, START/ON. | High | Face SVG |
| notes | Holtek's catalogue includes an HT1134A "Pin Ball LCD Game" mask product in the same HT113x family (listed in MAME's hh_ht11xx header). This unit may be built on it, but BrickEmuPy reports the die as HT-943I0 (unconfirmed link). | | |

Sources:
- https://github.com/mamedev/mame/blob/master/src/mame/handheld/hh_ht11xx.cpp - header lists "HT1134A Pin Ball" among the HT113x LCD games
- https://www.alldatasheet.com/view.jsp?Searchword=HT113 - Holtek HT1134A "Pin Ball LCD Game" datasheet listing (search snippet)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | unknown (Unknown) | **no change** | "Keychain Pin Ball" search: only BrickEmuPy and azya's X post (added 2024-07-23). |
| date | 1996-01-01 placeholder (Low) | **no change** (Low) | The Holtek mask-product datasheet "HT1134A Pin Ball LCD Game" (HT113x family, which MAME's hh_ht11xx header says "should fit here") is listed on alldatasheet as 12 pages dated "24th Dec 98" (search snippet; the PDF is behind Cloudflare). If this unit runs Holtek's stock Pin Ball program, a late-1990s date (c.1998) would fit better than 1996. That link is unconfirmed, since BrickEmuPy reports the die as HT-943I0, so the placeholder is kept and the note added. |

Sources:
- https://twitter.com/OneBitOnePixel/status/1815755083822379161 - "Added a new LCD game to BrickEmuPy - Keychain Pin Ball" (search result)
- https://www.alldatasheet.net/datasheet-pdf/pdf/64403/HOLTEK/HT113AA.html - alldatasheet page listing "HT1134A ... Pin Ball LCD Game 24th Dec 98" (search snippet only)


#### `MameGalaxian` – Galaxian

**Final:** name **Galaxian** (High); group **Bandai** (High); date **1997-01-01** (Medium); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Galaxian** (ギャラクシアン), sold as part of Bandai's **"Mame Game"** (マメゲーム) keychain series. "Mame" means bean (mini); it has nothing to do with the MAME emulator. Proposed: `Galaxian`. The maintainer decides whether to add a series prefix ("Mame Game Galaxian"). | High | Blog index of the series; Wikipedia list; eBay "BANDAI namco mame game ... 1997" |
| manufacturer | **Bandai**, under license from Namco (the blog labels it "バンダイ/ナムコ マメゲーム"). Group: **Bandai** | High | |
| date | 1997-01-01 | Medium | Series index and the Wikipedia list both say 1997, as do eBay and YouTube titles. No month found. |
| region / variant | Japan | High | Japanese packaging |
| model | none known. JAN 4902425572338 (Amazon.co.jp URL). | Medium (JAN) | |
| clone_of | none | High | |
| keys | keep auto (◀, ▶, CONTINUE, LIFE, ON/RESET, ♪, PAUSE, START/SHOOT). Note that btnLife and btnContinue are both wired to PP mask 2 in the `.brick`. Optional, because CONTINUE sits above and LIFE below between the arrows: `{"btnContinue":"UP\|X","btnLife":"DOWN\|B"}` | Medium | Face SVG |
| notes | BrickEmuPy distributes it without ROM. Egg-shaped keychain. The Mame Game series (1996-98) has 9 titles: Invader Game, Crazy Climber, Pac Junior, Galaxian, Bomberman, Columns, Mame Game de Hakken!! Tamagotchi 1 and 2, and Go-Go Connie-chan. The `MameTamagotch` config belongs to the same series. The game has continue and 3 difficulty levels. The blog author says it may be the only officially licensed LCD Galaxian. | | |

Sources:
- https://ameblo.jp/showagamer/entry-12498261735.html - Bandai Mame Game / Mame Buru series (1996-1998), list with "ギャラクシアン 1997", link labelled "バンダイ/ナムコ マメゲーム"
- https://en.wikipedia.org/wiki/List_of_Japanese_handheld_electronic_games - "Galaxian ギャラクシアン Mame Game Bandai Shmup 1997"
- https://www.amazon.co.jp/%E3%83%90%E3%83%B3%E3%83%80%E3%82%A4-4902425572338-%E3%83%9E%E3%83%A1%E3%82%B2%E3%83%BC%E3%83%A0-%E3%82%AE%E3%83%A3%E3%83%A9%E3%82%AF%E3%82%B7%E3%82%A2%E3%83%B3/dp/B00C18CEWG - "マメゲーム ギャラクシアン", Bandai, JAN 4902425572338
- https://www.ebay.com/itm/144108526875 - "Galaxian / BANDAI namco mame game With package / VINTAGE LCD Game/ 90Game 1997" (search title)
- https://www.youtube.com/watch?v=hl6od8-PVS0 - "ギャラクシアン マメゲーム バンダイ ナムコ ... 1997": continue, 3 difficulty levels (search snippet)


#### `MameTamagotch` – Mame Game de Hakken!! Tamagotchi

**Final:** name **Mame Game de Hakken!! Tamagotchi** (High); group **Bandai** (High); date **1997-01-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Mame Game de Hakken!! Tamagotchi** (マメゲームで発見!!たまごっち). The package prints "マメ" as small ruby above "ゲームで発見!! たまごっち", with a "Mame Game" series logo top-left. The unit face reads "MAME GAME". The Bandai package also carries a "TAMAGOTCH" logo, which explains BrickEmuPy's "Tamagotch" spelling. Alternates: "Tamagotchi Mame Game" (collectors); "Found in the Mame Game! Tamagotchi" (EN Wikipedia translation, not official). Series prefix: "Mame Game", Bandai's 1997 keychain mini-LCD line. Naming parallels the existing "Mori de Hakken! Tamagotchi". | High | Box photo on Tamagotchi Wiki (File:TamagotchiMameGame.png) shows the exact title. The face SVG has the "MAME GAME" logo and Tamagotchi characters. |
| manufacturer | **Bandai** (group: Bandai). This is NOT an unrelated Chinese keychain: the box carries the BANDAI logo and "©BANDAI 1997 MADE IN CHINA". | High | Box photo; Tamagotchi Wiki; EN Wikipedia list |
| date | 1997-01-01 (Japan-only release; month unknown) | High (year) / Unknown (month) | The box prints "©BANDAI 1997" (plus "©BANDAI 1996" for the Tamagotchi characters). Tamagotchi Wiki and EN Wikipedia both say 1997. BrickEmuPy's commit says "1996", which is the character copyright, not the release. |
| region / variant | Japan. This is **version 1** (falling-block puzzle). "Mame Game 2" is a different unit with 3 minigames (Rice Ball Relay, Operation Injection, Fortune-Telling). | High | The face says "MAME GAME" with no "2". Gameplay matches the wiki description of v1 (pairs of Tamagotchis falling; match 3 to clear). The config has a Start/Rotate button. |
| model | Unknown (no product code visible on the box photo) | Unknown | Box photo too small to read any code |
| clone_of | none | High | |
| keys | keep auto (◀ ▼ ▶, START/ROTATE=A, PAUSE=MINUS, ♪=R, ON/RESET=PLUS+MINUS all match the face) | High | Face SVG labels |
| notes | Distributed without ROM in BrickEmuPy. Holtek HTGL43Q0 MCU. Part of the same Bandai "Mame Game" series as `MameGalaxian`, so both belong in the Bandai group. The shell has Tamagotchi "comrade" character doodles. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Mame_Game_De_Hakken - title in Japanese and romanized, Bandai, 1997, Japan only, two versions and v1 gameplay. Box photo: https://static.wikia.nocookie.net/tamagotchi/images/9/95/TamagotchiMameGame.png
- https://en.wikipedia.org/wiki/List_of_Japanese_handheld_electronic_games - "Found in the Mame Game! Tamagotchi / マメゲームで発見！たまごっち / Bandai / Puzzle / 1997"; Mame Game 2 also 1997
- BrickEmuPy commit f8c68aa "add Mame Game Tamagotch 1996"


#### `MickeyVGS` – Mickey Deluxe Virtual Game

**Final:** name **Mickey Deluxe Virtual Game** (Low); group **VGS** (Low); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: `btnEarLeft`→`LEFT|L`, `btnEarRight`→`RIGHT|R`, `btnLeft`→`A`, `btnRight`→`B`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Mickey Deluxe Virtual Game** (BrickEmuPy's name). Retail listings call it "Disney Deluxe Virtual Game Mickey". Related or alternate names for the same toy family: "Virtual Mickey" / "Cyber Mickey" (Playworks "Mickey for Kids" line) and "Mickey Life Game" (Japan). | Low | No box text seen for this exact (blue, Mickey-head, ear-button) variant. The listing title is the only source for "Deluxe Virtual Game". |
| manufacturer | **VGS (Virtual Game System)** / Playworks International, Disney licensed (group: suggest a new "VGS" group, or an "Unbranded/Other" group). Not Tiger: nothing links Tiger to it. A TamaTalk claim that it was "from Takara Tomy" is a single forum post and probably refers to a later Takara Tomy Mickey pet. | Low | The brick name contains "VGS". The jennystroom.nl review of "Virtual Mickey" lists "Playworks International / VGS Virtual Game System" with "© DISNEY WWL MADE IN CHINA" on the back. TamaTalk thread (seen only in search snippets) says the Japanese version has "two buttons below the screen and one on each ear", which matches this face exactly. |
| date | 1997-01-01 (**guess**) | Low | Nothing found. The guess is based on the virtual-pet boom era and the Holtek HTGT43N0 MCU. azya's X post says little information exists. |
| region / variant | Blue Mickey-head shell with ear buttons, sold in Japan (Japanese-language listing). A 4-buttons-below version ("Virtual Mickey") and a round red "Mickey's Romance" Disney pet are different units. | Low-Medium | buy-tamagotchis listing photo matches the SVG shape: blue head, white ear buttons, 2 small bottom buttons |
| model | Unknown | Unknown | |
| clone_of | none | Medium | |
| keys | override suggested: `{"btnEarLeft": "LEFT\|L", "btnEarRight": "RIGHT\|R", "btnLeft": "A", "btnRight": "B"}` | Low-Medium | BrickEmuPy's own hotkeys put the ears on the Left/Right arrows (menu cycling), btnLeft (bottom-left) on Space (confirm) and btnRight (bottom-right) on Down. The Virtual Mickey review says the outer buttons cycle, one confirms and one cancels. Auto mapping puts the confirm/cancel buttons on the D-pad and the cycling ears on shoulders, which is awkward. |
| notes | Holtek HTGT43N0. azya (BrickEmuPy author) tweeted "Mickey Deluxe Virtual Game has been dumped and emulated" and found very little info on it. The face icons (food, bath, TV, book, shower, light, weather, medicine, report, Mickey-alert) show it is a care-type virtual pet. | | |

Sources:
- https://buy-tamagotchis.com/product/disney-deluxe-virtual-game-mickey-kids-mouse-virtual-pet-blue-japan/ - name "Disney Deluxe virtual game Mickey", blue, Japan; unit photo matches the face
- https://jennystroom.nl/powerup/2020/10/04/virtual-mickey-virtual-pet/ - Virtual Mickey (Cyber Mickey) by Playworks International / VGS, "© DISNEY WWL MADE IN CHINA", button roles
- https://www.tamatalk.com/threads/anyone-have-any-information-on-the-virtual-mickey-mouse.196779/ - (403 on fetch; search-snippet only) VGS, "Mickey For Kids", Japan "Mickey Life Game", ear-button variant
- https://x.com/OneBitOnePixel/status/1868701474781642990 - (search snippet) dump announcement; HTGT43N0

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| name / manufacturer / date / model | Mickey Deluxe Virtual Game; VGS/Playworks (Low); 1997 guess (Low); unknown | **No change** | Searched English ("Mickey" "Deluxe Virtual Game"; Virtual/Cyber Mickey Playworks "Mickey for Kids") and Japanese (ミッキー バーチャルゲーム デラックス 液晶 育成). MAME has no Mickey pet driver (GitHub code search). Only reseller listings came up, with no maker or year. A boutique-tamagotchis listing shows a red "Mickey's romance / Minnie" sibling. Re-fetched jennystroom.nl: it gives no year, only the barcode 6 2380200143 5 and colours red/yellow/black for the Playworks "Virtual Mickey". TamaTalk still returns 403. |

Sources:
- https://boutique-tamagotchis.fr/produit/disney-deluxe-virtual-game-mickey-kids-mouse-mickeys-romance-minnie-red-japan/ - red "Mickey's romance" sibling under the same "Deluxe virtual game" name (search result)
- https://www.ebay.com/itm/286021936724 - "Disney Deluxe virtual game Mickey kids Mouse Virtual Pet Blue Japan" (search result)


#### `Puyolin` – Puyolin

**Final:** name **Puyolin** (High); group **Compile** (High); date **1997-07-25** (Medium); model **–** (Unknown); clone of **none** (High); keys: `btnStart`→`A|PLUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Puyolin** (ぷよりん). The unit prints only "ぷよりん" and the box says "minigame". Romanizations seen: "Puyolin" (BrickEmuPy), "Puyorin" (EN Wikipedia, eBay), "Puyo-Rin" (blog). I found no official Latin-script title. Keep "Puyolin" for consistency with the source, or use "Puyorin" (Hepburn). | High (JP title) / Low (romanization) | Face SVG logo "ぷよりん" |
| manufacturer | **Compile** (group: new "Compile"; or an "Other" group) | High | ja.wikipedia (ぷよぷよシリーズ, "ぷよりん（キーチェーンゲーム）"), EN Wikipedia list, loopdept blog "(C)COMPILE", HARD OFF listing "コンパイル｜ぷよりん" |
| date | **1997-07-25** (Japan) | Medium | ja.wikipedia ぷよぷよシリーズ: "1997年7月25日発売" (no inline citation). Several other sources agree on 1997. |
| region / variant | Japan. This is the monochrome original. A later "カラーぷよりん" (Color Puyolin) was a separate successor. | Medium | ja.wikipedia |
| model | Unknown | Unknown | |
| clone_of | none | High | |
| keys | override: `{"btnStart": "A\|PLUS"}`. The big button is labelled "スタート／かいてん" (Start/Rotate), and BrickEmuPy binds it to Space. Under auto mapping, rotate sits only on PLUS. Everything else matches: ◀ ▶, ▼ = "セレクト" (Select), サウンド = Mute, ポーズ = Pause, オン／オフ, ACL. | High | Face SVG labels; .brick hotkeys |
| notes | Holtek HT-943Q0, 4 KB ROM. Endless ("とことん") Puyo Puyo rules with N/T modes, 3 puyo types. ja.wikipedia notes its naming and form closely copy the popular "テトリン" (Tetrin) keychain. Runs on 2×LR44. Sold in several colours (white, black, pink, green). | | |

Sources:
- https://ja.wikipedia.org/wiki/ぷよぷよシリーズ (raw wikitext fetched) - "ぷよりん（キーチェーンゲーム）… 1997年7月25日発売", Color Puyolin successor, Tetrin resemblance
- https://en.wikipedia.org/wiki/List_of_Japanese_handheld_electronic_games - Puyorin / Compile / 1997
- https://loopdept.hatenablog.com/entry/2023/05/16/171900 - (C)COMPILE, 1997, LR44×2
- https://kimimithegameeatingshemonster.com/2025/12/24/puyorin-lcd-puyo-rfectly-formed/ - box says "minigame", puyo-shaped rotate button
- https://netmall.hardoff.co.jp/product/4875363/ - (search snippet; 404 on fetch) "コンパイル｜ぷよりん"


#### `SpaceIntruderTK150I` – Space Intruder

**Final:** name **Space Intruder** (High); group **Brick Game** (Low); date **1998-01-01** (Low); model **TK-150I** (Medium); clone of **none** (Low); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Space Intruder** (the face prints "SPACE INTRUDER"). Model "TK-150I" comes from BrickEmuPy, presumably from the unit's back or label. Use "Space Intruder" plus model TK-150I, or "Space Intruder TK-150I". | High (title on face) | Face SVG |
| manufacturer | Unbranded Chinese brick-style handheld; no maker on the face (group: "Brick Game"/"Unbranded") | Low | No brand printed on the face. Holtek HTB943R0 MCU (same die family as Bandai's Mame Galaxian per the README). |
| date | Unknown. If a placeholder is required, use 1998-01-01 (**guess**, Holtek HT943-era brick games of the mid-to-late 1990s). | Unknown/Low | Search budget exhausted before any search ran |
| region / variant | n/a | | |
| model | TK-150I | Medium | BrickEmuPy README/brick name |
| clone_of | none in list. The ROM uses the same Holtek family as `MameGalaxian`; the games are possibly related but not confirmed. | Low | |
| keys | keep auto (LEFT/GAME, RIGHT/LIFE, DOWN, FIRE/START=A, ON/OFF=PLUS, PAUSE=MINUS, SOUND=R) | High | Face labels |
| notes | Space Invaders-style game with 10 columns, LIFE/GAME digit and HI SCORE. Needs a follow-up search for "TK-150I". | | |

Sources:
- local: BrickEmuPy assets/SpaceIntruderTK150I.svg (rendered), README

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | unbranded (Low) | **no change** | "TK-150I"/"TK150" gave no hits outside BrickEmuPy and its forks. **Caution:** web results for "Space Intruder LCD game" are dominated by **Techno Source "Space Intruder"** (2004-2006, made in China; AU distributor Funtastic). That is a *different* product: the DiskDepot YouTube video's thumbnail shows a game screen with planet backdrop art and a SCORE readout, unlike the TK-150I's 10-column brick-style matrix with LIFE/GAME/HI SCORE in a red bulge shell. Do not attribute the TK-150I to Techno Source or date it from those listings. |
| date | 1998-01-01 guess (Low/Unknown) | **no change** | Nothing found. A MAMEWorld post (search snippet, unreachable) dated 2024-02-15 says "Space Intruder TK-150 added", which is the emulator addition date, not the product date. |

Sources:
- https://www.youtube.com/watch?v=YZ3ScSgyPrY - "Space Intruder" Retro LCD Handheld Game (DiskDepot): thumbnail checked, different product (Techno Source type)
- https://www.ebay.com/p/1400173811 - "2006 Techno Source Space Intruder LCD Video Game" (different product)


### Samsung 4-bit

#### `GA878` – GA-878

**Final:** name **GA-878** (Medium); group **Brick Game** (Medium); date **2000-01-01** (Low); model **GA-878** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **GA-878**. The face prints only "GA-878" and no title. Suggested display: "Brick Game GA-878" or "Keychain GA-878" (BrickEmuPy). | Medium | Face SVG |
| manufacturer | Unbranded Chinese keychain (group: "Brick Game"/"Unbranded") | Medium | No brand on face; Samsung KS56-family MCU (BJ6220A) |
| date | Unknown. Placeholder suggestion: 2000-01-01 (**guess**: late-1990s/2000s keychain Tetris-with-clock) | Unknown/Low | Search budget exhausted |
| region / variant | n/a | | |
| model | GA-878 | High | Printed on face |
| clone_of | none. It shares the MCU (BJ6220A) and near-identical button layout with `MiconKC32`, but the ROMs differ (only ~35% similar), so it is not a clone. | High | Local md5/diff |
| keys | keep auto (HOUR=◀, MINUTE=▶, START=▼, ROTATE=A, SET/PAUSE=PLUS, MODE=MINUS, SOUND=R) | High | Face labels |
| notes | 8×11 block field with clock/alarm display (AM/PM, ALARM, GAME). Yellow keychain shell. | | |

Sources:
- local: BrickEmuPy assets/GA878.svg, GA878.bin vs MiconKC32.bin comparison

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer, date | unbranded / 2000-01-01 guess | **no change** | EN and JA searches for "GA-878"/"GA878" (keychain, テトリス キーホルダー) found only BrickEmuPy. The 2-letter "GA" prefix matches `GA888` (also unbranded, sold in Russia), which may mean the same OEM. That is speculation and is not used for a field. |

Sources: none new.


#### `KotoKunekunetchyo` – Kunekunetchyo

**Final:** name **Kunekunetchyo** (High); group **Hiro** (High); date **1997-09-01** (Medium); model **LCD Keychain Series No.4** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **くねっくねっちょ**. Note the small っ: the face logo, box and Hiro flyer all read くねっくねっちょ, not くねくねっちょ. Romanizations: "Kunekunetchyo" (beforemario.com and BrickEmuPy, de facto English), "Kunekuneccho" (Yokoi interview transcript). Strict Hepburn is "Kunekkunetcho". I recommend "Kunekunetchyo" as the established English spelling, or "Kunekkunetcho". Series printed on box: "LCDゲーム キーチェーンシリーズ No.4" (LCD Game Keychain Series). | High (JP title) / Medium (romanization) | Face SVG logo (zoomed render); box photo and 1997 Hiro flyer on beforemario.com |
| manufacturer | **Hiro** (ヒロ, HIRO Company Ltd., publisher), developed by **Koto Laboratory** (株式会社コトー, Gunpei Yokoi's company). The box prints both "KOTO LABORATORY & HIRO". Group: "Hiro" (the same company as `PocketBoyNo2`; a Hiro group would hold both), or "Koto Laboratory". | High | Box photo, flyer "©1997 KOTO CO., LTD.", "株式会社ヒロ" address on flyer |
| date | **1997-09-01** (Japan) | Medium-High | beforemario.com: "Koto's first products, a couple of keychain LCD games, were released in Japan by Hiro Company in September of 1997". The Hiro flyer dated 1997-11-01 shows it already on sale. Price ¥1,000. |
| region / variant | Japan only | High | |
| model | **LCD Keychain Series No.4** (キーチェーンシリーズ No.4) | High | Box and flyer |
| clone_of | none | High | |
| keys | keep auto (LEFT, RIGHT, UP, DOWN, ON/START=PLUS, SELECT=MINUS, SOUND=R). Optionally add A for start: `{"btnStart": "PLUS\|A"}`. | High | Face labels |
| notes | One of Gunpei Yokoi's two final games, released a month before his death (4 Oct 1997). The other is "Professor Henoheno" (No.5), the precursor of Gunpey. Snake game with food that moves across the field; Game A (stage clear) and Game B (fill the screen). Series: No.1 Tetris Jr., No.2 Space Invaders, No.3 Block Kuzushi, No.4 Kunekunetchyo, No.5 Henoheno, No.6 Tetris Jr. 2 (Dec 1997), Densha de Go! (Jan 1998). Samsung KS57C2504. | | |

Sources:
- http://blog.beforemario.com/2014/04/gunpei-yokois-final-two-games.html - Sept 1997 release by Hiro, Koto Laboratory, ¥1,000, series No.4, box photo and flyer images (flyer: blogger.googleusercontent.com/.../gunpei_yokoi_last_games_30.jpg; box: .../gunpei_yokoi_last_games_04.JPG)
- https://ameblo.jp/showagamer/entry-12498261766.html - "くねっくねっちょ (KOTO/HIRO)", 1997, Game A/B rules
- https://gmdisc.com/archives/384 - (search snippet; 403 on fetch) "横井軍平氏最後の液晶ゲーム作品『くねくねっちょ』"


#### `MiconKC32` – Micon KC-32

**Final:** name **Micon KC-32** (High); group **Brick Game** (Medium); date **2000-01-01** (Low); model **KC-32** (High); clone of **none** (High); keys: `btnSpeed`→`DOWN|R`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **KC-32**. The face prints "MICON" (brand) and "KC-32" (model), with no game title. Display: "Brick Game KC-32" or "Micon KC-32". Under the no-maker-prefix rule, "KC-32" alone is too bare, so "Micon KC-32" may be acceptable as the product name. | High (face text) | Face SVG |
| manufacturer | **Micon** (brand on unit; likely a Chinese/HK OEM brand) (group: "Brick Game"/"Unbranded", or a new "Micon") | Medium (brand) / Unknown (company) | Face SVG "MICON" |
| date | Unknown. Placeholder suggestion: 2000-01-01 (**guess**) | Unknown/Low | Search budget exhausted |
| region / variant | n/a | | |
| model | KC-32 | High | Printed on face |
| clone_of | none (see GA878: same MCU and layout, different ROM) | High | Local diff |
| keys | override: `{"btnSpeed": "DOWN\|R"}`. The SPEED button is the ▼ button in the arrow cluster, and BrickEmuPy binds it to the Down arrow/S. Auto mapping puts it only on R. Also: + GAME − = ◀/▶ (game select), ROTATE=A, START/PAUSE=PLUS, ON/OFF=MINUS, SOUND=L. | High | Face labels; .brick hotkeys |
| notes | 8×15 brick field with SCORE/GAME/P display. Black shell with red buttons. | | |

Sources:
- local: BrickEmuPy assets/MiconKC32.svg (rendered), MiconKC32.brick

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| region / country | n/a | **Japan market (Medium)** | Japanese resale listings: Suruga-ya buy-back item "キーチェーンゲーム テトリス(ブラック) MICON KC-32（LSI）" (maker field blank per snippet), Yahoo! Auctions "作動・機能OK MICON KC-32 テトリスゲーム ミニ...", Mercari "テトリス MICON KC-32 キーホルダー ゲーム", aucfan (3 sales). Sellers describe it as a Heisei-era retro keychain "Tetris" mini game. One Yahoo listing spells the brand "MICNN" and calls it 32-in-1 (snippet). No English-language market hits. |
| manufacturer | Micon brand (Medium) / company unknown | **no change** | No company behind "MICON" was identified. |
| date | 2000-01-01 guess (Low) | **no change** | No date in any listing text (all pages 403/JS-only). "Heisei retro" only bounds it to 1989-2019. |
| name | Micon KC-32 | Optional alt: the Japanese retail listings call it "Tetris" (unlicensed generic use). Keep "Micon KC-32". | listings |

Sources:
- https://www.suruga-ya.jp/kaitori_detail/148008676 - "キーチェーンゲーム テトリス(ブラック) MICON KC-32（LSI）" (search snippet; 403)
- https://page.auctions.yahoo.co.jp/jp/auction/w448723711 - "作動・機能OK MICON KC-32 テトリスゲーム ミニ..." (search snippet)
- https://jp.mercari.com/item/m49467598361 - "テトリス MICON KC-32 キーホルダー ゲーム" (title only)
- https://aucfan.com/intro/q-KC.2d32.20MICON/ - KC-32 MICON, 3 sales (search snippet)


#### `PocketPuppy` – Pocket Puppy

**Final:** name **Pocket Puppy** (High); group **Cool Tec** (Low); date **1997-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Pocket Puppy** | High | Printed on face SVG |
| manufacturer | Probably **Cool Tec** (group: new "Cool Tec", or an "Unbranded/Other" pets group) | Low | Virtual Pets wiki and eBay listings say "Pocket Puppy" is by Cool Tec (1997; "Model 80000"; "2nd Generation"). However, those listings describe a dog-shaped white shell, while this face is an egg with Dalmatian spots, so this unit may be a different shell or brand. Manley Toy Quest also sold a "Pocket Puppy Paws" (1997). |
| date | 1997-01-01 | Low | eBay listings for Cool Tec Pocket Puppy say 1997 (others say 1998) |
| region / variant | n/a | | |
| model | Unknown. Cool Tec listings mention "Model 80000", which is not confirmed for this shell. | Low | |
| clone_of | none | Medium | |
| keys | keep auto (Left, Right, Circle=A, Square=B; 4 unlabeled buttons on face) | Medium | |
| notes | Samsung KS57C21308. Icons: food, light, play (bat), medicine, bath, scale, game, attention ("?!"). The maker needs confirming from a back-of-unit photo. | | |

Sources:
- https://virtual-pets.fandom.com/wiki/Pocket_Puppy (API wikitext) - Pocket Puppy by Cool Tec, a.k.a. "Micro Pet", 4 buttons
- https://www.ebay.com/p/1005154151 - Cool Tec Pocket Puppy 2nd Generation, 1997, "Model 80000" mentions
- https://www.ebay.com/p/1524513323 - (search snippet) Manley Toy Quest "Pocket Pets Puppy" 1997

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer / date / model | Cool Tec? (Low); 1997 (Low); Model 80000? (Low) | **No change** | Searched for the egg/Dalmatian shell, azya's dump announcement, and maker/box text. Every hit is a Cool Tec dog-shaped keychain listing (eBay, WorthPoint, Pinterest) or Manley Toy Quest "Pocket Puppy Paws" (1997). Nothing shows the egg shell with Dalmatian spots and Circle/Square buttons. eBay and WorthPoint pages return 403, and the fandom API returns 402. MAME has no driver. Leave Low and flag that a back-of-unit photo is needed. |

Sources:
- https://www.ebay.com/itm/236571985679 - 1997 Cool Tec Pocket Puppy keychain (search result only)
- https://www.ebay.com/itm/115389360762 - "Cool Tec Pocket Puppy Tamagotchi Style GIGA PET NIB" (search result only)


### Oki 4-bit

#### `formel1` – Formel 1

**Final:** name **Formel 1** (High); group **Hartung** (Medium); date **1990-01-01** (Low); model **WG 032** (Low); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Formel 1** (printed on the face under a "GAME" logo) | High | Face SVG |
| manufacturer | **Hartung** (Hartung Spiele Berlin, German distributor/brand) (group: new "Hartung"). The Epoch link comes only from BrickEmuPy's README. I found no Epoch original title: search budget ran out, and handheldmuseum's Epoch and GAMAtronic lists have no F1 or Formel game. A Japanese video titled "エキサイトグランプリ エポック F1レーサー" (Epoch Excite Grand Prix / F1 Racer) is a possible candidate original but is unverified. | Medium (Hartung) / Low (Epoch) | Collector forums list "Formel 1" among Hartung Spiele Berlin LCD games |
| date | 1990-01-01 (**guess**; collectors date Hartung LCD games "late 80s/early 90s") | Low | pyra-handheld forum thread |
| region / variant | Germany | Medium | |
| model | Unknown | Unknown | |
| clone_of | none in list | Medium | |
| keys | keep auto (◀ ▶ big red buttons, GAME 1/2=PLUS, SET=R). Optionally `{"btnGame": "PLUS\|A"}`. | High | Face labels |
| notes | Oki MSM5055 (MSM50XX core). LCD has clock (PM 18:88), GAME A/GAME B, SETTING, SOUND, FUEL counter. Collectors mention a red "UFO" housing variant. TODO: identify the Epoch original. | | |

Sources:
- https://pyra-handheld.com/boards/threads/suche-lcd-spiele-telespiele-hartung-spiele-berlin.98602/ - Hartung Spiele Berlin titles incl. Formel 1, late 80s/early 90s
- https://www.handheldmuseum.com/Epoch/index.html, https://www.handheldmuseum.com/GAMAtronic/index.html - checked; no Formel/F1 entry
- https://www.youtube.com/watch?v=vCn0Nw-mtbk - (search snippet only) "エキサイトグランプリ エポック F1レーサー" - unverified candidate

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| model | Unknown | **WG 032** (Low) | One Kleinanzeigen listing title: "Formel 1 UFO LCD Spiel Hartung Spiele Berlin 90er Jahre WG 032". The code is not in the description and could be a seller reference. |
| date | 1990-01-01 guess (Low) | 1990-01-01 (Low, unchanged; slightly better supported) | The same seller says "Gaming-Klassiker von Anfang der 90er Jahre". A forum thread says "late 80s/early 90s". |
| manufacturer (Epoch original) | Hartung Medium / Epoch Low | **No change** | A search-result summary of lcd-spiele.de says Hartung games were also sold under brands such as Epoch and Systema, and that "Formel 1" is missing from its brand-variant overview. The site itself was unreachable (DNS), so this is unverified. The Japanese searches (エポック社 液晶ゲーム F1 / UFO型) found only "エキサイトグランプリ" (bridge-crossing race) and "エポックブックゲーム カーレース F1マシン". Neither is confirmed as the original. The YouTube page was blocked. The "Formel 1" UFO-shaped housing also came in red. |

Sources:
- https://www.kleinanzeigen.de/s-anzeige/formel-1-ufo-lcd-spiel-hartung-spiele-berlin-90er-jahre-wg-032/3391260424-227-245 - "WG 032", "Anfang der 90er Jahre", UFO shape
- https://www.kleinanzeigen.de/s-hartung-lcd/k0 - also a wanted ad for the red "Rotes UFO" Formel 1
- http://lcd-spiele.de/seiten/z-sonstige_1.html - Hartung titles were also sold as Epoch/Systema (search snippet only; host unreachable)


#### `SMBNelsonic` – Super Mario Bros. Game Watch

**Final:** name **Super Mario Bros. Game Watch** (High); group **Nelsonic** (High); date **1989-01-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Super Mario Bros.** The box reads "Nintendo SUPER MARIO BROS. GAME WATCH" and "Super Mario Bros. Game Watch by Nelsonic". The face prints "Nintendo", "SUPER MARIO BROS", "NELSONIC", SET/MODE. Series suffix: "Game Watch". With the suffix: "Super Mario Bros. Game Watch". (Also listed as "Mario Bros." by IGN per EN Wikipedia.) The handoff's "Tamagotchi Angel" README line is a generator bug. | High | Face SVG; handheldmuseum box photo |
| manufacturer | **Nelsonic** (Nintendo-licensed; Nelsonic was a division of M.Z. Berger) (group: Nelsonic) | High | Face, box, Mariowiki, Wikipedia |
| date | 1989-01-01 (USA) | High | Mariowiki (citing packaging), EN Wikipedia Nelsonic table, and a Service Merchandise Dec 1989 catalog showing it on sale at $19.97 |
| region / variant | USA (North America). Black/white/pink/red shells; the SVG is the black/blue version. | High | |
| model | Unknown. Handheldmuseum says "Model# ??". The Service Merchandise catalog code **LSMN1WBF** is a retailer code, not a Nelsonic model number. | Unknown (model) | |
| clone_of | none | High | |
| keys | keep auto (4 bezel buttons ←/→/↑ jump/↓ start; MODE=MINUS, SET=R). Optional: `{"btnUp": "UP\|A"}` for jump. | High | Face; Mariowiki controls (left, right, jump, start game; Mode/Set on side) |
| notes | Distributed without ROM. Credits: ROM photo and LCD vector by Sean Riddle. First of four Nelsonic Mario watches; the sequel is SMB3 (1990). Came with a headphone jack/headphones (Wikipedia). Oki MSM50XX. | | |

Sources:
- https://www.mariowiki.com/Super_Mario_Bros._(Nelsonic_Game_Watch) (API wikitext) - 1989 USA (packaging), controls, colours
- https://www.mariowiki.com/Nelsonic_Game_Watch - Nintendo licence 1989, Nelsonic/M.Z. Berger
- https://en.wikipedia.org/wiki/Nelsonic_Industries - table: Super Mario Bros. 1989, white/black, headphone jack
- https://www.handheldmuseum.com/Nelsonic/SMB.htm - box photo (Nelsonic-SMBBox.jpg), "Model# ??"
- https://vintagecomputing.com/?p=1076 - Dec 1989 Service Merchandise catalog, $19.97, code LSMN1WBF


#### `ZeldaNelsonic` – The Legend of Zelda Game Watch

**Final:** name **The Legend of Zelda Game Watch** (High); group **Nelsonic** (High); date **1989-10-05** (Medium); model **8321** (Medium); clone of **none** (High); keys: `btnSet`→`A|R`, `btnMode`→`B|MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **The Legend of Zelda** (bare title). Series is "Game Watch", which was Nelsonic's line name and is not printed on the face. The manual heading is "THE LEGEND OF ZELDA 8321". If the series is kept: "The Legend of Zelda Game Watch". | High | Manual PDF (primary), Wikipedia, Zelda Wiki, BrickEmuPy README |
| manufacturer | Nelsonic Industries, licensed by Nintendo. Re-released in Europe by Zeon (1992). (group: **Nelsonic**) | High | Wikipedia LCD games article, MAME hh_sm510 ("The 'Game Watch' wristwatches are by Nelsonic, not Nintendo") |
| date | 1989-10-05 (North America, first release) | Medium | Wikipedia LCD-games page and Zelda Wiki both give Oct 5 1989. A search summary of the Wikipedia Nelsonic article said "August 1989", which I did not verify. A Service Merchandise circular dated Dec 1989 lists it at $19.97. UK/PAL (Zeon) release: May 1992. |
| region / variant | n/a (US Nelsonic watch). Colours: black, red, white, pink. Same game in all. | Medium | Wikipedia / Zelda Wiki |
| model | **8321** (manual title "THE LEGEND OF ZELDA 8321"). Also "LZDN1WBF" appears in a Dec 1989 Service Merchandise circular; probably a retailer or colour SKU, unverified. | Medium (8321) / Low (LZDN1WBF) | Manual PDF; vintagecomputing.com |
| clone_of | none. SMBNelsonic is the sister Nelsonic watch on the same OKI MSM50xx hardware, but a different game. | High | |
| keys | **Override suggested:** `{"btnSet": "A\|R", "btnMode": "B\|MINUS"}` | Medium | The manual shows both side buttons are gameplay buttons. S2 (Set) starts the game and attacks with the boomerang or sword. S1 (Mode) picks up weapons, keys, bombs and triangles and drops bombs. The auto mapping (Mode=MINUS, Set=R) leaves both actions on awkward keys. |
| notes | Distributed without ROM; credits say "ROM photo and LCD vector image by Sean Riddle". Do not confuse with Nintendo's Game & Watch "Zelda" (ZL-65, 1989), a different product. A separate "Flip Watch" variant also exists. Zelda Dungeon's claim of 12 million units sold is unsourced. | | |

Sources:
- https://www.puissance-zelda.com/annexes/img/zw/f_manuel.pdf - scanned English manual "THE LEGEND OF ZELDA 8321"; S1 (Mode) and S2 (Set) functions
- https://en.wikipedia.org/wiki/The_Legend_of_Zelda_LCD_games - Nelsonic, NA Oct 5 1989, PAL May 1992 via Zeon, colours, manual "8321"
- https://zeldawiki.wiki/wiki/The_Legend_of_Zelda_Game_Watch - US Oct 5 1989, UK May 1992, model 8321
- https://www.vintagecomputing.com/index.php/archives/tag/1989 - Service Merchandise circular Dec 1989, $19.97, "LZDN1WBF"
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_sm510.cpp - "Game Watch" = Nelsonic; older Nelsonic watches on OKI MSM5055


### Sanyo LC57

#### `EdlaSpaceRescue` – Space Rescue

**Final:** name **Space Rescue** (Low); group **Edla** (High); date **1989-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Space Rescue**. The face shows only the brand logo "Edla® Game"; the title comes from the BrickEmuPy README. | Low | README only; no independent source found. The LCD art (ships, parachutists, red planet) fits the title. |
| manufacturer | Edla ("Edla® Game" logo on the face). Origin unknown; Hong Kong/Europe is a guess. (group: new **Edla**, or a generic "Unbranded/Other" group) | High (brand on face) / Unknown (company) | Face SVG. Searches for "Edla Game" in English, Russian and Polish found nothing. |
| date | 1989-01-01 (README says "1989?") | Low | BrickEmuPy author's own guess (with "?"); no source found |
| region / variant | n/a | | |
| model | unknown | Unknown | Nothing found |
| clone_of | none in the list. Unrelated to the Tronica MG-9, Casio CG-126 and Game Wizard "Space Rescue" games, which have different hardware and art (checked the Game Wizard photo on handheldmuseum). | Medium | |
| keys | keep auto | Medium | Face: LEFT/SET, RIGHT/SELECT, START, MODE. Auto Left/Right, Start=PLUS, Mode=MINUS is fine. |
| notes | The round face carries generic artwork (balloon, tank, pyramids, soccer, castle, racing flag) unrelated to space. It may be a shared Edla template shell used across several games. Game A / Game B modes plus a clock and alarm. Sanyo LC5732. Research incomplete: search budget ran out. | | |

Sources:
- (face SVG text: `assets/EdlaSpaceRescue.svg`)
- https://handheldmuseum.com/Misc/GameWizardSpaceRescue.htm - a different "Space Rescue" (Game Wizard), ruled out
- https://www.lulu-berlu.com/tronica-game-clock-handheld-game-space-rescue-mg-9-a30506-en.html - Tronica MG-9 Space Rescue (1982), different game, from search results only

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| all weak fields | Space Rescue (Low); Edla brand origin unknown; 1989? (Low) | **No change** | "Edla" "Space Rescue" found only BrickEmuPy and unrelated Space Rescue games (Tronica MG-9 1982, Casio CG-126 1988, Radio Shack 60-2229 1990, Commodoor, Game Wizard, Play&Time). The Play&Time "Game Watch Space Rescue" (WorthPoint, search result only) is another unbranded-style "Space Rescue" game watch. It may share an OEM with this one, but that is unverified. The README entry was added on 2025-12-29 (commit f40a8db "add Edla Game Space Rescue") with no source given. |

Sources:
- https://www.worthpoint.com/worthopedia/play-time-electronic-handheld-game-252296849 - "PLAY&TIME ELECTRONIC HANDHELD GAME WATCH SPACE RESCUE" (search result only; possible OEM sibling, unverified)
- https://www.arcade-history.com/?n=space-rescue-model-60-2229&page=detail&id=100726 - Radio Shack Space Rescue 60-2229 (1990), a different product (search result)


#### `PocketBoyNo2` – Dragon Makyuu no Nazo

**Final:** name **Dragon Makyuu no Nazo** (High); group **Hiro** (High); date **1990-01-01** (Low); model **Pocket Boy No.2** (Medium); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Dragon Makyū no Nazo** (ドラゴン魔宮の謎, roughly "Mystery of the Dragon's Demon Palace"). Series "Pocket Boy" No. 2 is printed on the unit. Possible display name: "Pocket Boy No.2: Dragon Makyū no Nazo" if the series is kept, otherwise "Dragon Makyū no Nazo". ASCII romanization: "Dragon Makyuu no Nazo". | High | Face SVG title banner "ドラゴン魔宮の謎" with "NO 2" mascot and "POCKET BOY". Matching YouTube title "ポケットボーイ NO.２ ドラゴン 魔宮の謎 ＨＩＲＯ"; Amazon.co.jp listing "ポケットボーイ ドラゴン魔宮の謎 POCKET BOY". |
| manufacturer | HIRO (株式会社ヒロ, Japanese toy company) (group: new **Hiro**; also needed for KotoKunekunetchyo "Koto Laboratory & HIRO") | High | README, YouTube title, blog |
| date | 1993-01-01 (README) **or** 1990-01-01 (YouTube video title) | Low | Sources conflict. BrickEmuPy README says 1993. The YouTube reviewer tags it 1990, and the reviewer's other Pocket Boy video (No.13) is also tagged 1990. An Ameblo blog says the series was sold in the "early 1990s". I kept 1993 because the emulator author handled the unit, but neither date is verified. |
| region / variant | n/a (Japan) | | |
| model | No. 2 (Pocket Boy series number). No catalogue number found. | Medium | Face |
| clone_of | none | | |
| keys | keep auto | High | Face: CONTROL (4-way), START, MODE, SOUND. Auto Start=PLUS, Mode=MINUS, Sound=R matches. |
| notes | Gameplay: a knight dodges spiders, bats and slimes and collects diamonds (YouTube). Inlay is a castle photo. Later HIRO "New Pocket Boy" series (1996-97) is separate. Sanyo LC5732. | | |

Sources:
- (face SVG text: `assets/PocketBoyNo2.svg`)
- https://www.youtube.com/watch?v=U5P9EddtrKk - title "ポケットボーイ NO.２ ドラゴン 魔宮の謎 ＨＩＲＯ ヒロ … 1990" (via oEmbed and search snippet; thumbnail shows the same castle inlay)
- https://www.amazon.co.jp/%E3%83%AC%E3%83%88%E3%83%AD-%E3%83%9D%E3%82%B1%E3%83%83%E3%83%88%E3%83%9C%E3%83%BC%E3%82%A4-%E3%83%89%E3%83%A9%E3%82%B4%E3%83%B3%E9%AD%94%E5%AE%AE%E3%81%AE%E8%AC%8E-POCKET-%E9%9B%BB%E6%B1%A0%E7%84%A1%E3%81%97/dp/B0DPCRH3VM - listing title (search result)
- https://ameblo.jp/aas192508/entry-12652541067.html - Pocket Boy sold by 株式会社ヒロ in the early 1990s

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1993-01-01 (README) vs 1990 (Low) | **1990-01-01** (Low; recommend switching away from the README's 1993) | Three weak sources all point to 1990. (1) The YouTube reviewer (ブラゼ) tags Pocket Boy No.2 and No.13 as 1990, but tags the later **New** Pocket Boy No.7 as 1995 and No.12 as 1996, so he separates the two series. (2) A Yahoo! Flea Market seller lists a "HIRO POCKET BOY 1990年製" (no title given; specs say 製造年 1990; made in China). (3) Ameblo blogs put the HIRO series "around 1990" or "early 1990s". Nothing found supports 1993. The README's "1993" may come from a © date on the back of the unit, which we haven't seen. If the maintainer trusts azya's handling of the unit, keep 1993. |

Sources:
- https://paypayfleamarket.yahoo.co.jp/item/z473185234 - "HIRO POCKET BOY 1990年製 電子ゲーム", 製造年 1990
- https://www.youtube.com/watch?v=U5P9EddtrKk - No.2 tagged 1990 (search result title)
- https://www.youtube.com/watch?v=lnxFSmV5cNg - Pocket Boy No.13 tagged 1990 (search result title)
- https://www.youtube.com/watch?v=8ugHxjcvC9E - New Pocket Boy No.7 tagged 1995 (search result title)
- https://www.youtube.com/watch?v=U0RCIbhWEvI - New Pocket Boy No.12 tagged 1996 (search result title)
- https://ameblo.jp/whdjapan/entry-12794310762.html - HIRO NEW POCKET BOY "1990年ごろ" (blog, weak; it contradicts the 1995-96 YouTube tags for New Pocket Boy)


### 6502 family (Sunplus, runs on GPL191X)

#### `DPowerDigiviceEU101` – D-Power Digivice (Europe)

**Final:** name **D-Power Digivice (Europe)** (High); group **Bandai** (High); date **2001-01-01** (Low); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **D-Power Digivice**. Suggested display name: "D-Power Digivice (Europe)"; add "v1.01" only if needed. The Japanese counterpart is the D-Ark (ディーアーク, 2001), which is a different toy and firmware. | High | Wikimon branding table: firmware "DP•EU V1.00", branding "D-Power Digivice", region Europe/Australia/Asia |
| manufacturer | Bandai (group: **Bandai**) | High | Wikimon, README |
| date | 2001-01-01 (Wikimon gives only "?? 2001-2002") | Low | No exact EU date found. The Digimon Tamers anime aired in Japan 2001-02 and in the US from Sept 2001. Listings call the toy "Bandai 2001", which is probably the copyright year. Use 2002-01-01 instead if the maintainer wants the actual EU street date (Tamers dub reached Europe in 2002); that is also unverified. |
| region / variant | Europe/Asia/Oceania "Version 1" (EU/AS line, not the US D-Power with PC link). Dump firmware is v1.01 per the brick name; Wikimon lists only "V1.00", so this is presumably a minor revision. | Medium | The face has 512 LCD segments = 32x16, which matches Wikimon's Version 1. Versions 2 and 3 are 30x32. |
| model | unknown | Unknown | Wikimon lists no product codes |
| clone_of | none | | |
| keys | keep auto | Medium | Real controls: 4 buttons plus a side card slot ("slash") and a pedometer shake sensor. In the brick, "Step" and "Card" are virtual inputs (A/B); Reset = PLUS+MINUS. Sensible. |
| notes | Distributed without ROM; device from x.com/OKDecc. Version 1 starters: Guilmon/Renamon/Terriermon; minigame is whack-a-mole. Colours for Version 1: silver with red/blue/green accents (the face is blue). Hong Kong Asia Cup limited editions exist. | | |

Sources:
- https://wikimon.net/D-Power_Digivice_Toy_(EU/AS) - versions table, "DP•EU V1.00", 2001-2002, 32x16 for V1, colours
- https://wikimon.net/D-Power - D-Ark/D-Power family, 4 buttons, card slot
- https://lcd.withthewill.net/DPowerEuIntro - EU Version 1 controls, pedometer, cards

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 2001-01-01 (Low) | **No change** | The English search found only Wikimon "?? 2001-2002" again, plus a Baidu timeline listing D-Power "US/HK/EU 2001" (weak). No UK/EU launch month was found. The Internet Archive has a D-Power **Version 3** Chinese/English manual, which doesn't apply to V1. |

Sources:
- https://baike.baidu.com/en/item/DIGIVICE/1487360 - D-Power US/HK/EU 2001 (search snippet; weak)
- https://archive.org/details/d-power-digivice-version-3-chinese-and-english-instruction-images - "D-Power is the US/EU/Asia name for D-Ark" (search snippet)


### EMC 4-bit

#### `E33_2in1` – Brick Game E-33 2 in 1

**Final:** name **Brick Game E-33 2 in 1** (High); group **Brick Game** (Medium); date **1994-01-01** (Low); model **E-33** (High); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Brick Game 2 in 1** (model E-33). Suggested display name: "Brick Game E-33 2 in 1". | High | Face SVG: "BRICK GAME 2 IN 1", "E-33" |
| manufacturer | Unbranded Chinese brick game. The face carries a red smiley-with-hands logo with the characters "机之宝" (a brand or trade mark; I could not identify the company). (group: proposed **Brick Game** / unbranded) | Medium | Face SVG |
| date | unknown; no defensible year. If one is required: 1990s (e.g. 1995-01-01 as an explicit guess). | Unknown | Searches not reached (budget exhausted). Unknown chip date. |
| region / variant | n/a | | |
| model | E-33 | High | Face |
| clone_of | none in the list. Shares the brick-game genre with E23PlusMarkII96in1 and E88_8in1, but runs on different hardware (EMC SC6383 vs Holtek). | Medium | |
| keys | keep auto | High | Face: LEFT/LEVEL, DOWN/MODE, RIGHT/SPEED, ROTATE/DIRECTION, START/PAUSE, ON/OFF, MUTE. Auto matches (Rotate=A, Start=PLUS, OnOff=MINUS, Mute=R). |
| notes | LCD has a fruit and teacup icon column and GAME A/GAME B, so the 2 games are likely Tetris-style plus a variant. EMC EM73000-family core. | | |

Sources:
- (face SVG text: `assets/E33_2in1.svg`)
- BrickEmuPy README line "Brick Game E-33 2 in 1 (SC6383)"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| manufacturer | Brick Game group, unidentified 机之宝 brand (Medium) | Brick Game group, maker **E-Star (Medium)** | The Hard Drop wiki lists "the E-23 and E-33 and E-88" as early E-Star models with 1, 2 and 8 games, which matches "2 IN 1". Several eBay listing titles read "Brick Game Handheld 2 in 1 Game Console E-33 E-Star" (search results; 403 on fetch). The face carries the same smiley-with-hands 机之宝 logo as the E-Star E-23. Caveat from one listing: a "Micro Genius Brick Game 4 in 1 Model E-33" exists, so the E-33 number was reused by others. The 2-in-1 with the E-Star logo is the E-Star one. Chinese/Taiwanese searches for 机之宝/機之寶 found nothing on the logo itself. |
| date | 1995-01-01 placeholder (Unknown) | **1994-01-01 (Low)** | Still no direct date. Hard Drop groups the E-33 with the early E-23/E-88 models (E-Star's earliest units "possibly 1992 or earlier"; copies c.1993-94; bigger-count models c.1995-97). The sibling E-88's die is 1994 and the E-23's is 1993. 1994 is an era estimate, not a source date. |

Sources:
- https://harddrop.com/wiki/Brick_Game - E-33 as an early E-Star 2-game model
- https://www.ebay.com/itm/388054316961 - "Vintage Brick Game Handheld 2 in 1 Game Console E-33 E-Star WORKING UNUSED" (title, search result)
- https://www.ebay.de/itm/404760249570 - "brick game 2 in 1 E-33" (title, search result)
- https://ynk.ucoz.ru/index/0-57 - Russian collector list includes "BRICK GAME 2 in 1(E-33 )"


### NEC uPD750x

#### `EpochMonsterPanic` – Monster Panic

**Final:** name **Monster Panic** (High); group **Epoch** (High); date **1981-01-01** (Medium); model **36520** (Low); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Monster Panic** (モンスターパニック). Series "Pocket Digit-Com" (EPOCH POCKET DIGIT-COM) is printed on the face; keep it out of the name. | High | Face SVG, Japanese review site, multiple listings |
| manufacturer | Epoch (エポック社) (group: **Epoch**). French release by ITMC. | High | Face, handheldmuseum, Electronic Plastic |
| date | 1981-01-01 | Medium | 1981: handheldmuseum, Electronic Plastic (id 247, small model), Handheld Empire, a JP YouTube description. 1982: one personal JP blog (its owner bought the unit Jan 1983). A separate larger "Super Wide" version is dated 1983 by Electronic Plastic. Going with 1981. |
| region / variant | n/a (Japanese Pocket Digit-Com original, orange/black case per face) | Medium | |
| model | 36520 | Low | handheldmuseum.com only |
| clone_of | none in the list. Outside the list: Gamatronic "Krazy Kong" / Grandstand "Crazy Kong" are the same game in another case; ITMC "Monster Panic" in France. | Medium | handheldmuseum, Handheld Empire |
| keys | keep auto | High | Face: JUMP (also START) above ATTACK (also LAP·RESET), ◄/DATE, ►/ALARM, MODE, ADJUST. Auto Attack=A, Jump=B, Mode=MINUS, Adjust=R is fine. |
| notes | Japanese review: the boy escapes a monster house with Frankenstein, Dracula, a gill-man, a mummy and a skeleton; the attack key kills three of them. NEC uPD750x. Clock, stopwatch, alarm and date functions. | | |

Sources:
- (face SVG text: `assets/EpochMonsterPanic.svg`)
- https://www.handheldmuseum.com/Epoch/MonsterPanic.htm - 1981, Model# 36520, Krazy Kong clone
- http://www.electronicplastic.com/game/?company=epoch&id=247 - Monster Panic 1981, made in Japan, clock
- http://www.electronicplastic.com/game/?company=epoch&id=255 - "Super Wide" version 1983 / ITMC
- https://handheldempire.com/game.jsp?game=740 - Epoch 1981; ITMC and Grandstand Crazy Kong related
- http://www5e.biglobe.ne.jp/kiden/shu.monpani.htm - "モンスターパニック / EPOCH POCKET DIGIT-COM / エポック社", gameplay, アタックキー
- https://note.com/shinonomesousaku/n/n974bf4e3b807 - says 1982 (conflict)
- https://paypayfleamarket.yahoo.co.jp/item/z483315694 - "エポック社 POCKET DIGIT-COM モンスターパニック"


### 6502 family (Sitronix?)

#### `MiniClassicsSoccer` – Soccer

**Final:** name **Soccer** (Medium); group **Stadlbauer** (Medium); date **2004-01-01** (Low); model **–** (Unknown); clone of **none** (Medium); keys: `btnStart`→`A|PLUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Soccer** (Nintendo Mini Classics series). Suggested display name: "Soccer" or "Mini Classics: Soccer". It is one of the Mini Classics "original titles", **not** a Game & Watch port. | Medium | Wikipedia lists Soccer under "Original titles"; eBay/YouTube titles say "Nintendo Mini Classics Soccer" |
| manufacturer | Stadlbauer (Austrian; MAME says "The 'Mini Classics' keychains are by Stadlbauer, not Nintendo"), sold under the Nintendo Mini Classics banner. US distribution by It's Outrageous!, Brazil by Candide. (group: new **Stadlbauer**, or **Nintendo** if the maintainer files Mini Classics under the licensor) | Medium | MAME hh_sm510, Wikipedia |
| date | 1998-01-01 | Low | eBay listing title "Nintendo Mini Classics Soccer … 1998"; series launched 1998. README says "1998/2004", origin unexplained (maybe a later production run or chip date). One YouTube unboxing calls it one of the last Mini Classics titles. No primary date found. |
| region / variant | This dump: white case with GAME, ALARM/START and PAUSE buttons. Another version exists: a thumbnail of an orange unit shows "TIME/ACTION". | Low | Face SVG; YouTube thumbnail Bl-t9Ra-uJo |
| model | unknown | Unknown | |
| clone_of | none | | |
| keys | Suggested override: `{"btnStart": "A\|PLUS"}` | Low | Face: GAME, ALARM/START (big button), PAUSE. The brick binds btnStart to Space/X (action-style hotkeys) and Pause to Enter, so ALARM/START is probably the in-game kick/action button. Auto mapping puts it only on PLUS. Not verified against a manual. |
| notes | Distributed without ROM. Config lives under `assets/test/` in BrickEmuPy (the face path is `./assets/test/MiniClassicsSoccer.svg`), so it may be experimental. Renamed from MiniClassicSoccer in Jul 2026. LCD shows "SPIN NOW!", a power "MAX" meter, a penalty-shootout goal scene and a clock/alarm. Sitronix STK55C324 (6502). | | |

Sources:
- (face SVG text: `assets/MiniClassicsSoccer.svg`)
- https://en.wikipedia.org/wiki/Nintendo_Mini_Classics - Stadlbauer; Soccer under "Original titles"; It's Outrageous / Candide
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_sm510.cpp - "Mini Classics keychains are by Stadlbauer, not Nintendo"
- https://www.ebay.com/itm/366530625808 - listing title "Nintendo Mini Classics Soccer Electronic Keychain Game Brand New Old-Stock 1998" (search result only; page 403)
- https://www.youtube.com/watch?v=Bl-t9Ra-uJo - "Soccer - Nintendo Mini Classics LCD" (thumbnail: orange TIME/ACTION version)
- https://www.youtube.com/watch?v=HA0N50rTCb0 - "Nintendo Mini Classics - Soccer - LCD handheld game"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1998-01-01 (Low) | **2004-01-01** (Low). Alternative: keep 1998-01-01. | This is inference; no dated primary source was found. The README's "1998/2004" most plausibly means the series/licence year (1998, printed as the Nintendo licence year on Mini Classics units) plus the year of this unit (2004). Arguments for a later date: (a) Wikipedia lists Soccer under the non-Game & Watch "Original titles", and says the third US distributor, It's Outrageous, "introduced several of the newer Mini Classics to America", including Soccer. (b) Wikipedia says the Toymax 1998-99 waves were only Game & Watch ports (SMB, DK Jr., Fire, Parachute, then Octopus and Cement Factory). (c) One YouTube unboxing calls Soccer one of the last Mini Classics titles (first pass). The eBay "1998" listing and backloggd's 1998 entry probably reflect the series' 1998 licence date. |
| keys | `{"btnStart": "A\|PLUS"}` (Low) | same override, **Medium** | A German museum inventory sheet (FAU ISER, Mini Classics SMB) says Mini Classics units have a D-pad and three buttons, where "a big action button is also used to set the alarm" ("Eine große Aktionstaste wird auch zum Einstellen des Alarms verwendet"). That confirms ALARM/START is the action button. |
| notes | - | **Dump/config bug:** `MiniClassicsSoccer.brick` still points to `./assets/test/MiniClassicsSoccer.svg` and `./assets/test/MiniClassicsSoccer.bin`, but the SVG was moved to `assets/MiniClassicsSoccer.svg` (commit b8763c2, 2026-07-17, "rename Mini Classics Soccer"). There is no `assets/test/` copy in the clone, so the face path is broken upstream. The face also has a D-pad (btnUp/Down/Left/Right). | local clone |

Sources:
- https://en.wikipedia.org/wiki/Nintendo_Mini_Classics - Soccer in "Original titles"; It's Outrageous introduced newer titles including Soccer; Toymax 1998/1999 waves
- https://iser.wisski.data.fau.de/sites/default/files/2024-06/Nintendo-MiniC.pdf - museum sheet: big action button also sets the alarm; Stadlbauer, Salzburg distributor; SMB unit 1998
- https://backloggd.com/series/nintendo-mini-classics/ - lists Soccer as 1998 (search snippet; probably the series date)


### 6502 family (Mitsubishi 740)

#### `PocketPikachu` – Pocket Pikachu

**Final:** name **Pocket Pikachu** (High); group **Nintendo** (High); date **1998-03-27** (High); model **MPG-001** (Medium); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Pocket Pikachu** (ポケットピカチュウ, Japan). It was sold as **Pokémon Pikachu** in North America and Europe. This is the original 1998 monochrome model, **not** Pocket Pikachu Color / Pokémon Pikachu 2 GS. | High | The face shows a monochrome dot matrix with clock, gift and coin (slot game) icons and a 5-digit step counter, which matches Serebii's description of the original. No IR peripheral in the brick. PP Color has a colour screen and IR link to Gold/Silver (official Nintendo page). |
| manufacturer | Nintendo, developed by Jupiter Corporation (group: **Nintendo**) | High | Wikipedia, Bulbapedia (search snippet), the Nintendo JP PP Color page credits "Nintendo / Creatures / GAME FREAK / JUPITER Corp." |
| date | 1998-03-27 (Japan, first release) | High | Serebii, Wikipedia, Japanese Pokémon Wiki snippet. NA: 1998-11-02; EU: Dec 1998. (PP Color: 1999-11-21 JP.) |
| region / variant | Most likely the Japanese unit, inferred from the BrickEmuPy name "Pocket Pikachu"; the display is icon-only, so the ROM region cannot be read from the face. | Low | |
| model | MPG-001 | Medium | pricecharting "#MPG-001" plus eBay listing titles "Pocket Pikachu … MPG-001 … Japan" (listings only) |
| clone_of | none | | |
| keys | keep auto | High | Pokémon fandom wiki: it "features the same five buttons on the front" as a Game Boy (D-pad, A, B, Start, Select); Serebii lists A, B, Start, Select, Reset and a control pad. The face SVG matches. "reflector" = pedometer step sensor (X), Reset = PLUS+MINUS. |
| notes | Distributed without ROM. Mitsubishi M37520-family MCU. Serebii says 7 steps = 1 watt; Wikipedia says 20. Step counter caps at 99,999. | | |

Sources:
- https://www.serebii.net/virtualpet/pokemonpikachu/ - JP release 1998-03-27; buttons; clock/gift/coin features
- https://en.wikipedia.org/wiki/Pok%C3%A9mon_Pikachu - JP 1998-03-27, NA 1998-11-02, EU Dec 1998; Pocket Pikachu naming
- https://pokemon.fandom.com/wiki/Pok%C3%A9mon_Pikachu - "same five buttons on the front" as Game Boy (read via the MediaWiki API)
- https://www.nintendo.co.jp/n09/pika_kg/index.html - Pocket Pikachu Color: 1999-11-21, colour screen, IR (rules out PP Color)
- https://www.pricecharting.com/de/game/pokemon-mini/pocket-pikachu-console - "#MPG-001"


### 6502 family (Generalplus)

#### `TamagotchiMini2017` – Tamagotchi Mini (20th Anniversary)

**Final:** name **Tamagotchi Mini (20th Anniversary)** (High); group **Bandai** (High); date **2017-04-08** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tamagotchi Mini** (Japan: Chibi Tamagotchi / ちびたまごっち). Suggested display name: "Tamagotchi Mini (20th Anniversary)" or "Tamagotchi Mini (2017)". | High | Tamagotchi wiki (fandom); 16x16 face grid matches the Mini's 16x16 screen |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | 2017-04-08 (Japan re-release, first of 2017). US: 2017-11-05. Original Mini: JP 2005-05-09, NA Aug 2005. | High | Fandom infobox; Bandai America US date also in search results (CNN, Romper, Christian Post) |
| region / variant | 2017 20th-anniversary edition. Fandom says it is "functionally identical in all regions" (secret character Bill, rice meal), so the dump region does not matter. | Medium | Fandom |
| model | unknown | Unknown | |
| clone_of | none in the list | | |
| keys | keep auto | Medium | Real buttons A (left, function), B (middle, clock), C (right, check). Auto: Left=LEFT\|Y, Center=A\|UP\|DOWN, Right=RIGHT\|B. Acceptable. For consistency with the packed E0C6200 Tamagotchis, use whatever A/B/C convention they use. |
| notes | **Dump flagged by BrickEmuPy: "High probability of errors in the dump"**; distributed without ROM. Suggest include behind Experimental with a note, or skip (Decision 4). Generalplus GPL191X. Fandom notes a 2017 glitch: death of Bill/Oyajitchi lasts 1 hour. Real behaviour, not a dump error. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Mini - infobox (JP 2005-05-09, re-release 2017-04-08; NA Aug 2005, re-release 2017-11-05), 16x16, A/B/C functions, 2017 version identical across regions (read via the MediaWiki API)
- https://money.cnn.com/2017/10/10/technology/tamagotchi-mini/index.html - 2017 US relaunch (search result)
- https://www.romper.com/life/where-to-buy-the-new-tamagotchi-2980591 - 2017 US release (search result)


### Seiko Epson E0C6200

#### `AlienFever` – Chibi Pachi: Alien Fever

*Already packed as `ALIENFEV`.*

**Final:** name **Chibi Pachi: Alien Fever** (High); group **Epoch** (High); date **1997-01-01** (Medium); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Chibi Pachi: Alien Fever**. Japanese ちびパチ エイリアンフィーバー; the kana spelling is assumed, not seen on a source. The inlay prints "ALIEN FEVER". "Chibi Pachi" is the series (pachislot keychains); "Alien Fever" alone is also defensible. | High (Latin title) / Low (kana) | MAME `alienfev` description "Chibi Pachi: Alien Fever"; face inlay |
| manufacturer | Epoch (group: **Epoch**) | High | MAME, README |
| date | 1997-01-01 | Medium | MAME SYST year 1997; no second source (search budget gone) |
| region / variant | n/a (Japan) | | |
| model | unknown | Unknown | |
| clone_of | none | | |
| keys | keep auto | High | MAME inputs Mode/Select/Sound/Handle match the brick (Handle = pachislot lever = A) |
| notes | MAME: "It's a Pachislot keychain game, the MCU constantly runs on the higher-speed OSC3"; E0C6S46, 39x16 LCD. ROM CRC in MAME e561599c. I did not compare it with the BrickEmuPy bin. | | |

Sources:
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `SYST( 1997, alienfev, … "Epoch", "Chibi Pachi: Alien Fever")`, hardware notes
- (face SVG inlay: `assets/AlienFever.svg`)


#### `DigimonV1` – Digital Monster Ver.1 (Japan, Rev. B)

*Already packed as `DIGIMON`.*

**Final:** name **Digital Monster Ver.1 (Japan, Rev. B)** (High); group **Bandai** (High); date **1997-06-26** (High); model **–** (Unknown); clone of **DigimonV1JB** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.1**. Japanese title on the box and unit: デジタルモンスター Ver.1. The franchise was later branded "Digimon", and international units were sold as "Digimon"/"Digital Monster". | High | wikimon, ja.wikipedia, chromagi (original name デジタルモンスター Ｖｅｒ．１) |
| manufacturer | Bandai (group: **Bandai**). Concept/planning by WiZ. | High | ja/en Wikipedia, wikimon |
| date | **1997-06-26** (Japan). Additional colours followed in November 1997. North America: 1997-12-05. | High | wikimon "June 26, 1997"; en.wikipedia "June 26th in Japan, and on December 5th in North America"; ja.wikipedia "1997年6月発売・追加カラー11月発売"; weblio/Wikipedia-derived JA text 1997年6月26日 |
| region / variant | Japan. Pre-rename copy of `DigimonV1JB` (Rev. B). | High (upstream commit) | commit c76422b rename |
| model | none found for the Japanese release. Bandai Asia export item no. **1855**, for reference only. | Low | chromagi 1855 listing (Asian export code, not Japanese) |
| clone_of | **DigimonV1JB** (same config and ROM, older file name) | High | commit c76422b |
| keys | keep auto. The legacy brick has no controls listed. If kept, give it the same mapping as V1JB: Top = B, Center = A, Bottom = X, Reset = PLUS+MINUS. | Medium | brick files |
| notes | Legacy duplicate. Recommend not listing it separately (or aliasing it to V1JB). The handoff README line "1JA" is a generator error; it is 1JB. The face shows a red shell. | | |

Sources:
- https://wikimon.net/Digital_Monster_Ver._1 - Japan 26 Jun 1997, US 1997, colours Gray/Brown/Yellow/Blue/Green/Red
- https://wikimon.net/Digital_Monster - version table (Ver.1 Jun 26 1997, Ver.2 Dec 1997, Ver.3 Mar 1998, Ver.4 May 1998, Ver.5 Aug 1998)
- https://en.wikipedia.org/wiki/Digital_Monster - Japan 26 Jun 1997, North America 5 Dec 1997; US renames Darkmon/Teddymon; "Megalithic Mainframe" in place of death
- https://ja.wikipedia.org/wiki/デジタルモンスター - Ver.1 1997年6月 (追加カラー11月), Ver.2 1997年12月 (追加カラー2月), Ver.3 1998年3月, Ver.4 1998年5月, Ver.5 1998年7月
- https://dejideji.com/dejimonsyodai/ - per-version months and colours (Ver.1 brown/grey first, red/blue/yellow/green late 1997)
- https://chromagi.com/bandai/vpet/1855?lang=en - Asian export Ver.1, product code 1855
- https://github.com/azya52/BrickEmuPy/commit/c76422b - DigimonV1 -> DigimonV1JB rename


#### `DigimonV1ES` – Digital Monster Ver.1 (Europe)

*Already packed as `DIGIMONES`.*

**Final:** name **Digital Monster Ver.1 (Europe)** (Medium); group **Bandai** (High); date **1998-01-01** (Low); model **–** (Unknown); clone of **DigimonV1JA** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | The game is Ver.1. Suggest **`Digital Monster Ver.1 (Europe)`** (or `(English)`) **instead of the packed "Digital Monster (Spanish)"**. The exact text printed on European units/boxes was not found; international releases were marketed as "Digimon" ("Digital Monster"). | Medium (variant) / Low (exact printed title) | see notes |
| manufacturer | Bandai (group: Bandai). European distribution was by Bandai's European arm. The Spanish distributor (Bandai España?) is unconfirmed. | High (Bandai) / Unknown (distributor) | |
| date | **1998-01-01** proposed (Low). The European launch date was not found. The English-text Ver.1 first came out in North America on **1997-12-05**; Europe also received Ver.2 (Japan Dec 1997), which points to a 1998 European rollout. Keeping 1997-12-05 (the first English-language release) is the alternative. The packed 1997-01-01 is too early: the product didn't exist before June 1997. | Low | en.wikipedia (NA date); withthewill (Europe got Ver.1 and Ver.2) |
| region / variant | Europe / English-text export ROM. In English-text units a Digimon "returns to the Megalithic Mainframe" (computer icon) instead of dying (gravestone). The "ES" suffix is upstream's own; no evidence was found of a Spanish-language Digimon v-pet. The pets have no text on screen, so a "Spanish" ROM would be meaningless; ES most likely means the unit was dumped from a Spanish-market (European) unit. | Medium | vpet-emu-zepp labels this ROM "Digimon Ver. 1 (EU)"; web-brick README "Digimon Ver. 1 (English)"; withthewill: "The V-Pets from the US, Europe and Australia have all English text" and "I've seen European V-Pet ver.1 in yellow with light grey trim, blue with light grey" - the BrickEmuPy face `DigimonV1ES.svg` is exactly a **blue shell with light-grey trim** |
| model | unknown for Europe. Asia export Ver.1 is 1855; the European code is probably the same family. | Low | chromagi |
| clone_of | DigimonV1JA (regional ROM of the same game) | Medium | same game, localised ROM |
| keys | keep auto (Center = A, Top = B, Bottom = X, Reset = PLUS+MINUS) | Medium | brick |
| notes | Rename away from "(Spanish)" unless the maintainer has evidence of a Spain-specific unit. Different ROM from the Japanese ones (separate .bin). Death and egg graphics differ from the Japanese ROM per collectors; US manuals rename Devimon -> Darkmon and Monzaemon -> Teddymon. Has the CON_DGM peripheral. | | |

Sources:
- https://github.com/mabuchner/vpet-emu-zepp - DigimonV1ES.bin = "Digimon Ver. 1 (EU)"
- https://github.com/itsbigcforme67/web-brick - lists "Digimon Ver. 1 (English)"
- https://withthewill.net/threads/digimon-v-pet-colours.8641/ - European Ver.1 colours (blue with light grey); US/Europe/Australia units have English text; Europe also got Ver.2; regional keychains
- https://withthewill.net/threads/virtual-pets-released-outside-of-japan.26496/ - Ver.1 and Ver.2 "(Europe too)"
- https://en.wikipedia.org/wiki/Digital_Monster - NA 5 Dec 1997; Megalithic Mainframe; Darkmon/Teddymon
- https://github.com/azya52/BrickEmuPy/commit/c76422b - ES added 24 Mar 2026

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1998-01-01 (Low) | **No change** | The English and Spanish searches ("mascota virtual" Bandai 1998 España) found no European or Spanish v-pet launch date. They did find that the Digimon anime premiered in Spain on 30 March 2000 (La 2), which was after the original v-pet era. Wikimon's Digital Monster page gives only the Ver.1 Japan date of 26 June 1997. chromagi lists only Asian-English exports (#1855 Ver.1, Bandai H.K.). |
| name / region | "Ver.1 (Europe)" rather than Spanish (Medium) | **No change, slightly reinforced.** No Spanish-language or Spain-specific Ver.1 was found anywhere. Recommend dropping "(Spanish)". | same searches |

Sources:
- https://wikimon.net/Digital_Monster - Ver.1 26 June 1997; Ver.6 Australia-exclusive 2002; no EU data
- https://chromagi.com/bandai/vpet/1855?lang=en - Ver.1 #1855 Asian-English export by Bandai H.K. (search snippet)


#### `DigimonV1JA` – Digital Monster Ver.1 (Japan, Rev. A)

*Already packed as `DIGIMON`.*

**Final:** name **Digital Monster Ver.1 (Japan, Rev. A)** (Medium); group **Bandai** (High); date **1997-06-26** (High); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.1**. Suggested display name: `Digital Monster Ver.1 (Japan, Rev. A)` | High (title) / Low (meaning of "A") | as above; "JA" = Japan rev. A per vpet-emu-zepp's "JP-A" label |
| manufacturer | Bandai (group: Bandai) | High | |
| date | 1997-06-26 | High (for the product) / Unknown (for this particular revision) | wikimon, en/ja Wikipedia |
| region / variant | Japan, ROM revision A. The only difference from JB is the ROM (brick otherwise identical; same face DigimonV1J.svg). | Medium | brick diff; vpet-emu-zepp ROM table |
| model | none found for Japan (Asia export: 1855) | Low | chromagi |
| clone_of | none (treat as the parent of the Ver.1 family). If the maintainer's JA.bin == JB.bin, it is a duplicate of DigimonV1JB. | Medium | see the key finding |
| keys | keep auto (Center = A, Top = B, Bottom = X, Reset = PLUS+MINUS). The unit's three buttons are unlabeled (A/B/C in the manuals: top = A select, middle = B confirm, bottom = C cancel). The auto mapping is acceptable. | Medium | brick, face SVG |
| notes | Added upstream on 24 Mar 2026. **Verify that the ROM differs from JB.** The handoff says they are identical. Has the CON_DGM link peripheral (battles), which is not needed for solo play. | | |

Sources:
- https://github.com/mabuchner/vpet-emu-zepp - ROM table: DigimonV1JA.bin "Digimon Ver. 1 (JP-A)", V1JB "(JP-B)", V1ES "(EU)"
- https://github.com/itsbigcforme67/web-brick - "Digital Monster Ver. 1 (two revisions)"
- https://github.com/azya52/BrickEmuPy/commit/c76422b
- https://wikimon.net/Digital_Monster_Ver._1


#### `DigimonV1JB` – Digital Monster Ver.1 (Japan, Rev. B)

*Already packed as `DIGIMON`.*

**Final:** name **Digital Monster Ver.1 (Japan, Rev. B)** (Medium); group **Bandai** (High); date **1997-06-26** (High); model **–** (Unknown); clone of **DigimonV1JA** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.1**. Suggested display name: `Digital Monster Ver.1 (Japan, Rev. B)` | High / Low (meaning of "B") | as above |
| manufacturer | Bandai (group: Bandai) | High | |
| date | 1997-06-26 (product launch; revision date unknown) | High / Unknown | |
| region / variant | Japan, ROM revision B. This was the original "DigimonV1" config before 24 Mar 2026. | High | commit c76422b |
| model | none found for Japan (Asia export: 1855) | Low | chromagi |
| clone_of | DigimonV1JA (same game, other ROM revision); or none if the maintainer prefers JB as the parent, since it is the longer-established dump | Medium | |
| keys | keep auto | Medium | |
| notes | Same face and config as JA. Can't say which revision is earlier or later. Two Japanese production waves are documented (June 1997, then November 1997 colours), but nothing ties a ROM to either wave. | | |

Sources:
- https://github.com/azya52/BrickEmuPy/commit/c76422b
- https://github.com/mabuchner/vpet-emu-zepp
- https://dejideji.com/dejimonsyodai/


#### `DigimonV2` – Digital Monster Ver.2

*Already packed as `DIGIMONV2`.*

**Final:** name **Digital Monster Ver.2** (High); group **Bandai** (High); date **1997-12-01** (High); model **–** (Unknown); clone of **DigimonV2J** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.2** (デジタルモンスター Ver.2) | High | wikimon, ja.wikipedia, chromagi |
| manufacturer | Bandai (group: Bandai) | High | |
| date | **1997-12-01** (Japan, December 1997; exact day not found). Additional clear colours February 1998. | High (month) | ja.wikipedia "1997年12月発売・追加カラー2月発売"; wikimon "December 1997"; dejideji |
| region / variant | Japan. Pre-rename copy of DigimonV2J. | High | commit c76422b |
| model | none found for Japan (Asia export: **1856**) | Low | chromagi 1856 |
| clone_of | **DigimonV2J** | High | commit c76422b |
| keys | keep auto (same as V2J if controls are added) | Medium | |
| notes | Legacy duplicate; recommend not listing it separately. The BrickEmuPy face shows a grey shell with yellow trim and buttons. | | |

Sources:
- https://wikimon.net/Digital_Monster_Ver._2 - Japan December 1997, US 1997; colours white, black, clear, smoke
- https://ja.wikipedia.org/wiki/デジタルモンスター
- https://chromagi.com/bandai/vpet/1856/ - Asia export code 1856
- https://github.com/azya52/BrickEmuPy/commit/c76422b


#### `DigimonV2J` – Digital Monster Ver.2

*Already packed as `DIGIMONV2`.*

**Final:** name **Digital Monster Ver.2** (High); group **Bandai** (High); date **1997-12-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.2** (no region suffix needed once the legacy V2 is dropped) | High | |
| manufacturer | Bandai (group: Bandai) | High | |
| date | 1997-12-01 | High (month) / day unknown | ja.wikipedia, wikimon, dejideji |
| region / variant | Japan ROM ("J") | High | upstream naming |
| model | none found for Japan (Asia export 1856) | Low | chromagi |
| clone_of | none | High | |
| keys | keep auto | Medium | |
| notes | Same ROM as legacy `DigimonV2`. Has CON_DGM. | | |

Sources: as for DigimonV2.


#### `DigimonV3` – Digital Monster Ver.3

*Already packed as `DIGIMONV3`.*

**Final:** name **Digital Monster Ver.3** (High); group **Bandai** (High); date **1998-03-01** (High); model **–** (Unknown); clone of **DigimonV3J** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.3** (デジタルモンスター Ver.3) | High | |
| manufacturer | Bandai (group: Bandai) | High | |
| date | **1998-03-01** (Japan, March 1998; day not found) | High (month) | ja.wikipedia "1998年3月発売"; wikimon "March 1998"; chromagi |
| region / variant | Japan. Pre-rename copy of DigimonV3J. | High | commit c76422b |
| model | none found for Japan (Asia export **1857B**) | Low | chromagi 1857B listing URL |
| clone_of | **DigimonV3J** | High | |
| keys | keep auto | Medium | |
| notes | Legacy duplicate. The face shows an orange shell with navy trim and green buttons. Japanese colours were orange, purple, clear orange and clear purple. | | |

Sources:
- https://wikimon.net/Digital_Monster_Ver._3 - Japan March 1998, US 1998; colours
- https://ja.wikipedia.org/wiki/デジタルモンスター
- https://chromagi.com/bandai/vpet/1857b-4892762018517/ (seen in search results, export code 1857B)
- https://github.com/azya52/BrickEmuPy/commit/c76422b


#### `DigimonV3J` – Digital Monster Ver.3

*Already packed as `DIGIMONV3`.*

**Final:** name **Digital Monster Ver.3** (High); group **Bandai** (High); date **1998-03-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.3** | High | |
| manufacturer | Bandai (group: Bandai) | High | |
| date | 1998-03-01 | High (month) | |
| region / variant | Japan ROM | High | |
| model | none found for Japan (Asia export 1857B) | Low | |
| clone_of | none | High | |
| keys | keep auto | Medium | |
| notes | Same ROM as legacy `DigimonV3`. Has CON_DGM. | | |

Sources: as for DigimonV3.


#### `DigimonV4J` – Digital Monster Ver.4

*Already packed as `DIGIMONV4`.*

**Final:** name **Digital Monster Ver.4** (High); group **Bandai** (High); date **1998-05-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Digital Monster Ver.4** (デジタルモンスター Ver.4) | High | wikimon, ja.wikipedia, chromagi |
| manufacturer | Bandai (group: Bandai) | High | |
| date | **1998-05-01** (Japan, May 1998; day not found) | High (month) | ja.wikipedia "1998年5月発売"; wikimon "May 1998"; dejideji; chromagi |
| region / variant | Japan ROM (only Ver.4 config, so no suffix needed) | High | |
| model | none found for Japan (Asia export **1858**, GTIN 4543112014467) | Low | chromagi 1858 |
| clone_of | none | High | |
| keys | keep auto | Medium | |
| notes | Japanese colours were clear red and clear blue. Silver "D-1 Grand Prix" special editions (metallic red/blue, 10,000 each) came later. The BrickEmuPy face shows a red shell with white trim. Added upstream on 24 Mar 2026. | | |

Sources:
- https://wikimon.net/Digital_Monster_Ver._4 - Japan May 1998, US 1998; clear red and clear blue
- https://ja.wikipedia.org/wiki/デジタルモンスター - 1998年5月発売; D-1スペシャル
- https://dejideji.com/dejimonsyodai/ - D-1 limited colours (10000個限定)
- https://chromagi.com/bandai/vpet/1858-4543112014467/ - Asia export code 1858


#### `nikko` – Venus Diet Monogatari

*Already packed as `VENUSDM`.*

**Final:** name **Venus Diet Monogatari** (High); group **Nikko** (High); date **1997-01-01** (Medium); model **–** (Unknown); clone of **none** (Low); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Unknown.** The only name anywhere is upstream's "Nikko virtual pet" (README and commit "add Nikko virtual pet", 26 May 2024). No evidence was found for "Venus" (the packed `VENUSDM`): searches for Nikko + Venus + virtual pet returned nothing. Unless the maintainer has a source for "Venus", suggest the neutral `Virtual Pet` under the Nikko group. | Unknown | README; commit 0033af5; searches listed in notes |
| manufacturer | Nikko (group: **Nikko**, existing group). Nikko is the Japanese toy/RC maker (ニッコー). Brand attribution rests only on the dumper's label. | Low | upstream label only |
| date | 1997-01-01 suggested. This is a guess based on the 1997 virtual-pet boom; the tama-style 3-button egg/bean keychain points to 1997-98. | Low | inference |
| region / variant | n/a | | |
| model | unknown | Unknown | |
| clone_of | none known. The display is a dot matrix of 32 columns (the SVG has col_0..col_31), like Tamagotchi P1, but the ROM is 12 KB at a 32,768 Hz clock (a low-power E0C6S4x-type setup) and is not known to be a reskin. | Low | brick + SVG |
| keys | keep auto (Left = LEFT\|Y, Center = A\|UP\|DOWN, Right = RIGHT\|B). The face has three unlabeled yellow buttons, two upper (left/right) and one lower centre, the usual tama-style select/confirm/cancel layout. | Medium | rendered face SVG |
| notes | The face (rendered from `nikko.svg`) is a green bean/egg-shaped keychain shell with a keyring loop, white hearts and small flowers printed above and below the LCD, and three yellow buttons. There is no printed text or logo in the SVG. The ROM **is** distributed with BrickEmuPy (unlike the Bandai pets). A MAMEWorld EmuChat post announced the addition ("BrickEmuPy - Nikko virtual pet added"), but the site couldn't be fetched (DNS failure). Searches tried: "Nikko virtual pet 1997", "Nikko Venus virtual pet", "Nikko digital pet / cyber pet 1997", Japanese "ニッコー 育成ゲーム 1997 液晶 キーチェーン", and a "Venus" keychain pet. None found the product. Follow-up ideas: Yahoo Auctions "ニッコー 育成", eBay "Nikko virtual pet" photos, the MAMEWorld thread, or asking azya52. | | |

Sources:
- https://github.com/azya52/BrickEmuPy/commit/0033af5 - "add Nikko virtual pet" (nikko.bin, .brick, .svg), May 26, 2024
- https://github.com/azya52/BrickEmuPy/blob/main/README.md - "Nikko virtual pet"
- https://www.mameworld.info/ubbthreads/showthreaded.php?Cat=2&Number=398587&sb=5&fpart=1&vc=1 - thread title "BrickEmuPy - Nikko virtual pet added." (seen in search results only; not fetchable)
- https://github.com/itsbigcforme67/web-brick - lists "Nikko" with ROM "published by BrickEmuPy"

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| name | Unknown ("Nikko virtual pet") | **Beans Collection: Venus Diet Monogatari** (High). Japanese title as written on listings: ビーンズコレクション ヴィーナス ダイエットものがたり. Suggested display name: `Venus Diet Monogatari`, with the series prefix "Beans Collection" if the maintainer keeps series prefixes. A loose English gloss is "Venus Diet Story". | MAME `hh_e0c6200.cpp` driver header: "Nikko Beans Collection: Venus Diet Monogatari". `SYST( 1997, venusdm, ... "Nikko", "Beans Collection: Venus Diet Monogatari")`. MAME 0.274 whatsnew credits it as "[hap, azya]", so the BrickEmuPy author also worked on it. The ROM hash is identical (computed locally). A Yahoo-Auctions-derived aucfan listing title reads "貴重 ビーンズコレクション ヴィーナス ダイエットものがたり NIKKO ニッコージャパン" (search snippet). |
| manufacturer | Nikko (Low) | **Nikko (Nikko Japan, ニッコー / ニッコージャパン)**; group **Nikko** (High) | MAME, and the aucfan listing title "NIKKO ニッコージャパン". Amazon.co.jp lists a ニッコー "携帯ゲーム ビーンズコレクション ヴィーナス【ホワイト】" (search snippet). |
| date | 1997-01-01 (Low) | **1997-01-01** (Medium) | MAME SYST year 1997 (single source, but a curated one). No month found. |
| model | unknown | Possibly **103204** (Low) | The Amazon.co.jp snippet gives manufacturer reference 103204 for "ビーンズコレクション ヴィーナス【ホワイト】". That may be a white-shell variant, or another Venus title in the line, rather than this green unit. The page itself returned 500, so this could not be checked. |
| notes | - | MAME describes the hardware as an E0C6S46 with a 32x20 LCD and 1-bit sound. BrickEmuPy uses the E0C6200 core. "Beans Collection" is a Nikko keychain line; it is unrelated to Takara Tomy Arts' later "Beans Collection" plush line. It is a "Venus"-themed diet pet game, which fits the hearts and flowers on the shell. | |

Sources:
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - venusdm driver: maker, title, 1997, E0C6S46, 32*20 LCD, ROM CRC 2228b081 / SHA1 22f6a2ed...
- https://github.com/mamedev/www.mamedev.org/blob/master/releases/whatsnew_0274.txt - "Beans Collection: Venus Diet Monogatari [hap, azya]" (found via GitHub code search)
- https://aucfan.com/intro/q-~a5b3a5eca5afa5b7a5e7a5f3204e494b4b4f/ - listing title "ビーンズコレクション ヴィーナス ダイエットものがたり NIKKO ニッコージャパン" (search snippet; fetch 503)
- https://www.amazon.co.jp/dp/B002TEX8QI - ニッコー 携帯ゲーム ビーンズコレクション ヴィーナス【ホワイト】, ref 103204 (search snippet; fetch 500)


#### `RadioShackStackChallenge` – Stack Challenge

*Already packed as `STACKCH`.*

**Final:** name **Stack Challenge** (High); group **Tandy** (High); date **1991-01-01** (High); model **60-2247** (High); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Stack Challenge** (logo on the unit: "Radio Shack STACK CHALLENGE"; catalogue: "Stack Challenge™") | High | Face SVG logo; 1991 Christmas Toys flyer; 1992 RS catalogue p.163; MAME `stackch` "Stack Challenge" |
| manufacturer | Radio Shack, a division of Tandy Corporation. The real OEM is unknown. (group: **Tandy**) | High | Face art shows the "Radio Shack" brand; MAME credits "Tandy Corporation" |
| date | **1991-01-01** (year only; on sale by December 1991) | High (year) | Radio Shack "Christmas Toys 1991" sale flyer (Dec 1991) cover: "5 NEW! Stack Challenge™. Move falling bricks in a row to lower wall. 60-2247 ... 19.95". 1992 annual catalogue (No. 472) p.163: "(2) NEW! Stack Challenge™ ... Ten skill levels. With sound effects. Requires 2 "AA" batteries. 60-2247 ... 19.95". Not in the 1991 annual catalogue games pages (p.154). MAME also says 1991. |
| region / variant | n/a (USA, Radio Shack) | High | |
| model | **60-2247** | High | Both catalogue scans above; handheldmuseum.com; MAME comment "(model 60-2247)" |
| clone_of | none | High | Generic Tetris-style brick game; no other brick in the list shares it |
| keys | keep auto | High | Printed labels on the face: LEFT/LEVEL, RIGHT/HEIGHT, DOWN/START, ROTATE / CONTRAST (big button), and small PAUSE, SOUND, ON/OFF. Auto mapping (Left/Down/Right on the d-pad, Rotate=A, Pause=MINUS, OnOff=PLUS, Sound=R) fits. |
| notes | MAME BTANB: the real unit ignores two simultaneous presses (for example left+rotate), so fast play is impossible. That is authentic behaviour, not an emulation bug. The README's "(C6246D2E)" is a chip/mask label, not the model number. The OEM maker is unidentified; handheldmuseum notes that Tandy games were usually licensed from other makers. | | |

Sources:
- https://www.radioshackcatalogs.com/flipbook/m-sale_1991_12.html (page image https://www.radioshackcatalogs.com/flipbook/catalogs/misc_sale/1991_12/1.jpg) - Dec 1991 "Christmas Toys 1991" flyer, NEW! Stack Challenge 60-2247 $19.95
- https://www.radioshackcatalogs.com/flipbook/1992_radioshack_catalog.html (page image .../flipbook/catalogs/main/1992/163.jpg) - 1992 catalogue No. 472 p.163, NEW! Stack Challenge 60-2247 $19.95
- https://www.radioshackcatalogs.com/flipbook/1991_radioshack_catalog.html (pages 153-157) - not listed in the 1991 catalogue's LCD games
- https://handheldmuseum.com/Tandy/StackChallenge.htm - "Radio Shack Stack Challenge", Model# 60-2247, year "19xx", "Tetris clone"
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `stackch`: Tandy Corporation, "Stack Challenge", 1991, model 60-2247, E0C6S46, BTANB note; ROM hash matches BrickEmuPy's .bin


#### `TamagotchiAngel` – Tenshitchi no Tamagotchi

*Already packed as `TAMAANG`.*

**Final:** name **Tenshitchi no Tamagotchi** (High); group **Bandai** (High); date **1997-08-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tenshitchi no Tamagotchi** (てんしっちのたまごっち). The shell art reads "ANGEL GOTCH". English release title: "Tamagotchi Angel". Alternative display name: `Tamagotchi Angel (Japan)`. | High (titles); Medium (that the dump is Japanese) | fandom; JA Wikipedia; MAME `tamaang` "Tenshitchi no Tamagotchi (Japan)". The rendered face shows the *classic* P1-style icons (scale, fork and knife, bat and ball, duck / scold, syringe, bulb, attention). Fandom says only the **first Japanese production run** used those icons; later Japanese and all English units have angel-themed icons. |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1997-08-01** (Japan, month only). English "Tamagotchi Angel": February 1998 (NA). | High | JA Wikipedia list (1997年8月, citing an official Bandai/Shogakukan guidebook p.43); fandom; dejideji and rekidai-carnival lists |
| region / variant | Japan (first production run) | Medium | Face icons; the MAME set with the same ROM size (16 KB) is the Japanese one |
| model | null (no Bandai JP product code found). Low: US Angel eBay MPN "1810". | Unknown / Low | One eBay listing only |
| clone_of | none | High | Its own game. "Takeshi no Tenshitchi no Tamagotchi" (Dec 1997, HANA-BI promo) has the same contents with a different shell. |
| keys | keep auto (screenFrame = X is the tap/vibration sensor) | High | Three unlabelled buttons; Angel was the first model with a vibration sensor |
| notes | 2024/2025 replicas exist (Gen 3 branding). The Japanese ROM differs from the English one (opening screen grave vs UFO, icons). | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Angel - JP Aug 1997, NA Feb 1998, ¥2,079, first-run classic icons, vibration sensor
- https://ja.wikipedia.org/wiki/たまごっち - list: てんしっちのたまごっち（1997年8月）
- https://dejideji.com/tamagochi-syoki-ichiran/ - てんしっち 1997年8月, 3 colours
- https://rekidai-carnival.com/tamagotchi-rekidai-hontai/ - 1997.08 てんしっちのたまごっち; 1998.02 Tamagotchi Angel
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `tamaang` "Tenshitchi no Tamagotchi (Japan)", PCB TAL-1, E0C6S48


#### `TamagotchiGenjintch` – Genjintchi no Tamagotchi

*Already packed as `TAMAGENJIN`.*

**Final:** name **Genjintchi no Tamagotchi** (High); group **Bandai** (High); date **1997-11-01** (Medium); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Genjintchi no Tamagotchi** (原人っちのたまごっち). Shell logo (face SVG): "Genjintch". Short forms: "Genjintchi", "Genjintch". It is not "Hakken!! Tamagotchi" or "Tamagotchi Genjin". | High | fandom; JA Wikipedia list; face SVG shows "Genjintch" on a bone |
| manufacturer | Bandai; a tie-in with Toei's film 北京原人 Who are you? (Peking Man) (group: **Bandai**) | High | |
| date | **1997-11-01** (Japan, month only) | Medium | JA Wikipedia list "原人っちのたまごっち（1997年11月）" and fandom "November 1997". Conflict: PC Watch (25 Nov 1997) says the film opened 13 Dec 1997 and that 原人っち would be a limited item sold at cinemas. So November is probably advance or theatre sales; December is possible. |
| region / variant | n/a (Japan-only limited edition) | High | |
| model | null | Unknown | Not found |
| clone_of | none | High | Fandom: programming is similar to Mothra no Tamagotchi, but it is a different game |
| keys | keep auto (screenFrame = X is a 4th input on K0.3, presumably a sensor/test input) | Medium | Three unlabelled yellow buttons on the face |
| notes | Only one shell design was made: brown with white bones and a red screen rim, which matches the SVG. ¥2,079. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Genjintchi_Tamagotchi - 原人っちのたまごっち, November 1997, theatres showing "Peking Man", ¥2,079
- https://ja.wikipedia.org/wiki/たまごっち - 原人っちのたまごっち（1997年11月）『北京原人 Who are you?』公開記念
- https://pc.watch.impress.co.jp/docs/article/yajiuma/9711.htm - 12月13日公開予定 ... 「原人っち」... 限定販売として映画館などで販売


#### `TamagotchiMorino` – Mori de Hakken!! Tamagotchi

*Already packed as `TAMAMORI`.*

**Final:** name **Mori de Hakken!! Tamagotchi** (Medium); group **Bandai** (High); date **1998-02-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Mori de Hakken!! Tamagotchi** (森で発見!!たまごっち). Nicknames: Morino, Forestgotchi. Cancelled English title: "Tamagotchi Garden". | Medium on "!!" vs "!" | JA Wikipedia and rekidai-carnival write "!!", matching the whole "…で発見!!" series (新種発見!!, ゲームで発見!!, 星で発見!!, TVで発見!!). Fandom and some auction titles write a single "!" (森で発見！たまごっち). The handoff's example "Mori de Hakken! Tamagotchi" uses one "!". Box-logo check still pending. |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1998-02-01** (Japan, month only) | High | JA Wikipedia (citing guidebook p.45), fandom, dejideji, rekidai all say February 1998 |
| region / variant | n/a (Japan only; the US release was cancelled) | High | |
| model | null | Unknown | |
| clone_of | none | High | |
| keys | keep auto (screenFrame = X is the tap/vibration sensor used to scare predators) | High | |
| notes | The face SVG is the Japanese "Yellow Ivy" wave-1 shell. Four JP shells: ivy, leaves, chestnut, white. ¥2,079. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Mori_de_Hakken!_Tamagotchi - Feb 1998, vibration sensor, Tamagotchi Garden cancelled
- https://tamagotchi.fandom.com/wiki/Mori_de_Hakken!_Tamagotchi/Shell_list - wave 1 Feb 1998 shells
- https://ja.wikipedia.org/wiki/たまごっち - 森で発見!!たまごっち（1998年2月）
- https://dejideji.com/tamagochi-syoki-ichiran/ - 森で発見 1998年2月
- https://rekidai-carnival.com/tamagotchi-rekidai-hontai/ - "森で発見!! たまごっち" 1998.02


#### `TamagotchiMothra` – Mothra no Tamagotchi

*Already packed as `TAMAMOT`.*

**Final:** name **Mothra no Tamagotchi** (High); group **Bandai** (High); date **1997-12-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Mothra no Tamagotchi** (モスラのたまごっち). Face art: "モスラ" logo and "MOTHRA NO たまごっち". Fandom English name: "Mothra Tamagotchi". | High | Face SVG; JA Wikipedia; MAME `tamamot` "Mothra no Tamagotchi (Japan)" |
| manufacturer | Bandai; tie-in with Toho's Rebirth of Mothra II (group: **Bandai**) | High | |
| date | **1997-12-01** (Japan, month only). Likely 1997-12-13, the film's opening day. | High (month); Low (day) | JA Wikipedia (1997年12月, limited production); fandom (December 1997). PC Watch: sold at cinemas as a limited item for the 13 Dec 1997 film. JA Wikipedia also mentions a limited version given to 5,000 advance-ticket buyers. |
| region / variant | n/a (Japan only) | High | |
| model | null | Unknown | |
| clone_of | none | High | |
| keys | keep auto (no sensor input on this one) | High | |
| notes | Fandom says this was the first Japanese model with a low-battery screen. A translucent shell was sold through a Toho scratch lottery. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Mothra_Tamagotchi - モスラのたまごっち, December 1997, ¥2,079
- https://ja.wikipedia.org/wiki/たまごっち - モスラのたまごっち（1997年12月）『モスラ2 海底の大決戦』公開記念、限定生産
- https://pc.watch.impress.co.jp/docs/article/yajiuma/9711.htm - 12/13 film, limited sale at cinemas
- https://wikizilla.org/wiki/Mothra_Tamagotchi - Bandai 1997 tie-in
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `tamamot`


#### `TamagotchiP1` – Tamagotchi (Gen. 1, International)

*Already packed as `TAMA`.*

**Final:** name **Tamagotchi (Gen. 1, International)** (Medium); group **Bandai** (High); date **1997-05-01** (High); model **1800** (Low); clone of **TamagotchiP1J** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tamagotchi (Gen. 1)**. Suggested variant label: `Tamagotchi (Gen. 1, International)` or `(Gen. 1, World)`, to pair with `Tamagotchi (Gen. 1, Japan)`. | Medium (that it is the English/World ROM) | BrickEmuPy keeps P1 and P1J separate, and MAME's base set `tama` is "Tamagotchi (Gen. 1, World)". There is no ROM in the clone to verify. |
| manufacturer | Bandai (Bandai America / Bandai UK for this release) (group: **Bandai**) | High | |
| date | **1997-05-01** (first English-language release, Canada). UK 1997-05-08; US 1997-05-28. | Medium | Fandom: Canada May 1, Britain May 8, US May 28 1997. JA Wikipedia: "TAMAGOTCHI（海外版たまごっち）（1997年5月）". If the maintainer prefers the US date, use 1997-05-28. |
| region / variant | International / English | Medium | |
| model | Low: US item **#1800** (UPC 045557018009) | Low | eBay seller MPN/UPC only |
| clone_of | **TamagotchiP1J** (the Japanese original is the ultimate origin) | High | Same game; localized text and graphics (death screen angel vs ghost, ounces vs grams) |
| keys | keep auto | High | |
| notes | A "英語版たまごっち" (reimport of the English unit) was also sold in Japan from June 1997. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_(1996_Pet) - JP 1996-11-23, NA 1997-05-28, Canada 1997-05-01, UK 1997-05-08, regional differences
- https://ja.wikipedia.org/wiki/たまごっち - TAMAGOTCHI（海外版）1997年5月; 英語版たまごっち 1997年6月
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `tama` "Tamagotchi (Gen. 1, World)"
- https://www.ebay.com/itm/186981121728 - "Bandai Tomagotchi 1996-1997 - (1800)" (model # only, Low)

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1997-05-01 (Medium) | **1997-05-01** (High) | Exact US launch day: **May 1, 1997**. Smithsonian Magazine says "Tamagotchi was released on May 1, 1997" and "On the first day of sales, May 1, 1997, San Francisco's F.A.O. Schwartz sold its entire stock of 3,000 by 3:00 p.m." English Wikipedia also says "release in the United States on May 1, 1997", citing Bandai America's archived 1998 press page and the Smithsonian article. This contradicts the fandom wiki's "NA May 28 / Canada May 1". May 28 is probably a later nationwide rollout or a fandom error. |
| notes | - | Replace "US 1997-05-28" in the notes with "US 1997-05-01 (FAO Schwarz launch); fandom's May 28 is unsupported". The UK date of 1997-05-08 (fandom) is unverified. | |

Sources:
- https://www.smithsonianmag.com/innovation/keeping-tamagotchi-alive-180979264/ - US release May 1, 1997; FAO Schwarz first-day sell-out
- https://en.wikipedia.org/wiki/Tamagotchi - US May 1, 1997 (cites Bandai America 1998 press page, web.archive.org/web/19980709195449/http://www.bandai.com/pressroom/98tama.html)


#### `TamagotchiP1J` – Tamagotchi (Gen. 1, Japan)

*Already packed as `TAMAJ`.*

**Final:** name **Tamagotchi (Gen. 1, Japan)** (High); group **Bandai** (High); date **1996-11-23** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Tamagotchi (Gen. 1, Japan)** (たまごっち; fans call it "P1") | High | |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1996-11-23** (replaces the packed 1996-01-01) | High | JA Wikipedia ("1996年11月23日 ... 正式発売"; test sales in Tokyo/Osaka from 1996-10-10); fandom; BEEP ("発売日：1996年11月23日", ¥1,980). The Westport Tech Museum headline also says Nov 23 1996. |
| region / variant | Japan | High | Config name |
| model | null | Unknown | |
| clone_of | none (this is the origin; TamagotchiP1 is its clone) | High | |
| keys | keep auto | High | |
| notes | Gold and silver limited editions (5,000 units) in Dec 1996; new colours in Mar 1997. PCB TMG-M1, E0C6S46 (MAME). | | |

Sources:
- https://ja.wikipedia.org/wiki/たまごっち - 1996年11月23日発売; 10月10日 test sales
- https://tamagotchi.fandom.com/wiki/Tamagotchi_(1996_Pet) - JP release date November 23, 1996
- https://www.beep-shop.com/tamagotchi/history/ - 発売日：1996年11月23日, 1,980円
- https://www.westporttechmuseum.com/blogs/techtastic/today-in-toy-history-november-23rd-1996-bandi-releases-the-tamagotchi-in-japan - Nov 23 1996 (title in search results)


#### `TamagotchiP2` – Tamagotchi (Gen. 2, Japan)

*Already packed as `TAMAG2`.*

**Final:** name **Tamagotchi (Gen. 2, Japan)** (Medium); group **Bandai** (High); date **1997-02-01** (High); model **–** (Unknown); clone of **none** (Medium); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Shinshu Hakken!! Tamagotchi** (新種発見!!たまごっち, "New Species Discovered!!"), officially "Generation 2". Suggested display name: `Tamagotchi (Gen. 2, Japan)`, matching the Gen. 1 naming. "Shin Tamagotchi" is not a real title. | High (title); Low-Medium (region of dump) | JA Wikipedia; fandom; BEEP; mynavi. The region is inferred from MAME `tamag2` "Tamagotchi (Gen. 2, Japan)". An English Gen 2 exists, and BrickEmuPy does not say which ROM it has. |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1997-02-01** (Japan) | High | BEEP "発売日：1997年2月1日"; JA Wikipedia list "新種発見!!たまごっち（1997年2月1日）" (guidebook p.42); mynavi and dejideji say Feb 1997. English Gen 2 was 1997 (exact date not found). |
| region / variant | Japan (probable) | Low | See name |
| model | null | Unknown | |
| clone_of | none (optionally TamagotchiP1J) | Medium | MAME: same hardware, "nearly the same game, they only changed the graphics", with new characters and a new game. I treat it as a sequel, not a regional variant. |
| keys | keep auto | High | |
| notes | New colours in May 1997. The Japanese version's death screen is a ghost and grave; the English version shows a UFO. | | |

Sources:
- https://www.beep-shop.com/tamagotchi/history/ - 新種発見!!たまごっち 発売日：1997年2月1日 1,980円
- https://ja.wikipedia.org/wiki/たまごっち - 新種発見!!たまごっち（1997年2月1日）
- https://ja.wikipedia.org/wiki/新種発見!!たまごっち - 1997年2月, English version existed
- https://news.mynavi.jp/article/20220615-2364411/ - "新種発見!! たまごっち" 1997/2
- https://tamagotchi.fandom.com/wiki/Tamagotchi_(1996_Pet) - Gen 2 = 新種発見!!たまごっち
- https://raw.githubusercontent.com/mamedev/mame/master/src/mame/handheld/hh_e0c6200.cpp - `tamag2` "Tamagotchi (Gen. 2, Japan)"


#### `TamagotchiUmino` – Umi de Hakken!! Tamagotchi

*Already packed as `TAMAUMI`.*

**Final:** name **Umi de Hakken!! Tamagotchi** (Medium); group **Bandai** (High); date **1998-03-01** (High); model **–** (Unknown); clone of **none** (High); keys: keep auto

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Umi de Hakken!! Tamagotchi** (海で発見!!たまごっち). Nicknames: Umino, Oceangotchi. English release: "Tamagotchi Ocean". Alternative display name: `Tamagotchi Ocean (Japan)`. | Medium on "!!" | Same "!!" vs "!" conflict as Morino (JA Wikipedia/rekidai "!!", fandom "！"). The Yahoo Auctions title quoted in search results reads "海で発見!! たまごっち". |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1998-03-01** (Japan, month only; replaces the packed 1998-01-01). English "Tamagotchi Ocean": August 1998 (US). | High | JA Wikipedia (1998年3月, guidebook p.45), fandom (March 1998), dejideji (1998年3月), rekidai (1998.03; Tamagotchi Ocean 1998.08) |
| region / variant | Japan (probable) | Medium | Brick and SVG are named "Umino". The face is a plain translucent light-blue shell, which matches the JP 水色 colour; US shells were opaque with a "Tamagotchi Ocean" logo. The BrickEmuPy commit says "add Tamagotchi Umino/Ocean", so this is not fully settled. |
| model | null | Unknown | |
| clone_of | none | High | |
| keys | keep auto (screenFrame = X is the tap/vibration sensor) | High | |
| notes | It was the last English vintage release before Bandai America stopped the line. ¥2,079 / $15.99. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Tamagotchi_Ocean - 海で発見！たまごっち, JP March 1998, US August 1998, translucent JP shells
- https://ja.wikipedia.org/wiki/たまごっち - 海で発見!!たまごっち（1998年3月）
- https://dejideji.com/tamagochi-syoki-ichiran/ - 海で発見 1998年3月 (波/琥珀/水色/紫)
- https://rekidai-carnival.com/tamagotchi-rekidai-hontai/ - 1998.03 "海で発見!! たまごっち"; 1998.08 Tamagotchi Ocean


#### `TamagotchiYasashii` – Yasashii Tamagotchi

*Already packed as `TAMAYASA`.*

**Final:** name **Yasashii Tamagotchi** (High); group **Bandai** (High); date **1998-10-01** (Low); model **–** (Unknown); clone of **none** (High); keys: `btnSelect`→`A`, `btnCancel`→`B`, `btnFood`→`LEFT`, `btnSmile`→`UP`, `btnHeart`→`RIGHT`, `btnClean`→`Y`, `btnBulb`→`X`, `btnWeight`→`DOWN`, `btnReset`→`PLUS+MINUS`

**First pass**

| Field | Value | Confidence | Basis |
|---|---|---|---|
| name | **Yasashii Tamagotchi** (やさしいたまごっち, "gentle/easy Tamagotchi"). Not "Tamagotchi Plus". The shell logo reads "TAMAGOTCH". | High | fandom; JA Wikipedia; BEEP; face SVG |
| manufacturer | Bandai (group: **Bandai**) | High | |
| date | **1998-10-01** (Japan, month only). Alternative: 1998-09-01. | Low-Medium | Sources conflict. October 1998: JA Wikipedia (citing the official guidebook 『かえってきた!たまごっちプラスとあそぶほん』 p.47) and BEEP. September 1998: fandom (infobox and shell list "Wave 1 (September 1998)"), dejideji, rekidai-carnival. I chose October because it rests on an official printed source, while the September lists may copy fandom. The face art itself prints "1998" in the icon strip. |
| region / variant | n/a (Japan only) | High | |
| model | null | Unknown | |
| clone_of | none | High | |
| keys | **override recommended** (see below) | Medium | Printed labels (face SVG): big back-lit buttons しょくじ (meal), ごきげん (mood/happy), あいじょう (love/care), トイレ (toilet), でんき (lights), チェックメーター (check meter); small えらぶ (select) and とりけし (cancel); a recessed reset. Auto mapping puts the most-used Select on PLUS and Cancel on L, and spreads the 2x3 care grid over A/B/X/Y/MINUS/R. |
| notes | A large (about 10 cm) desk model on 2xAA batteries with a stand and a volume switch; ¥2,814. Every care function has its own button instead of an icon menu. Proposed override: `{"btnSelect":"A","btnCancel":"B","btnFood":"LEFT","btnSmile":"UP","btnHeart":"RIGHT","btnClean":"Y","btnBulb":"X","btnWeight":"DOWN","btnReset":"PLUS+MINUS"}`. This puts confirm/cancel on A/B and the six care buttons on the unused d-pad plus X/Y. It is optional; the auto map works, it is only awkward. | | |

Sources:
- https://tamagotchi.fandom.com/wiki/Yasashii_Tamagotchi - September 1998, ¥2,814, 2xAA, six big buttons + Select/Cancel + front reset
- https://tamagotchi.fandom.com/wiki/Yasashii_Tamagotchi/Shell_list - three shells, wave 1 September 1998
- https://ja.wikipedia.org/wiki/たまごっち - やさしいたまごっち（1998年10月）
- https://www.beep-shop.com/tamagotchi/history/ - やさしいたまごっち 発売月：1998年10月
- https://dejideji.com/tamagochi-syoki-ichiran/ - やさしいたまごっち 1998年9月 (白/青/黄色)
- https://rekidai-carnival.com/tamagotchi-rekidai-hontai/ - 1998.09 やさしいたまごっち

**Second pass (gap-fill)**

| Field | Old value (conf) | New value (conf) | Evidence |
|---|---|---|---|
| date | 1998-10-01 (Low-Medium) | **1998-10-01, no change** (Low-Medium) | New supporting evidence: the National Diet Library has 『やさしいたまごっち大百科』 (勁文社, ISBN 4-7669-3069-X) issued **1998.10**. One search snippet (HMV) gave September 1998 for the same book. A companion book appearing in Oct 1998 fits a Sept or Oct toy launch and does not decide between them. No Bandai press release was found. |

Sources:
- https://ndlsearch.ndl.go.jp/books/R100000002-I000002698519 - やさしいたまごっち大百科, 勁文社, issued 1998.10 (via NDL OpenSearch API)


## Appendix: batch-level findings (verbatim from each research batch)

### Batch 1

1. **MAME has nothing for these.** I downloaded and grepped `hh_tms1k`, `hh_sm510`, `hh_ucom4`, `hh_hmcs40` and `hh_cop400`; there is no `hh_tcm`/`hh_t6a`/TLCS-47 driver. The only hits are different products: Bandai FL Pengo (VFD), Kiteyo Parman (VFD), and Dokodemo Dorayaki Doraemon (VFD, PT-412). Don't import MAME dates for these LCD games.
2. **Popy back labels carry hard data.** Each Animest unit's back label/mold gives "MODEL NO. XX-nn" and "PAT. PEND. <year>". That confirmed CO-02, AR-03, DO-01, DO-02 and PA-01 and fixed the years. HHM and HE dates of 1982 for the Doraemon games are contradicted by the 1983 labels.
3. **Popy vs Bandai (Decision 1).** The 1982 units (Space Cobra, Arale) carry only the ポピー mark. The 1983 units mix: Dorayaki House still has a Popy label; Isoge! Doraemon and Parman Dai-Pinch carry BANDAI on the label while the box still reads "POPY ELECTRONICS GAME / ANIMEST" (Popy merged into Bandai in March 1983). Recommendation: one **Popy** group for all Animest titles, with "Bandai" as the parent noted. The alternative is filing them under Bandai. The JSON uses "Popy".
4. **Series prefixes (Decision 6).** On these faces, "ANIMEST" is a badge, not part of the title: box titles never put it in front ("スペースコブラ プロフェッショナル", "ドラヤキハウス", "急げ!ドラえもん"). The same goes for Gakken's "LCD CARD GAME", Bandai's "SUPER GAME DIGITAL" / "LCD GAME DIGITAL" and Bandai's "Pocket Club P-1". Recommend dropping all of them from names. The JSON names omit them.
5. **New group "Gakken"** is needed for GakkenSoccer and JumpingBoy, and for TomJerryPrank in another batch. Jumping Boy is "© Konami", so it is Konami-developed; Konami is an existing group, but the brand on the unit is Gakken.
6. **README date errors:** GundamGaidenII "1985" is wrong (≈1990); GakkenSoccer "1981" is unverified (mask numbering suggests 1982).
7. **Toshiba mask numbers roughly track release order** (T6813/6814/6815 ≈ 1982, T6850 = late 1982, T6909-6981 = 1983, T7790 = 1984). This is useful as weak dating evidence; T7704 (Gundam Gaiden II) breaks the pattern.
8. **GundamGaidenII's .brick looks unfinished** (`assets/test/` paths and a link peripheral), so gate it or note it.

### Batch 2

1. **Handoff README-line bug (Apollo pets).** The BrickEmuPy README has one "Apollo 2 in 1 virtual pet" line, which belongs to `Apollo2in1`. It has no Prince & Princess line, so the generator copied the 2-in-1 line into both `ApolloPrince&Princess_*` sections. The real title is "Prince & Princess", from Russian collector sources.
2. **Same-ROM pair (Decision 5).** `ApolloPrince&Princess_ch`/`_en` differ only in a hardware language strap (`port_pullup` PA bit 1). I recommend one entry each, named `Prince & Princess (Chinese)` / `(English)`, because players will see different text.
3. **File-name bug.** `Apollo2in1.brick` points to `Apollo2in1.bin`, but the file is `apollo2in1.bin`. Fix the case on import.
4. **New clone link outside my batch.** `BigCat9in1` is a ROM derivative of `Gyaoppi9in1` (Super Gyaoppi 9-in-1): 72 % of bytes are identical at the same offsets. Set `clone_of: "Gyaoppi9in1"`.
5. **New groups (Decision 1).** This batch supports adding **Apollo** (5 configs: two bricks and three pet configs), **Gakken** (Tom & Jerry Prank), **Spica** (D'Artagnan) and either an **Unbranded** group or a **Sparkle Crystal** group (Elfin Twins; possibly Big Cat if it is not filed under Gyaoppi's maker).
6. **Series prefix (Decision 6).** Tom & Jerry Prank's face prints "Gakken LCD CARD GAME" as a maker line, not part of the title. Collector databases call the series "Cornerless card". I recommend dropping the prefix from the name.
7. **Brick-game naming.** Apollo sold many same-count variants (18 in 1 B0302 English talking vs B0311 Russian talking), so I recommend keeping the B-code in the name, for example `126 in 1 (B0202)`.
8. **Auto-key problems.** Gakken's GAME A/B buttons are also the in-game left/right controls, and Spica's JUMP is the up of a rocker. Overrides for both are given above. Other overrides are optional.
9. **Dump credits.** The Spica and Tom & Jerry ROMs are by Milan Galcik (brick `credits`). The Apollo pet, Big Cat and Elfin Twins units were provided by t.me/oksiniah (and Crazy_Emulator for Prince & Princess). None of these configs is flagged as a questionable dump.
10. The WebSearch budget (200 per turn, shared) ran out on the final Spica model-number query. More searching could firm up Spica's date and country and the Apollo pet dates.

### Batch 3

1. **Handoff generator bugs.**
   - The GA888 README line in the handoff repeats E-88's line. The real BrickEmuPy entry is "Block Game & Echo Key GA888 (Tetris Jr. clone) (HT-943I0)".
   - "Keychain 55 in 1" is actually Gametech's **Pikorin 55**, identified by the face logo and a MAME SHA1 match.
2. **MAME cross-reference.** `src/mame/handheld/hh_ht11xx.cpp` (credits azya) has three of these ROMs with identical SHA1: `brke23p2` (E-Star, 1993), `ga888` (unknown, 199?) and `piko55` (Gametech). It is the best primary source for these titles.
3. **Super Gyaoppi and Big Cat 9-in-1 share code.** The first 22 KB of `Gyaoppi9in1.bin` and `BigCat9in1.bin` are byte-identical; only the data area differs. Big Cat is probably a reskin or derivative from the same OEM; that brick belongs to another batch. If it is filed as a clone, `clone_of: "Gyaoppi9in1"` is the natural choice, since the Gyaoppi line dates to 1997.
4. **Manufacturer groups (Decision 1).**
   - Holtek brick games and keychains: E-23 and E-88 carry "BRICK GAME" plus a smiley-hands logo ("机之宝" on the E-23), and sources name E-Star (Taiwan). GA888 and Keychain Pin Ball carry no brand. Suggest one **"Brick Game"** group with makers in the notes, or "E-Star" if maker groups are preferred.
   - Pikorin 55 is a real Japanese brand: suggest **"Gametech"**.
   - Mame Game Galaxian (and MameTamagotch) belong under **Bandai**.
   - Super Gyaoppi: "U.I." per one wiki and one review, otherwise unbranded.
5. **McDonald's Tetris Nugget (Decision 2).** It is a genuine McDonald's China promotional item with the arches on the LCD, so the "McDonald's" group is the honest home. Suggest reworking `GW_rom::is_lcdonald_family` to key on the core/ROM type rather than the group name. A separate "McDonald's (China)" group is a fallback.
6. **Tamagotchi V3 (Decisions 4 and 5).** The two configs differ only by a PD pull-up bit, which is probably the region strap, so the US and EU entries play differently (date and unit format). Keep both, name them "Connection V3 (US)" and "Connexion V3 (Europe)", set `clone_of` on the EU entry, and add a dump-quality note.
7. **ROM not in the clone.** TetrisNugget, both Tamagotchi V3 configs and MameGalaxian are "distributed without ROM" in BrickEmuPy. The `.bin` files are absent from the clone's assets; the handoff has their sizes, so the maintainer presumably has them.
8. **Date confidence.** Many Holtek "dates" here are chip mask or die years (E-23 1993, E-88 1994), not retail launches. GA888 and Keychain Pin Ball dates are placeholders.

### Batch 4

1. **"Mame Game" is Bandai.** The brief assumed MameTamagotch was an unrelated Holtek keychain, but it is Bandai's 1997 "Mame Game" (マメゲーム) keychain line, with box photo evidence. `MameTamagotch` and `MameGalaxian` (another batch) both belong in the **Bandai** group. Bandai used Holtek MCUs for this line, so a Holtek chip does not mean an unbranded Chinese unit.
2. **Hiro group.** `KotoKunekunetchyo` (Hiro LCD Keychain Series No.4, 1997) and `PocketBoyNo2` (Hiro Pocket Boy No.2) share the publisher Hiro (株式会社ヒロ), which argues for a single "Hiro" group. Koto Laboratory is the developer (credit it in notes).
3. **New single-title makers:** Compile (`Puyolin`), Hartung (`formel1`), VGS/Playworks (`MickeyVGS`), Cool Tec? (`PocketPuppy`). These could each get a group, or go into an "Other" group.
4. **Unbranded brick/keychain games** in this batch (`SpaceIntruderTK150I`, `GA878`, `MiconKC32`) have no reliable dates. If dates are mandatory for import, they need an explicit "unknown/guess" policy (e.g. a decade placeholder plus a note). `GA878` and `MiconKC32` share the MCU (BJ6220A) and button layout but have different ROMs, so they are not clones.
5. **Key overrides proposed:** Puyolin (Start/Rotate onto A), Micon KC-32 (Speed = ▼ onto DOWN), Mickey (ears = cycle, bottom buttons = confirm/cancel). Optional A-button convenience mappings for Koto, Formel 1 and SMB.
6. **Title spelling.** Kunekunetchyo's real title has a small っ (くねっくねっちょ). BrickEmuPy's romanization omits it, and the beforemario.com English spelling is the same "Kunekunetchyo".

### Batch 5

- **Search budget:** the turn-wide WebSearch limit was hit during this batch. Rows marked Low or Unknown for Edla Space Rescue (maker and date), E-33 (date and brand), Pocket Boy No.2 (1990 vs 1993), the D-Power EU date and the Alien Fever kana could improve with a follow-up search turn.
- **Face SVGs are good primary evidence.** Render them with cairosvg; the scratchpad venv `venv4` has it. Pocket Boy No.2's real title, ドラゴン魔宮の謎, appears only as art on the face, not in any text element.
- **New menu groups implied by this batch:** Hiro (Pocket Boy No.2; also shared with KotoKunekunetchyo), Edla, Stadlbauer (Mini Classics, if not filed under Nintendo), and a generic Brick Game / unbranded group (E-33).
- **Decision 6 (series names):** "Game Watch" (Nelsonic) is not printed on the Zelda face, and the manual calls it "The Legend of Zelda 8321". Dropping the series prefix gives "The Legend of Zelda", which is ambiguous with Nintendo's own Game & Watch Zelda. So for Nelsonic I lean toward keeping "Game Watch" as a suffix: "The Legend of Zelda Game Watch". The same applies to SMBNelsonic.
- **Decision 4 (questionable dumps):** TamagotchiMini2017 carries the upstream "high probability of errors" flag. Its config only recently moved out of a test path (commit "fix Tamagotchi Mini path", 2026-10-12). MiniClassicsSoccer's face and ROM still live under `assets/test/`, so both look experimental upstream.
- **Key overrides worth applying:** ZeldaNelsonic, where both side buttons are gameplay actions per the manual; MiniClassicsSoccer is lower confidence.

### Batch 6

1. **The legacy Digimon configs are stale duplicates.** `DigimonV1`, `DigimonV2` and `DigimonV3` are the pre-24-Mar-2026 names of `DigimonV1JB`, `DigimonV2J` and `DigimonV3J`. For Decision 5, keep one entry per real ROM and drop or alias the three legacy names. Correct the handoff's README line for `DigimonV1` from "1JA" to "1JB".
2. **JA vs JB ROM equality is suspicious.** The handoff says V1, V1JA and V1JB share one ROM, but upstream treats JA and JB as two revisions with separate dumps. If the maintainer's two files are identical, the JA file is probably a copy of JB (or vice versa). Re-obtain or verify before packing both.
3. **The packed "Digital Monster (Spanish)" is probably a misnomer.** Community tools call this ROM the EU/English Ver.1, collectors confirm a European blue/light-grey Ver.1 matching the face SVG, and there is no Spanish-language unit (the pets show no text). Suggest `Digital Monster Ver.1 (Europe)`. The packed date 1997-01-01 predates the product (Japan launch 26 Jun 1997). Use 1998-01-01 (Low) or 1997-12-05 (first English release, NA).
4. **Naming.** The official Japanese titles are デジタルモンスター Ver.1 to Ver.4, i.e. "Digital Monster Ver.N". Bandai's own Japanese styling has no space ("Ver.1"); wikimon writes "Ver. 1". Pick one style for the whole series. "Digimon" is the franchise and international nickname, not the 1997 Japanese product title.
5. **Dates.** Only Ver.1 has an exact day (1997-06-26). Ver.2, 3 and 4 are month-precise (1997-12, 1998-03, 1998-05) from ja.wikipedia, wikimon and dejideji, which all agree.
6. **Model numbers.** No Japanese catalogue numbers were found. The Bandai Asia export item codes (1855, 1856, 1857B, 1858) are known but belong to the Asian/Australian English releases, not the Japanese ROMs. I left `model` null in the JSON and mention the codes in the notes.
7. Search budget was exhausted during the Nikko research; the Nikko entry needs a follow-up.

### Batch 7

1. **Open checks resolved.**
   - TAMAJ: 1996-01-01 should be **1996-11-23**.
   - TAMAUMI: 1998-01-01 should be **1998-03-01**.
   - STACKCH: **1991**, catalogue **60-2247**, from primary catalogue scans.
2. **Other date corrections to apply** (the packed values were guesses):
   - Angel 1997-08-01
   - Genjintchi 1997-11-01
   - Mothra 1997-12-01
   - Morino 1998-02-01
   - P2 1997-02-01
   - P1 (international) 1997-05-01
   - Yasashii 1998-10-01 (Sept/Oct conflict)
3. **Naming: "!" vs "!!".** Bandai's 1997-98 "…で発見!!" family is written with "!!" in Japanese Wikipedia and Bandai-derived chronologies. Fandom and the handoff's example use a single "!". I recommend "!!" for both Mori and Umi; use the same choice for both. Without a clear box photo this stays Medium.
4. **Japanese vs English titles.** The Angel and Umino dumps look Japanese (Angel's face shows first-run JP icons; Umino's face is a JP translucent shell). Their English releases ("Tamagotchi Angel", "Tamagotchi Ocean") ran localized ROMs. So I suggest the Japanese romanized titles (Tenshitchi no Tamagotchi, Umi de Hakken!! Tamagotchi), or the English title with a "(Japan)" suffix. For P1/P1J/P2, keep the `Tamagotchi (Gen. N, Region)` pattern.
5. **Clone direction (decision 5 / filing under origin).** TamagotchiP1 (international) should be `clone_of: TamagotchiP1J`, because the Japanese P1 is the origin. P2 is a sequel with new graphics and characters, so I list it as a separate game.
6. **Model numbers.** I found no Japanese Bandai product codes for vintage Tamagotchi. Only the US item numbers (#1800 Gen 1, #1810 Angel) appear, and only in eBay listings (Low). Stack Challenge is the only firm model number in this batch.
7. **The screenFrame input** (mapped to X) on Angel, Morino, Umino and Genjintchi stands in for the vibration/tap sensor (Angel/Morino/Ocean have one per fandom). The X mapping is sensible.
8. **WebSearch budget** ran out before I could double-check the exact US Gen 1 launch day; I used fandom's dates (Canada 1997-05-01, US 1997-05-28).
