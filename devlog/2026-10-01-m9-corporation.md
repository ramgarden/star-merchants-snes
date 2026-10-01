# 2026-10-01 - M9 Corporation (lean single-player corp, verified)

Scope note: MILESTONES.md M9 listed multiplayer prep + the CEYLAD
megaholds exploit. Shipped the lean single-player core instead: corp
charter + ship exchange with the megaholds bug fixed BY DESIGN (holds
are capped, overflow goes to a fighter pool). Team/multiplayer stays
future work.

## What shipped (Stardock Hardware dept, 7 rows)

- CORP CHARTER 5000 (row 4): pays 5000cr, sets corp bit; gcorpk =
  bit0 + shipcls*2; denies with NEED 5000 CREDITS when poor.
- EXCHANGE SHIP (row 5): cycles hull SCOUT/MERCHANT/FREIGHTER/DREAD
  (names 4x12, stats 4x4, all far data, prices DATA-DRIVEN: 0/5000/
  5000/30000). Gates: cargo-fit (CARGO WONT FIT, no megaholds),
  price (CANT AFFORD HULL). Fighter overflow -> gpool (uint8, 255
  clamp); pool reabsorbs on next swap (POOL TAPPED). gm1 = hull-name
  preview, gm2 = outcome (see buffer-aliasing bug below).
- SRAM v7 (+gcorpk/gpool/gshipcls, round-trip host-proven). New game
  ship IS merchant-class (20 holds/30 fighters), so gshipcls inits 1.

## Bank-0 budget war (was ~27 B free, M9 needed ~150 B)

1. Dieted ~85 B of reviewed-only strings (bust/fence/laylow/ale/
   gossip/mug/commission/bounty/port-hold lines; full list in the
   e0bad50..HEAD diff), gpool uint16->uint8: still +126 B over.
2. Moved the 4 hardware row literals (PROBE/BEACON/GENESIS/PHOTON,
   ~61 B RODATA) into FARRODATA + 4 new BSS buffers (gfar4-7) filled
   by the hub-entry hook (fn22). Screen bytes IDENTICAL (host draw
   stub sees same lengths; screenshots pixel-identical rows).
3. Tour ROM (gselfdrive=7) overflowed by 2 B while ship ROM fit:
   `= 0u` compiles to 3-byte STZ, `= 7u` to 4-byte LDA/STA (plus one
   branch-size flip nearby). Bank-0 is now byte-full. Dieted the
   duplicated CHECK B/S CLASS LETTERS denial (2x24 B -> 2x17 B, -14 B).
   Ship ROM (35B9) links with a few bytes to spare; M10 MUST budget
   bank-0 first (diet or FARCODE growth).

## Tour-4 broke (host T5 failed: 4000cr/30ftrs/1ph/0xp, no fight)

Root cause (external py65 tracer, no firmware change): dept-2 BACK
moved sel 4->6, tour pressed A on sel 4 = CHARTER (-5000), then
wandered the hub. Fix: +2 DOWNs before BACK (DOWNx3 3->6, A),
tail frames +4 (max 131, still 8-bit safe). +8 B far cascade:
FT_SC5/SCP5/SC6/SCP6/FT_M8 +8, M8B 3856->3864, M9B 4144->4152,
verify base 784->792 / 1072->1080, TOTAL 1480, FAR_SIZE +1488.
All constants cross-checked by script (head-40 identical, tail =
old+4) + verify_far + host. T5 back to exact M7 numbers
(11300cr/XP35/11ftrs/photons-spent). Lesson: tour tables are
position-coupled to menu row counts -- changing a dept's row count
means re-walking every tour that visits it.

## Bugs the host/tracer caught in new M9 code (all fixed)

- Exchange hull-NAME read used x4 stride on 12-stride names (garbage
  preview). Fixed to x12 (charter pattern).
- Prices were hardcoded (15000/30000) while far stats said otherwise;
  SCOUT row even encoded 5000 for a free downgrade. Prices now read
  from stats (hi*256+lo via doubling chain); rows: 0/5000/5000/30000.
- `if (gi)` pool-tapped flag after the HULL m8cpy: m8cpy uses gi as
  its cursor, so gi was always nonzero -> POOL TAPPED overwrote every
  fit message. Flag now lives in gtmp (ftrmax, dead in fit branch) --
  and only AFTER its last compare/overflow use (first attempt reused
  too early and broke the overflow compare; caught by the new
  overflow probe before commit).
- NEW RULE (hardware-only class, screenshot-proven): m8cpy clobbers
  gi/gact/gfar_o. No flag/scratch may live in those across an m8cpy
  call. goc/gk/gtmp/gom/gn survive (goc is the selector, read-only).
- Msg/row buffer aliasing (screenshot-proven): charter/exchange
  results were written (goc 0/1) into gfarmsg/gfarm2, which ALSO hold
  the CHARTER/EXCHANGE menu rows -> after one exchange the menu
  showed FREIGHTER/HULL SWAPPED as rows 4-5. Results now go to
  dedicated gfar8/9 (m8cpy goc 7/8); dock_exec shows gm1=gfar8,
  gm2=gfar9. Rows survive.

## Verification

- Build gates (Mesen-S header score 20 + verify_far 1568 B/42 fields
  + same-bank-calls gate) green; ship ROM 35B9.
- Host 74/74: T1-T8 unchanged, T9 = 16 tour asserts (dock/sec-1/
  500 turns/0cr/corpk 5/pool 0/shipcls 2/holdmax 40/fighters 30/
  FREIGHTER+HULL SWAPPED lines/SRAM-v7 round-trip/dept 2 stable) +
  4 direct fn21 probes (overflow 90->80+10pool, cargo-deny,
  price-deny, pool-tapped).
- Screenshots: Mesen-S + snes9x render identically (exchange end
  state: CR 0 TR 500, FREIGHTER/HULL SWAPPED, all 7 rows correct).
  docs/screens/corp_exchange.png.
- Mid-tick sampling note: external tracer once showed credits 10120
  mid-charter -- torn 16-bit update (lo STA landed before hi STA),
  benign, same class as the 8-bit tour-frame rule.
