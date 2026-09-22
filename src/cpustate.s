; CPU-state probe helpers (cc65 callable, 8-bit A/X/Y convention).
; _read_d: stores D (direct page register) to WRAM $0200/$0201 (lo/hi).
; _read_csp: stores ZP $80/$81 (c_sp, valid iff DP=0) to $0202/$0203.

.setcpu "65816"

.export _read_d
.export _read_csp
.export _probe

.segment "CODE"

.a8
.i8

; Definitive no-stack probe: reads D and c_sp, paints the answer.
; GREEN = DP==0 and c_sp==$1FFF. RED = DP==0, c_sp wrong. BLUE = DP!=0.
; Uses NO software stack (no locals, no helpers) — only HW stack for JSR/RTS.
.proc _probe: near
    rep #$20
    .a16
    tdc
    sta $0200               ; save D (16-bit store)
    sep #$20
    .a8
    phb
    pla
    sta $0204               ; save B (data bank)
    php
    pla
    sta $0205               ; save P (M/X flags live here)
    lda $80
    sta $0202               ; save c_sp lo (direct page)
    lda $81
    sta $0203               ; save c_sp hi
    lda $80
    sta $0202               ; save c_sp lo (direct page)
    lda $81
    sta $0203               ; save c_sp hi

    lda #$80
    sta $2100               ; force blank
    stz $2121               ; CGADD = 0

    lda $0200
    ora $0201
    bne isblue              ; D != 0 -> BLUE
    lda $0204
    bne ismagenta           ; B != 0 -> MAGENTA
    lda $0202
    cmp #$FF
    bne isred
    lda $0203
    cmp #$1F
    bne isred
    lda $0205
    and #$30                ; M (bit5) + X (bit4) must both be 1 (8-bit)
    cmp #$30
    bne iswhite             ; M/X wrong -> WHITE
    lda #$E0                ; GREEN: perfect
    sta $2122
    lda #$03
    sta $2122
    bra donec
iswhite:
    lda #$FF                ; WHITE: M/X flags wrong (16-bit mode!)
    sta $2122
    lda #$7F
    sta $2122
    bra donec
ismagenta:
    lda #$1F                ; MAGENTA ($7C1F): data bank wrong
    sta $2122
    lda #$7C
    sta $2122
    bra donec
isred:
    lda #$1F                ; RED: c_sp wrong
    sta $2122
    lda #$00
    sta $2122
    bra donec
isblue:
    lda #$00                ; BLUE: DP wrong
    sta $2122
    lda #$7C
    sta $2122
donec:
    lda #$FF
    sta $2122
    lda #$7F
    sta $2122
    lda #$0F
    sta $2100               ; screen on, full bright
forever:
    bra forever
.endproc

.proc _read_d: near
    rep #$20                ; 16-bit A for a clean 16-bit store
    .a16
    tdc                 ; C = D (16-bit transfer regardless of M flag)
    sta $0200           ; stores D low -> $0200, D high -> $0201
    sep #$20                ; back to cc65 8-bit convention
    .a8
    rts
.endproc

.proc _read_csp: near
    lda $80             ; c_sp low (direct-page addressed)
    sta $0202
    lda $81             ; c_sp high
    sta $0203
    rts
.endproc
