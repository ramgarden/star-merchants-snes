/* Host test driver: drives the REAL game logic (transformed copy of
 * src/main.c) through scripted scenarios and records PASS/FAIL plus
 * key state into trep[] for the py65 runner to assert.
 *
 * Locals are fine here (host gcc semantics via cc65 6502 target would
 * need c_sp; to stay stack-free this file uses file-scope test vars).
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;

uint8_t host_dummy;
uint8_t hostregs_unused;
uint16_t hj_pad;
uint8_t gth;
const uint8_t font_pic[3072] = {0};
uint8_t sram_mem[2048];
uint8_t trep[160];
uint16_t gtest_t;
uint8_t tslot;

extern uint16_t gsram_a;
extern uint8_t gsram_d;
extern uint8_t gstate;
extern uint8_t gselfdrive;
extern uint8_t gsi;
extern uint8_t gdemo;
extern uint8_t gcmdopen;
extern uint8_t gcmdsel;
extern uint8_t gportsel;
extern uint8_t gportcom;
extern uint8_t gporthag;
extern uint8_t gportmsg;
extern uint8_t gportfrom;
extern uint8_t gdept;
extern uint8_t gdocksel;
extern uint8_t gdockmsg;
extern uint8_t ghubonce;
extern uint8_t ghubug;
extern uint8_t ghubfence;
extern uint8_t gsel;
extern uint8_t gopt0;
extern uint8_t gopt1;
extern uint8_t gopt2;
extern uint8_t gopt3;
extern uint8_t gslot;
extern uint8_t gentry;
extern uint8_t gshow;
extern uint16_t gj_held;
extern uint16_t gj_prev;
extern uint8_t gshow;
extern char gname[9];
extern char gship[9];
extern uint16_t gsec;
extern uint16_t gturns;
extern uint16_t gcredits;
extern uint16_t gfighters;
extern uint16_t gshields;
extern uint16_t gholds;
extern uint16_t gholdmax;
extern int16_t galign;
extern uint16_t gxp;
extern uint8_t gore;
extern uint8_t gorg;
extern uint8_t gequ;
extern uint16_t gday;
extern uint16_t gbank;
extern uint8_t gcitadel;
extern uint16_t gcolship;
extern uint8_t gqsec;
extern uint16_t gfuel;
extern uint16_t gpftrs;
extern uint8_t gprobes;
extern uint8_t gbeacons;
extern uint8_t gtorp;
extern uint8_t gphotons;
extern uint8_t gfightover;
extern uint8_t gcomm;
extern uint16_t gbankday;
extern uint16_t gsfr;
extern uint16_t gsfr;
extern uint8_t gfar_fn;
extern uint8_t gfarmagic;
extern char gfarmsg[33];
extern char gfarm2[33];
extern char gfarm3[33];
extern char gfar8[33];
extern char gfar9[33];
extern void far_exec(void);
extern uint16_t guniv;
extern uint16_t gfersec;
extern uint8_t gfer;
extern uint8_t gferd;
extern uint8_t ggrudpk;
extern uint16_t gfertreas;
extern uint8_t gfergrd;
extern uint8_t galien;
extern uint16_t ghash;
extern uint8_t gencls;
extern uint16_t genftrs;
extern uint8_t gcorpk;
extern uint8_t gpool;
extern uint8_t gshipcls;
extern uint8_t gvisited[64];
extern uint16_t gunivseed;
extern uint16_t gwarps[6];
extern void sram_sync(void);
extern void sram_load(void);
extern uint16_t gday;
extern uint8_t gport;
extern uint8_t gplanet;
extern uint8_t gplevel;
extern uint8_t gftrs;
extern uint8_t gsclk;
extern const char *gdstr;
extern const char *gm1;
extern const char *gm2;
extern uint8_t gcmdsel;
extern void exec_cmd(void);
extern void port_name(void);
extern void show_title(void);
extern void tick_once(void);
extern void show_sector(void);
extern void show_dock(void);
extern void dock_hub(void);
extern void dock_hub(void);

void sram_wr(void) { sram_mem[gsram_a & 2047u] = gsram_d; }
void sram_rd(void) { gsram_d = sram_mem[gsram_a & 2047u]; }
void font_load(void) {}

static void boot_init(void) {
    gopt0 = 1u;
    gopt1 = 1u;
    gopt2 = 1u;
    gopt3 = 1u;
    gdemo = 0u;
    gcmdopen = 0u;
    gsi = 0u;
    gsfr = 0u;
    gj_held = 0u;
    gj_prev = 0u;
    gtest_t = 0u;
    while (gtest_t < 8u) {
        gname[gtest_t] = 32u;
        gship[gtest_t] = 32u;
        gtest_t++;
    }
    gname[8] = 0;
    gship[8] = 0;
    gshow = 1u;
    show_title();
}

static void run_ticks(void) {
    gtest_t = 0u;
    while (gtest_t < 4600u) {
        tick_once();
        gtest_t++;
    }
}

void test_main(void) {
    gtest_t = 0u;
    while (gtest_t < 32u) {
        trep[gtest_t] = 0u;
        gtest_t++;
    }
    /* T0: tour-B dense trajectory (gstate,gdept,gdocksel,gsi)
       every 64 ticks across gsfr 2880..3840 (bank/police/UG span) */
    boot_init();
    gselfdrive = 2u;
    tslot = 0u;
    gtest_t = 0u;
    while (gtest_t < 4100u) {
        tick_once();
        gtest_t++;
        if ((gtest_t & 63u) == 0u && gtest_t >= 2880u &&
            gtest_t <= 3840u && tslot < 64u) {
            trep[16u + tslot] = gstate;
            tslot++;
            trep[16u + tslot] = gdept;
            tslot++;
            trep[16u + tslot] = gdocksel;
            tslot++;
            trep[16u + tslot] = gsi;
            tslot++;
        }
        if (gtest_t == 3904u) {
            trep[80] = gstate;
            trep[81] = gdocksel;
        }
        if (gtest_t == 3968u) {
            trep[82] = gstate;
            trep[83] = gdocksel;
        }
        if (gtest_t == 4032u) {
            trep[84] = gstate;
            trep[85] = gdocksel;
        }
    }
    /* T1: M4 tour (selfdrive 1) ends at Continue resume */
    boot_init();
    gselfdrive = 1u;
    run_ticks();
    trep[0] = (gstate == 11u) ? 1u : 0u;
    trep[1] = gstate;
    trep[2] = (uint8_t)(gsec & 255u);
    trep[3] = (uint8_t)(gturns & 255u);
    trep[4] = (uint8_t)(gcredits & 255u);
    trep[5] = (uint8_t)((gcredits >> 8) & 255u);
    trep[6] = gore;
    trep[7] = gorg;
    /* T2: M5 tour (selfdrive 2) ends at police commission denial */
    boot_init();
    gselfdrive = 2u;
    run_ticks();
    trep[8] = 0u;
    if (gstate == 13u) {
        if (gdockmsg == 0u) {
            trep[8] = 1u;
        }
    }
    trep[9] = gstate;
    trep[10] = gdockmsg;
    trep[11] = (uint8_t)(gcredits & 255u);
    trep[12] = (uint8_t)((gcredits >> 8) & 255u);
    trep[13] = gprobes;
    trep[14] = (uint8_t)(gholdmax & 255u);
    /* T3: bank interest accrual over 5 warp-days */
    gsec = 1u;
    gday = 5u;
    gbankday = 0u;
    gbank = 1000u;
    dock_hub();
    trep[88] = 0u;
    if (gbank == 1165u) {
        if (gbankday == 5u) {
            trep[88] = 1u;
        }
    }
    trep[89] = (uint8_t)(gbank & 255u);
    trep[90] = (uint8_t)((gbank >> 8) & 255u);
    trep[91] = (uint8_t)(gbankday & 255u);
    /* T4: M6 tour (selfdrive 3) ends at genesis denial */
    boot_init();
    gselfdrive = 3u;
    run_ticks();
    trep[96] = 0u;
    if (gstate == 9u) {
        if (gsec == 3u) {
            if (gcitadel == 1u) {
                if (gcolship == 10u) {
                    if (gqsec == 20u) {
                        trep[96] = 1u;
                    }
                }
            }
        }
    }
    trep[97] = gstate;
    trep[98] = (uint8_t)(gsec & 255u);
    trep[99] = (uint8_t)(gturns & 255u);
    trep[100] = gcitadel;
    trep[101] = (uint8_t)(gcolship & 255u);
    trep[102] = gqsec;
    trep[103] = (uint8_t)(gfuel & 255u);
    trep[104] = (uint8_t)(gpftrs & 255u);
    trep[105] = (uint8_t)(gholds & 255u);
    trep[106] = (uint8_t)(gxp & 255u);
    /* T5: M7 combat tour (selfdrive 4) end state. Expected: victory
       over sector-3 hostiles (class 13): sector 3, 499 turns, 11300
       credits, 23 XP, 11 fighters left, 0 photons, back in
       sector view. */
    boot_init();
    gselfdrive = 4u;
    gtest_t = 0u;
    while (gtest_t < 4600u) {
        tick_once();
        gtest_t++;
    }
    trep[116] = gstate;
    trep[70] = galien;
    trep[71] = (uint8_t)((gxp >> 8) & 255u);
    trep[72] = (uint8_t)(ghash & 255u);
    trep[73] = (uint8_t)((ghash >> 8) & 255u);
    trep[74] = gencls;
    trep[117] = (uint8_t)(gsec & 255u);
    trep[118] = (uint8_t)(gturns & 255u);
    trep[119] = (uint8_t)((gturns >> 8) & 255u);
    trep[120] = (uint8_t)(gcredits & 255u);
    trep[121] = (uint8_t)((gcredits >> 8) & 255u);
    trep[122] = (uint8_t)(gxp & 255u);
    trep[123] = (uint8_t)(gfighters & 255u);
    trep[124] = gphotons;
    trep[125] = gfightover;
    /* T6: far-code trampoline (m8dispatch fn 1 = m8_hello). On hardware
       this crosses banks via jsl/rtl; on host it is a direct call.
       Expected: magic A5 + "FAR-OK" in the far message buffer. */
    gfar_fn = 1u;
    far_exec();
    trep[107] = gfarmagic;
    trep[108] = (uint8_t)gfarmsg[0];
    trep[109] = (uint8_t)gfarmsg[1];
    trep[110] = (uint8_t)gfarmsg[2];
    trep[111] = (uint8_t)gfarmsg[3];
    trep[112] = (uint8_t)gfarmsg[4];
    trep[113] = (uint8_t)gfarmsg[5];
    /* T7: M8 Ferrengi/alien far functions (direct probes, no tour).
       Home = guniv rim = 1000; day 10: guard 210, treasury 41000. */
    boot_init();
    guniv = 1000u;
    gsclk = 0u;
    gsec = 1000u;
    gfighters = 30u;
    gshields = 0u;
    gcredits = 10000u;
    gday = 10u;
    ggrudpk = 0u;
    galign = 0;
    gxp = 0u;
    gport = 1u;
    gplanet = 0u;
    gplevel = 0u;
    gftrs = 0u;
    gfar_fn = 13u;
    far_exec();
    trep[92] = 0u;
    if (gfersec == 1000u) {
        if (gplanet == 1u) {
            if (gplevel == 4u) {
                if (gport == 0u) {
                    if (gftrs == 210u) {
                        if (gfertreas == 41000u) {
                            if (gfarm3[0] == 70u) {
                                trep[92] = 1u;
                            }
                        }
                    }
                }
            }
        }
    }
    gfar_fn = 10u;
    far_exec();
    trep[93] = 0u;
    if (gferd == 2u) {
        if (gcredits == 9500u) {
            if (gencls == 17u) {
                if (gfarmsg[0] == 68u) {
                    if (gfarm2[0] == 84u) {
                        trep[93] = 1u;
                    }
                }
            }
        }
    }
    gcredits = 100u;
    gfar_fn = 10u;
    far_exec();
    trep[94] = 0u;
    if (gferd == 1u) {
        if (gfer == 1u) {
            if (ggrudpk == 16u) {
                if (genftrs == 45u) {
                    trep[94] = 1u;
                }
            }
        }
    }
    gfighters = 1u;
    gshields = 50u;
    gcredits = 10000u;
    gfar_fn = 10u;
    far_exec();
    trep[95] = 0u;
    if (gferd == 2u) {
        if (gcredits == 10000u) {
            if (gfarm2[9] == 83u) {
                trep[95] = 1u;
            }
        }
    }
    ghash = 112u;
    galign = 0;
    gxp = 0u;
    gfar_fn = 12u;
    far_exec();
    trep[114] = 0u;
    if (galien == 1u) {
        if (galign == -10) {
            if (gxp == 12u) {
                if (gfarm2[0] == 86u) {
                    trep[114] = 1u;
                }
            }
        }
    }
    gfighters = 3000u;
    gcredits = 10000u;
    gxp = 0u;
    galign = 0;
    gfar_fn = 13u;
    far_exec();
    trep[115] = 0u;
    if (gftrs == 0u) {
        if (gcredits == 51000u) {
            if (gxp == 50u) {
                if (galign == -25) {
                    if (ggrudpk & 64u) {
                        trep[115] = 1u;
                    }
                }
            }
        }
    }
    gfar_fn = 14u;
    far_exec();
    trep[126] = 0u;
    if (gfarmsg[0] == 70u) {
        if (gfarm2[0] == 82u) {
            trep[126] = 1u;
        }
    }
    trep[127] = gfergrd;
    /* T8: M8 bank-0 hook paths (trep 60-63 sit in the print-only T0
       trajectory area; no asserts read them). Home state from T7. */
    gsec = 1000u;
    port_name();
    trep[60] = 0u;
    if (gdstr[0] == 70u) {
        gsec = 3u;
        port_name();
        if (gdstr[0] == 82u) {
            trep[60] = 1u;
        }
    }
    gsec = 1000u;
    gplanet = 1u;
    gcmdsel = 5u;
    exec_cmd();
    trep[61] = 0u;
    if (gstate == 9u) {
        if (gm1[0] == 70u) {
            if (gm2[0] == 82u) {
                trep[61] = 1u;
            }
        }
    }
    gsec = 1000u;
    gftrs = 30u;
    gcredits = 10000u;
    gfighters = 30u;
    gshields = 0u;
    gcmdsel = 6u;
    exec_cmd();
    trep[62] = 0u;
    if (gstate == 9u) {
        if (gcredits == 9500u) {
            if (gm1[0] == 68u) {
                trep[62] = 1u;
            }
        }
    }
    gcredits = 100u;
    gcmdsel = 6u;
    exec_cmd();
    trep[63] = 0u;
    if (gstate == 15u) {
        if (gencls == 17u) {
            if (gfer == 1u) {
                trep[63] = 1u;
            }
        }
    }
    /* T9: M9 corp tour (selfdrive 7) end state + SRAM v7 round-trip.
       Runner asserts exact values (trep 128-140). */
    boot_init();
    gselfdrive = 7u;
    gtest_t = 0u;
    trep[139] = 0u;
    trep[141] = 0u;
    while (gtest_t < 4600u) {
        tick_once();
        gtest_t++;
        if (gsi >= 44u) {
            if (trep[139] == 0u) {
                trep[139] = (uint8_t)gm1[0];
                trep[141] = gdept;
                trep[143] = (uint8_t)gm2[0];
            }
        }
    }
    trep[128] = gstate;
    trep[129] = (uint8_t)(gsec & 255u);
    trep[130] = (uint8_t)(gturns & 255u);
    trep[131] = (uint8_t)((gturns >> 8) & 255u);
    trep[132] = (uint8_t)(gcredits & 255u);
    trep[133] = (uint8_t)((gcredits >> 8) & 255u);
    trep[134] = gcorpk;
    trep[135] = gpool;
    trep[136] = gshipcls;
    trep[137] = (uint8_t)(gholdmax & 255u);
    trep[138] = (uint8_t)(gfighters & 255u);
    trep[142] = gdept;
    sram_sync();
    gcorpk = 0u;
    gpool = 9u;
    gshipcls = 0u;
    sram_load();
    trep[140] = 0u;
    if (gcorpk == trep[134]) {
        if (gpool == trep[135]) {
            if (gshipcls == trep[136]) {
                trep[140] = 1u;
            }
        }
    }
    /* T9 overflow probe: SCOUT(15max)+10pool=25 total -> MERCHANT(30max):
       fits, pool tapped. Setup MERCHANT->FREIGHTER overflow instead:
       shipcls 2, fighters 50, pool 10 (total 60 > 50) -> DREAD(80max)
       with 60000cr: pool 60-80? No: 60 <= 80 fits. Use FREIGHTER hold
       cap: fighters 50 pool 10 -> fighters 60, pool 0, HULL SWAPPED.
       True overflow: shipcls 0 (SCOUT ftr 15), fighters 15, pool 10
       (total 25 > MERCHANT 30? No, fits). Overflow needs total > max:
       shipcls 1 (MERCHANT ftr 30), fighters 30, pool 10 (total 40 > 30
       FREIGHTER 50? No: next is FREIGHTER(50): 40 <= 50 fits).
       Cleanest: shipcls 2 (FREIGHTER ftr 50)->DREAD(80): fighters 50,
       pool 40 (total 90 > 80): pool 10, fighters 80, OVERFLOW msg. */
    gcorpk = 5u;
    gshipcls = 2u;
    gfighters = 50u;
    gpool = 40u;
    gholds = 0u;
    gcredits = 60000u;
    gfar_fn = 21u;
    far_exec();
    trep[144] = 0u;
    if (gpool == 10u) {
        if (gfighters == 80u) {
            if (gshipcls == 3u) {
                if (gcredits == 30000u) {
                    if (gfar9[0] == 79u) {
                        trep[144] = 1u;
                    }
                }
            }
        }
    }
    /* T9 cargo-deny probe: MERCHANT(20holds)->FREIGHTER with 41 holds. */
    gcorpk = 3u;
    gshipcls = 1u;
    gfighters = 30u;
    gpool = 0u;
    gholds = 41u;
    gcredits = 10000u;
    gfar_fn = 21u;
    far_exec();
    trep[145] = 0u;
    if (gshipcls == 1u) {
        if (gcredits == 10000u) {
            if (gfar9[2] == 82u) {
                trep[145] = 1u;
            }
        }
    }
    /* T9 price-deny probe: same swap, holds fit, 0 credits. */
    gholds = 0u;
    gcredits = 0u;
    gfar_fn = 21u;
    far_exec();
    trep[146] = 0u;
    if (gshipcls == 1u) {
        if (gfar9[2] == 78u) {
            trep[146] = 1u;
        }
    }
    /* T10: M10 persistence tour (selfdrive 8): warp x2, menu,
       Continue, Resume. Runner asserts exact end state. */
    boot_init();
    gselfdrive = 8u;
    run_ticks();
    trep[148] = gstate;
    trep[149] = (uint8_t)(gsec & 255u);
    trep[150] = (uint8_t)(gturns & 255u);
    trep[151] = (uint8_t)(gcredits & 255u);
    trep[152] = (uint8_t)((gcredits >> 8) & 255u);
    trep[153] = gvisited[0];
    trep[154] = gvisited[1];
    /* T10 unit probes: visited pattern round-trip + record version. */
    gvisited[0] = 0xAAu;
    gvisited[63] = 0x55u;
    sram_sync();
    gvisited[0] = 0u;
    gvisited[63] = 0u;
    sram_load();
    trep[155] = 0u;
    if (gvisited[0] == 0xAAu) {
        if (gvisited[63] == 0x55u) {
            trep[155] = 1u;
        }
    }
    trep[156] = sram_mem[2];
    /* T11: M10 seed tour (selfdrive 9): wizard -> 16 fixed mashes ->
       launch -> warp to 2. Seed/timing are host-deterministic; the
       runner asserts the exact seeded warps of sector 2. */
    boot_init();
    gselfdrive = 9u;
    run_ticks();
    trep[157] = gstate;
    trep[158] = (uint8_t)(gsec & 255u);
    trep[159] = 0u;
    if (gstate == 9u) {
        if (gsec == 2u) {
            if (gunivseed == 2426u) {
                if (gwarps[0] == 43u) {
                    if (gwarps[1] == 335u) {
                        trep[159] = 1u;
                    }
                }
            }
        }
    }
    /* T9 pool-tapped probe: SCOUT->MERCHANT, 10+5=15 <= 30, pool 5>0. */
    gcorpk = 1u;
    gshipcls = 0u;
    gfighters = 10u;
    gpool = 5u;
    gholds = 0u;
    gcredits = 10000u;
    gfar_fn = 21u;
    far_exec();
    trep[147] = 0u;
    if (gfighters == 15u) {
        if (gpool == 0u) {
            if (gshipcls == 1u) {
                if (gcredits == 5000u) {
                    if (gfar9[0] == 80u) {
                        trep[147] = 1u;
                    }
                }
            }
        }
    }
trep[15] = 1u;
}
