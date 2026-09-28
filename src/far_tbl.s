; ============================================================================
; Star Merchants - bank-1 table byte reader
; ============================================================================
; _farbyt: leaf helper. With _gfar_o (16-bit) holding an offset from
; $018000, returns the ROM byte at $018000+_gfar_o in A. Used by C to
; read the FARRODATA tour tables (far_data.s). 8-bit A/X/Y on entry
; and exit (cc65 convention); X widened only for the long-indexed read.
;
; _far_exec / _farenter: bank-0 -> bank-1 code trampoline (M8+). Bank-0
; ROM is full, so post-M7 logic lives in FARCODE (bank 1) and is entered
; via jsl (bank-0 C cannot emit jsl, and bank-1 C cannot jsr bank-0: jsr
; uses the current program bank). Chain:
;   C (bank 0) -> jsr _far_exec -> jsl _farenter -> jsr _m8dispatch (C,
;   same bank) -> rts -> rtl -> rts -> C caller.
; _gfar_fn selects the far function. Behaves exactly like a normal C
; call for the bank-0 caller (A/X clobbered, Y preserved by cc65
; convention; no stack use beyond the hardware return addresses).
; ============================================================================

.setcpu "65816"

.export _farbyt
.export _farbyt1
.export _far_exec
.export _farenter
.import _gfar_o
.import _m8dispatch

.segment "CODE"

.proc _farbyt: near
    rep #$10
    .i16
    ldx _gfar_o
    lda $018000,x
    sep #$10
    .i8
    ldx #$00
    rts
.endproc

.proc _far_exec: near
    jsl _farenter
    rts
.endproc

.segment "FARCODE"

.proc _farenter: near
    jsr _m8dispatch
    rtl
.endproc

; _farbyt1: bank-1 twin of _farbyt (identical body). Bank-1 C MUST call
; this, never _farbyt: jsr uses PBR, so a bank-1 jsr _farbyt would land
; in bank-1 zeros ($01E8xx) instead of bank-0 code ($00E8xx) -- BRK-sled
; wander, garbage reads, occasional hangs (root-caused 2026-09-28, M8).
.proc _farbyt1: near
    rep #$10
    .i16
    ldx _gfar_o
    lda $018000,x
    sep #$10
    .i8
    ldx #$00
    rts
.endproc