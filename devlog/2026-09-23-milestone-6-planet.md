# 2026-09-23 — Milestone 6: planet management + bank-1 font move

## What shipped

`ST_PLANET` screen (sector palette L LAND, 7th command): planet
header (level via sector name table), citadel, 3 colonist groups,
fighters/fuel/shields, quasar S/A + MRL 30 + TWARP range; 8 options
(CLAIM/UPGRADE, DEPLOY, LOAD, UNLOAD, JETTISON, QUASAR, DETONATE,
LEAVE) with L/R group select. Claim (5 turns +25 XP), upgrades
L1-L5 (credit ladder 2k/5k/10k/20k + colonist minimums + turn),
deploy (10 ftrs + 5 shields), 10-unit transport, jettison (align),
quasar setter (20% steps, 100 fuel), genesis (torp + turn + 25 XP),
detonator (1-shot, +50 XP / -50 align). Single-colony model
(gcolsec restore/fresh guard, seeded sector-1 colony), warp-day
production, SRAM v4 (+20B). `SHIP WITH gselfdrive=0`.

Screenshots: `m6_claim5` (CLAIMED! + full table), `m6_quasar`
(QSR 20/20 + reconciled stats), `m6_claim` (genesis-denial sticky
+ FTR 20/HL 15/XP 25/TR 494), `m6_title` (bank-1 font proof).

## The bank-1 font saga (read this before touching FARFONT)

Bank 0 overflowed (CODE+RODATA = 32.5KB/32KB) → moved the 3KB font
to LoROM bank 1 (cfg BANK1 area → file 0x8000, `_font_load`
long-addressing bulk copy in sram.s). First builds rendered
garbage: root cause was a one-nibble slip, `lda $180000,x`
(bank $18, unmapped) instead of `lda $018000,x` (bank $01).
Evidence chain that nailed it: file layout verified byte-perfect
(pic at 0x8000 ✓), loop disassembly verified correct, $FF-fill
probe rendered BLACK (index 15 is black in our palettes — proving
writes land!), CGRAM tell proved the copy returns. Only the source
bank was wrong. snes9x + Mesen-S agree, so it is ROM truth.

Takeaways: (1) LoROM bank N = `$NN8000` 24-bit — count the nibbles;
(2) the $FF-fill-then-read pattern is the fastest VRAM-path
bisector (keep it in the toolbox); (3) bank-1 pattern (far asset +
leaf copy helper) is now THE way to add bulk const data (M11 song
data will live here).

## Verification notes

- Host harness T4 (11 asserts: claim/deploy/load/quasar/warp/
  denial + production) green; full suite 25/25, no hangs.
- Reviewed-only (no tour frames under the 255 cap): upgrade levels
  2-5 purchases, unload/jettison/detonate/genesis-positive, bust
  paths, colony restore on revisit. All share verified code shapes.
- Emulator discipline re-confirmed: exactly ONE launch + ONE hold
  per screenshot (second holds freeze on both emulators in this
  environment); judge speed on the first hold, only chase fast runs.
