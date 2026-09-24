; ============================================================================
; Star Merchants - bank-1 table byte reader
; ============================================================================
; _farbyt: leaf helper. With _gfar_o (16-bit) holding an offset from
; $018000, returns the ROM byte at $018000+_gfar_o in A. Used by C to
; read the FARRODATA tour tables (far_data.s). 8-bit A/X/Y on entry
; and exit (cc65 convention); X widened only for the long-indexed read.
; ============================================================================

.setcpu "65816"

.export _farbyt
.import _gfar_o

.segment "CODE"

.proc _farbyt: near
    rep #$10
    .i16
    ldx _gfar_o
    lda $018000,x
    sep #$10
    .i8
    rts
.endproc
