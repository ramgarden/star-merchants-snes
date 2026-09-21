#!/usr/bin/env python3
"""Verify a SNES ROM exactly the way Mesen-S does.

Replicates BaseCartridge::GetHeaderScore() and BaseCartridge::LoadRom() from
Mesen-S (https://github.com/SourMesen/Mesen-S, Core/BaseCartridge.cpp) so a ROM
can be validated locally, before ever opening the emulator.

Usage: python tools/verify_rom.py <rom.sfc> [<rom2.sfc> ...]
"""
import struct
import sys

# struct SnesCartInformation (all uint8, 80 bytes total)
OFF_MAKER = 0
OFF_GAMECODE = 2
OFF_EXPANSION_RAM_SIZE = 13
OFF_SPECIAL_VERSION = 14
OFF_CARTRIDGE_TYPE = 15
OFF_CART_NAME = 16          # 21 bytes, file offset addr+0x7FC0
OFF_MAP_MODE = 37           # addr+0x7FD5
OFF_ROM_TYPE = 38           # addr+0x7FD6
OFF_ROM_SIZE = 39           # addr+0x7FD7
OFF_SRAM_SIZE = 40          # addr+0x7FD8
OFF_DESTINATION = 41
OFF_DEVELOPER_ID = 42
OFF_VERSION = 43
OFF_CHECKSUM_COMPLEMENT = 44  # addr+0x7FDC
OFF_CHECKSUM = 46             # addr+0x7FDE
STRUCT_SIZE = 80

SEI, CLI, JMP, JML, JSR, JSL, STZ = 0x78, 0x18, 0x4C, 0x5C, 0x20, 0x22, 0x9C
REP, SEP, LDA, LDX, LDY = 0xC2, 0xE2, 0xA9, 0xA2, 0xA0
BRK, SBC, CPY = 0x00, 0xFF, 0xCC

BASE_ADDRESSES = [0, 0x200, 0x8000, 0x8200, 0x408000, 0x408200]


def get_header_score(rom: bytes, addr: int) -> int:
    """Exact port of BaseCartridge::GetHeaderScore."""
    if len(rom) < addr + 0x7FFF:
        return -1

    info = rom[addr + 0x7FB0 : addr + 0x7FB0 + STRUCT_SIZE]
    if len(info) < STRUCT_SIZE:
        return -1

    score = 0
    mode = info[OFF_MAP_MODE] & ~0x10
    if (mode == 0x20 or mode == 0x22) and addr < 0x8000:
        score += 1
    elif (mode == 0x21 or mode == 0x25) and addr >= 0x8000:
        score += 1

    if info[OFF_ROM_TYPE] < 0x08:
        score += 1
    if info[OFF_ROM_SIZE] < 0x10:
        score += 1
    if info[OFF_SRAM_SIZE] < 0x08:
        score += 1

    checksum = info[OFF_CHECKSUM] | (info[OFF_CHECKSUM + 1] << 8)
    complement = info[OFF_CHECKSUM_COMPLEMENT] | (info[OFF_CHECKSUM_COMPLEMENT + 1] << 8)
    if checksum + complement == 0xFFFF and checksum != 0 and complement != 0:
        score += 8

    reset_vector_addr = addr + 0x7FFC
    reset_vector = rom[reset_vector_addr] | (rom[reset_vector_addr + 1] << 8)
    if reset_vector < 0x8000:
        return -1

    op = rom[addr + (reset_vector & 0x7FFF)]
    if op in (CLI, SEI, JMP, JML, JSR, JSL, STZ):
        score += 8
    elif op in (REP, SEP, LDA, LDX, LDY):
        score += 4
    elif op in (BRK, SBC, CPY):
        score -= 8

    return max(0, score)


def load_rom_decision(rom: bytes) -> dict:
    """Exact port of BaseCartridge::LoadRom's header selection."""
    best_score = -1
    is_lo_rom = True
    has_header = False
    header_offset = None
    info = None

    for addr in BASE_ADDRESSES:
        score = get_header_score(rom, addr)
        if score >= 0 and score >= best_score:
            best_score = score
            is_lo_rom = (addr & 0x8000) == 0
            has_header = (addr & 0x200) != 0
            header_offset = min(addr + 0x7FB0, len(rom) - STRUCT_SIZE)
            info = rom[header_offset : header_offset + STRUCT_SIZE]

    return {
        "accepted": best_score >= 0,
        "score": best_score,
        "is_lo_rom": is_lo_rom,
        "has_copier_header": has_header,
        "header_offset": header_offset,
        "title": info[OFF_CART_NAME : OFF_CART_NAME + 21].decode("ascii", "replace") if info else "",
        "map_mode": info[OFF_MAP_MODE] if info else None,
        "rom_size_byte": info[OFF_ROM_SIZE] if info else None,
        "reset_vector": (rom[header_offset + 0x4C] | (rom[header_offset + 0x4D] << 8)) if info else None,
    }


def main() -> int:
    if len(sys.argv) < 2:
        print(__doc__)
        return 2

    rc = 0
    for path in sys.argv[1:]:
        with open(path, "rb") as f:
            rom = f.read()
        d = load_rom_decision(rom)
        print(f"== {path} ({len(rom)} bytes)")
        for key, val in d.items():
            print(f"   {key}: {val}")
        if not d["accepted"]:
            print("   RESULT: REJECTED by Mesen-S header scoring")
            rc = 1
        else:
            print(f"   RESULT: accepted by Mesen-S header scoring (score {d['score']})")
    return rc


if __name__ == "__main__":
    sys.exit(main())
