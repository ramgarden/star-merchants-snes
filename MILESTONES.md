# Star Merchants SNES - Development Milestones

Based on TradeWars 2002 gameplay analysis (Iago's War Manual, TradeWars Museum, historical docs).

## Milestone 0: Foundation (Complete)
- [x] Build script (build.ps1) compiles & links a LoROM via cc65 (custom crt0, no PVSnesLib linkage)
- [x] ROM format: header at 0x7FC0, vectors at 0x7FE0, checksum — verified, loads in Mesen-S
- [x] Emulator: Mesen-S 0.4.0 installed (C:\dev\snes\tools\mesen-s) — regular Mesen 0.9.9 is NES-only, do not use
- [x] ROM validation: tools/verify_rom.py + build gate replicate Mesen-S header scoring
- [x] **Black screen RESOLVED 2026-09-22** — two root causes, see
  devlog/2026-09-22-black-screen-root-causes.md: (1) cc65 stack-frame
  locals/stack-passed args hang the CPU — C code must use globals +
  parameterless functions only; (2) CGRAM power-on is not black — init
  ALL 128 BG colors. Renders verified in snes9x AND Mesen-S.
- [x] Title screen renders (font/palette/tilemap init via direct PPU registers in main.c)
- [x] Input basics (START = bit 4 of $4218 ($10), auto-joy settle on $4212
  bit 0; blinking PRESS START waits for START, then holds)

## Milestone 1: Title Screen & Boot (Complete 2026-09-23)
- [x] **Title Screen** - ANSI art logo "STAR MERCHANTS" (white 3D + gray
  shadow), red "2026", planet, freighter, starfield, warp lines, version,
  credits, blinking PRESS START — verified by screenshot in both emulators
- [x] **Attract Mode** - 10s idle on title auto-plays a Stardock sector demo
  ("DEMO: STARDOCK SECTOR" banner); any of A/B/START exits back to title —
  verified by screenshot (build/m3fin_attract.png)
- [x] **Input Handler** - Joypad polling ($4218/$4219, auto-joy settle on
  $4212, edge detect + D-pad repeat), full menu/sector navigation (D-pad,
  A/B/X/Y, Start/Select) — exercised end-to-end through the real
  tick/dispatch path by the in-ROM self-drive tour (`gselfdrive`)
- [x] **Save/Load** - 2KB SRAM declared in header (was ROM-only); 45-byte
  "SM" magic + checksum record (names, options, sector, turns, credits,
  ship stats, cargo); auto-save on new game + every warp; Continue loads
  into a "RETURNING TRADER" resume screen — save→warp→Continue round-trip
  verified by screenshot (build/m3p_load.png: SECTOR 3, TURNS 499)

## Milestone 2: Main Menu & Character Creation (Complete 2026-09-22)
- [x] **Main Menu** - New Game, Continue, Options, Credits; wrap cursor,
  A/START select, B back to title — verified by screenshot
- [x] **New Game Wizard** - Trader name + ship name entry (8 slots, A-Z
  0-9 charset cycling, slot cursor, A advance/confirm, B back) — verified
- [x] **Difficulty/Options** - Turn rate, universe size, starting credits,
  Ferrengi aggression (3 values each, L/R adjust, persists) — verified
  incl. summary screen showing all choices + launch stub (Milestone 3 hook)
- [x] Continue = functional SRAM resume (was "NO SAVED GAME" stub)
- Verified via in-ROM scripted self-drive (`gselfdrive` in main.c: injected
  pad masks through the real tick/dispatch path, PID-tracked screenshots);
  live START-button transition still unverified (no Start keypress arrives
  in either test emulator — see devlog/2026-09-22-milestone-2-menu.md)

## Milestone 3: Sector View (Core Gameplay) (Complete 2026-09-23)
- [x] **Sector Display** - ANSI header (sector # + nebula), warp list
  (unvisited red like TW ANSI, selected cyan), port (name + class),
  planet (name + level), fighters (red HOSTILE) — Stardock fixed + hash
  universe verified (build/m3n_clk1.png, m3k_warp.png)
- [x] **Navigation** - D-pad/L/R warp cycling, A/START warp (1 turn,
  "WARP COMPLETE / SECTOR n / TURNS LEFT" report), visited tracking
  (512-sector bitmap) — warp 1→3 with 500→499 turns verified
- [x] **Sector Scan** - Density scanner (Y, per-warp 0-99 + red HAZ),
  Holo-scan (X palette: port/planet/fighter detail) — both verified
  (build/m3k_holo.png density, m3m_warp.png holo)
- [x] **Status Bar** - Credits, turns, fighters, shields, holds/max,
  signed alignment, XP — verified on every sector shot
- [x] **Command Line** - `CMD:[n] (?=HELP)? :` prompt + `WARP>n` target
  line, X command palette (D REDISPLAY / H HOLO / S DENSITY / C COURSE /
  P PORT / Q QUIT), SELECT help, B back to menu, course plotter
  (HOPS 4 / FUEL 12 TURNS math verified, build/m3m_dense.png), port stub
  hook for Milestone 4 — all verified

## Milestone 4: Starport Trading (Complete 2026-09-23)
- [x] **Port Screen** - "PORT STARDOCK CLS S" header, per-commodity rows
  (Fuel Ore / Organics / Equipment with S=port-sells cyan / B=port-buys
  yellow letters, unit price, STK stock for S sides), cargo + credits
  status, B/S/H/R/L option list, COM select footer — verified
  (build/m4q_dock.png: B 23 / S 17 STK38 / S 56 STK43, YOU F5 CR 10000)
- [x] **Trading Logic** - Buy (B): credit/hold/stock checks, "BOUGHT 1
  ORGANICS / PAID 17 CR / HOLDS 5/20" — verified; Sell (S): cargo/side
  checks, "SOLD 1 FUEL ORE / GAINED 23 CR XP +1" — verified; Haggle (H):
  once per visit, ±12.5% prices, +5 XP — verified; Steal/Rob (R): 2
  units from stock, ALIGN -5 / XP +10, bust risk (fine 500 + ALIGN -20)
  — "STOLE 2 ORGANICS!" verified
- [x] **Port Classes** - All 8 commodity classes with authentic B/S side
  patterns (BBS/BSB/SBB/SSB/SBS/BSS/SSS/BBB) + Stardock "S" special;
  class gating enforced ("PORT WON'T SELL/BUY THAT"). Class 0
  (holds/fighters/shields) and Class 9 StarDock hub are M5 (Shipyard/
  Hardware Emporium) scope
- [x] **Steal/Sell Cycle** - Full economy reconciled on screenshots
  (10000 +23 sell -17 buy = 10006; F5→F4, O0→O3; STK38→STK35);
  psychic-probe integration waits on M5 Hardware Emporium (no probes yet)
- [x] **Port Regeneration** - Time-based restock adaptation: stocks roll
  per dock from sector hash + warp-day counter (gday, capped), persisted
  in SRAM v2 record; docking costs 1 turn, trading auto-saves

## Milestone 5: Stardock (Central Hub) (Complete 2026-09-23)
- [x] **Shipyard** - Class 0 outfitter: BUY HOLDS 5000 (+5 max), BUY
  FIGHTERS 500 (+5), BUY SHIELDS 1000 (+5), all with credit checks —
  holds verified ("HOLDS +5 MAX 25"); fighters/shields share the
  identical buy path (reviewed)
- [x] **Hardware Emporium** - BUY PROBE 500 / BEACON 100 / GENESIS 5000
  (caps, M6 hooks: gprobes/gbeacons/gtorp persisted in SRAM v3) —
  probe verified ("E-PROBE ABOARD TOT 1"); beacon/genesis reviewed
- [x] **Galactic Bank** - DEPOSIT/WITHDRAW 1000, LEDGER, 3%/day interest
  accrual on hub entry (capped 32 days, 60000 max) — full round-trip
  verified ("DEPOSITED BAL 1000" → "WITHDREW BAL 0", CR 4500→3500→4500)
- [x] **Federation Police** - COMMISSION (needs 500 ALIGN → ISS flag for
  M7), BOUNTY (align-gated payout, once per visit), RECORD (align/XP
  display) — commission denial verified ("NEED 500 ALIGN FOR ISS")
- [x] **Underground** - SEE BOSS (needs -100 ALIGN), FENCE (+150 once
  per visit), LAY LOW (capped redemption) — implemented + host-proven
  nav; evil-positive path reviewed (unreachable in a fast tour)
- [x] **Tavern** - ALE (10cr, +1 XP), GOSSIP (4 rotating Grimy/TriCron
  rumors + alley-robbery risk), LIBRARY (ship database: ISS/Freighter/
  Cruiser/Corvette specs) — reviewed (no tour frames left under the
  255 cap); gossip/ale share verified msg patterns
- [x] **Hub plumbing** - Sector-1 P opens the hub (8 departments);
  TRADING POST routes into the M4 commodity port (M4 tour re-verified
  end-to-end through it: SECTOR 3 / 10006 / 498); ledger/interest/SRAM
  v3 host-proven (bank 1165 after 5 days); full tour logic proven on
  host harness (14/14) with zero hangs over 2.2M cycles

## Milestone 6: Planet Management (Complete 2026-09-23)
- [x] **Planet Display** - "PLANET STARDOCK LV 5": citadel level, 3
  colonist groups (COL O120 G80 E60 seeded home colony), fighters,
  fuel stockpile, shields, quasar S/A levels, MRL 30, TWARP range —
  verified (build/m6_claim5.png)
- [x] **Citadel Upgrades** - CLAIM (5 turns, +25 XP) then L1-L5 for
  credits (2000/5000/10000/20000) + colonist minimums + 1 turn —
  "CLAIMED! CITADEL LV 1" verified; upgrade path host-proven
- [x] **Quasar Cannon** - Settable sector/atmosphere level (20% steps,
  100 fuel per set, wraps at 100) — "QUASAR SET 20PCT / FUEL STOCK
  400" verified (build/m6_quasar.png)
- [x] **Planetary Fighters** - DEPLOY moves 10 ship fighters + 5
  shields per press (availability-checked, capped); MRL 30 displayed
  for M7 odds — ship 30→20 verified in status
- [x] **Shield Generator** - L5-gated display, planetary shield stock
  (5000 cap, invulnerable-note for M7); deploy included in DEPLOY
- [x] **Transwarp Generator** - Range row (L4+ shows planet level in
  hops, 0 otherwise) — display-verified, drive itself is M7 scope
- [x] **Colonist Transport** - LOAD/UNLOAD in 10s (holds-capped,
  group-select via L/R), JETTISON (frees holds, ALIGN -10) — load
  verified (ORE 120→110, ship 10, HL 5/20 → host asserts)
- [x] **Genesis Torpedo** - Sector-L with no planet: denial without
  torp ("NEED GENESIS TORP (5000)") verified as sticky end
  (build/m6_claim.png); creation path (+25 XP) reviewed (no tour
  frames to buy a torp)
- [x] **Atomic Detonator** - Destroys planet (+50 XP, -50 ALIGN,
  1-turn cost, 1-shot inventory from new game) — reviewed (same
  verified action shape as steal/detonate paths); sector re-entry
  safely re-seeds via the colony-restore guard
- [x] **Plumbing** - Single-colony model (gcolsec restore/fresh guard,
  seeded sector-1 colony), warp-day production (fuel + fighters),
  SRAM v4 (+20B planet record), L LAND added as 7th palette command,
  full M6 tour host-proven (claim/deploy/load/quasar/warp/denial)

## Milestone 7: Combat System (Complete 2026-09-25, verified 2026-09-28)
- ROLLED BACK 2026-09-24: first implementation (commit 6f1d999) shipped a
  `#pragma rodata-name` that pushed all C string literals to bank 1 ->
  garbled text on every screen. Code reverted to verified M6 state
  (b9eef19); re-implementing from that base. See
  devlog/2026-09-24-m7-rollback.md
- Combat tour tables were placeholders (wrong sizes/offsets/content for
  tours 4-6); restored from pre-move arrays, `FT_*` fixed, all 12
  table/offset pairs machine-verified. New 37-press combat demo tour
  (Stardock photon buy -> warp sector 3 -> ATTACK -> photon blind ->
  attack victory -> leave). See devlog/2026-09-25-m7-combat-verification.md
- [x] **Ship vs Ship** - Offensive/defensive odds, fighter counts, shields
  (host-proven: class 13 THO SEN, 33 fighters, odds 13/11)
- [x] **Combat Math** - (enemy_ftrs * enemy_odds) / your_odds = min fighters to win
  (host-proven exact: blind 39->19, fighters 30->11, bounty 1800, +23 XP)
- [x] **Ship Types** - 15 classes (MerCru, ScoMar, MisFri, CorBat, CorFla, ColTra, CarTra, MerFre, ImpSta, HavGun, StaMas, ConSte, TkhOri, ThoSen, TauMul)
  (THO SEN + stats screenshot-verified on hardware)
- [x] **Photon Missile** - Destroys port/planet shields, blinds defenses
  (screenshot-verified blind path to victory; buy path seen on hardware)
- [ ] **Corbomite** - Retaliation damage on ship destruction (reviewed only)
- [ ] **Escape Pod** - Survive destruction, trade for ScoMar + 1000 creds (reviewed only)
- [x] **Turn guards** - Escape/retreat clamp at 0 turns (was unguarded wrap to 65535)
- [x] **Fight screenshots** - Intro (COMBAT: THO SEN, 33/70, 13/11, MIN TO WIN 39),
  victory (ENEMY DESTROYED, BOUNTY 1800 CR XP +23), sector aftermath
  (11300cr/11ftrs/499t) — all captured on Mesen-S, stable across runs
- [x] **C-stack purge** - Root-caused the fight automatism reboots to cc65's TOS
  multiply/divide helpers (`pushax`/`tosumulax`/`tosudivax`, used only in
  fight math); rewrote odds/bounty/XP with shift-add/subtract step math.
  Helpers fully unlinked; host still byte-exact (see devlog)

## Milestone 8: Ferrengi & Aliens (Complete 2026-09-28)
- [x] **Ferrengal** - Rim-sector L4 planet "FERRENGAL", guard 200+day
  (cap 250, `uint8` adaptation of spec 5000), treasury 40000+day·100
  (cap 60000, `uint16` adaptation of 100k) — sector screenshot-verified
  (PORT:NONE, PLANET:FERRENGAL L4, FTRS:201 HOSTILE)
- [x] **Ferrengi Ships** - ASS TRA / BAT CRU / DREADNT with odds 5,6,7
  over 5 (exact 1.0/1.2/1.4 ratios); bounties 2000+ — fight intro
  screenshot-verified (COMBAT: ASS TRA, 33→25 FTRS, ODDS 5/5)
- [x] **Ferrengi Behavior** - Auto-tribute on engage (rich pay 500cr
  and stand down — "TRIBUTE PAID 500CR", CR 10000→9000 verified; poor
  fight with +1 grudge), 2-bit×3 grudge pack in SRAM v6, day-derived
  regen, retaliation scaling (+5 fighters/grudge)
- [x] **Neutralization** - ≤1 fighter + ≥50 shields = stand down
  ("FERRENGI STAND DOWN"); ≥3000 fighters at home = guard wiped +
  treasury looted (+50 XP, −25 ALIGN) — all host-proven (T7/T8)
- [x] **Aliens** - 1/8 victories roll good/evil rank (VORLON/ASGARD/
  ELDARI vs KRULL/MORGU/XARTH) with align ∓ / XP + shifts on the
  victory line ("VORLON GOOD SLAIN" host-proven; KRULL +10/+12 on the
  M7 tour: T5 now expects XP 35 by design)
- [x] **FARCODE infra** - Bank-0 ROM was full (12 B), so post-M7 logic
  lives in LoROM bank 1 (`src/m8.c`, `_far_exec` jsl trampoline);
  root-caused two hardware-only bug classes along the way (cross-bank
  JSR helpers → BRK-sled; asm X-discipline) with build gates for both.
  See devlog/2026-09-28-m8-ferrengi.md

## Milestone 9: Corporation (COMPLETE 2026-10-01, lean single-player core)

Multiplayer/team play stays future work; shipped the corp core with the
CEYLAD megaholds exploit fixed by design (holds capped, overflow pooled).

- [x] **Corporation** - Charter in Stardock Hardware (5000cr, gcorpk =
  corp bit + shipcls bits, NEED 5000 CREDITS denial)
- [x] **Ship Exchange** - 4 hulls (SCOUT/MERCHANT/FREIGHTER/DREAD, far
  names + data-driven stats/prices 0/5000/5000/30000), cargo-fit gate,
  fighter overflow to uint8 pool (255 clamp) + reabsorption
- [x] **SRAM v7** - +gcorpk/gpool/gshipcls with host-proven round-trip;
  new-game ship is merchant-class (gshipcls inits 1)
- [x] **Verified** - Tour-7 + host T9 (16 tour + 4 probe asserts, 74/74
  with T1-T8); Mesen-S + snes9x screenshot-verified (exchange end
  state). Bank-0 went byte-full (diet + far hardware rows got M9 in).
  See devlog/2026-10-01-m9-corporation.md

## Milestone 10: Endgame & Polish
- [ ] **Victory Conditions** - Credits, planets, XP, alignment leaderboards
- [ ] **High Score / Hall of Fame** - SRAM persistence
- [x] **Explored-universe persistence** (COMPLETE 2026-10-02) - The
  64-byte visited bitmap is saved to SRAM (record v7→v8) so explored
  sectors stay white (not red) across Continue/resume. Sector contents
  regenerate deterministically from the sector number (no per-game
  random universe; `gseed` is title glitter only). Tour-8 + host T10
  (83/83), Mesen-S + snes9x screenshot-verified (white 44 in sector
  990). Known quirk kept: sectors >512 alias onto 0-511 (bug-fix
  slice). See devlog/2026-10-02-m10-visited-persistence.md
- [x] **Universe seed** (COMPLETE 2026-10-02) - New-game button-mash
  seed entry (16 presses, glyph cells + weave bar, B backs out,
  A confirms) mixed into sector generation (`ghash ^= gunivseed`;
  seed 0 = legacy universe, tours 1-8 bypass). SRAM v9. Tour-9 +
  host T11 (86/86); screenshot-verified (full-seed screen, seeded
  sector 2 warps 43/335). Bank-0 funded by far-sourcing all Stardock
  menu rows + dock footers. See devlog/2026-10-02-m10-universe-seed.md
- [ ] **ANSI Animations** - Starfield, warp, combat, port entry (see
  devlog/2026-09-23-ansi-graphics-path.md for the researched upgrade
  path: solid-bg tiles, full palette ramps, backdrop BG layer, sprites
  for cursor/starfield only — full-screen sprites are impossible on
  hardware: 128 sprites / 32-per-scanline max)
- [ ] **Bug Fixes** - 6666 bug (MRL=0), holds bug, shield bug, cloak reliability
- [ ] **Options Menu** - ANSI on/off, sound, difficulty, controller config
- [ ] **Cartridge Build** - Header, checksum, LoROM/HiROM, FastROM toggle

## Milestone 11: Sound & Music (SPC700)

Own milestone (promoted from the old M10 Music/SFX line): driving the
SPC700 is a full workstream, not polish. No PVSnesLib linkage, so the
audio driver (BRR samples + tracker/song data, e.g. SNESMod-style)
must be wired into the custom crt0/NMI setup manually.

- [ ] **SPC700 Driver** - Boot the SPC, upload BRR samples + song data,
  mixer/tick integration with NMI-safe communication ports ($2140-$2143)
- [ ] **Music** - Title, sector, port, combat, stardock themes (BBS-door
  spirit: spare FM-ish loops, not orchestral)
- [ ] **SFX** - Warp whoosh, cash-register trade blip, scan sweep,
  docking clunk, steal alarm, UI cursor tick
- [ ] **Verification** - SPC playback captured in emulator per theme;
  mute toggle in Options; CPU budget check (audio tick must not break
  the vblank-driven render path)

## Milestone 12: Physical Release Prep
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

## Next Action: Milestone 7 — Combat System (see above).
 Hooks ready: gpftrs/gpsh (planetary defenses), MRL display,
 gfighters/gshields (ship), gcomm (ISS gate), quasar levels.