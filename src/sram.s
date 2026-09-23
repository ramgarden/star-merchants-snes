; ============================================================================
; Star Merchants - SRAM byte access ($70:0000+, LoROM SRAM)
; ============================================================================
; Two parameterless leaf helpers using shared globals (globals-only C
; constraint): _gsram_a (16-bit offset), _gsram_d (data byte).
; C calls with 8-bit A/X/Y (cc65 convention); X is widened to 16-bit
; only for the long-indexed access and restored before RTS.
; ============================================================================

.setcpu "65816"

.export _sram_wr
.export _sram_rd
.import _gsram_a
.import _gsram_d

.segment "CODE"

.proc _sram_wr: near
    rep #$10
    .i16
    ldx _gsram_a
    sep #$10
    .i8
    lda _gsram_d
    sta $700000,x
    rts
.endproc

.proc _sram_rd: near
    rep #$10
    .i16
    ldx _gsram_a
    sep #$10
    .i8
    lda $700000,x
    sta _gsram_d
    rts
.endproc
