# 2026-09-23 — Milestone 4: starport trading

## What shipped

Full TW2002 port loop in `src/main.c` + `src/sram.s` (unchanged),
new `ST_PORT` state: dock (1 turn) → commodity table → buy/sell/
haggle/steal/leave. Stardock is Class "S" with BSS-pattern sides
(port buys Fuel, sells Organics/Equipment); classes 1-8 use authentic
B/S side patterns enforced at transaction time. Prices/stocks roll
from the sector hash + warp-day counter (restock adaptation, capped);
every transaction auto-saves (SRAM record bumped to v2: +gday).
New game starts with 5 Fuel Ore aboard (5/20 holds) so selling works
immediately. `SHIP WITH gselfdrive=0` (verified).

Screenshots (snes9x): `m4q_dock` (port table: B 23 / S 17 STK38 /
S 56 STK43, YOU F5 CR 10000, WELCOME + turn deducted), `m4r_sell`
(SOLD 1 FUEL ORE / GAINED 23 CR XP +1), `m4s_buy` (BOUGHT 1 ORGANICS /
PAID 17 CR / HOLDS 5/20), `m4q_buy` (STOLE 2 ORGANICS! / ALIGN -5
XP +10; cumulative F4 O3 CR 10006 STK35), `m4r_buy` (HAGGLE ACCEPTED /
XP +5), `m4fin_load` (full tour: RETURNING TRADER / SECTOR 3 /
CREDITS 10006 / TURNS 498 — dock+warp turn costs and port economy
persisted through the SRAM round-trip).

## Economy reconciliation (all from screenshots)

Start: F5 O0 E0 CR 10000 TR 500. Sell fuel (+23) → F4 10023.
Buy org (-17, STK38→37) → O1 10006. Steal 2 org (STK→35) → O3,
ALIGN -5 XP +10. Haggle (+5 XP). Dock -1 turn, warp -1 turn → 498.
Every number cross-checks; prices deterministic across runs
(sector-hash gen).

## Tour notes

Port demo inserted pre-warp (sector 1): palette → P → dock, then
sell/buy/steal/haggle/leave, then warp → menu → Continue → loadret
sticky. Dwell gaps (12 frames) between port actions so timed shots
land; all tour values kept ≤253 (uint8 truncation lesson from M3).
Bust path (`(gday+gsec)&3==3`) is code-reviewed, not screenshotted
(tour engineers gday=0/sec=1 → success).

## Deferred to M5 (explicitly)

Class 0 (holds/fighters/shields shop) and Class 9 StarDock hub
(shipyard, Hardware Emporium incl. probes for probe-assisted steals,
Bank, Police/UG, Tavern/Library). Steal bust records likewise.
