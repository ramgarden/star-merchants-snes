# 2026-09-25 — M7 combat verification (tour, host, screenshots, reboot hunt)

## Tour 4–6 tables were placeholders (found + fixed)

Commit `055f074` moved tours to bank-1 FARRODATA but transcribed tours
4–6 wrong: `_scf4` is 72 bytes (not 84), `_scp4` is 92 bytes (looks like
a copy of `_scp3`), `_scf6`/`_scp6` content differs from the original
bank-0 arrays, and every `FT_*` offset for tours 4–6 misses the real
layout (`FT_SC4` 584 vs actual 580, etc.). Tours 1–3 offsets were exact.

Fix: restored `_scf4`/`_scp4`/`_scf6`/`_scp6` from the pre-move arrays
(`ce7eac9`: scf4 84 frames, scp4 80 values + 4 implicit zero tail = 84,
scf6/scp6 14), set `FT_*` to cumulative offsets
(580/664/748/752/756/770, total 784), machine-verified all 12
table/offset pairs. Tour 5 was already byte-identical.

## Combat-demo tour authored (replaces the WIP tavern-parker)

The transcribed ce7eac9 tour never reaches combat (ends parked in the
tavern: photon buy + ale + gossip). Wrote a new 37-press tour 4:
new game → hub hardware → photon buy → leave → warp sector 3 →
palette → ATTACK → photon (blind) → attack (victory) → leave.
Sector-3 recon (python port of `gen_sector`): 11 hostiles, class 13
(THO SEN, odds 13/11). Predicted victory math by hand: blind halves
min-to-win 39→19, fighters 30→11, bounty 13*100+500=1800, credits
10000-500+1800=11300, XP +23, turns 499, photons 0.

Host T5 asserts all 10 values — PASS on first run, byte-exact.
Photon-blind is load-bearing for the win (unblinded 39 > 30 fighters
= defeat), so T5 proves photon + odds + bounty together.

## Turn-clamp fixes (escape/retreat at 0 turns)

`fight_escape`/`fight_retreat` did unconditional `gturns--` (every
other turn-spend in the game guards zero; underflow mints 65535
turns). First attempt (deny-with-message) overflowed bank 0 by 68B;
shipped clamp form (`if (gturns > 0u) gturns--`, ~25B). Bank 0 now has
46B spare — every future addition must be weighed in single bytes.

## Eyeball failure log (process lesson)

Spent a long loop on a "paradox" (double-ale economics, ghost cursor
moves, turns 244) that dissolved the moment I machine-parsed instead
of hand-reading: `scp4[56]` is X/8, not A/6 — I had misindexed by one
pair. Corollaries proven along the way and now recorded: turnslo 244
= 500 & 255 (byte truncation in my probe, not missing turns); tour
holds never exceed 16 ticks (repeat threshold 25); no adjacent
non-zero tour actions anywhere (every press edges exactly once).
Rule: decode tables with scripts, never by eye.

Also fixed: T5 probe loops reset `gtest_t` before `boot_init` (which
leaves it at 8) — a +8 sample-label shift that poisoned an hour of
timeline math. Reset counters AFTER boot everywhere.

## Emulator reboot phenomenon (RESOLVED: cc65 TOS C-stack helpers)

Mid-tour runs intermittently returned to title around fight
execution, on Mesen-S, snes9x, and the user's RetroArch (live input,
no rewind/runahead/turbo). Long elimination (logic/host-clean, draws
verified one by one on hardware, SRAM/vectors/stack/input/farbyt
clean, mul/div helpers disassembled-clean) plus a reversible
bisection ladder (gen-stubbed stable, draw-stubbed stable, full-draw
reboots; then foot-literal stable, foot-with-odds reboots) isolated
it to `fight_odds()` with nonzero operands — specifically the cc65
TOS (top-of-stack) multiply/divide helpers (`pushax`, `tosumulax`,
`tosudivax`, `popptr1`), which take the C stack via the `c_sp`
zeropage pointer. They are used nowhere else in the game (verified by
scanning all codegen), so no verified path ever touched them; the
host (py65, zeroed RAM, 6502 codegen vs ROM 65816 codegen) never
noticed.

Fix: rewrote `fight_odds`, the victory bounty (`*100`), and both
`/100` XP displays with shift-add/subtract step math in globals
(`gom`/`goc` scratch, BSS = $0 ROM). Bit-exact for ship-combat
ranges (product < 65536 always). Result: TOS helpers fully unlinked
from the ROM (`pushax`/`popptr1`/`tosumulax`/`tosudivax` gone from
the map), host T5 still byte-exact, and the full fight (entry with
THO SEN/33/70/13/11, MIN TO WIN 39, photon blind, victory
1800cr/+23xp, sector aftermath 11300/11ftrs) screenshot-verified
stable across many captures with zero reboots since. Only remaining
runtime mults are straight-line `mulax3`/`mulax10` (hardware-proven).
Bank 0 now has 12B spare — treat as full.

Rule added to the hard-constraints list: no C-stack-taking
expression helpers (`pushax`/`tos*`/`popptr1`, i.e. no multi-step
`?:` in arithmetic, no `(a*b)/c` single expressions) — stage
multi-step math through globals so every intermediate stays in AX.

## Cleanup (per milestone request: dead weight only, keep tours)

Deleted unreferenced scratch: `src/main.c.bak`, `src/font.s.bak`,
`src/testreset.s.off`, `src/main.c.title` (all untracked/ignored,
nothing references them). Removed stale "tours 4-6 in bank 0"
comment. Kept the selfdrive system (emulator-screenshot vehicle),
`cpustate.s` probe (per AGENTS.md), and the `hj_pad` host-injection
path (still never exercised end-to-end — future vehicle).

## Ship state

`gselfdrive=0`, full victory tour in bank 1, ROM builds + gate pass,
host suite 35/35 green (25 existing + 10 new T5).
