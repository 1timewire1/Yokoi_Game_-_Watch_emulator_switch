# David & John asset patch

Adds the "David and John" open-source games' artwork to a `.ykp` rompack that
was built before that support existed - without rebuilding the whole pack.
Everything needed is in this folder; no repo clone, no network access, no
Python packages beyond the standard library.

## Usage

1. Copy your `.ykp` rompack into this folder.
2. Run:

   ```bash
   python inject_pack_assets.py
   ```

   It auto-detects your pack if there's exactly one `.ykp` file here. If
   there's more than one, or your pack lives elsewhere, name it explicitly:

   ```bash
   python inject_pack_assets.py path/to/your_pack.ykp
   ```

3. Done. The pack is updated in place (the original is backed up alongside
   it as `<pack>.ykp.bak` first, so nothing is lost if anything goes wrong).

## What this actually does

Your rompack already works for every other game - this only adds the 15
files in `assets/` (David & John's console/segment art) to it by name,
leaving every other game's data untouched. If you only started missing
David & John games after updating the emulator, this is all you need; you
don't have to regenerate the whole rompack from your MAME ROMs.

## Not what you're looking for?

This only adds *art assets* to an existing pack. If you're building a
rompack from scratch, see `CONVERT_ROM/README.md` in the main repo instead.
