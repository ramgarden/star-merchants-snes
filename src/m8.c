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
 * + 784 tour bytes. Must match FT_M8 in main.c. */
#define M8B 3856u

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
extern uint16_t gom;
extern uint8_t goc;
extern uint16_t gtmp;
extern uint8_t gact;
extern uint8_t gsclk;
uint8_t farbyt1(void);

/* M8-owned BSS: far text line buffers (bank-0 draw code renders from
 * here) and a proof magic. gfarmsg = encounter ship name (persists
 * across fight redraws); gfarm2 = tribute/alien/deny line; gfarm3 =
 * home sector name (filled at sector gen). 32 cols + NUL each. */
char gfarmsg[33];
char gfarm2[33];
char gfarm3[33];
uint8_t gfarmagic;

void m8_hello(void);
void m8cpy(void);
void m8ferset(void);
void m8enc(void);
void m8alien(void);
void m8gen(void);
void m8deny(void);
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
        } else {
            gfarm3[gi] = gact;
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
    } else {
        gfarm3[32] = 0u;
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
    }
    gfar_fn = 0u;
}
