; Star Merchants font data (4bpp 8x8 tiles, 96 glyphs for ASCII 32..127)
; Glyph bytes are shared with the PVSnesLib example font asset.
; Lives in LoROM bank 01 (FARFONT segment); copied to VRAM at boot by
; the _font_load helper (long-addressing bulk copy, see sram.s).

.segment "FARFONT"

.global _font_pic
_font_pic:
.incbin "../pvsneslib_extracted/pvsneslib/snes-examples/graphics/Backgrounds/Mode1Scroll/pvsneslibfont.pic"
