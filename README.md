# Star Merchants for Super Nintendo
A VibeBrew "spiritual successor" of the old BBS game Tradewars 2002.

## Screenshots

All captures are real emulator screenshots (snes9x) of the actual ROM.

| Title screen | Sector view (warped to Sector 3) |
|---|---|
| ![Title screen](docs/screens/title.png) | ![Sector view](docs/screens/sector.png) |
| ANSI starfield, planet, freighter, blinking PRESS START | Warps (red = unvisited), port/planet/ftrs, status bar, CMD prompt |

| Command palette + warp | Starport trading |
|---|---|
| ![Sector palette](docs/screens/sector_palette.png) | ![Starport trading](docs/screens/port.png) |
| Warp to sector 3, palette open on HOLO-SCAN, density results | Class S port table: S/B side letters, prices, stocks, cargo |

| Haggle + economy | Course plotter |
|---|---|
| ![Haggle](docs/screens/haggle.png) | ![Course plotter](docs/screens/course.png) |
| Haggle accepted (+5 XP); status shows the reconciled trade economy | HOPS 4 / FUEL 12 TURNS back to Stardock |

| Attract mode | Continue / SRAM resume |
|---|---|
| ![Attract mode](docs/screens/attract.png) | ![Continue](docs/screens/continue.png) |
| 10s idle plays a Stardock sector demo | Save → warp → Continue round-trip restores sector, credits, turns |

| Planet management | Quasar cannon |
|---|---|
| ![Planet](docs/screens/planet.png) | ![Quasar](docs/screens/quasar.png) |
| Claimed homeworld: citadel, colonists, fighters, 8 commands | Quasar set 20%, fuel stock decremented, economy reconciled |

## Current Status

Milestones 0–6 complete and screenshot-verified: ANSI title + attract
mode, main menu + new-game wizard, sector view (warp nav, density/holo
scans, command palette, course plotter), starport trading (buy/sell/
haggle/steal), Stardock hub (shipyard, hardware, bank, police,
underground, tavern), planet management (citadels, colonists, quasar,
genesis/detonator), and SRAM save/load with Continue resume. See
`MILESTONES.md` for the roadmap, `AGENTS.md` and `devlog/` for the full
technical state.

**Milestone 7 (combat) rolled back 2026-09-24**: the first M7 commit
broke text rendering (a bank-1 rodata pragma moved all C string
literals out of bank 0 -> garbled text on every screen). Code was
reverted to the screenshot-verified M6 state and M7 is being
re-implemented from that base. See `devlog/2026-09-24-m7-rollback.md`.

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
