/* Star Merchants - bank-1 far code (Milestone 8+).
 *
 * Bank-0 ROM is FULL (12B spare as of M7), so all post-M7 game logic lives
 * here in FARCODE (LoROM bank 1, $018000+), entered from bank-0 C only via
 * the _far_exec jsl trampoline (src/far_tbl.s) with the function id in
 * gfar_fn. m8dispatch() fans out with an if-chain (never switch: cc65
 * emits rodata jump tables for switch, unaddressable from bank 1).
 *
 * HARD RULES for this file (enforced by gates in scripts/build.ps1):
 * - globals + parameterless functions + inline code ONLY. No C locals,
 *   no C function arguments, ever (same as main.c: stack-frame helpers
 *   hang the CPU; TOS multiply/divide helpers are banned too -- stage
 *   multi-step math through globals so every intermediate stays in AX).
 * - NO string literals ("...") and NO const arrays: with --rodata-name
 *   FARRODATA they would land in bank 1 while 16-bit references assume
 *   the current data bank (DBR=0 always). All ROM data (tables, message
 *   text) lives in src/far_data.s and is read byte-wise via farbyt()
 *   into BSS buffers (e.g. gfarmsg) that bank-0 draw code can address.
 * - NO calls to bank-0 C functions (draw_text, show_*, sram_sync...):
 *   jsr uses the current program bank, so from bank 1 it would land in
 *   bank 1, not bank 0. Far code computes STATE into BSS globals; the
 *   bank-0 caller renders/saves after far_exec returns. The only
 *   cross-boundary calls allowed are farbyt1() (the bank-1 twin of the
 *   farbyt leaf in far_tbl.s -- NEVER farbyt() itself: a bank-1 jsr to
 *   the bank-0 leaf lands in bank-1 zeros and wanders a BRK-sled) and
 *   functions defined in this file (same-bank jsr is fine).
 * - NO initialized globals (crt0 only zerobss; DATA is never copied --
 *   same as main.c). Zero-init + runtime init only.
 */
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef short int16_t;
/* Far offset of the M8 tables from $018000: FT_BASE (3072, after FARFONT)
 * + 792 tour bytes. Must match FT_M8 in main.c. */
#define M8B 3864u
/* Far offset of the M9 tables: M8B + 288 M8 bytes. Must match FT_M9. */
#define M9B 4152u
/* M10 hub rows: M9B + 400 M9 bytes + 88 tour-7 + 112 tour-8.
 * Must match FT_HUB in main.c. Seed labels follow at +192/+216/+240,
 * glyphs at +264. */
#define HUBB 4752u
#define SEEDB 4944u
#define SEEDG 5016u
/* M10 dept rows: SEEDG + 16 glyph bytes. Must match DPT base. */
#define DPTB 5032u
/* M10 dock footers: DPTB + 360 dept bytes, 28-byte stride. */
#define FOOTB 5392u

/* Shared game state owned by main.c */
extern uint8_t gfar_fn;
extern uint16_t gfar_o;
extern uint16_t gsec;
extern uint16_t guniv;
extern uint16_t gfighters;
extern uint16_t gshields;
extern uint16_t gcredits;
extern int16_t galign;
extern uint16_t gxp;
extern uint16_t gfersec;
extern uint8_t gfer;
extern uint8_t gferd;
extern uint8_t gferpal;
extern uint8_t gcorpk;
extern uint8_t gpool;
extern uint8_t gshipcls;
extern uint16_t gholds;
extern uint16_t gholdmax;
extern uint8_t gdocksel;
extern uint8_t gdept;
extern uint8_t ggrudpk;
extern uint16_t gfertreas;
extern uint8_t gfergrd;
extern uint8_t galien;
extern uint8_t gftrs;
extern uint8_t gport;
extern uint8_t gplanet;
extern uint8_t gplevel;
extern uint8_t gencls;
extern uint16_t genftrs;
extern uint16_t gensh;
extern uint8_t gencorb;
extern uint8_t gblind;
extern uint16_t gday;
extern uint16_t ghash;
extern uint8_t gi;
extern uint8_t gk;
extern uint16_t gn;
extern uint16_t gom;
extern uint8_t goc;
extern uint16_t gtmp;
extern uint8_t gact;
extern uint8_t gsclk;
extern uint16_t gsfr;
extern uint16_t gunivseed;
extern uint8_t gseedct;
extern uint8_t gseedbtn;
extern uint8_t gseedbt2;
uint8_t farbyt1(void);

