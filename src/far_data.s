; Self-drive tour 1-3 script tables (BANK1, FARRODATA segment).
; Too large for bank 0 ROM; read via the _farbyt leaf helper in
; far_tbl.s (16-bit X offset from $018000). C passes per-table
; offsets (FT_* in main.c); it never takes these symbols'
; addresses directly (16-bit refs would truncate the bank).
;
; FARRODATA starts at $018C00: right after the 3072-byte FARFONT
; ($018000-$018BFF). If the font size changes, the FT_* bases move.

.export _scf, _scp, _scf2, _scp2, _scf3, _scp3
.export _scf4, _scp4, _scf5, _scp5, _scf6, _scp6, _scf7, _scp7

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

; Tour 4 (gselfdrive=4): scf4[88], scp4[88]
; Combat demo: new game -> Stardock hardware -> photon buy -> leave ->
; warp sector 3 (hostile: 11 fighters, class 13 THO SEN) -> command
; palette -> ATTACK -> photon (blind) -> attack (victory, bounty 1800)
; -> leave. 39 presses; frames are (press,release) pairs 3 apart so the
; whole demo fits in ~2100 ticks. M9: hardware dept grew 5->7 rows
; (BACK 4->6), so two extra DOWNs precede the BACK press; tail frames
; shift +4 (all <256, still 8-bit safe).
_scf4:
    .byte 3,4,6,7,9,10,12,13,15,16
    .byte 18,19,21,22,24,25,27,28,30,31
    .byte 33,34,36,37,39,40,42,43,45,46
    .byte 48,49,51,52,54,55,57,58,60,61
    .byte 62,63,64,65,67,68,70,71,73,74
    .byte 76,77,79,80,82,83,85,86,88,89
    .byte 91,92,94,95,97,98,100,101,103,104
    .byte 106,107,109,110,112,113,115,116,118,119
    .byte 121,122,124,125,127,128,130,131
; scf4 check: 40 head + 4 inserted (62-65) + 44 tail (old 63-127 shifted +4)

_scp4:
    .byte 1,0,6,0,1,0,1,0,6,0
    .byte 6,0,8,0,3,0,3,0,3,0
    .byte 3,0,6,0,3,0,3,0,6,0
    .byte 3,0,3,0,3,0,6,0,3,0
    .byte 3,0,3,0,6,0,7,0,3,0
    .byte 6,0,8,0,3,0,3,0,3,0
    .byte 3,0,3,0,3,0,6,0,3,0
    .byte 6,0,2,0,6,0,6,0,0,0
    .byte 0,0,0,0,0,0,0,0
; scp4 check: 40 head + (DOWN,0,DOWN,0) + old tail 6,0,7,0... unchanged

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

; ============================================================================
; Milestone 9: Corporation tables (offsets FT_M9* in main.c, farbyt only).
; Layout from FT_M9 (NUL-terminated, zero-padded to stride):
;   +0   hull names 4x12  ("SCOUT","MERCHANT","FREIGHTER","DREAD")
;   +48  hull stats 4x4   (holdsmax, ftrmax, price-lo, price-hi;
;   prices are data: SCOUT 0, MERCHANT 5000, FREIGHTER 5000, DREAD 30000)
;   +64  dept rows 2x24   (CHARTER row, EXCHANGE row)
;   +112 corp msgs 8x24   (M0..M7, see FT_M9M*)
; Total 304 bytes. Megaholds bug NOT replicated: holds are capped.
; ============================================================================
    .byte "SCOUT", 0, 0, 0, 0, 0, 0, 0
    .byte "MERCHANT", 0, 0, 0, 0
    .byte "FREIGHTER", 0, 0, 0
    .byte "DREAD", 0, 0, 0, 0, 0, 0, 0
    .byte 10, 15, 0, 0
    .byte 20, 30, 136, 19
    .byte 40, 50, 136, 19
    .byte 60, 80, 48, 117
    .byte "CORP CHARTER 5000", 0, 0, 0, 0, 0, 0, 0
    .byte "EXCHANGE SHIP", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "BUY PROBE 500", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "BUY BEACON 100", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "BUY GENESIS 5000", 0, 0, 0, 0, 0, 0, 0, 0
    .byte "BUY PHOTON 500", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "CORP CHARTERED!", 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "NEED 5000 CREDITS", 0, 0, 0, 0, 0, 0, 0
    .byte "CORP FLEET ACTIVE", 0, 0, 0, 0, 0, 0, 0
    .byte "HULL SWAPPED", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "CANT AFFORD HULL", 0, 0, 0, 0, 0, 0, 0, 0
    .byte "CARGO WONT FIT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    .byte "OVERFLOW TO POOL", 0, 0, 0, 0, 0, 0, 0, 0
    .byte "POOL TAPPED", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

; Tour 7 (gselfdrive=7): scf7[44], scp7[44]
; Corp demo: new game -> Stardock hardware -> CHARTER (5000cr) ->
; EXCHANGE (FREIGHTER 5000cr) -> end in dept 2 on HULL SWAPPED.
; Head (30) reuses tour-4 verified path; corp tail: DOWNx4,A,DOWN,A.
_scf7:
    .byte 3,4,6,7,9,10,12,13,15,16
    .byte 18,19,21,22,24,25,27,28,30,31
    .byte 33,34,36,37,39,40,42,43,45,46
    .byte 48,49,51,52,54,55,57,58,60,61
    .byte 63,64,66,67

_scp7:
    .byte 1,0,6,0,1,0,1,0,6,0
    .byte 6,0,8,0,3,0,3,0,3,0
    .byte 3,0,6,0,3,0,3,0,6,0
    .byte 3,0,3,0,3,0,3,0,6,0
    .byte 3,0,6,0
