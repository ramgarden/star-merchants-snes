# Star Merchants SNES - Agent Instructions

SNES homebrew game (spiritual successor to Tradewars 2002). Licensed GPL-3.0.

## Structure

```
src/                    # C source (cc65 + PVSnesLib)
  main.c                # Minimal text-mode ANSI test (entrypoint)
scripts/
  build.ps1             # PowerShell build script (PVSnesLib + ca65/ld65)
  clean.ps1             # Clean build artifacts
  package.ps1           # ROM packaging
  install-runner.ps1    # Emulator/flashcart setup
assets/
  sprites/              # Graphics
  music/                # SPC700 audio
  sfx/                  # Sound effects
  palettes/             # Color palettes
  fonts/                # Font data
  ansi/                 # ANSI art references
tools/                  # Custom tooling (empty)
devlog/                 # Development notes
```

## Development Notes

- **Target**: SNES (65c816 CPU, SPC700 audio)
- **Toolchain**: PVSnesLib (C library) + ca65/ld65 (assembler/linker)
- **Build**: `& "scripts\build.ps1"` compiles + links `.sfc`
- **Emulator**: `-Run` flag launches ROM in Mesen-S / bsnes / snes9x

## Verified Commands

| Action | Command |
|--------|---------|
| Clean + build ROM | `& "scripts\build.ps1" -Clean` |
| Full build + run emulator | `& "scripts\build.ps1" -Run` |
| Build with target | `& "scripts\build.ps1" -Target "LoROM_SlowROM"` |
| PVSNESLIB home override | Set `$env:PVSNESLIB_HOME` before running |

## Key Files

- `src/main.c` — Minimal text-mode test using direct SNES registers + CP437 font
- `scripts/build.ps1` — Build orchestration; default `PVSNESLIB_HOME = C:\dev\snes\tools\pvsneslib\pvsneslib`
- `scripts/clean.ps1` — `Remove-Item -Recurse -Force build`
- `scripts/package.ps1` / `install-runner.ps1` — Placeholders (empty)

## Toolchain Quirks

- `build.ps1` requires `PVSNESLIB_HOME` env var or the default `C:\dev\snes\tools\pvsneslib\pvsneslib`
- cc65 cfg `C:\cc65\cfg\snes.cfg` needed for linking; provides MEMORY/SEGMENTS for LoROM
- PVSnesLib object files must exist at `lib\$Target\*.obj` under the PVSnesLib dir
- `#include <snes.h>` from PVSnesLib conflicts with `stdint.h` / `stddef.h`; define `uint8_t`/`uint16_t` manually if needed

## Next Steps (verified)

1. ✅ Implement build.ps1 — **done**, fixed PVSNESLIB_HOME path
2. ✅ Create src/main.c — **done**, minimal ANSI text-mode test compiles & links
3. Define memory map in src/ (lorom/hirom, vector table, header)
4. Set up asset conversion pipelines (gfx→CHR, audio→SPC, ANSI→font+tilemap)
5. Add emulator test targets (bsnes, snes9x, Mesen-S)
6. Prototype ANSI rendering: CP437 font → 2bpp tiles → BG layer scrolling

## Key References

- PVSnesLib: https://github.com/alekmaul/pvsneslib
- Awesome SNESdev: https://github.com/matthewh/awesome-snesdev
- cc65/ca65 docs: https://cc65.github.io/doc/
- TradeWars ANSI art references in assets/ansi/