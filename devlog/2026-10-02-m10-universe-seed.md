# 2026-10-02 - M10 slice: universe seed (button mash + SRAM v9)

New-game wizard now forges a 16-bit universe seed: ST_SEED screen
("SEED THE UNIVERSE" / "MASH 16 BUTTONS", 16 glyph cells, weave bar,
B backs out), 16 fresh presses of any non-B button, full seed holds
one beat, A/START confirms into launch. Each press folds raw pad
bytes + frame timing into gunivseed (far fn23, double-add = helper-
free shift). `sec_seed` mixes the seed (`ghash = gsec ^ gunivseed`;
seed 0 = legacy universe exactly, so tours 1-8 bypass with 0 and
every verified number holds). Seed persists in SRAM v8→v9.

## Bank-0 (86 B slack, slice wanted ~450 B)

- cc65 `-O` ALREADY pools identical string literals (proved: sharing
  16 duplicated literals over 45 sites saved only 22 B). Reverted to
  keeping the 22 B; lesson: measure, don't assume.
- Real funding: far-sourced ALL Stardock menu rows (dept-0 hub 8 rows
  ~89 B; dept 1/3/4/5/6 15 rows ~158 B) + dock footers (~137 B net,
  2 refill sites: dock_back + hub-exec) into FARRODATA with BSS
  buffers (gfar10-32, gfootA/B) filled by the hub-entry hook (fn22
  extended). Screen bytes identical everywhere.
- Cut the separate weave phase (bar fills live per press instead).

## Host T11 + tour-9 (86/86 with T1-T10)

Tour-9 (appended, no cascade): tour-1 wizard head + 16 fixed mashes
(no B) + confirm + launch + warp. Fixed mash => seed 2426 (0x097A),
sector-2 warps 43/335 (seed-0: 422/823) -- asserted exact. NOTE: the
seed is position-dependent (gsfr timing; T11 runs last).
Host RAM model grew ($200+$1400; ROM moved to $1600, runner updated)
after far buffers overflowed the 3.5 KB host RAM.

## Screenshots (Mesen-S + snes9x identical)

Full-seed screen (16 cells + full bar, via temp SCN9 hold) and the
seeded sector 2 (warps 43/335/753/854). docs/screens/seed_mash.png,
docs/screens/seeded_sector.png.
