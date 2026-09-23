# 2026-09-23 — Milestone 5: Stardock hub + host test harness

## What shipped

`ST_DOCK` hub (sector-1 P): 8 departments, 6 with 4-item buy lists —
shipyard (holds/fighters/shields), hardware (probe/beacon/genesis),
bank (deposit/withdraw/ledger + ~3%/day interest), police
(commission/bounty/record), underground (boss/fence/lay-low),
tavern (ale/gossip/library), trading post → M4 commodity port,
LEAVE. SRAM v3 (+gbank/probes/beacons/torp/comm/bankday). New
globals reset on new game + load. `SHIP WITH gselfdrive=0`.

Screenshots: `m5_hub` (8-dept list), `m5_holds` (HOLDS +5 MAX 25),
`m5_ship2/3` (hardware dept, E-PROBE ABOARD TOT 1), `m5_dep`
(DEPOSITED BAL 1000), `m5_wd` (WITHDREW BAL 0), `m5_police2`
(NEED 500 ALIGN FOR ISS), `m5_reg` (M4 regression via hub:
SECTOR 3 / 10006 / 498). Reviewed-only (no tour frames left under
the 255 cap): fighters/shields/beacon/genesis buys, ledger/record/
library/gossip/ale displays, UG + bounty/commission positives, bust.

## Host test harness (tests/host/, NEW permanent infra)

Compiles the REAL game logic (transformed copy) with cc65 --cpu 6502
and runs it under py65: T1 M4 full tour, T2 M5 hub tour, T3 interest
math — 14/14 PASS, ~2.2M cycles, zero hangs. Tour tables double as
the test vectors. Run: `tests\host\build_test.ps1` then
`python tests/host/run_tests.py`.

Lessons that cost real time (constraints for future turns):
- `static`-stripping the copy for linkage is fine, BUT the tick_once
  refactor left a stray extra `}` that 65816 codegen silently
  accepted while 6502 errored. Fixed (inert in shipped ROMs).
- `BRA` is 65C02-only: test CRT must use `JMP` under `--cpu 6502`.
- py65 `reset()` leaves PC at $0000 — the driver must load PC from
  the reset vector manually.
- ld65 map omits TU-local symbols; use `-Ln labels.txt` (note its
  `._` dotted-local form) for the report block address.
- No C compiler on this box (only Python); cc65+py65 fills the gap.

## Debugging notes

- Two more "lost edge" scares, both hand-trace errors, both settled
  by the host trajectory recorder in minutes: (1) bank BACK is sel3,
  not sel2 (LEDGER sits between); (2) SCN2=102 truncated the fence
  pair — bumped to 106. Rule: trust the trajectory, not the trace.
- Emulator speed variance (~3-170fps across runs) remains the main
  verification tax; sticky-end tours + bracketed shots still work.
- snes9x process hygiene: kill ALL instances and confirm zero remain
  before a verification run (stale zombies capture confusion).