/* M8-owned BSS: far text line buffers (bank-0 draw code renders from
 * here) and a proof magic. gfarmsg = encounter ship name (persists
 * across fight redraws); gfarm2 = tribute/alien/deny line; gfarm3 =
 * home sector name (filled at sector gen); gfar4-7 = Stardock hardware
 * rows 0-3 (refilled at every hub entry); gfar8/9 = corp message lines
 * (M9 charter/exchange results -- MUST be separate buffers: writing
 * results into gfarmsg/gfarm2 would clobber the CHARTER/EXCHANGE menu
 * rows, screenshot-proven 2026-09-28). 32 cols + NUL each.
 * gfar10-17 = Stardock hub dept-0 rows (refilled at every hub entry).
 * gseedcells/gseedbar = universe-seed mash cells + weave bar. */
char gfarmsg[33];
char gfarm2[33];
char gfarm3[33];
char gfar4[33];
char gfar5[33];
char gfar6[33];
char gfar7[33];
char gfar8[33];
char gfar9[33];
char gfar10[33];
char gfar11[33];
char gfar12[33];
char gfar13[33];
char gfar14[33];
char gfar15[33];
char gfar16[33];
char gfar17[33];
char gseedcells[17];
char gseedbar[17];
char gfar18[33];
char gfar19[33];
char gfar20[33];
char gfar21[33];
char gfar22[33];
char gfar23[33];
char gfar24[33];
char gfar25[33];
char gfar26[33];
char gfar27[33];
char gfar28[33];
char gfar29[33];
char gfar30[33];
char gfar31[33];
char gfar32[33];
char gfootA[33];
char gfootB[33];
uint8_t gfarmagic;

void m8_hello(void);
void m8cpy(void);
void m8ferset(void);
void m8enc(void);
void m8alien(void);
void m8gen(void);
void m8deny(void);
void m9charter(void);
void m9exchange(void);
void m9rows(void);
void m10mash(void);
void m8dispatch(void);

void m8_hello(void) {
    /* Proof function (fn 1): build the proof line from immediates only. */
    gfarmsg[0] = 'F';
    gfarmsg[1] = 'A';
    gfarmsg[2] = 'R';
    gfarmsg[3] = '-';
    gfarmsg[4] = 'O';
    gfarmsg[5] = 'K';
    gfarmsg[6] = 0u;
    gfarmagic = 0xA5u;
}

