; Host-test reset stub (6502): init stacks, call _test_main, hang.
; The py65 driver stops when trep[15] (done flag) is set, or when the
; cycle budget expires (hang detection). _brk_end is never reached in
; practice; a BRK would vector through $FFFE (pointed at hang).

.setcpu "6502"

.export __STARTUP__
.import _test_main

.segment "STARTUP"

__STARTUP__:
    sei
    cld
    ldx #$FF
    txs
    jsr _test_main
hang:
    jmp hang

.segment "VECTORS"
    .word hang
    .word __STARTUP__
    .word hang
