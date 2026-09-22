; ============================================================================
; Star Merchants - SNES crt0 for ca65/ld65 (LoROM, bank 00)
; ============================================================================
; Reset enters 65816 emulation mode from the vector at $00:FFFC. We switch to
; native mode, set up the stacks, clear BSS and call the C main().
; All code/data must live in the first 32 KB (one LoROM bank).
;
; Segment placement is defined in scripts/snes-lorom.cfg:
;   STARTUP -> $00:8000 (file offset 0x0000)
;   HEADER  -> $00:FFC0 (file offset 0x7FC0)
;   VECTORS -> $00:FFE0 (file offset 0x7FE0)
; ============================================================================

.setcpu "65816"

.import _main
.import _probe
.import zerobss
.import __STACKSTART__
.importzp c_sp

; cc65-generated modules import __STARTUP__; exporting it from here makes
; the linker use our crt0 instead of pulling the generic none.lib startup.
.export __STARTUP__

; ---------------------------------------------------------------------------
; Internal cartridge header (32 bytes, $FFC0-$FFDF).
; The checksum/complement words are patched by scripts/build.ps1 after the
; ROM has been padded to its final size.
; ---------------------------------------------------------------------------
.segment "HEADER"
    .byte "STAR MERCHANTS"          ; Title (14 chars)
    .repeat 7                       ; pad title to 21 chars
        .byte $20
    .endrepeat
    .byte $20                       ; Map mode: LoROM, SlowROM
    .byte $00                       ; Cartridge type: ROM only
    .byte $08                       ; ROM size: 2 Mbit (256 KB)
    .byte $00                       ; RAM size: none
    .byte $01                       ; Destination: USA
    .byte $00                       ; Fixed value
    .byte $00                       ; ROM version
    .word $0000                     ; Checksum complement (patched)
    .word $0000                     ; Checksum (patched)

; ---------------------------------------------------------------------------
; Startup code
; ---------------------------------------------------------------------------
.segment "STARTUP"

__STARTUP__:
ResetHandler:
    sei                             ; Disable interrupts
    cld                             ; Clear decimal mode
    clc
    xce                             ; Switch to native mode (E=0)

    ; NOTE: D (direct page) is UNDEFINED at reset on real hardware.
    ; Set DP=$0000 FIRST, before any direct-page access (c_sp lives at
    ; $80/$81 and cc65 helpers use (DP) addressing everywhere). A bad DP
    ; silently breaks the C software stack: loop counters never terminate
    ; and function args read as garbage.
    rep #$30                        ; 16-bit A/X/Y
    .a16
    .i16

    lda #$0000
    tcd                             ; Direct page at $0000 (full 16-bit)

    ldx #$1FFF
    txs                             ; Hardware stack at $1FFF (page 1 WRAM)

    lda #__STACKSTART__
    sta c_sp                        ; cc65 software stack pointer ($0080)
                                    ; DP=0 guaranteed above, so this lands

    sep #$30                        ; 8-bit A/X/Y
    .a8
    .i8

    lda #$00
    pha
    plb                             ; Data bank register = $00

    stz $4200                       ; Disable NMI, IRQ and auto-joypad read

    jsr ClearRegisters              ; PPU/CPU registers to a known state
    jsr zerobss                     ; Clear C static storage

    jsr _main

@forever:
    bra @forever

; ---------------------------------------------------------------------------
; Interrupt handlers (NMI is enabled by main.c, everything else is unused)
; ---------------------------------------------------------------------------
NMI_Handler:
    rti

IRQ_Handler:
    rti

COP_Handler:
    rti

BRK_Handler:
    rti

ABORT_Handler:
    rti

; ---------------------------------------------------------------------------
; Clear $2100-$2133 (PPU) and $4200-$420D (CPU) registers to zero.
; Leaves the system in force-blank with all layers/sprites disabled.
; ---------------------------------------------------------------------------
ClearRegisters:
    ldx #$00
@ppu:
    stz $2100,x
    inx
    cpx #$34                        ; $2100-$2133
    bne @ppu
    ldx #$00
@cpu:
    stz $4200,x
    inx
    cpx #$0E                        ; $4200-$420D
    bne @cpu
    rts

; ---------------------------------------------------------------------------
; Vector table (32 bytes, $FFE0-$FFFF)
; ---------------------------------------------------------------------------
.segment "VECTORS"
    .word $0000                     ; $FFE0 reserved
    .word $0000                     ; $FFE2 reserved
    .word COP_Handler               ; $FFE4 native COP
    .word BRK_Handler               ; $FFE6 native BRK
    .word ABORT_Handler             ; $FFE8 native ABORT
    .word NMI_Handler               ; $FFEA native NMI
    .word ResetHandler              ; $FFEC native RESET (unused)
    .word IRQ_Handler               ; $FFEE native IRQ
    .word $0000                     ; $FFF0 reserved
    .word $0000                     ; $FFF2 reserved
    .word COP_Handler               ; $FFF4 emulation COP
    .word $0000                     ; $FFF6 reserved
    .word ABORT_Handler             ; $FFF8 emulation ABORT
    .word NMI_Handler               ; $FFFA emulation NMI
    .word ResetHandler              ; $FFFC emulation RESET
    .word IRQ_Handler               ; $FFFE emulation IRQ/BRK
