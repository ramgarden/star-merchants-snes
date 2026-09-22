# 2026-09-22 — Milestone 2: menu engine + the input saga

## What shipped

Main menu state machine in `src/main.c` (9 states: title, menu, credits,
continue stub, options, trader/ship name entry, summary, launch stub),
all globals-only C per the hard constraints. Verified screen by screen
via in-ROM scripted self-drive (see below): menu cursor/select/back,
options adjust (TURN RATE NORMAL→FAST captured mid-change), name entry
("AA" typed via Up/slot-advance), ship blanks, summary layout+values,
launch stub with trader name carried through. Screenshots in
`build/sd2_*.png`, `build/sd3_*.png`, `build/sd5_*.png`.

## Self-drive method (`gselfdrive` in main.c, SHIP WITH 0)

No test emulator delivers joypad input reliably (see below), so the ROM
tests itself: with the flag set, `read_pads()` is replaced by
`script_pads()`, which plays a (frame, action) table through the REAL
tick/dispatch/render path (edges, transitions, redraws all exercised;
only the 2-line `$4218/$4219` hardware read is bypassed). Tables are
8-bit action codes (1=START,2=UP,3=DOWN,4=LEFT,5=RIGHT,6=A,7=B) after an
earlier uint16-table incident (see gotchas). Screenshots at wall times
mapped from script frames; states are self-identifying so drift is fine.

## Input findings (all verified, all disappointing)

- snes9x 1.62.3 win32: A/B/dpad keys arrive (DisplayInput overlay
  confirms). **Start/Select bindings NEVER assert, for any key**
  (Space/Enter/N/V all tried, holds and taps, focus verified). The overlay
  echoes the conf (not core state), so it can't confirm Start either.
- snes9x **rewrites snes9x.conf on exit**: DisplayInput and key remaps
  revert to stock (PauseWhenInactive=FALSE survived). Snapshot the
  relevant lines before every input run; restore stock when done.
- snes9x `PauseWhenInactive` defaults TRUE; focus fights can freeze the
  core (identical frozen frames seconds apart). Rule out zombies and
  check the window title contains the ROM name on every run.
- Mesen-S: Mapping3 (WASD+JKL) keys produce nothing observable; copying
  them into Mapping1 in `Documents/Mesen-S/settings.xml` (back it up
  first) also produced nothing. Mesen-S input path unresolved.
- Emulator speed varies run to run (~5-35 ticks/s observed on this box);
  never assume wall-time == frame-time. Prefer persistent end-states and
  bracketed shots over exact-time captures.

## Gotchas hit this session

- `const uint16_t scf[90]` with values >255 silently truncated AND
  `SCN` exceeding the table size reads OOB into adjacent RODATA
  (garbage script actions). Keep script tables 8-bit and SCN == length.
- `gframe = 0u` in every `show_*` desyncs absolute-frame scripts; the
  script clock `gsfr` (incremented in `wait_vblank`, never reset) is the
  timing source. (Blink/repeat keep using `gframe`, fine.)
- Mid-transition force-blank screenshots (all-black) are normal one-off
  transients, not hangs — re-shoot before concluding.
- `snes9x/snes9x.conf` is NOT tracked by git (in-repo copy); the
  DisplayInput=TRUE overlay was invaluable for input debugging.
- START edge code path (`gj_new & PB_START`) is reviewed-correct but has
  never fired live (no Start arrives). Same shape as the script-proven
  A/B edges. Verify on hardware or a working emulator before release.

## Next (Milestone 3)

Sector view: needs a second BG or tile-reuse plan for the ANSI grid +
status bar + command line; text-entry builds directly on the name-entry
machinery (`gbuf`/`charset`/`name_index` pattern).
