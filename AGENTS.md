# Star Merchants SNES - Agent Instructions

SNES homebrew game (spiritual successor to Tradewars 2002). Licensed GPL-3.0.
**Read `devlog/` for the detailed history of what has been tried and verified.**

## Current Status (2026-09-23)

- ✅ Black screen RESOLVED (two root causes, see
  devlog/2026-09-22-black-screen-root-causes.md)
- ✅ **Title screen art SHIPPED and screenshot-verified in snes9x AND
  Mesen-S** — ANSI homage: starfield, blue planet + cyan limb, white 3D
  "STAR MERCHANTS" + gray shadows, red "2026" + dark-red shadow, blue warp
  lines, gray freighter + cyan windows, credits footer, blinking
  "PRESS START" (blink ON/OFF both captured); main loop runs, waits for
  START (bit 4 of $4218), then holds
- ✅ **Milestone 1 COMPLETE 2026-09-23** — attract mode (10s idle → Stardock
  demo, verified), input handler (proven via self-drive through real tick
  path), SRAM save/load (2KB header, magic+checksum record, auto-save on
  new game + warp, Continue resume verified: SECTOR 3 / TURNS 499)
- ✅ **Milestone 3 COMPLETE 2026-09-23** — sector view: ANSI display
  (red unvisited warps, port/planet/fighter lines), warp nav with turn
  cost + visited bitmap, density + holo scans, status bar, CMD prompt +
  command palette (D/H/S/C/P/Q), course plotter, port stub for M4.
  Screenshot-verified state by state (see devlog/2026-09-23-milestone-3-sector.md).
  New constraint learned: script-tour tables MUST stay 8-bit (frame
  numbers >255 silently truncate and collapse the tail into one tick)
- ✅ **Milestone 4 COMPLETE 2026-09-23** — starport trading: dock
  (1 turn), commodity table (S/B side letters, prices, STK stocks),
  buy/sell with credit/hold/stock/cargo checks, once-per-visit haggle
  (+5 XP, ±12.5%), steal (2 units, ALIGN -5/XP +10, bust risk), leave.
  Full economy reconciled across screenshots; SRAM v2 (+gday);
  new game starts with 5 Fuel Ore. See devlog/2026-09-23-milestone-4-port.md.
  Class 0 / Class 9 StarDock hub deferred to M5.
- Do **NOT** trust the 2026-09-21 "title screen working" claim — it was
  inferred from disassembly, never screenshotted; every capture then was black

## Hard Constraints (violating these re-introduces the black screen)

- C code: **globals + parameterless functions + inline register writes
  ONLY. No C locals, no C function arguments, ever.** cc65 stack-frame
  helpers (`decspN`/`ldax0sp`/`addeq0sp`/`pusha`, `(c_sp),Y` access) hang
  the CPU on this setup even though D=0, DBR=0, c_sp=$1FFF, M=X=1 were all
  verified perfect. See `src/main.c` header comment.
- CGRAM: **init ALL 128 BG colors** (8 pals × 16), not just used slots.
  Power-on CGRAM is garbage, not black — uninitialized slots show through.
- crt0: DP=$0000 via full-16-bit TCD BEFORE any direct-page access
  (`sta c_sp`); `src/cpustate.s` `_probe` kept as unreferenced no-stack
  diagnostic that paints D/B/c_sp/M/X state as colors.

## Structure

```
src/                    # C + asm sources (cc65 toolchain, custom crt0 — PVSnesLib NOT linked)
  main.c                # Title + menu + sector engine (globals-only C, direct PPU registers;
                        # gselfdrive=1 = scripted self-drive tour for testing, SHIP WITH 0)
  sram.s                # 2 leaf helpers for SRAM byte access ($70:0000+, 16-bit X widened locally)
  crt0.s                # Custom reset/NMI/IRQ + SNES header (2KB SRAM declared) + vector table
  font.s                # .incbin of PVSnesLib 96-glyph 4bpp font (see devlog for format)
  cpustate.s            # No-stack bring-up probe (unreferenced; paints CPU state as colors)
scripts/
  build.ps1             # Compile+link+pad+checksum, then gate on Mesen-S's exact header logic
  snes-lorom.cfg        # ld65 config: ROM/HEADER/VECTORS areas -> header at file 0x7FC0
  capture.ps1           # Screenshot an emulator window to PNG (full window by default;
                        # flags: -ClientOnly -NoClose -UsePrintWindow -ProcessName) for
                        # automated visual debugging
  clean.ps1             # Remove build dir
tools/
  verify_rom.py         # Exact Python port of Mesen-S BaseCartridge::GetHeaderScore/LoadRom
devlog/                 # Session-by-session technical history — READ THIS
pvsneslib_extracted/    # Reference material + known-good Mode1Scroll.sfc + font asset
```

