"""Manufacturer ids used by conversion scripts.

These numeric ids must match `source/std/GW_ROM.h`.
"""

from __future__ import annotations

from typing import Any


MANUFACTURER_NINTENDO: int = 0
MANUFACTURER_TRONICA: int = 1
MANUFACTURER_ELEKTRONIKA: int = 2
MANUFACTURER_TIGER: int = 3
MANUFACTURER_DAVID_AND_JOHN: int = 4
# 5 is retired (it was MANUFACTURER_EXPERIMENTAL, a catch-all bucket that no
# longer exists - see GW_ROM.h); deliberately not reused.
# McDonald's Happy Meal LCD games, ported from the LCDonald project rather
# than a MAME-dumped ROM.
MANUFACTURER_MCDONALDS: int = 6
# "Burger Bard" - LCDonald's white-label re-skin of the McDonald's games (see
# GW_ROM.h) - its own menu group.
MANUFACTURER_BURGER_BARD: int = 7
MANUFACTURER_KONAMI: int = 8
MANUFACTURER_NELSONIC: int = 9
# Currently a single device (Electronic Number Muncher); more expected.
MANUFACTURER_VTECH: int = 10
# Seiko Epson E0C6S46/S48 keychain games (MAME hh_e0c6200).
MANUFACTURER_BANDAI: int = 11
MANUFACTURER_EPOCH: int = 12
MANUFACTURER_NIKKO: int = 13
MANUFACTURER_TANDY: int = 14

KNOWN_MANUFACTURER_IDS = (
    MANUFACTURER_NINTENDO, MANUFACTURER_TRONICA, MANUFACTURER_ELEKTRONIKA,
    MANUFACTURER_TIGER, MANUFACTURER_DAVID_AND_JOHN, MANUFACTURER_MCDONALDS,
    MANUFACTURER_BURGER_BARD, MANUFACTURER_KONAMI, MANUFACTURER_NELSONIC,
    MANUFACTURER_VTECH, MANUFACTURER_BANDAI, MANUFACTURER_EPOCH,
    MANUFACTURER_NIKKO, MANUFACTURER_TANDY,
)

# id -> C++ symbol in GW_ROM.h, for the generated GW_ROM_RGDS/*.cpp files.
MANUFACTURER_CPP_SYMBOLS = {
    MANUFACTURER_NINTENDO: "GW_rom::MANUFACTURER_NINTENDO",
    MANUFACTURER_TRONICA: "GW_rom::MANUFACTURER_TRONICA",
    MANUFACTURER_ELEKTRONIKA: "GW_rom::MANUFACTURER_ELEKTRONIKA",
    MANUFACTURER_TIGER: "GW_rom::MANUFACTURER_TIGER",
    MANUFACTURER_DAVID_AND_JOHN: "GW_rom::MANUFACTURER_DAVID_AND_JOHN",
    MANUFACTURER_MCDONALDS: "GW_rom::MANUFACTURER_MCDONALDS",
    MANUFACTURER_BURGER_BARD: "GW_rom::MANUFACTURER_BURGER_BARD",
    MANUFACTURER_KONAMI: "GW_rom::MANUFACTURER_KONAMI",
    MANUFACTURER_NELSONIC: "GW_rom::MANUFACTURER_NELSONIC",
    MANUFACTURER_VTECH: "GW_rom::MANUFACTURER_VTECH",
    MANUFACTURER_BANDAI: "GW_rom::MANUFACTURER_BANDAI",
    MANUFACTURER_EPOCH: "GW_rom::MANUFACTURER_EPOCH",
    MANUFACTURER_NIKKO: "GW_rom::MANUFACTURER_NIKKO",
    MANUFACTURER_TANDY: "GW_rom::MANUFACTURER_TANDY",
}


def normalize_manufacturer_id(value: Any, default: int = MANUFACTURER_NINTENDO) -> int:
    """Coerce a manufacturer value into a known numeric id.

    Any non-int/unknown value falls back to ``default``.
    """

    try:
        numeric = int(value)
    except Exception:
        return int(default)

    if numeric not in KNOWN_MANUFACTURER_IDS:
        return int(default)

    return numeric