void m8cpy(void) {
    gi = 0u;
    while (gi < 32u) {
        gact = farbyt1();
        if (goc == 0u) {
            gfarmsg[gi] = gact;
        } else if (goc == 1u) {
            gfarm2[gi] = gact;
        } else if (goc == 2u) {
            gfarm3[gi] = gact;
        } else if (goc == 3u) {
            gfar4[gi] = gact;
        } else if (goc == 4u) {
            gfar5[gi] = gact;
        } else if (goc == 5u) {
            gfar6[gi] = gact;
        } else if (goc == 6u) {
            gfar7[gi] = gact;
        } else if (goc == 7u) {
            gfar8[gi] = gact;
        } else if (goc == 8u) {
            gfar9[gi] = gact;
        } else if (goc == 9u) {
            gfar10[gi] = gact;
        } else if (goc == 10u) {
            gfar11[gi] = gact;
        } else if (goc == 11u) {
            gfar12[gi] = gact;
        } else if (goc == 12u) {
            gfar13[gi] = gact;
        } else if (goc == 13u) {
            gfar14[gi] = gact;
        } else if (goc == 14u) {
            gfar15[gi] = gact;
        } else if (goc == 15u) {
            gfar16[gi] = gact;
        } else if (goc == 16u) {
            gfar17[gi] = gact;
        } else if (goc == 17u) {
            gfar18[gi] = gact;
        } else if (goc == 18u) {
            gfar19[gi] = gact;
        } else if (goc == 19u) {
            gfar20[gi] = gact;
        } else if (goc == 20u) {
            gfar21[gi] = gact;
        } else if (goc == 21u) {
            gfar22[gi] = gact;
        } else if (goc == 22u) {
            gfar23[gi] = gact;
        } else if (goc == 23u) {
            gfar24[gi] = gact;
        } else if (goc == 24u) {
            gfar25[gi] = gact;
        } else if (goc == 25u) {
            gfar26[gi] = gact;
        } else if (goc == 26u) {
            gfar27[gi] = gact;
        } else if (goc == 27u) {
            gfar28[gi] = gact;
        } else if (goc == 28u) {
            gfar29[gi] = gact;
        } else if (goc == 29u) {
            gfar30[gi] = gact;
        } else if (goc == 30u) {
            gfar31[gi] = gact;
        } else if (goc == 31u) {
            gfar32[gi] = gact;
        } else if (goc == 32u) {
            gfootA[gi] = gact;
        } else {
            gfootB[gi] = gact;
        }
        if (gact == 0u) {
            return;
        }
        gi++;
        gfar_o++;
    }
    if (goc == 0u) {
        gfarmsg[32] = 0u;
    } else if (goc == 1u) {
        gfarm2[32] = 0u;
    } else if (goc == 2u) {
        gfarm3[32] = 0u;
    } else if (goc == 3u) {
        gfar4[32] = 0u;
    } else if (goc == 4u) {
        gfar5[32] = 0u;
    } else if (goc == 5u) {
        gfar6[32] = 0u;
    } else if (goc == 6u) {
        gfar7[32] = 0u;
    } else if (goc == 7u) {
        gfar8[32] = 0u;
    } else if (goc == 8u) {
        gfar9[32] = 0u;
    } else if (goc == 9u) {
        gfar10[32] = 0u;
    } else if (goc == 10u) {
        gfar11[32] = 0u;
    } else if (goc == 11u) {
        gfar12[32] = 0u;
    } else if (goc == 12u) {
        gfar13[32] = 0u;
    } else if (goc == 13u) {
        gfar14[32] = 0u;
    } else if (goc == 14u) {
        gfar15[32] = 0u;
    } else if (goc == 15u) {
        gfar16[32] = 0u;
    } else if (goc == 16u) {
        gfar17[32] = 0u;
    } else if (goc == 17u) {
        gfar18[32] = 0u;
    } else if (goc == 18u) {
        gfar19[32] = 0u;
    } else if (goc == 19u) {
        gfar20[32] = 0u;
    } else if (goc == 20u) {
        gfar21[32] = 0u;
    } else if (goc == 21u) {
        gfar22[32] = 0u;
    } else if (goc == 22u) {
        gfar23[32] = 0u;
    } else if (goc == 23u) {
        gfar24[32] = 0u;
    } else if (goc == 24u) {
        gfar25[32] = 0u;
    } else if (goc == 25u) {
        gfar26[32] = 0u;
    } else if (goc == 26u) {
        gfar27[32] = 0u;
    } else if (goc == 27u) {
        gfar28[32] = 0u;
    } else if (goc == 28u) {
        gfar29[32] = 0u;
    } else if (goc == 29u) {
        gfar30[32] = 0u;
    } else if (goc == 30u) {
        gfar31[32] = 0u;
    } else if (goc == 31u) {
        gfar32[32] = 0u;
    } else if (goc == 32u) {
        gfootA[32] = 0u;
    } else {
        gfootB[32] = 0u;
    }
}

