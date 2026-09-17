# Star Merchants SNES - Development Milestones

Based on TradeWars 2002 gameplay analysis (Iago's War Manual, TradeWars Museum, historical docs).

## Milestone 0: Foundation (Current)
- [x] PVSnesLib + ca65/ld65 toolchain working (build compiles main.c)
- [x] Build script (build.ps1) compiles & links framework
- [x] Emulator launch target configured (-Run flag)
- [x] Memory map defined (LoROM, 8 Mbit, vector table, header) via snes.cfg
- [x] CP437 font with box-drawing chars → 2bpp tiles in main.c
- [x] ANSI text-mode BG layer rendering prototype in main.c

## Milestone 1: Title Screen & Boot
- [ ] **Title Screen** - ANSI art logo "STAR MERCHANTS", version, credits
- [ ] **Attract Mode** - Cycle demo screens (sector map, starport, combat)
- [ ] **Input Handler** - Joypad polling, menu navigation (D-pad, A/B/X/Y, Start/Select)
- [ ] **Save/Load** - SRAM detection, new game / continue / options

## Milestone 2: Main Menu & Character Creation
- [ ] **Main Menu** - New Game, Continue, Options, Credits
- [ ] **New Game Wizard** - Trader name, ship name, corporation (optional)
- [ ] **Difficulty/Options** - Turn rate, universe size, starting credits, Ferrengi aggression

## Milestone 3: Sector View (Core Gameplay)
- [ ] **Sector Display** - ANSI grid: sector #, warps (1-6), port/planet/ftrs icons
- [ ] **Navigation** - Warp (W), Transwarp (T), Course Plotter (C)
- [ ] **Sector Scan** - Density scanner (D), Holo-scanner (H), Long-range (L)
- [ ] **Status Bar** - Credits, holds, fighters, shields, alignment, XP, turns left
- [ ] **Command Line** - Text input buffer, command parser (P/R/S/T/C/D/M/Q/etc.)

## Milestone 4: Starport Trading
- [ ] **Port Screen** - Class (1-9, 0, S), buy/sell prices, quantities, haggle
- [ ] **Trading Logic** - Buy (B), Sell (S), Haggle (H), Steal (R), Rob (R)
- [ ] **Port Classes** - BBS, BSB, SBB, SSB, SBS, BSS, SSS, BBB, Class 0
- [ ] **Steal/Sell Cycle** - 5XP optimal pricing, psychic probe integration
- [ ] **Port Regeneration** - 5%/day, 10-day cap, bust records (14-day cycle)

## Milestone 5: Stardock (Central Hub)
- [ ] **Shipyard** - Buy/sell ships, holds, drives, scanners, cloaks, twarp
- [ ] **Hardware Emporium** - Mines, probes, genesis torps, adets, ptorps, beacons
- [ ] **Galactic Bank** - Deposit/withdraw, 4% daily interest (citadel treasury)
- [ ] **Federation Police** - Bounties (good align), commissions (+500 align → ISS)
- [ ] **Underground** - Name change, hit contracts (evil align), bounties
- [ ] **Tavern** - Grimy Trader (track traders, UG password), TriCron, Singles Bar
- [ ] **Library** - Ship database, alien derelicts
- [ ] **Singles Bar** - Flavor text, robbery risk

## Milestone 6: Planet Management
- [ ] **Planet Display** - Level, citadel, colonists (Ore/Org/Eq groups), production
- [ ] **Citadel Upgrades** - L1-L5 requirements, build times, resource costs
- [ ] **Quasar Cannon** - Sector/Atmosphere levels, fuel consumption (bug: 50% actual)
- [ ] **Planetary Fighters** - MRL (0% to avoid 6666 bug), offensive/defensive odds
- [ ] **Shield Generator** - L5 only, 1639+ shields = invulnerable (shield bug)
- [ ] **Transwarp Generator** - L4+, range in hops
- [ ] **Colonist Transport** - Load/unload, optimal 1000 holds/group, jettison for -align
- [ ] **Genesis Torpedo** - Create planet in empty sector
- [ ] **Atomic Detonator** - Destroy planet, 50 XP, -50 align

## Milestone 7: Combat System
- [ ] **Ship vs Ship** - Offensive/defensive odds, fighter counts, shields
- [ ] **Combat Math** - (enemy_ftrs * enemy_odds) / your_odds = min fighters to win
- [ ] **Ship Types** - 15 classes (MerCru, ScoMar, MisFri, CorBat, CorFla, ColTra, CarTra, MerFre, ImpSta, HavGun, StaMas, ConSte, TkhOri, ThoSen, TauMul)
- [ ] **Photon Missile** - Destroys port/planet shields, blinds defenses
- [ ] **Corbomite** - Retaliation damage on ship destruction
- [ ] **Escape Pod** - Survive destruction, trade for ScoMar + 1000 creds

## Milestone 8: Ferrengi & Aliens
- [ ] **Ferrengal** - L4 planet, 30% sector QC, 40% MRL, 5000 ftrs, 100k treasury
- [ ] **Ferrengi Ships** - Assault Trader (1.0), BattleCruiser (1.2), Dreadnaught (1.4)
- [ ] **Ferrengi Behavior** - Demand tribute, grudge system (3 grudges/ship), regen
- [ ] **Neutralization** - 1 ftr + max shields = immune; 3000 ftrs in home sector = kill all
- [ ] **Aliens** - Good/evil ranks, alignment-based XP/align shifts on kill

## Milestone 9: Corporation & Multiplayer Prep
- [ ] **Corporation** - Create/join, CEO Flagship, corp planets, corp fighters
- [ ] **Ship Exchange** - Unlock ships in citadel, transfer fighters/shields
- [ ] **Corp Megaholds (CEYLAD)** - Bug/exploit: unlimited holds via ship swap
- [ ] **Team Tactics** - Sector defense, planet building, coordinated stealing

## Milestone 10: Endgame & Polish
- [ ] **Victory Conditions** - Credits, planets, XP, alignment leaderboards
- [ ] **High Score / Hall of Fame** - SRAM persistence
- [ ] **Music/SFX** - SPC700: title, sector, port, combat, stardock themes
- [ ] **ANSI Animations** - Starfield, warp, combat, port entry
- [ ] **Bug Fixes** - 6666 bug (MRL=0), holds bug, shield bug, cloak reliability
- [ ] **Options Menu** - ANSI on/off, sound, difficulty, controller config
- [ ] **Cartridge Build** - Header, checksum, LoROM/HiROM, FastROM toggle

## Milestone 11: Physical Release Prep
- [ ] **ROM Validation** - bsnes/higan accuracy, hardware flashcart test
- [ ] **Manual/Box Art** - ANSI-style manual, cartridge label
- [ ] **Build Pipeline** - Clean/release builds, versioning, checksums

---

## Screen Flow Reference (TradeWars 2002 v1.03d)

```
Boot → Title → Main Menu → New Game → Sector View (Main Loop)
                                      ↓
                              ┌───────┼───────┐
                              ↓       ↓       ↓
                         Port     Stardock  Planet
                              ↓       ↓       ↓
                         Trade    Shipyard  Manage
                         Steal    Hardware  Colonists
                         Haggle   Bank      Upgrade
                              ↓       Police   Citadel
                         Combat    UG       Genesis
                              ↓       Tavern   Adet
                         Escape    Lib/Bar   Twarp
```

## Key Constants (from Iago's War Manual)
- Max sectors: 1000 (default), up to 20000 (TWGS)
- Max fighters/sector: 5000 (no planet), 30000 (with planet)
- Max planet fighters: 32000
- Max planet shields: 1639 (invulnerable)
- Max holds: 150 (ISS), 85 (CorFla), 70 (StaMas), 250 (ColTra)
- Alignment thresholds: -100 (steal), +500 (commission), +1000 (ISS), +200 (UG entry)
- Steal formula: XP/20 = holds, XP*7 = credits (safe)
- Port regen: 5%/day, capped at 10 days since last visit
- Citadel interest: 4% daily on treasury
- Cloak: 25,000 creds/day, 100% reliable in v1.03d

## Controller Mapping (SNES)
| Button | TradeWars Command | Function |
|--------|------------------|----------|
| D-pad  | Navigation       | Menu cursor, sector warp dirs |
| A      | Confirm/Enter    | Execute command, dock, land |
| B      | Cancel/Back      | Abort, retreat, exit menu |
| X      | Command Menu     | Open command palette (P/R/S/T/C/D/M/Q) |
| Y      | Scan/Info        | Density scan, holo-scan, planet info |
| L      | Prev Sector      | Cycle warp destinations |
| R      | Next Sector      | Cycle warp destinations |
| Start  | Pause/Menu       | Main menu, save, options |
| Select | Help/Status      | Toggle status bar, command help |

---

## Next Action: Milestone 0 Completion
1. Verify cc65/ca65/ld65 + PVSnesLib includes link successfully
2. Write build.ps1 wrapper around cl65
3. Compile hello_world.c → .sfc
4. Run in bsnes/Mesen-S
5. Prototype ANSI BG layer with CP437 font