## Verified Commands

| Action | Command |
|--------|---------|
| Build + validate ROM | `& "scripts\build.ps1"` (fails loudly if Mesen-S would reject it) |
| Clean + build | `& "scripts\build.ps1" -Clean` |
| Build + launch Mesen-S | `& "scripts\build.ps1" -Run` |
| Validate any ROM file | `python tools\verify_rom.py <file.sfc>` |
| Screenshot Mesen-S | `& "scripts\capture.ps1" -Out build\shot.png -DelaySeconds 3` |
| Launch Mesen-S on a ROM | `Start-Process "C:\dev\snes\tools\mesen-s\Mesen-S.exe" '"path\to.sfc"'` |

## Toolchain (paths verified)

- cc65 V2.19 at `C:\cc65` (`cl65` auto-links `none.lib` runtime: stack helpers,
  `zerobss`, `zeropage` — this is what resolves `c_sp`/`zerobss` imports in crt0)
- PVSnesLib at `C:\dev\snes\tools\pvsneslib\pvsneslib` — **only used as an asset
  source**; its prebuilt .obj libs are tcc/COFF format and do NOT link with ld65
- **Emulator: Mesen-S 0.4.0** at `C:\dev\snes\tools\mesen-s\Mesen-S.exe`
  (installed from the official GitHub release zip; shows a one-time config wizard)
- Python 3.14 available (`python` on PATH) — useful for hexdumps/CRC/hashing;
  PowerShell 5.1 has unreliable unsigned-int arithmetic (see Quirks)
- snes9x 1.62.3 at `snes9x/snes9x-x64.exe` (in-repo) — second-opinion
  emulator; renders identically to Mesen-S for our PPU config

## CRITICAL: Emulator Facts (root cause of the historical "infinite loop")

- **Mesen 0.9.9 (winget `SourMesen.Mesen`) is an NES-ONLY emulator.** Proven from
  its own source (`SourMesen/Mesen` tag 0.9.9, `Core/RomLoader.cpp`): it only
  recognizes NES formats (iNES/FDS/NSF/NSFE/UNIF) or a game-DB CRC match, and
  logs `Invalid rom file.` for **any** `.sfc`. No SNES ROM can ever load in it.
  The multi-console unified Mesen was never publicly released.
- **Mesen-S** (SourMesen/Mesen-S, latest release 0.4.0) is the SNES sibling with
  the same UI/debugger. Use it. bsnes/snes9x also work for basic testing
- Mesen-S accepts a ROM by **scoring header candidates** at base addresses
  `0, 0x200, 0x8000, 0x8200, 0x408000, 0x408200` (`Core/BaseCartridge.cpp`).
  Our ROM and the reference `Mode1Scroll.sfc` both score 20
- `tools/verify_rom.py` replicates that scoring exactly — run it BEFORE launching
  the emulator whenever the build script's gate is bypassed

## ROM Format Requirements (verified)

- LoROM: SNES internal header at file offset `0x7FC0` (21-byte title, map mode
  `$20` = LoROM/SlowROM at `0x7FD5`, ROM size byte `$08` = 2 Mbit at `0x7FD7`)
- Vectors at file offsets `0x7FE0-0x7FFF`; reset vector `0x7FFC` must point
  >= $8000 (bank 0); Mesen-S additionally checks the opcode at the reset target
  (SEI/CLI/JMP/JSR/etc. score up; BRK/SBC/CPY score down)
- Checksum/complement at `0x7FDC-0x7FDF`: 16-bit word-sum of the whole padded
  ROM excluding those 4 bytes; complement = `0xFFFF - sum`; `sum + comp = 0xFFFF`
- ld65 writes MEMORY areas **sequentially** into the output file in config order
  and `fill = yes` pads each area to its full size. `scripts/snes-lorom.cfg`
  exploits this: ROM `$7FC0` + HEADER `$20` + VECTORS `$20` = exactly 32 KB,
  header lands at file offset `0x7FC0`, vectors at `0x7FE0`. build.ps1 then pads
  to 256 KB and patches the checksum
- crt0.s **must** `.export __STARTUP__` — cc65-generated modules import it;
  without the export ld65 pulls none.lib's generic startup + condes.o and fails
  on unresolved constructor symbols

## Memory Map / Runtime Setup (decoded & verified this session)