void m8ferset(void) {
    /* NOTE: all multi-step math below is written with inline-only ops
     * (uint8 shifts, uint16 +=/-=/&/compare). 16-bit <<, >>, ++, *,
     * / would emit none.lib helpers taking bank-0 addresses that a
     * bank-1 jsr cannot reach (root-caused 2026-09-28: BRK-sled). */
    gencls = gk;
    gencls += 15u;
    gom = gk;
    gom += gom;
    gom += gom;
    gom += gom;
    genftrs = gom;
    gom = gk;
    gom += gom;
    genftrs += gom;
    genftrs += 20u;
    if (gk == 0u) {
        goc = (uint8_t)(ggrudpk & 3u);
    } else if (gk == 1u) {
        goc = (uint8_t)((ggrudpk >> 2) & 3u);
    } else {
        goc = (uint8_t)((ggrudpk >> 4) & 3u);
    }
    gom = goc;
    gom += gom;
    gom += gom;
    genftrs += gom;
    genftrs += goc;
    gensh = 30u;
    gom = gk;
    gom += gom;
    gom += gom;
    gom += gom;
    gensh += gom;
    gom = gk;
    gom += gom;
    gensh += gom;
    gencorb = 0u;
    gblind = 0u;
    gom = gk;
    gom += gom;
    gom += gom;
    gom += gom;
    gfar_o = M8B;
    gfar_o += gom;
    goc = 0u;
    m8cpy();
    if (gfighters <= 1u && gshields >= 50u) {
        gferd = 2u;
        gfer = 0u;
        gfar_o = 72u + M8B;
        goc = 1u;
        m8cpy();
        return;
    }
    if (gcredits >= 500u) {
        gcredits -= 500u;
        if (gk == 0u) {
            ggrudpk = (uint8_t)(ggrudpk & 252u);
        } else if (gk == 1u) {
            ggrudpk = (uint8_t)(ggrudpk & 243u);
        } else {
            ggrudpk = (uint8_t)(ggrudpk & 207u);
        }
        gferd = 2u;
        gfer = 0u;
        gfar_o = 24u + M8B;
        goc = 1u;
        m8cpy();
        return;
    }
    gferd = 1u;
    gfer = 1u;
    if (goc < 3u) {
        goc++;
        if (gk == 0u) {
            ggrudpk = (uint8_t)((ggrudpk & 252u) | goc);
        } else if (gk == 1u) {
            ggrudpk = (uint8_t)((ggrudpk & 243u) | (goc << 2));
        } else {
            ggrudpk = (uint8_t)((ggrudpk & 207u) | (goc << 4));
        }
        genftrs += 5u;
    }
    gfar_o = 48u + M8B;
    goc = 1u;
    m8cpy();
}

void m8enc(void) {
    gfer = 0u;
    gferd = 0u;
    gferpal = 2u;
    gfar_o = M8B + 240u;
    goc = 0u;
    m8cpy();
    gfar_o = M8B + 264u;
    goc = 1u;
    m8cpy();
    if (gsec == gfersec) {
        goc = 0u;
        gtmp = gsec;
        while (gtmp >= 16u) {
            gtmp -= 16u;
            goc++;
        }
        goc &= 3u;
        gk = goc;
        if (gk > 2u) {
            gk = 2u;
        }
        m8ferset();
        return;
    }
    if (gftrs == 0u) {
        return;
    }
    ghash = gsec;
    gom = ghash;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    ghash ^= gom;
    gom = ghash;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    ghash += gom;
    gom = ghash;
    gom += gom;
    gom += gom;
    gom += gom;
    ghash ^= gom;
    goc = 0u;
    gtmp = ghash;
    while (gtmp >= 32u) {
        gtmp -= 32u;
        goc++;
    }
    goc &= 7u;
    if (goc >= 2u) {
        return;
    }
    goc = 0u;
    gtmp = ghash;
    while (gtmp >= 1024u) {
        gtmp -= 1024u;
        goc++;
    }
    goc &= 3u;
    gk = goc;
    if (gk > 2u) {
        gk = 2u;
    }
    m8ferset();
}

