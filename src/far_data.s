; Self-drive tour 1-3 script tables (BANK1, FARRODATA segment).
; Too large for bank 0 ROM; read via the _farbyt leaf helper in
; far_tbl.s (16-bit X offset from $018000). C passes per-table
; offsets (FT_* in main.c); it never takes these symbols'
; addresses directly (16-bit refs would truncate the bank).
;
; FARRODATA starts at $018C00: right after the 3072-byte FARFONT
; ($018000-$018BFF). If the font size changes, the FT_* bases move.

.export _scf, _scp, _scf2, _scp2, _scf3, _scp3
.export _scf4, _scp4, _scf5, _scp5, _scf6, _scp6

.segment "FARRODATA"

_scf:
    .byte 3,4,9,10,15,16,21,22,27,28
    .byte 40,41,46,47,52,53,58,59,64,65
    .byte 70,71,76,77,82,83,88,89,94,95
    .byte 100,101,106,107,112,113,116,117,120,121
    .byte 124,125,128,129,132,133,136,137,140,141
    .byte 148,149,152,153,160,161,164,165,168,169
    .byte 176,177,180,181,184,185,188,189,196,197
    .byte 200,201,208,209,212,213,216,217,220,221
    .byte 224,225,228,229,232,233,236,237,240,241
    .byte 244,245

_scp:
    .byte 1,0,3,0,3,0,3,0,6,0
    .byte 7,0,2,0,2,0,6,0,7,0
    .byte 2,0,6,0,1,0,1,0,6,0
    .byte 3,0,2,0,6,0,8,0,3,0
    .byte 3,0,3,0,3,0,6,0,6,0
    .byte 3,0,6,0,2,0,5,0,6,0
    .byte 3,0,3,0,3,0,6,0,2,0
    .byte 6,0,3,0,3,0,6,0,2,0
    .byte 6,0,3,0,6,0,7,0,3,0
    .byte 6,0

_scf2:
    .byte 3,4,9,10,15,16,21,22,27,28
    .byte 40,41,46,47,52,53,58,59,64,65
    .byte 70,71,76,77,82,83,88,89,94,95
    .byte 100,101,106,107,112,113,116,117,120,121
    .byte 124,125,128,129,132,133,136,137,140,141
    .byte 144,145,148,149,152,153,156,157,160,161
    .byte 164,165,168,169,172,173,176,177,180,181
    .byte 184,185,188,189,192,193,196,197,200,201
    .byte 204,205,208,209,212,213,216,217,220,221
    .byte 224,225,228,229,232,233,236,237,240,241
    .byte 244,245,248,249,252,253

_scp2:
    .byte 1,0,3,0,3,0,3,0,6,0
    .byte 7,0,2,0,2,0,6,0,7,0
    .byte 2,0,6,0,1,0,1,0,6,0
    .byte 3,0,2,0,6,0,8,0,3,0
    .byte 3,0,3,0,3,0,6,0,3,0
    .byte 6,0,6,0,2,0,6,0,3,0
    .byte 6,0,6,0,2,0,6,0,3,0
    .byte 6,0,6,0,3,0,6,0,3,0
    .byte 6,0,3,0,6,0,6,0,3,0
    .byte 6,0,3,0,6,0,3,0,6,0
    .byte 3,0,6,0,6,0

_scf3:
    .byte 3,4,9,10,15,16,21,22,27,28
    .byte 40,41,46,47,52,53,58,59,64,65
    .byte 70,71,76,77,82,83,88,89,94,95
    .byte 100,101,106,107,112,113,116,117,120,121
    .byte 124,125,128,129,132,133,136,137,140,141
    .byte 144,145,148,149,152,153,156,157,160,161
    .byte 164,165,168,169,172,173,176,177,180,181
    .byte 184,185,188,189,192,193,196,197,200,201
    .byte 204,205,208,209,212,213,216,217,220,221
    .byte 224,225

_scp3:
    .byte 1,0,3,0,3,0,3,0,6,0
    .byte 7,0,2,0,2,0,6,0,7,0
    .byte 2,0,6,0,1,0,1,0,6,0
    .byte 3,0,2,0,6,0,8,0,3,0
    .byte 3,0,3,0,3,0,3,0,6,0
    .byte 6,0,3,0,6,0,3,0,6,0
    .byte 3,0,3,0,3,0,6,0,3,0
    .byte 3,0,6,0,3,0,6,0,8,0
    .byte 3,0,3,0,3,0,3,0,3,0
    .byte 6,0

