# 2026-09-22 — Black screen resolved: TWO root causes, title art shipped

The title screen now renders in full color in both snes9x and Mesen-S
(starfield, blue planet, white 3D "STAR MERCHANTS", red "2026", blue warp
lines, gray freighter, blinking PRESS START). The "black screen blocker"
from 2026-09-21 turned out to be TWO independent bugs. The 2026-09-21 claim
"title screen working" was NEVER visually verified (all captures were
black) — it was inferred from hand-decoded bytes. Lesson: trust
screenshots, not disassembly.

## Root cause 1: cc65 stack-frame locals hang the CPU (the big one)

Any C code using **stack-frame locals** (`decspN` + `ldax0sp`/`stax0sp`/
`addeq0sp` + `(c_sp),Y` access) or **stack-passed function args** (`pusha`)
never completes: the program stalls with the screen in force-blank
(BLACK). Code using only globals (absolute addressing), inline constants,
and parameterless functions works perfectly.

Evidence chain (each step one build + one screenshot):
- Minimal C (no locals, no calls, inline palette+INIDISP): GREEN backdrop.
- Same + `for (w...)` font-load loop with **local** `w`: BLACK.
- Same loop with **global** counter: GREEN, loop completes.
- Pure-asm probe (`src/cpustate.s` `_probe`, zero stack use): GREEN and
  reports D=0, DBR=0, c_sp=$1FFF, M=X=1 — machine state is PERFECT.
- All helpers disassembled from the ROM and verified correct; loop logic
  verified correct from cc65-generated `build/main.s`. Still hangs.
- Empty local-counter loop was once thought to work (GREEN) — INVALID:
  crt0 was calling `_probe`, not `_main`, in those builds. Every "green
  loop" screenshot from the probe era is void. PID-tracked clean-slate
  captures were introduced after that (`runclean.ps1` pattern).

Rule going forward (in `src/main.c` header comment too): **globals only,
parameterless functions only, inline register writes. No C locals, no C
function arguments, ever.** The exact failure mechanism inside the stack
helpers is still unknown (all static state verified); left as TBD — the
globals discipline unblocks all milestones regardless.

Related fix in `src/crt0.s` (keep): set DP=$0000 FIRST via full-16-bit
TCD before any direct-page access (`sta c_sp` at $80/$81). The old order
relied on the emulator zeroing D at reset; real hardware leaves it
undefined. Also `.import _probe` + `src/cpustate.s` kept as an unreferenced
bring-up diagnostic (paints D/B/c_sp/M/X state as colors, no stack use).

## Root cause 2: CGRAM power-on state is NOT black

Writing only the used (color0,color1) pairs leaves palettes 1-7 pointing
at power-on garbage: the first color title build showed a GREEN ship
(gray expected), RED planet (blue expected), etc. — colors that exist
NOWHERE in our palette data.

Fix: init ALL 128 BG colors (8 pals x 16: c0=black, c1=theme color,
c2..c15=black) in `load_palettes()`. Verified with a 7-band palette probe
(white/gray/red/darkred/blue/dimblue/cyan, all exact).

## Verified PPU config (Mode 1 text layer, all values on the wire)

BGMODE=$01, BG1SC=$68 (map at VRAM words $6800, 32x32), BG12NBA=$03
(font at VRAM words $3000), VMAIN=$80 (inc after high byte), TM=$01,
NMITIMEN=$81, INIDISP=$0F. Tile for ASCII c is (c-32); tilemap high byte
is palette<<2. START button is bit 4 of $4218 ($10) — the old code checked
bit 3 ($08, actually UP). Auto-joy settle: spin on $4212 bit 0 before
reading $4218.

## Also fixed this session

- `scripts/capture.ps1`: rewrote around PrintWindow->CopyFromScreen with
  forced z-order (TOPMOST toggle, SetProcessDPIAware, fresh bounds after
  focus, foreground poll). New flags: -ClientOnly, -NoClose,
  -UsePrintWindow, -ProcessName. Verified: Mesen-S pulled from behind
  Notepad and captured complete; same-PID launch/capture/close tracking.
- Cross-emulator workflow: snes9x (`snes9x/snes9x-x64.exe`, also captured
  via -ProcessName) renders identically; the known-good Mode1Scroll.sfc
  renders perfectly in it, which is how the black screen was proven to be
  OUR bug, not the environment's. Mesen-S 0.4.0 has NO --testrunner
  (string-checked the exe); screenshots are the debugger.