void m8alien(void) {
    galien = 0u;
    gfarm2[0] = 0u;
    goc = 0u;
    gtmp = ghash;
    while (gtmp >= 16u) {
        gtmp -= 16u;
        goc++;
    }
    goc &= 7u;
    if (goc != 7u) {
        return;
    }
    gk = (uint8_t)(ghash & 7u);
    if (gk > 5u) {
        gk = 5u;
    }
    galien = (uint8_t)(gk + 1u);
    gtmp = gk;
    gtmp += gtmp;
    gfar_o = M8B + 228u;
    gfar_o += gtmp;
    goc = farbyt1();
    gfar_o++;
    gtmp = farbyt1();
    gxp += gtmp;
    if (goc >= 128u) {
        gom = goc;
        gom -= 128u;
        galign += gom;
    } else {
        gom = (uint16_t)(128u - goc);
        galign -= gom;
    }
    if (galign > 999) {
        galign = 999;
    }
    if (galign < -999) {
        galign = -999;
    }
    gom = gk;
    gom += gom;
    gom += gom;
    gom += gom;
    gtmp = gom;
    gom = gk;
    gom += gom;
    gom += gom;
    gtmp += gom;
    gfar_o = M8B + 156u;
    gfar_o += gtmp;
    goc = 1u;
    m8cpy();
    if (gk < 3u) {
        gfarm2[gi] = ' ';
        gi++;
        gfarm2[gi] = 'G';
        gi++;
        gfarm2[gi] = 'O';
        gi++;
        gfarm2[gi] = 'O';
        gi++;
        gfarm2[gi] = 'D';
        gi++;
    } else {
        gfarm2[gi] = ' ';
        gi++;
        gfarm2[gi] = 'E';
        gi++;
        gfarm2[gi] = 'V';
        gi++;
        gfarm2[gi] = 'I';
        gi++;
        gfarm2[gi] = 'L';
        gi++;
    }
    gfarm2[gi] = ' ';
    gi++;
    gfarm2[gi] = 'S';
    gi++;
    gfarm2[gi] = 'L';
    gi++;
    gfarm2[gi] = 'A';
    gi++;
    gfarm2[gi] = 'I';
    gi++;
    gfarm2[gi] = 'N';
    gi++;
    gfarm2[gi] = 0u;
}

void m8gen(void) {
    /* Ferrengal = rim sector (universe edge): deterministic, reachable,
     * no division needed (see header rules on bank-1 helper calls). */
    gfersec = guniv;
    if (gfersec < 2u) {
        gfersec = 2u;
    }
    gfer = 0u;
    gferd = 0u;
    if (gsec != gfersec) {
        return;
    }
    gport = 0u;
    gplanet = 1u;
    gplevel = 4u;
    gfergrd = 200u;
    gom = (uint16_t)(gday & 63u);
    gfergrd += (uint8_t)gom;
    if (gfergrd > 250u) {
        gfergrd = 250u;
    }
    if (ggrudpk & 64u) {
        gfertreas = 0u;
    } else {
        gfertreas = 40000u;
        gom = gday;
        if (gom > 200u) {
            gom = 200u;
        }
        gtmp = gom;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gfertreas += gtmp;
        gtmp = gom;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gtmp += gtmp;
        gfertreas += gtmp;
        gtmp = gom;
        gtmp += gtmp;
        gtmp += gtmp;
        gfertreas += gtmp;
        if (gfertreas > 60000u) {
            gfertreas = 60000u;
        }
    }
    if (gfighters >= 3000u) {
        gcredits += gfertreas;
        if (gcredits > 60000u) {
            gcredits = 60000u;
        }
        gxp += 50u;
        galign -= 25;
        if (galign < -999) {
            galign = -999;
        }
        ggrudpk |= 64u;
        gfergrd = 0u;
        gftrs = 0u;
    } else {
        gftrs = gfergrd;
    }
    gfar_o = M8B + 144u;
    goc = 2u;
    m8cpy();
}

void m8deny(void) {
    gfar_o = M8B + 96u;
    goc = 0u;
    m8cpy();
    gfar_o = M8B + 120u;
    goc = 1u;
    m8cpy();
}