- All code/data in LoROM bank 00 (first 32 KB). File offset F == CPU `$8000+F`
- crt0 (`src/crt0.s`): emulation->native switch, DB=0 via `pha/plb`,
  DP=$0000 via full-16-bit `tcd` BEFORE any direct-page access, hardware
  stack and cc65 `c_sp` both at `$1FFF`, 8-bit A/X/Y when calling C (cc65
  convention), ClearRegisters zeroes `$2100-$2133` and `$4200-$420D`,
  then `zerobss`, `_main`. Label addresses shift every build — see
  `build/starmerchants.map`, do NOT trust hardcoded addresses.
- PPU config in main.c: BGMODE=1 (4bpp), BG1SC=$68 (map at VRAM word $6800,
  32x32), BG12NBA=$03 (char base word $3000), VMAIN=$80 (increment after high)
- VRAM: font tiles 96 x 8x8 4bpp at words $3000-$35FF; BG map words
  $6800-$6BFF; **tile index for ASCII char c is `c-32`**; tilemap high byte
  is `palette << 2` (palettes 0-7)
- Font data format (PVSnesLib `pvsneslibfont.pic`): planar 4bpp — each 8x8 tile
  is 32 bytes; bytes 0-15 = bitplanes 0/1 interleaved per row, bytes 16-31 =
  planes 2/3; glyph pixels use **color index 1** (index 0 = transparent)
- Palette: 8 BG palettes, c0=black everywhere, c1 = white/gray/red/darkred/
  blue/dimblue/cyan/yellow; auto-joy + NMI enabled via NMITIMEN=$81;
  START button = bit 4 of $4218
  ($10 — NOT bit 3); joy settle: spin on $4212 bit 0 before reading $4218.
  Full joypad: $4218 = B,Y,Sel,Start,Up,Down,Left,Right (bits 7-0);
  $4219 = A,X,L,R (bits 7-4).
- Menu engine (Milestone 2): 9-state machine in main.c, all screens
  screenshot-verified via self-drive; **live joypad input does NOT arrive
  in test emulators** (snes9x 1.62.3 never asserts Start/Select from any
  key; Mesen-S keys unresponsive; snes9x rewrites snes9x.conf on exit and
  its input overlay echoes the conf, not core state — see
  devlog/2026-09-22-milestone-2-menu.md). START edge path reviewed but
  never fired live; verify on hardware later.

## Toolchain Quirks

- Linking uses `scripts/snes-lorom.cfg` (NOT `C:\cc65\cfg\snes.cfg`, headerless)
- `#include <snes.h>` from PVSnesLib conflicts with `stdint.h`/`stddef.h`;
  main.c defines `uint8_t`/`uint16_t` manually
- PowerShell 5.1: `Byte -shl 8` returns a Byte (truncates!) — cast `[int]` first;
  `0xFFFFFFFF` parses as Int32 `-1` (sign-extends) — use `4294967295L` + `-band`
- Hexdump with Python, not PowerShell array slicing (slice-with-array bugs)

## Debugging Workflow (this is how we avoid burning credits in loops)

1. **Build gate**: build.ps1 runs the Mesen-S header-scoring port; a structurally
   invalid ROM fails the build with a specific message
2. **Visual state**: `scripts/capture.ps1` screenshots the running emulator window
   to a PNG; read the PNG directly (vision) — no user round-trips needed.
   Screenshots ARE the debugger: Mesen-S 0.4.0 has no `--testrunner`
   (verified by string-searching the exe), and there is no input injection,
   so checkpoint screens (backdrop colors, text stages) + PID-tracked
   clean-slate captures are the workflow (see devlog/2026-09-22-*)
3. **Runtime debugging**: hand-decode ROM bytes (see `build/starmerchants.map`
   for current labels) or use Mesen-S's GUI debugger; bisect with visible
   checkpoints (backdrop color per init stage, text-stage screens)
4. **Calibrate against the reference**: `Mode1Scroll.sfc` in pvsneslib_extracted
   is a known-good 256 KB LoROM (renders perfectly in snes9x); diff
   headers/layout against it when in doubt. snes9x at
   `snes9x/snes9x-x64.exe` renders identically to Mesen-S — use it as a
   second opinion (capture with `-ProcessName snes9x-x64`)

## Next Steps

1. Milestone 4 starport trading (see MILESTONES.md) — hooks ready:
   `gport`/`gportcls`/`cls_str`, `P PORT` palette item, cargo save fields

## Key References

- PVSnesLib: https://github.com/alekmaul/pvsneslib
- Mesen-S source (header scoring): https://github.com/SourMesen/Mesen-S
- Mesen 0.9.9 source (NES-only proof): https://github.com/SourMesen/Mesen
- Awesome SNESdev: https://github.com/matthewh/awesome-snesdev
- cc65/ca65 docs: https://cc65.github.io/doc/
- Full-analogy game design: see MILESTONES.md (TradeWars 2002 reference)
