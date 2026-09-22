# Star Merchants for Super Nintendo
A VibeBrew "spiritual successor" of the old BBS game Tradewars 2002.

## Current Status

The build pipeline produces a structurally valid 256 KB LoROM with a
fully working ANSI-style title screen ("STAR MERCHANTS", planet,
freighter, blinking PRESS START) — screenshot-verified in **Mesen-S**
(the SNES Mesen) and snes9x. See `AGENTS.md` and `devlog/` for the full
technical state (including the two resolved root causes of the old
black-screen blocker) and next steps.

## Quick Start

```powershell
& "scripts\build.ps1" -Run      # build, validate, launch in Mesen-S
& "scripts\build.ps1" -Clean    # clean build dir
python tools\verify_rom.py build\starmerchants.sfc   # standalone ROM validation
```

Requires cc65 at `C:\cc65`. The emulator of record is **Mesen-S** at
`C:\dev\snes\tools\mesen-s\Mesen-S.exe` (note: regular Mesen 0.9.9 is
NES-only and cannot load `.sfc` files).

## Documentation Map

| File | Contents |
|------|----------|
| `AGENTS.md` | Agent/handoff instructions: verified facts, workflow, current status |
| `devlog/` | Session-by-session technical history (read before touching anything) |
| `MILESTONES.md` | Roadmap + TradeWars 2002 design reference |
| `scripts/build.ps1` | Build script with emulator-exact ROM validation |
| `tools/verify_rom.py` | Standalone Mesen-S header-scoring port (Python) |
