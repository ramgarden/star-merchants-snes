# 2026-09-21 — ROM format, emulator discovery, and the black screen

Session goal: make the ROM load in "Mesen" and stop the multi-session
"Invalid rom file." infinite loop.

## 1. The infinite loop was never about the ROM

**Root cause: the emulator.** The project used **Mesen 0.9.9** (winget
`SourMesen.Mesen`). Pulled its source (`github.com/SourMesen/Mesen`, tag
`0.9.9`) and found `Core/RomLoader.cpp::LoadFile`: it only dispatches
NES-family formats (`NES\x1a` iNES, FDS, NSF/NSFE, UNIF, StudyBox) or a
GameDatabase CRC match; everything else hits the fallback:

```cpp
Log("Invalid rom file.");   // Core/RomLoader.cpp:69 — the exact message we kept getting
```

Mesen 0.9.9's last release is Feb 2020 and it is NES-only. The multi-console
unified Mesen (with SNES) existed only as unreleased WIP in the archived repo.
**No SNES ROM — valid or not — can ever load in Mesen 0.9.9.** Every previous
"fix the ROM" iteration was doomed.

The SNES sibling emulator is **Mesen-S** (github.com/SourMesen/Mesen-S, latest
release 0.4.0, same author/UI/debugger). Installed it this session:

- `C:\dev\snes\tools\mesen-s\Mesen-S.exe` (single self-contained .exe from the
  official `Mesen-S.0.4.0.zip` release asset)

Note: the historical "it loaded exactly once" report is inconsistent with the
Mesen-0.9.9-is-NES-only fact — that one success was most likely in snes9x,
bsnes, or a Mesen-S window from an earlier session.

## 2. What Mesen-S actually checks (replicated exactly)

Mesen-S does NOT hard-validate the header. `Core/BaseCartridge.cpp` scores
candidate base addresses `{0, 0x200, 0x8000, 0x8200, 0x408000, 0x408200}` via
`BaseCartridge::GetHeaderScore` and loads the best score >= 0:

- map mode byte (`& ~0x10`) == 0x20/0x22 for addr < 0x8000 → +1
  (0x21/0x25 for addr >= 0x8000 → +1, HiROM)
- RomType < 0x08 → +1; RomSize < 0x10 → +1; SramSize < 0x08 → +1
- checksum + complement == 0xFFFF, both nonzero → +8
- reset vector (at addr+0x7FFC) < 0x8000 → candidate rejected (-1)
- opcode at `addr + (resetVector & 0x7FFF)`: CLI/SEI/JMP/JML/JSR/JSL/STZ → +8,
  REP/SEP/LDA/LDX/LDY → +4, BRK/SBC/CPY → -8

`tools/verify_rom.py` is an exact Python port of this (runs against any .sfc).
`scripts/build.ps1` contains the same logic as `Get-SnesHeaderScore` and fails
the build if the emulator would reject the image. Our ROM and PVSnesLib's
known-good `Mode1Scroll.sfc` both score **20** (LoROM, reset target $8000 = SEI).

## 3. ROM layout fix (why the header used to be missing)

Old `snes-lorom.cfg` put HEADER/VECTORS as ordinary segments appended after
code (link map showed header at CPU $9027 = file $1027, vectors at $9049 —
nowhere near $FFC0/$FFFC). Mesen scans only the standard locations → zeros →
reject. Also crt0.s had a duplicate `.word $0000` making the header 34 bytes.

Fixes, verified by byte-dumping the final ROM:

- `scripts/snes-lorom.cfg`: dedicated MEMORY areas with `fill=yes` —
  ROM $7FC0 + HEADER $20 + VECTORS $20. **ld65 writes areas sequentially into
  the output file in MEMORY order** (verified empirically), so the header lands
  at file 0x7FC0 and vectors at 0x7FE0; the linked image is exactly 32 KB.
- `src/crt0.s`: rewritten — 32-byte header, complete native+emulation vector
  table at $FFE0-$FFFF (incl. emu COP/ABORT at $FFF4/$FFF8), DB=0 set via
  `pha/plb` BEFORE any absolute addressing, DP=$0000 via `tcd`, hardware stack
  and cc65 `c_sp` at $1FFF, 8-bit A/X/Y before calling C, `.export __STARTUP__`
  (cc65-generated modules import it; without it ld65 pulls none.lib's generic
  startup + condes.o and the link fails).
- build.ps1 pads to 256 KB (matches header ROM-size byte $08 = 2 Mbit), then
  patches checksum/complement at 0x7FDC-0x7FDF (word-sum of all bytes excluding
  those 4; complement = 0xFFFF - sum; pair sums to 0xFFFF).

Result: Mesen-S loads it — info box shows `Game: STAR MERCHANTS, Type: LoROM,
Map Mode: $20, Rom Type: $00, 256 KB`, title bar shows `Game loaded (NTSC)`.

## 4. CURRENT BLOCKER: black screen

The game loads but nothing renders. Evidence so far (all from hand-decoding
the ROM bytes — file offset F == CPU $8000+F):

