# 2026-10-02 - M10 slice: explored-universe persistence (SRAM v8)

## Universe model (answers "random per new game?")

NO per-game random universe. Sector contents are a pure function of
sector number (+ size option): `sec_seed`/`sec_next` xorshift hash
(`src/main.c`), sector 1 fixed (warps 2,3,4,5). `gseed` is title-
starfield glitter only. New games differ in player state + visited
flags, not geography. Save/Continue persisted everything EXCEPT the
visited bitmap (`gvisited[64]`, RAM-only) — after Continue all warps
rendered red again. This slice persists the 64 bitmap bytes (SRAM
v7→v8, appended after `gshipcls`, ~148 B record in 2 KB SRAM).

Known quirk (NOT fixed here, belongs to the M10 bug-fix slice):
mark/is_visited mask `(gsec-1) & 511u`, so sectors >512 alias onto
0-511 while universe size goes to 2000. Persistence restores the 64
bytes faithfully; aliasing behavior is unchanged.

## Bank-0: two 64-iteration loops overflowed by 48 B

Dieted ~56 B of reviewed-only strings (all unreachable in every
tour, host re-proves): UNDOCKED/GENESIS-pair/REST/DETonate/TRADE-AT-
PORT/HOSTILES-pair/WON'T-SELL/WON'T-BUY denials. Ship ROM links
again. Also hoisted `gscr_f = 1u` out of the 7 script_pads arms
(~18 B) to fund the tour-8 dispatch arm.

## Tour-8 + host T10 (83/83 with T1-T9)

New tours APPEND at far end (no cascade): tour-8 = tour-1 head
(new game → sector 1) + warp0 chain 1,2,422,680,44,880,990 (chain
computed in Python against the xorshift gen; closes with white 44
in 990's list) + B,DOWN,A (menu→Continue) + A (Resume).
End state: sector 990 / 494 turns / 10000cr / vis0 03 + unit probes
(pattern round-trip, record version byte 8).

## Screenshots

Sector 990 post-Continue-resume: warp 44 WHITE, rest red; Mesen-S +
snes9x identical. docs/screens/visited_warps.png.