void m9charter(void) {
    if (gcorpk & 1u) {
        gfar_o = M9B + 256u;
        goc = 7u;
        m8cpy();
        gom = gshipcls;
        gom += gom;
        gom += gom;
        gtmp = gom;
        gom += gom;
        gtmp += gom;
        gfar_o = M9B;
        gfar_o += gtmp;
        goc = 8u;
        m8cpy();
        return;
    }
    if (gcredits < 5000u) {
        gfar_o = M9B + 232u;
        goc = 7u;
        m8cpy();
        gfar9[0] = 0u;
        return;
    }
    gcredits -= 5000u;
    gcorpk = 1u;
    gom = gshipcls;
    gom += gom;
    gcorpk += gom;
    gpool = 0u;
    gfar_o = M9B + 208u;
    goc = 7u;
    m8cpy();
    gom = gshipcls;
    gom += gom;
    gom += gom;
    gtmp = gom;
    gom += gom;
    gtmp += gom;
    gfar_o = M9B;
    gfar_o += gtmp;
    goc = 8u;
    m8cpy();
}

void m9exchange(void) {
    goc = gshipcls;
    goc++;
    if (goc >= 4u) {
        goc = 0u;
    }
    gk = goc;
    gom = gk;
    gom += gom;
    gom += gom;
    gtmp = gom;
    gom += gom;
    gtmp += gom;
    gfar_o = M9B;
    gfar_o += gtmp;
    goc = 7u;
    m8cpy();
    gom = gk;
    gom += gom;
    gom += gom;
    gtmp = gom;
    gfar_o = M9B + 48u;
    gfar_o += gtmp;
    gact = farbyt1();
    gn = gact;
    gfar_o++;
    gact = farbyt1();
    gtmp = gact;
    gfar_o++;
    gact = farbyt1();
    goc = gact;
    gfar_o++;
    gact = farbyt1();
    gi = gact;
    gom = gi;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += gom;
    gom += goc;
    if (gholds > gn) {
        gfar_o = M9B + 328u;
        goc = 8u;
        m8cpy();
        return;
    }
    if (gom > 0u) {
        if (gcredits < gom) {
            gfar_o = M9B + 304u;
            goc = 8u;
            m8cpy();
            return;
        }
        gcredits -= gom;
    }
    gholdmax = gn;
    gom = gfighters;
    gom += gpool;
    if (gom > gtmp) {
        gom -= gtmp;
        if (gom > 255u) {
            gom = 255u;
        }
        gpool = gom;
        gfighters = gtmp;
        gfar_o = M9B + 352u;
        goc = 8u;
        m8cpy();
    } else {
        /* Fit branch: ftrmax (gtmp) is dead from here, so reuse it for
         * the pool-tapped flag. It must NOT live in gi: m8cpy uses
         * gi/gact/gfar_o as cursor/byte/addr and would clobber it
         * during the HULL write below. Pool still holds entry value. */
        gtmp = 0u;
        if (gpool > 0u) {
            gtmp = 1u;
        }
        gfighters = gom;
        gpool = 0u;
        gfar_o = M9B + 280u;
        goc = 8u;
        m8cpy();
        if (gtmp) {
            gfar_o = M9B + 376u;
            goc = 8u;
            m8cpy();
        }
    }
    gshipcls = gk;
    gcorpk &= 1u;
    gom = gk;
    gom += gom;
    gcorpk += gom;
}

