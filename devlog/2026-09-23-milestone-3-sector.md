# 2026-09-23 — Milestone 3: sector view + M1 closeout (SRAM, attract)

## What shipped

Sector-view main loop in `src/main.c` (TW2002 core, adapted to 32 cols):
Stardock fixed sector + deterministic hash universe (500/1000/2000 by
options), warp cycling/execution with turn cost, density + holo scans,
status bar, CMD prompt + X command palette (D/H/S/C/P/Q), course plotter,
port stub (M4 hook). M1 closed out: attract mode (10s title idle →
Stardock demo, A/B/START exits) and SRAM save/load (2KB declared in
header, 45-byte magic+checksum record, auto-save on new game + warp,
Continue → RETURNING TRADER resume). `SHIP WITH gselfdrive=0`
(verified); tour tables stay in ROM, inert.

Screenshots (all snes9x unless noted): `m3n_clk1` (sector-1 base),
`m3k_warp` (warp→sector 3, palette open, density, TR 499),
`m3o_warp` (WARP COMPLETE report), `m3m_warp` (HOLO-SCAN),
`m3m_dense` (COURSE HOPS 4/FUEL 12 — math checks: 1+(3&3)=4, 4*3=12),
`m3o_dense` (NO PORT IN SECTOR stub end), `m3p_load` (save→warp→
Continue round-trip: SECTOR 3, TURNS 499), `m3fin_title` (ship title),
`m3fin_attract` (demo banner).

## Bugs caught by screenshots (all fixed, all verified after)

1. **fmt_num clobbered the warp-loop counter** (`gi` reused as digit
   index → infinite loop, black screen from force-blank). Fix: dedicated
   `gk`. Same class audited everywhere (visited helpers use `gk`
   because they run inside the `gi` warp loop).
2. **16-bit `*`/`%` removed preemptively** (universe hash, mods). Only
   `&`, constant shifts, add/sub remain — same op class as M2-proven
   code. (Whether mul/mod helpers actually hang is unproven; removal is
   cheap insurance, not a diagnosis.)
3. **Script frame numbers ≥256 silently truncate in uint8 tables**
   (274→18 etc.), collapsing the whole tail into one tick = one edge.
   Fix: whole tour re-spaced to fit ≤195. Same lesson as the 2026-09-22
   `uint16_t`-table incident — check bounds when extending the tour.

## False "hang" alarms (game was innocent both times)

- Sticky end-states + static screens look frozen. A debug line drawn
  once (launch-screen F/S/I/ST) never updates — it is NOT a live
  counter. If progress is in doubt, suspect the SCRIPT (exhausted?
  misaligned? firing in the wrong state?) before the engine.
- Emulator speed varies wildly run to run (~3–70fps observed on this
  box). Never equate wall-time with frame-time; states self-identify,
  so bracket and read the screen.
- The tour schedule must be re-derived by hand after ANY prefix change:
  the M2 prefix visits the Continue stub, so sector entry is 6 frames
  later than a naive read suggests. Trace (action,state)→result.

## Verification technique that worked

- Temporary per-tick hex counter (script clock) poked to two VRAM cells
  in `wait_vblank` (row 27, unused on all screens). Made speed and
  liveness unambiguous on every shot. REMOVED before ship (see
  `git log` — do not ship diagnostics).
- Dwell design: leave report screens up for 24 script frames so timed
  shots land; one continuous foreground-held run per screenshot series.

## Next (Milestone 4)

Starport trading: buy/sell ore-organics-equipment, port classes affect
which side trades, haggle/steal/rob stubs. Hooks ready: `gport` /
`gportcls` / `cls_str`, `P PORT` palette item, save fields for cargo.