- Reset code (file $0000-$0047) decodes byte-perfect: sei/cld/clc/xce → sep
  #$30 → DB=0 via pha/plb → stz $4200 → rep #$30 → hw stack $1FFF → c_sp=$1FFF
  (two overlapping direct-page stores, $80=$FF then $81=$1F — correct) → tcd
  (DP=$0000) → sep #$30 → jsr $8033 (ClearRegisters: stz $2100,x loop to $2133
  then $4200,x loop to $420D) → jsr $8307 (zerobss, BSS empty) → jsr $818B
  (_main) → bra forever. Five `rti` handlers at $802E-$8032 match the vectors.
- `_main` at $818B decodes to exactly the C source: INIDISP=$80, BGMODE=1,
  BG1SC=$68, BG12NBA=$03, VMAIN=$80, scroll regs, NMITIMEN=$81, then
  jsr $8069 (load_font), jsr $80BA (load_palette), jsr $8162 (clear_map),
  draw_text x2 (args pushed via pusha=$82D0, strings at $832A/$8339),
  TM=$212C=1, **INIDISP=$0F at $81EE**, then loop: jsr $8048 (wait_vblank —
  decoded correct), frame++ / blink / JOY1L & $08.
- `load_palette` ($80BA) decoded correct: CGADD=0; write $00,$00 (black) then
  $FF,$7F (white) — glyph color index 1 = white.
- If $81EE executes, the screen turns on with TM=1 (BG1). Pure black therefore
  means the CPU never reaches $81EE (a helper hangs or corrupts state) OR the
  PPU config is subtly wrong so BG1 renders all-black.

Prime suspects (not yet decoded to completion):
- `load_font` ($8069): 1536-word VRAM write loop; loop counter/termination not
  fully verified (calls vram_set=$82E6, helpers at $82FC/$8253/$8257)
- `clear_map` ($8162): 1024-entry loop, same status
- `draw_text` ($8105): string-scan logic partially decoded

## 5. Next-step plan (checkpoint bisect — do this first)

Instrument main.c with visible checkpoints: right after entering main, set
CGRAM[0] to GREEN with TM=0 and INIDISP=0x0F (screen = solid green proves main
started). Then after each init stage switch the backdrop color:

| Checkpoint | Backdrop |
|---|---|
| main() entry | GREEN |
| after load_font() | BLUE |
| after load_palette() | MAGENTA |
| after clear_map() | YELLOW |
| after draw_text() | CYAN (then TM=1) |

Build, launch Mesen-S, capture with `scripts/capture.ps1` (writes a PNG — read
it directly, it shows which stage is reached: color frozen = the call between
the previous and current checkpoint hangs/crashes). Each bisect iteration is
one build + one screenshot, no human needed.

Alternative tools: Mesen-S's built-in debugger (user-driven; it is the reason
we chose Mesen-S) — CPU/PPU windows, breakpoints at $81EE, VRAM/CGRAM viewers.
Mesen-S also supports Lua scripting for scripted memory reads.

## 6. Hard-won toolchain/OS facts

- ld65 writes MEMORY areas sequentially into the output file (config order);
  `fill=yes` pads each area to full size. ZP/WRAM areas with `file=""` are not
  written.
- cc65: `-t none` auto-links none.lib (pusha/ldaxsp/zerobss/zeropage...);
  generated code imports `__STARTUP__` — our crt0 must export it.
- cc65 convention: call C with 8-bit A/X/Y; the runtime stack helpers use ZP
  `c_sp` ($80) with `(dp),y` addressing → DP must be $0000 (tcd) before main.
- Direct-page `sta $80`-style stores in crt0 executed BEFORE `tcd` rely on the
  emulator initializing DP=0 at reset; real hardware leaves DP undefined —
  when touching crt0, set DP first.
- PowerShell 5.1: `Byte -shl 8` yields a Byte (truncates high bits); cast
  `[int]` first. `0xFFFFFFFF` parses as Int32 -1 (sign-extends on -bxor);
  use `4294967295L`. Array slices with computed ranges break — use Python for
  hexdumps (Python 3.14 is on PATH; zlib.crc32 for CRCs).
- Mesen (NES) 0.9.9's CRC32 of a file = zlib.crc32 (verified match on our ROM:
  0x423B0A23).

## 7. Reference material

- `pvsneslib_extracted/pvsneslib/snes-examples/graphics/Backgrounds/Mode1Scroll/Mode1Scroll.sfc`
  — known-good 256 KB LoROM built by PVSnesLib; our verify score matches it
- `pvsneslibfont.pic/.pal` in the same dir — the font asset we .incbin (96
  glyphs, ASCII 32-127, planar 4bpp; palette color 0=$001F color 1=$00FF in the
  asset, we substitute black/white)
- Mesen-S header scoring: `SourMesen/Mesen-S` `Core/BaseCartridge.cpp`
- Mesen 0.9.9 NES-only proof: `SourMesen/Mesen` tag 0.9.9 `Core/RomLoader.cpp`