void m9rows(void) {
    gfar_o = M9B + 64u;
    goc = 0u;
    m8cpy();
    gfar_o = M9B + 88u;
    goc = 1u;
    m8cpy();
    gfar_o = M9B + 112u;
    goc = 3u;
    m8cpy();
    gfar_o = M9B + 136u;
    goc = 4u;
    m8cpy();
    gfar_o = M9B + 160u;
    goc = 5u;
    m8cpy();
    gfar_o = M9B + 184u;
    goc = 6u;
    m8cpy();
    gfar_o = HUBB;
    goc = 9u;
    m8cpy();
    gfar_o = HUBB + 24u;
    goc = 10u;
    m8cpy();
    gfar_o = HUBB + 48u;
    goc = 11u;
    m8cpy();
    gfar_o = HUBB + 72u;
    goc = 12u;
    m8cpy();
    gfar_o = HUBB + 96u;
    goc = 13u;
    m8cpy();
    gfar_o = HUBB + 120u;
    goc = 14u;
    m8cpy();
    gfar_o = HUBB + 144u;
    goc = 15u;
    m8cpy();
    gfar_o = HUBB + 168u;
    goc = 16u;
    m8cpy();
    gfar_o = DPTB;
    goc = 17u;
    m8cpy();
    gfar_o = DPTB + 24u;
    goc = 18u;
    m8cpy();
    gfar_o = DPTB + 48u;
    goc = 19u;
    m8cpy();
    gfar_o = DPTB + 72u;
    goc = 20u;
    m8cpy();
    gfar_o = DPTB + 96u;
    goc = 21u;
    m8cpy();
    gfar_o = DPTB + 120u;
    goc = 22u;
    m8cpy();
    gfar_o = DPTB + 144u;
    goc = 23u;
    m8cpy();
    gfar_o = DPTB + 168u;
    goc = 24u;
    m8cpy();
    gfar_o = DPTB + 192u;
    goc = 25u;
    m8cpy();
    gfar_o = DPTB + 216u;
    goc = 26u;
    m8cpy();
    gfar_o = DPTB + 240u;
    goc = 27u;
    m8cpy();
    gfar_o = DPTB + 264u;
    goc = 28u;
    m8cpy();
    gfar_o = DPTB + 288u;
    goc = 29u;
    m8cpy();
    gfar_o = DPTB + 312u;
    goc = 30u;
    m8cpy();
    gfar_o = DPTB + 336u;
    goc = 31u;
    m8cpy();
    if (gdept == 0u) {
        gfar_o = FOOTB;
    } else {
        gfar_o = FOOTB + 28u;
    }
    goc = 32u;
    m8cpy();
    gfar_o = FOOTB + 56u;
    if (gdept == 1u) {
        gfar_o += 0u;
    } else if (gdept == 2u) {
        gfar_o += 28u;
    } else if (gdept == 3u) {
        gfar_o += 56u;
    } else if (gdept == 4u) {
        gfar_o += 84u;
    } else if (gdept == 5u) {
        gfar_o += 112u;
    } else if (gdept == 6u) {
        gfar_o += 140u;
    }
    goc = 33u;
    m8cpy();
}

void m10mash(void) {
    /* Fold one button mash into the universe seed: double-add is a
     * helper-free left shift (16-bit << would emit none.lib calls).
     * Raw button bytes + frame timing supply entropy; the cell glyph
     * is count-indexed (bank-0 passes no button map). */
    gom = gunivseed;
    gom += gom;
    gom += gseedbtn;
    gom += gseedbt2;
    gom += gsfr;
    gunivseed = gom;
    gfar_o = SEEDG;
    gfar_o += gseedct;
    gact = farbyt1();
    gseedcells[gseedct] = gact;
    gseedbar[gseedct] = '#';
    gi = gseedct;
    gi++;
    gseedcells[gi] = 0u;
    gseedbar[gi] = 0u;
}

void m8dispatch(void) {
    if (gfar_fn == 1u) {
        m8_hello();
    } else if (gfar_fn == 10u) {
        m8enc();
    } else if (gfar_fn == 12u) {
        m8alien();
    } else if (gfar_fn == 13u) {
        m8gen();
    } else if (gfar_fn == 14u) {
        m8deny();
    } else if (gfar_fn == 20u) {
        m9charter();
    } else if (gfar_fn == 21u) {
        m9exchange();
    } else if (gfar_fn == 22u) {
        m9rows();
    } else if (gfar_fn == 23u) {
        m10mash();
    }
    gfar_fn = 0u;
}
