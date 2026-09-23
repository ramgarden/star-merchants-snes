# 2026-09-23 — ANSI graphics upgrade path + sound milestone (research)

## Question

Could the text engine switch from direct tilemap string drawing to
per-character sprites with per-sprite palettes, to better match the
BBS-terminal look? And is there enough memory for it?

## Answer: full-screen sprites are impossible on hardware

SNES PPU limits: **128 sprites total, max 32 per scanline** (plus a
34-tiles-per-scanline bandwidth cap), 4 sprite palettes. A 32x28 text
screen is ~900 characters — an order of magnitude over budget. This
is a silicon limit, not a memory limit, so no amount of free ROM/RAM
changes it.

For the record we are nowhere near memory ceilings either: 64 KB
VRAM mostly empty, ROM using ~15 KB of a 32 KB bank (LoROM maps
megabytes), 128 KB WRAM barely touched.

## Where sprites genuinely help (small counts)

- Text cursor / selection arrows as sprites (frees tilemap cells)
- Hardware starfield: a dozen OAM stars with per-sprite palettes =
  twinkle + parallax behind text, near-zero CPU
- Blinking prompts / warp flashes without tilemap redraws

## The real upgrade path (all inside the tilemap engine)

1. **Solid-block background tiles.** Font glyphs use color index 1
   only, so per-character color today = 1-of-8 palette choice, always
   on transparency (index 0). One solid tile per palette color gives
   true BBS-style per-character backgrounds. Cost: ~16 tiles.
2. **Full 16-color palette ramps.** We init all 128 CGRAM entries but
   each palette defines only c1. Bright/dim/shadow variants per
   palette are free and immediately deepen the art.
3. **A second BG layer for backdrops.** Mode 1 has three BGs; we use
   one. Planet gradients / station silhouettes behind the text layer
   = authentic ANSI layering with zero text-engine changes.
4. **ANSI attributes as PPU features.** Bold = brighter palette
   entry, blink = frame-toggled tile, underline = underscore-row
   tiles.

Recommended scheduling: one polish spike under M10 "ANSI Animations",
after gameplay milestones land — no engine rewrite required.

## Sound: promoted to its own milestone (M11)

Driving the SPC700 (BRR samples + tracker/song data, e.g.
SNESMod-style) wired into the custom crt0/NMI setup by hand — no
PVSnesLib linkage to lean on. Work items: SPC boot + upload path,
NMI-safe communication ($2140-$2143), 5 themes in BBS-door spirit
(spare loops), warp/trade/scan/dock/alarm/cursor SFX, per-theme
emulator capture verification, mute toggle, CPU-budget check against
the vblank render path. Estimated size: one full milestone.
