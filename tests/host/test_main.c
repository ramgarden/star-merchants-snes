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
uint8_t trep[128];
uint16_t gtest_t;
uint8_t tslot;

extern uint16_t gsram_a;
extern uint8_t gsram_d;
extern uint8_t gstate;
extern uint8_t gselfdrive;
extern uint8_t gsi;
extern uint8_t gsiprev;
extern uint8_t grpt;
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
extern uint8_t gcomm;
extern uint16_t gbankday;
extern uint16_t gsfr;
extern uint16_t gsfr;
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
    gsiprev = 0u;
    grpt = 25u;
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
    trep[15] = 1u;
}