; Tour 4 (gselfdrive=4): scf4[84], scp4[84]
; Combat demo: new game -> Stardock hardware -> photon buy -> leave ->
; warp sector 3 (hostile: 11 fighters, class 13 THO SEN) -> command
; palette -> ATTACK -> photon (blind) -> attack (victory, bounty 1800)
; -> leave. 37 presses + 5 release pairs; frames are (press,release)
; pairs 3 apart so the whole demo fits in ~2100 ticks.
_scf4:
    .byte 3,4,6,7,9,10,12,13,15,16
    .byte 18,19,21,22,24,25,27,28,30,31
    .byte 33,34,36,37,39,40,42,43,45,46
    .byte 48,49,51,52,54,55,57,58,60,61
    .byte 63,64,66,67,69,70,72,73,75,76
    .byte 78,79,81,82,84,85,87,88,90,91
    .byte 93,94,96,97,99,100,102,103,105,106
    .byte 108,109,111,112,114,115,117,118,120,121
    .byte 123,124,126,127

_scp4:
    .byte 1,0,6,0,1,0,1,0,6,0
    .byte 6,0,8,0,3,0,3,0,3,0
    .byte 3,0,6,0,3,0,3,0,6,0
    .byte 3,0,3,0,3,0,6,0,3,0
    .byte 6,0,7,0,3,0,6,0,8,0
    .byte 3,0,3,0,3,0,3,0,3,0
    .byte 3,0,6,0,3,0,6,0,2,0
    .byte 6,0,6,0,0,0,0,0,0,0
    .byte 0,0,0,0

; Tour 5 (gselfdrive=5): scf5[4], scp5[4]
_scf5:
    .byte 2,3,8,9

_scp5:
    .byte 6,0,6,0

; Tour 6 (gselfdrive=6): scf6[14], scp6[14]
_scf6:
    .byte 2,3,4,5,8,9,12,13,14,15
    .byte 18,19,24,25

_scp6:
    .byte 3,0,3,0,6,0,2,0,2,0
    .byte 6,0,6,0

; ============================================================================
; Milestone 8: Ferrengi + alien tables (offsets FT_M8* in main.c, bytes read
; via farbyt() only -- never address these symbols from C).
; Layout from FT_M8 (all NUL-terminated, zero-padded to stride):
;   +0   ship names 3x8   ("ASS TRA","BAT CRU","DREADNT")
;   +24  msg lines 5x24   (PAID,UNPAID,STAND,DENY,HINT -- see FT_M8M*)
;   +144 home name 12     ("FERRENGAL")
;   +156 alien names 6x12 (VORLON/ASGARD/ELDARI good, KRULL/MORGU/XARTH evil)
;   +228 alien shifts 6x2 (align+128, xp; good kills cost alignment)
; Total 240 bytes.
; ============================================================================
    .byte "ASS TRA", 0
    .byte "BAT CRU", 0
    .byte "DREADNT", 0
    .byte "TRIBUTE PAID 500CR", 0, 0, 0, 0, 0, 0
    .byte "TRIBUTE UNPAID: FIGHT!", 0, 0
    .byte "FERRENGI STAND DOWN", 0, 0, 0, 0, 0
    .byte "FERRENGAL DEFENDED", 0, 0, 0, 0, 0, 0
    .byte "RETURN WITH 3000 FTRS", 0, 0, 0
    .byte "FERRENGAL", 0, 0, 0
    .byte "VORLON", 0, 0, 0, 0, 0, 0
    .byte "ASGARD", 0, 0, 0, 0, 0, 0
    .byte "ELDARI", 0, 0, 0, 0, 0, 0
    .byte "KRULL", 0, 0, 0, 0, 0, 0, 0
    .byte "MORGU", 0, 0, 0, 0, 0, 0, 0
    .byte "XARTH", 0, 0, 0, 0, 0, 0, 0
    .byte 118, 12, 120, 10, 116, 15, 138, 12, 136, 10, 140, 15
; Fight-entry lines for every battle (bank-0 keeps no literals here):
; +240 normal ENGAGING, +264 normal PREPARE.
    .byte "ENGAGING ENEMY!", 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "PREPARE FOR BATTLE", 0, 0, 0, 0, 0, 0