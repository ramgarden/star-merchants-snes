/* Star Merchants - title + menu engine + sector view (Milestones 1-3).
 *
 * Title: ANSI homage (TradeWars 2002 style) on BG1 (Mode 1, 4bpp):
 * font tiles at VRAM words $3000, tilemap 32x32 at VRAM words $6800.
 * Tile for ASCII c is (c-32); tilemap high byte is palette << 2.
 * Menu: state machine TITLE->MENU->(NEW GAME wizard/CONTINUE/OPTIONS/
 * CREDITS)->summary->launch->SECTOR (Milestone 3 main loop).
 * Sector view: authentic TW2002 adaptation for 32 cols: sector header,
 * warp list (unvisited red, like ANSI TW), port/planet/fighter lines,
 * message area, status bar, command line; warp cycling, density/holo
 * scans, course plotter, port stub (Milestone 4 hook), quit to menu.
 * Universe is deterministic per sector (hash-based) sized by options.
 *
 * CODING CONSTRAINTS (verified 2026-09-22, see devlog):
 * 1. cc65 stack-frame locals and stack-passed args HANG on this
 *    toolchain/setup. This file uses ONLY globals + parameterless
 *    functions + inline register writes. No C locals, no C args, ever.
 * 2. CGRAM power-on state is NOT black: init ALL 128 BG colors, not
 *    just the ones used, or uninitialized slots show garbage colors.
 *
 * SELFDRIVE: gselfdrive=1 replaces hardware pad reads with the scripted
 * scf/scp table tour (no emulator input needed - neither test emulator
 * delivers Start/Select from the keyboard; see devlog). SHIP WITH 0.
 */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
#define REG_INIDISP  (*(volatile uint8_t*)0x2100)
#define REG_BGMODE   (*(volatile uint8_t*)0x2105)
#define REG_BG1SC    (*(volatile uint8_t*)0x2107)
#define REG_BG12NBA  (*(volatile uint8_t*)0x210B)
#define REG_BG1HOFS  (*(volatile uint8_t*)0x210D)
#define REG_BG1VOFS  (*(volatile uint8_t*)0x210E)
#define REG_VMAIN    (*(volatile uint8_t*)0x2115)
#define REG_VMADDL   (*(volatile uint8_t*)0x2116)
#define REG_VMADDH   (*(volatile uint8_t*)0x2117)
#define REG_VMDATAL  (*(volatile uint8_t*)0x2118)
#define REG_VMDATAH  (*(volatile uint8_t*)0x2119)
#define REG_CGADD    (*(volatile uint8_t*)0x2121)
#define REG_CGDATA   (*(volatile uint8_t*)0x2122)
#define REG_TM       (*(volatile uint8_t*)0x212C)
#define REG_NMITIMEN (*(volatile uint8_t*)0x4200)
#define REG_HVBJOY   (*(volatile uint8_t*)0x4212)
#define REG_JOY1L    (*(volatile uint8_t*)0x4218)
#define REG_JOY1H    (*(volatile uint8_t*)0x4219)
#define VRAM_FONT_WORDS 0x3000u
#define VRAM_MAP_WORDS  0x6800u
#define MAP_W 32u
#define PAL_WHITE 0u
#define PAL_GRAY  1u
#define PAL_RED   2u
#define PAL_DRED  3u
#define PAL_BLUE  4u
#define PAL_DBLUE 5u
#define PAL_CYAN  6u
#define PAL_YEL   7u
#define ST_TITLE 0u
#define ST_MENU 1u
#define ST_CREDITS 2u
#define ST_CONTINUE 3u
#define ST_OPTIONS 4u
#define ST_NEWNAME 5u
#define ST_NEWSHIP 6u
#define ST_NEWSUM 7u
#define ST_LAUNCH 8u
#define ST_SECTOR 9u
#define ST_ATTRACT 10u
#define ST_LOADRET 11u
#define ST_PORT 12u
#define ST_DOCK 13u
#define ST_PLANET 14u
#define ST_FIGHT 15u
/* Joypad buttons: PVSnesLib KEYPAD_BITS layout (include/snes/input.h:
 * KEY_A=BIT(7) KEY_B=BIT(15) KEY_SELECT=BIT(13) KEY_START=BIT(12)
 * KEY_RIGHT=BIT(8) KEY_LEFT=BIT(9) KEY_DOWN=BIT(10) KEY_UP=BIT(11)
 * KEY_R=BIT(4) KEY_L=BIT(5) KEY_X=BIT(6) KEY_Y=BIT(14)).
 * The SNES auto-read word ($4219<<8|$4218) already arrives in exactly
 * this layout ($4218: A/X/L/R+id, $4219: B/Y/Sel/Start/U/D/L/R), so the
 * raw register word is used directly with NO conversion (cf. PVSnesLib
 * padsCurrent/padsDown and the controller.c demo; superfamicom.org
 * polling-controller-input cheat sheet; SNESdev wiki controller
 * reading). read_pads() below is the single global entry point every
 * tick_* screen handler calls: gj_pad = held (padsCurrent), gj_new =
 * fresh presses (padsDown), gj_dir = gj_new (edge). Press semantics are
 * one-shot everywhere: a held button never auto-repeats (holding Up on
 * the menu moves one item, not a turbo scroll). The only hold-repeat in
 * the game is Up/Down letter scrolling in tick_name, which reuses the
 * gframe-since-redraw counter (first repeat at 25 frames, then every
 * 8 frames) on top of edges. */
#define KEY_A 0x0080u
#define KEY_B 0x8000u
#define KEY_SELECT 0x2000u
#define KEY_START 0x1000u
#define KEY_RIGHT 0x0100u
#define KEY_LEFT 0x0200u
#define KEY_DOWN 0x0400u
#define KEY_UP 0x0800u
#define KEY_R 0x0010u
#define KEY_L 0x0020u
#define KEY_X 0x0040u
#define KEY_Y 0x4000u
#define SCN 92u
#define SCN2 106u
#define SCN3 92u
#define SCN4 84u
#define SCN5 4u
#define SCN6 14u
/* Bank-1 tour table bases (FARRODATA starts at $018C00, right after
   the 3072-byte FARFONT at $018000-$018BFF); see far_data.s/far_tbl.s.
   C reads these tables only through farbyt(); no 16-bit addressing. */
#define FT_BASE 0x0C00u
#define FT_SC   (FT_BASE + 0u)
#define FT_SCP  (FT_BASE + 92u)
#define FT_SC2  (FT_BASE + 184u)
#define FT_SCP2 (FT_BASE + 290u)
#define FT_SC3  (FT_BASE + 396u)
#define FT_SCP3 (FT_BASE + 488u)
#define FT_SC4  (FT_BASE + 584u)
#define FT_SCP4 (FT_BASE + 668u)
#define FT_SC5  (FT_BASE + 752u)
#define FT_SCP5 (FT_BASE + 756u)
#define FT_SC6  (FT_BASE + 760u)
#define FT_SCP6 (FT_BASE + 774u)
extern const uint8_t font_pic[3072];
uint16_t gw;
uint16_t gseed;
uint16_t gframe;
uint8_t gshow;
const char *gdstr;
uint8_t gdx;
uint8_t gdy;
uint8_t gdpal;
uint8_t gc;
uint8_t gp;
uint8_t gr;
uint8_t gstate;
uint8_t gsel;
uint8_t gosel;
uint8_t gopt0;
uint8_t gopt1;
uint8_t gopt2;
uint8_t gopt3;
uint8_t gslot;
uint8_t gentry;
uint8_t gci;
uint16_t gj_pad;
uint16_t gj_prev;
uint16_t gj_new;
uint16_t gj_dir;
uint16_t gj_held;
uint8_t gsi;
uint8_t gselfdrive;
uint16_t gsfr;
uint8_t gsclk;
uint8_t gact;
const uint8_t *gftab;
const uint8_t *gatab;
uint8_t gflim;
uint8_t gscr_f;
uint16_t gscr_b;
uint16_t gscr_b2;
uint16_t gfar_o;
char gname[9];
char gship[9];
char *gbuf;
char gch[2];
uint16_t gsec;
uint16_t gturns;
uint16_t gcredits;
uint16_t gfighters;
uint16_t gshields;
uint16_t gholds;
uint16_t gholdmax;
int16_t galign;
uint16_t gxp;
uint8_t gore;
uint8_t gorg;
uint8_t gequ;
uint8_t gwarpsel;
uint8_t gwcount;
uint16_t gwarps[6];
uint8_t gport;
uint8_t gportcls;
uint8_t gplanet;
uint8_t gplevel;
uint8_t gftrs;
uint8_t gcmdopen;
uint8_t gcmdsel;
uint8_t gdemo;
uint16_t ghash;
uint16_t gn;
uint16_t gdiv;
uint8_t gstarted;
uint8_t gdigit;
uint8_t gi;
uint8_t gj;
uint8_t gk;
uint16_t gtmp;
char gnum[6];
const char *gm1;
const char *gm2;
const char *gm3;
const char *gm4;
uint8_t gm1pal;
uint8_t gmsgmode;
uint16_t gsav;
uint16_t guniv;
uint8_t gvisited[64];
uint16_t gsram_a;
uint8_t gsram_d;
uint8_t gsram_ck;
uint8_t gportsel;
uint8_t gportcom;
uint8_t gporthag;
uint8_t gportmsg;
uint8_t gportn;
uint16_t gday;
uint8_t gstock[3];
uint8_t gprc[3];
uint8_t gdept;
uint8_t gdocksel;
uint8_t gdockmsg;
uint8_t gportfrom;
uint8_t ghubonce;
uint8_t ghubug;
uint8_t ghubfence;
uint16_t gbank;
uint8_t gprobes;
uint8_t gbeacons;
uint8_t gtorp;
uint8_t gcomm;
uint16_t gbankday;
uint8_t gcitadel;
uint16_t gcolore;
uint16_t gcolorg;
uint16_t gcolequ;
uint16_t gpftrs;
uint16_t gpsh;
uint8_t gqsec;
uint8_t gqatm;
uint16_t gfuel;
uint16_t gcolship;
uint8_t gadet;
uint16_t gcolsec;
uint8_t gcolsel;
uint8_t gplsel;
uint8_t gplmsg;
uint8_t gphotons;
uint8_t gencls;
uint16_t genftrs;
uint16_t gensh;
uint8_t gencorb;
uint8_t gblind;
uint16_t gbounty;
uint8_t gfightover;
uint8_t gfightsel;
uint16_t glastsec;
void sram_wr(void);
void sram_rd(void);
void font_load(void);
uint8_t farbyt(void);
const uint8_t palc1[16] = {
    0xFF, 0x7F, 0x10, 0x42, 0x1F, 0x00, 0x0C, 0x00,
    0x00, 0x7C, 0x00, 0x2C, 0xE0, 0x7F, 0xFF, 0x03,
};
const uint8_t mrows[4] = { 11u, 13u, 15u, 17u };
const char charset[38] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
const uint8_t bitmask[8] = { 1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u };
const char rulerow[33] = "--------------------------------";
const uint8_t clsides[9] = { 6u, 5u, 3u, 1u, 2u, 4u, 0u, 7u, 4u };
const uint16_t citcost[6] = { 0u, 0u, 2000u, 5000u, 10000u, 20000u };
/* M7 combat: 15 ship classes, offensive/defensive odds (TW2002 tables) */
const uint8_t shipoff[15] = {
    10u, 8u, 11u, 12u, 10u, 6u, 10u, 10u,
    12u, 13u, 11u, 11u, 12u, 13u, 7u,
};
const uint8_t shipdef[15] = {
    10u, 8u, 10u, 11u, 12u, 6u, 10u, 11u,
    12u, 10u, 12u, 13u, 12u, 11u, 9u,
};
/* M7 combat tour (selfdrive 4): new game -> photon buy -> warp sec 3 ->
   ATTACK -> photon -> attack -> victory. 84 steps, 8-bit frames. */
static void show_sum(void);
static void show_sector(void);
static void show_menu(void);
static void show_loadret(void);
static void show_dock(void);
static void show_planet(void);
static void show_fight(void);
static void gen_enemy(void);
static void planet_fresh(void);
static void planet_genesis(void);
static void draw_num(void);
static void sram_sync(void);
static void sram_load(void);
static void port_dock(void);
static void dock_hub(void);
static void tick_once(void);
static void load_palettes(void) {
    REG_CGADD = 0;
    gp = 0u;
    while (gp < 8u) {
        REG_CGDATA = 0x00; REG_CGDATA = 0x00;
        REG_CGDATA = palc1[gp * 2u];
        REG_CGDATA = palc1[gp * 2u + 1u];
        gr = 0u;
        while (gr < 14u) {
            REG_CGDATA = 0x00; REG_CGDATA = 0x00;
            gr++;
        }
        gp++;
    }
}
static void load_font(void) {
    REG_VMADDL = 0x00u;
    REG_VMADDH = 0x30u;
    font_load();
}
static void clear_map(void) {
    REG_VMADDL = 0x00u;
    REG_VMADDH = 0x68u;
    for (gw = 0; gw < 1024u; gw++) {
        REG_VMDATAL = 0;
        REG_VMDATAH = 0;
    }
}
static void draw_text(void) {
    while (*gdstr) {
        gc = (uint8_t)*gdstr;
        gdstr++;
        if (gc < 32u || gc > 127u) gc = 32u;
        if (gdx >= MAP_W) break;
        REG_VMADDL = (uint8_t)((VRAM_MAP_WORDS + (uint16_t)gdy * MAP_W + gdx) & 0xFFu);
        REG_VMADDH = (uint8_t)((VRAM_MAP_WORDS + (uint16_t)gdy * MAP_W + gdx) >> 8);
        REG_VMDATAL = (uint8_t)(gc - 32u);
        REG_VMDATAH = (uint8_t)(gdpal << 2);
        gdx++;
    }
}
static void wait_vblank(void) {
    while (REG_HVBJOY & 0x80u) {
    }
    while (!(REG_HVBJOY & 0x80u)) {
    }
    gsfr++;
}
/* Tours 1-3: tables in bank 1 (FARRODATA), read via farbyt().
   Tours 4-6 (M7): small tables in bank 0, indexed directly. */
/* All tours (1-6) now use bank-1 FARRODATA tables via farbyt(). */
static void script_pads(void) {
    gsclk = (uint8_t)(gsfr >> 4);
    if (gselfdrive == 4u) {
        gscr_f = 1u; gscr_b = FT_SC4; gscr_b2 = FT_SCP4; gflim = SCN4;
    } else if (gselfdrive == 5u) {
        gscr_f = 1u; gscr_b = FT_SC5; gscr_b2 = FT_SCP5; gflim = SCN5;
    } else if (gselfdrive == 6u) {
        gscr_f = 1u; gscr_b = FT_SC6; gscr_b2 = FT_SCP6; gflim = SCN6;
    } else if (gselfdrive == 3u) {
        gscr_f = 1u; gscr_b = FT_SC3; gscr_b2 = FT_SCP3; gflim = SCN3;
    } else if (gselfdrive == 2u) {
        gscr_f = 1u; gscr_b = FT_SC2; gscr_b2 = FT_SCP2; gflim = SCN2;
    } else {
        gscr_f = 1u; gscr_b = FT_SC; gscr_b2 = FT_SCP; gflim = SCN;
    }
    while (gsi < gflim) {
        gfar_o = (uint16_t)(gscr_b + gsi);
        gact = farbyt();
        if (gsclk < gact) break;
        gfar_o = (uint16_t)(gscr_b2 + gsi);
        gact = farbyt();
        if (gact == 0u) { gj_held = 0u; }
        else if (gact == 1u) { gj_held = KEY_START; }
        else if (gact == 2u) { gj_held = KEY_UP; }
        else if (gact == 3u) { gj_held = KEY_DOWN; }
        else if (gact == 4u) { gj_held = KEY_LEFT; }
        else if (gact == 5u) { gj_held = KEY_RIGHT; }
        else if (gact == 6u) { gj_held = KEY_A; }
        else if (gact == 7u) { gj_held = KEY_B; }
        else if (gact == 8u) { gj_held = KEY_X; }
        else { gj_held = KEY_Y; }
        gsi++;
    }
    gj_pad = gj_held;
    gj_new = (uint16_t)(gj_pad & (uint16_t)(gj_pad ^ gj_prev));
    gj_prev = gj_pad;
    gj_dir = gj_new;
}
/* THE global pad entry point: every tick_* handler calls this once per
 * frame, then reads gj_new (fresh-press edge, for A/B/START/SELECT/X/Y
 * actions) and gj_dir (same edge, for cursor/warp/menu movement: one
 * step per press, never auto-repeat). Hardware path: wait out the $4212
 * auto-read busy bit (wikibooks/superfamicom.org pattern), then latch
 * the $4218/$4219 word which is already in KEY_* layout. Selfdrive
 * path: scripted tour. */
static void read_pads(void) {
    if (gselfdrive) {
        script_pads();
        return;
    }
    while (REG_HVBJOY & 0x01u) {
    }
    gj_pad = (uint16_t)REG_JOY1L;
    gj_pad |= (uint16_t)((uint16_t)REG_JOY1H << 8);
    /* Shoulders double as D-pad Left/Right: the port ("L/R SELECT"),
     * planet ("L/R SHIP") and options ("L/R:CHANGE") hints promise L/R,
     * so fold shoulder bits into the pad word and every screen's
     * Left/Right handling answers to both, still edge-style one-shot. */
    if (gj_pad & KEY_L) { gj_pad |= KEY_LEFT; }
    if (gj_pad & KEY_R) { gj_pad |= KEY_RIGHT; }
    gj_new = (uint16_t)(gj_pad & (uint16_t)(gj_pad ^ gj_prev));
    gj_prev = gj_pad;
    gj_dir = gj_new;
}
static void draw_stars(void) {
    gseed = 1234u;
    for (gw = 0; gw < 70u; gw++) {
        gseed += 997u;
        gdx = (uint8_t)(gseed & 31u);
        gdy = (uint8_t)((gseed >> 5) & 31u);
        if (gdy >= 28u) continue;
        gc = (uint8_t)((gseed >> 10) & 3u);
        if (gc == 0u) { gdstr = "."; gdpal = PAL_GRAY; }
        else if (gc == 1u) { gdstr = "*"; gdpal = PAL_WHITE; }
        else if (gc == 2u) { gdstr = "+"; gdpal = PAL_BLUE; }
        else { gdstr = "."; gdpal = PAL_DBLUE; }
        draw_text();
    }
}
static void draw_planet(void) {
    gdy = 1u; gdx = 24u; gdpal = PAL_BLUE;  gdstr = "######";      draw_text();
    gdy = 2u; gdx = 22u; gdpal = PAL_BLUE;  gdstr = "#########";   draw_text();
    gdy = 3u; gdx = 21u; gdpal = PAL_BLUE;  gdstr = "###########"; draw_text();
    gdy = 4u; gdx = 21u; gdpal = PAL_BLUE;  gdstr = "###########"; draw_text();
    gdy = 5u; gdx = 21u; gdpal = PAL_DBLUE; gdstr = "###########"; draw_text();
    gdy = 6u; gdx = 22u; gdpal = PAL_DBLUE; gdstr = "#########";   draw_text();
    gdy = 7u; gdx = 23u; gdpal = PAL_DBLUE; gdstr = "#######";     draw_text();
    gdy = 8u; gdx = 25u; gdpal = PAL_DBLUE; gdstr = "###";         draw_text();
    gdy = 2u; gdx = 22u; gdpal = PAL_CYAN;  gdstr = "##";          draw_text();
    gdy = 3u; gdx = 21u; gdpal = PAL_CYAN;  gdstr = "##";          draw_text();
}
static void draw_lines(void) {
    gdy = 11u; gdx = 0u; gdpal = PAL_BLUE; gdstr = rulerow; draw_text();
    gdy = 13u; gdx = 0u; gdpal = PAL_BLUE; gdstr = rulerow; draw_text();
}
static void draw_title(void) {
    gdx = 13u; gdy = 5u;  gdpal = PAL_GRAY; gdstr = "S T A R";           draw_text();
    gdx = 8u;  gdy = 7u;  gdpal = PAL_GRAY; gdstr = "M E R C H A N T S"; draw_text();
    gdx = 13u; gdy = 9u;  gdpal = PAL_DRED; gdstr = "2 0 2 6";           draw_text();
    gdx = 12u; gdy = 4u;  gdpal = PAL_WHITE; gdstr = "S T A R";           draw_text();
    gdx = 7u;  gdy = 6u;  gdpal = PAL_WHITE; gdstr = "M E R C H A N T S"; draw_text();
    gdx = 12u; gdy = 8u;  gdpal = PAL_RED;   gdstr = "2 0 2 6";           draw_text();
}
static void draw_ship(void) {
    gdpal = PAL_GRAY;
    gdy = 15u; gdx = 18u; gdstr = "/\\";                  draw_text();
    gdy = 16u; gdx = 18u; gdstr = "||";                  draw_text();
    gdy = 17u; gdx = 14u; gdstr = "============";        draw_text();
    gdy = 18u; gdx = 13u; gdstr = "/------------\\";     draw_text();
    gdy = 19u; gdx = 11u; gdstr = "===================="; draw_text();
    gdy = 20u; gdx = 12u; gdstr = "\\________________/"; draw_text();
    gdy = 21u; gdx = 14u; gdstr = "\\  \\  \\";         draw_text();
    gdpal = PAL_CYAN;
    gdy = 18u; gdx = 15u; gdstr = "ooooo";                draw_text();
    gdpal = PAL_WHITE;
    gdy = 21u; gdx = 14u; gdstr = "*";                    draw_text();
    gdy = 21u; gdx = 18u; gdstr = "*";                    draw_text();
    gdy = 21u; gdx = 22u; gdstr = "*";                    draw_text();
}
static void draw_footer(void) {
    gdpal = PAL_GRAY;
    gdy = 23u; gdx = 6u;  gdstr = "A TRADEWARS TRIBUTE"; draw_text();
    gdy = 24u; gdx = 8u;  gdstr = "(C) 2026 GPL-3.0";    draw_text();
}
static void draw_prompt(void) {
    gdpal = PAL_WHITE;
    gdy = 26u; gdx = 10u;
    if (gshow) { gdstr = "PRESS START"; }
    else { gdstr = "           "; }
    draw_text();
}
static void show_title(void) {
    REG_INIDISP = 0x80u;
    gdemo = 0u;
    clear_map();
    draw_stars();
    draw_planet();
    draw_lines();
    draw_title();
    draw_ship();
    draw_footer();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_TITLE;
    gframe = 0u;
    gshow = 1u;
    draw_prompt();
}
static void draw_menulines(void) {
    gdy = 7u;  gdx = 0u; gdpal = PAL_BLUE; gdstr = rulerow; draw_text();
    gdy = 21u; gdx = 0u; gdpal = PAL_BLUE; gdstr = rulerow; draw_text();
}
static void draw_cursor(void) {
    gdpal = PAL_CYAN;
    gdx = 8u;
    gdy = mrows[gsel];
    gch[0] = '>';
    gch[1] = 0;
    gdstr = gch;
    draw_text();
}
static void draw_hint_title(void) {
    gdpal = PAL_GRAY;
    gdy = 24u; gdx = 9u; gdstr = "A:SELECT B:TITLE"; draw_text();
}
static void draw_hint_back(void) {
    gdpal = PAL_GRAY;
    gdy = 24u; gdx = 12u; gdstr = "B:BACK"; draw_text();
}
static void show_menu(void) {
    REG_INIDISP = 0x80u;
    gdemo = 0u;
    gcmdopen = 0u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 4u; gdx = 9u;  gdstr = "STAR MERCHANTS"; draw_text();
    gdy = 11u; gdx = 10u; gdstr = "NEW GAME";  draw_text();
    gdy = 13u; gdx = 10u; gdstr = "CONTINUE";  draw_text();
    gdy = 15u; gdx = 10u; gdstr = "OPTIONS";   draw_text();
    gdy = 17u; gdx = 10u; gdstr = "CREDITS";   draw_text();
    draw_cursor();
    draw_hint_title();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_MENU;
    gframe = 0u;
}
static void show_credits(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 6u; gdx = 9u;  gdstr = "STAR MERCHANTS"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 9u;  gdx = 6u;  gdstr = "A TRADEWARS TRIBUTE"; draw_text();
    gdy = 11u; gdx = 8u;  gdstr = "SNES HOMEBREW 2026";  draw_text();
    gdy = 13u; gdx = 8u;  gdstr = "FONT: PVSNESLIB";     draw_text();
    gdy = 15u; gdx = 7u;  gdstr = "LICENSE: GPL-3.0";    draw_text();
    draw_hint_back();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_CREDITS;
    gframe = 0u;
}
static void show_continue(void) {
    sram_load();
    if (gtmp == 0u) {
        REG_INIDISP = 0x80u;
        clear_map();
        draw_menulines();
        gdpal = PAL_WHITE;
        gdy = 6u; gdx = 12u; gdstr = "CONTINUE"; draw_text();
        gdpal = PAL_RED;
        gdy = 12u; gdx = 7u; gdstr = "NO SAVED GAME FOUND"; draw_text();
        gdpal = PAL_GRAY;
        gdy = 14u; gdx = 6u; gdstr = "START A NEW GAME"; draw_text();
        gdy = 15u; gdx = 9u;  gdstr = "FIRST, TRADER";    draw_text();
        draw_hint_back();
        REG_TM = 0x01u;
        REG_INIDISP = 0x0Fu;
        gstate = ST_CONTINUE;
        gframe = 0u;
        return;
    }
    show_loadret();
}
/* ---- SRAM save/load ($70:0000+, 75 bytes, sum checksum) ---- */
static void sram_put(void) {
    sram_wr();
    gsram_ck += gsram_d;
    gsram_a++;
}
static void sram_get(void) {
    sram_rd();
    gsram_ck += gsram_d;
    gsram_a++;
}
static void sram_sync(void) {
    gsram_a = 0u;
    gsram_ck = 0u;
    gsram_d = 83u; sram_put();
    gsram_d = 77u; sram_put();
    gsram_d = 5u; sram_put();
    gi = 0u;
    while (gi < 8u) {
        gsram_d = gname[gi];
        sram_put();
        gi++;
    }
    gi = 0u;
    while (gi < 8u) {
        gsram_d = gship[gi];
        sram_put();
        gi++;
    }
    gsram_d = gopt0; sram_put();
    gsram_d = gopt1; sram_put();
    gsram_d = gopt2; sram_put();
    gsram_d = gopt3; sram_put();
    gsram_d = (uint8_t)(gsec & 255u); sram_put();
    gsram_d = (uint8_t)(gsec >> 8); sram_put();
    gsram_d = (uint8_t)(gturns & 255u); sram_put();
    gsram_d = (uint8_t)(gturns >> 8); sram_put();
    gsram_d = (uint8_t)(gcredits & 255u); sram_put();
    gsram_d = (uint8_t)(gcredits >> 8); sram_put();
    gsram_d = (uint8_t)(gfighters & 255u); sram_put();
    gsram_d = (uint8_t)(gfighters >> 8); sram_put();
    gsram_d = (uint8_t)(gshields & 255u); sram_put();
    gsram_d = (uint8_t)(gshields >> 8); sram_put();
    gsram_d = (uint8_t)(gholds & 255u); sram_put();
    gsram_d = (uint8_t)(gholds >> 8); sram_put();
    gsram_d = (uint8_t)(gholdmax & 255u); sram_put();
    gsram_d = (uint8_t)(gholdmax >> 8); sram_put();
    gsram_d = (uint8_t)((uint16_t)galign & 255u); sram_put();
    gsram_d = (uint8_t)((uint16_t)galign >> 8); sram_put();
    gsram_d = (uint8_t)(gxp & 255u); sram_put();
    gsram_d = (uint8_t)(gxp >> 8); sram_put();
    gsram_d = gore; sram_put();
    gsram_d = gorg; sram_put();
    gsram_d = gequ; sram_put();
    gsram_d = (uint8_t)(gday & 255u); sram_put();
    gsram_d = (uint8_t)(gday >> 8); sram_put();
    gsram_d = (uint8_t)(gbank & 255u); sram_put();
    gsram_d = (uint8_t)(gbank >> 8); sram_put();
    gsram_d = gprobes; sram_put();
    gsram_d = gbeacons; sram_put();
    gsram_d = gtorp; sram_put();
    gsram_d = gcomm; sram_put();
    gsram_d = (uint8_t)(gbankday & 255u); sram_put();
    gsram_d = (uint8_t)(gbankday >> 8); sram_put();
    gsram_d = gcitadel; sram_put();
    gsram_d = (uint8_t)(gcolore & 255u); sram_put();
    gsram_d = (uint8_t)(gcolore >> 8); sram_put();
    gsram_d = (uint8_t)(gcolorg & 255u); sram_put();
    gsram_d = (uint8_t)(gcolorg >> 8); sram_put();
    gsram_d = (uint8_t)(gcolequ & 255u); sram_put();
    gsram_d = (uint8_t)(gcolequ >> 8); sram_put();
    gsram_d = (uint8_t)(gpftrs & 255u); sram_put();
    gsram_d = (uint8_t)(gpftrs >> 8); sram_put();
    gsram_d = (uint8_t)(gpsh & 255u); sram_put();
    gsram_d = (uint8_t)(gpsh >> 8); sram_put();
    gsram_d = gqsec; sram_put();
    gsram_d = gqatm; sram_put();
    gsram_d = (uint8_t)(gfuel & 255u); sram_put();
    gsram_d = (uint8_t)(gfuel >> 8); sram_put();
    gsram_d = (uint8_t)(gcolship & 255u); sram_put();
    gsram_d = (uint8_t)(gcolship >> 8); sram_put();
    gsram_d = gadet; sram_put();
    gsram_d = (uint8_t)(gcolsec & 255u); sram_put();
    gsram_d = (uint8_t)(gcolsec >> 8); sram_put();
    gsram_d = gphotons; sram_put();
    gsram_d = gsram_ck;
    sram_wr();
}
static void sram_load(void) {
    gtmp = 0u;
    gsram_a = 0u;
    gsram_ck = 0u;
    sram_get();
    if (gsram_d != 83u) return;
    sram_get();
    if (gsram_d != 77u) return;
    sram_get();
    if (gsram_d != 5u) return;
    gi = 0u;
    while (gi < 8u) {
        sram_get();
        gname[gi] = gsram_d;
        gi++;
    }
    gname[8] = 0;
    gi = 0u;
    while (gi < 8u) {
        sram_get();
        gship[gi] = gsram_d;
        gi++;
    }
    gship[8] = 0;
    sram_get(); gopt0 = gsram_d;
    sram_get(); gopt1 = gsram_d;
    sram_get(); gopt2 = gsram_d;
    sram_get(); gopt3 = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gsec = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gturns = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gcredits = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gfighters = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gshields = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gholds = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gholdmax = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); galign = (int16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gxp = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gore = gsram_d;
    sram_get(); gorg = gsram_d;
    sram_get(); gequ = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gday = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gbank = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gprobes = gsram_d;
    sram_get(); gbeacons = gsram_d;
    sram_get(); gtorp = gsram_d;
    sram_get(); gcomm = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gbankday = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gcitadel = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gcolore = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gcolorg = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gcolequ = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gpftrs = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gpsh = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gqsec = gsram_d;
    sram_get(); gqatm = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gfuel = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gtmp = gsram_d;
    sram_get(); gcolship = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gadet = gsram_d;
    sram_get(); gtmp = gsram_d;
    sram_get(); gcolsec = (uint16_t)(gtmp | ((uint16_t)gsram_d << 8));
    sram_get(); gphotons = gsram_d;
    sram_rd();
    if (gsram_d != gsram_ck) {
        gtmp = 0u;
        return;
    }
    gwarpsel = 0u;
    gcmdopen = 0u;
    gcmdsel = 0u;
    gportsel = 0u;
    gportcom = 0u;
    gporthag = 0u;
    gportmsg = 0u;
    gportfrom = 0u;
    gdept = 0u;
    gdocksel = 0u;
    gdockmsg = 0u;
    gcolsel = 0u;
    gplsel = 0u;
    gplmsg = 0u;
    gmsgmode = 0u;
    gm1 = "";
    gm2 = "";
    gm3 = "";
    gm4 = "";
    gm1pal = PAL_WHITE;
    gdemo = 0u;
    gtmp = 1u;
}
static void show_loadret(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 5u; gdx = 8u; gdstr = "RETURNING TRADER"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 8u;  gdx = 6u; gdstr = "TRADER"; draw_text();
    gdy = 10u; gdx = 6u; gdstr = "SHIP";   draw_text();
    gdpal = PAL_CYAN;
    gdy = 8u;  gdx = 14u; gdstr = gname; draw_text();
    gdy = 10u; gdx = 14u; gdstr = gship; draw_text();
    gdpal = PAL_GRAY;
    gdy = 12u; gdx = 6u; gdstr = "SECTOR";  draw_text();
    gdy = 13u; gdx = 6u; gdstr = "CREDITS"; draw_text();
    gdy = 14u; gdx = 6u; gdstr = "TURNS";   draw_text();
    gdpal = PAL_WHITE;
    gdy = 12u; gdx = 20u; gn = gsec; draw_num();
    gdy = 13u; gdx = 20u; gn = gcredits; draw_num();
    gdy = 14u; gdx = 20u; gn = gturns; draw_num();
    gdpal = PAL_GRAY;
    gdy = 24u; gdx = 8u; gdstr = "A:RESUME B:MENU"; draw_text();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_LOADRET;
    gframe = 0u;
}
static void draw_optval(void) {
    gdx = 20u;
    if (gosel == 0u) {
        if (gopt0 == 0u) { gdstr = "SLOW"; }
        else if (gopt0 == 1u) { gdstr = "NORMAL"; }
        else { gdstr = "FAST"; }
    } else if (gosel == 1u) {
        if (gopt1 == 0u) { gdstr = "500 SECTORS"; }
        else if (gopt1 == 1u) { gdstr = "1000 SECTORS"; }
        else { gdstr = "2000 SECTORS"; }
    } else if (gosel == 2u) {
        if (gopt2 == 0u) { gdstr = "5000"; }
        else if (gopt2 == 1u) { gdstr = "10000"; }
        else { gdstr = "20000"; }
    } else {
        if (gopt3 == 0u) { gdstr = "CALM"; }
        else if (gopt3 == 1u) { gdstr = "NORMAL"; }
        else { gdstr = "NASTY"; }
    }
    draw_text();
}
static void show_options_keep(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 5u; gdx = 12u; gdstr = "OPTIONS"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 10u; gdx = 6u; gdstr = "TURN RATE"; draw_text();
    gdy = 11u; gdx = 6u; gdstr = "UNIVERSE";  draw_text();
    gdy = 12u; gdx = 6u; gdstr = "CREDITS";   draw_text();
    gdy = 13u; gdx = 6u; gdstr = "FERRENGI";  draw_text();
    gdpal = PAL_WHITE;
    gp = gosel;
    gosel = 0u;
    gdy = 10u; draw_optval();
    gosel = 1u;
    gdy = 11u; draw_optval();
    gosel = 2u;
    gdy = 12u; draw_optval();
    gosel = 3u;
    gdy = 13u; draw_optval();
    gosel = gp;
    gdpal = PAL_CYAN;
    gdx = 4u;
    gdy = (uint8_t)(10u + gosel);
    gch[0] = '>';
    gch[1] = 0;
    gdstr = gch;
    draw_text();
    gdpal = PAL_GRAY;
    gdy = 24u; gdx = 6u; gdstr = "L/R:CHANGE B:BACK"; draw_text();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_OPTIONS;
    gframe = 0u;
}
static void show_options(void) {
    gosel = 0u;
    show_options_keep();
}
static void opt_dec(void) {
    if (gosel == 0u) { if (gopt0 > 0u) gopt0--; }
    else if (gosel == 1u) { if (gopt1 > 0u) gopt1--; }
    else if (gosel == 2u) { if (gopt2 > 0u) gopt2--; }
    else { if (gopt3 > 0u) gopt3--; }
}
static void opt_inc(void) {
    if (gosel == 0u) { if (gopt0 < 2u) gopt0++; }
    else if (gosel == 1u) { if (gopt1 < 2u) gopt1++; }
    else if (gosel == 2u) { if (gopt2 < 2u) gopt2++; }
    else { if (gopt3 < 2u) gopt3++; }
}
static void show_name(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    if (gentry == 0u) {
        gdy = 8u; gdx = 10u; gdstr = "TRADER NAME:"; draw_text();
        gbuf = gname;
    } else {
        gdy = 8u; gdx = 11u; gdstr = "SHIP NAME:"; draw_text();
        gbuf = gship;
    }
    gr = 0u;
    while (gr < 8u) {
        gc = gbuf[gr];
        if (gc == 32u) gc = 95u;
        gch[0] = gc;
        gch[1] = 0;
        gdstr = gch;
        gdx = (uint8_t)(12u + gr);
        gdy = 12u;
        gdpal = PAL_WHITE;
        draw_text();
        gr++;
    }
    gdx = (uint8_t)(12u + gslot);
    gdy = 13u;
    gdpal = PAL_CYAN;
    gch[0] = '^';
    gch[1] = 0;
    gdstr = gch;
    draw_text();
    gdpal = PAL_GRAY;
    gdy = 21u; gdx = 9u;  gdstr = "UP/DN:LETTER";  draw_text();
    gdy = 22u; gdx = 8u;  gdstr = "L/R:MOVE A:NEXT"; draw_text();
    gdy = 23u; gdx = 8u;  gdstr = "START:OK B:BACK"; draw_text();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    if (gentry == 0u) { gstate = ST_NEWNAME; }
    else { gstate = ST_NEWSHIP; }
    gframe = 0u;
}
static void name_index(void) {
    gci = 0u;
    while (gci < 37u) {
        if (charset[gci] == gbuf[gslot]) break;
        gci++;
    }
    if (gci >= 37u) gci = 0u;
}
static void name_confirm(void) {
    if (gentry == 0u) {
        gentry = 1u;
        gslot = 0u;
        show_name();
    } else {
        show_sum();
    }
}
static void show_sum(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 5u; gdx = 10u; gdstr = "NEW TRADER"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 8u;  gdx = 6u; gdstr = "TRADER"; draw_text();
    gdy = 10u; gdx = 6u; gdstr = "SHIP";   draw_text();
    gdpal = PAL_WHITE;
    gdy = 8u;  gdx = 14u; gdstr = gname; draw_text();
    gdy = 10u; gdx = 14u; gdstr = gship; draw_text();
    gdpal = PAL_GRAY;
    gdy = 12u; gdx = 6u; gdstr = "TURN RATE"; draw_text();
    gdy = 13u; gdx = 6u; gdstr = "UNIVERSE";  draw_text();
    gdy = 14u; gdx = 6u; gdstr = "CREDITS";   draw_text();
    gdy = 15u; gdx = 6u; gdstr = "FERRENGI";  draw_text();
    gdpal = PAL_WHITE;
    gosel = 0u;
    gdy = 12u; draw_optval();
    gosel = 1u;
    gdy = 13u; draw_optval();
    gosel = 2u;
    gdy = 14u; draw_optval();
    gosel = 3u;
    gdy = 15u; draw_optval();
    gdpal = PAL_GRAY;
    gdy = 24u; gdx = 8u; gdstr = "A:LAUNCH B:BACK"; draw_text();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_NEWSUM;
    gframe = 0u;
}
static void show_launch(void) {
    if (gopt2 == 0u) { gcredits = 5000u; }
    else if (gopt2 == 1u) { gcredits = 10000u; }
    else { gcredits = 20000u; }
    if (gopt0 == 0u) { gturns = 250u; }
    else if (gopt0 == 1u) { gturns = 500u; }
    else { gturns = 1000u; }
    gfighters = 30u;
    gshields = 0u;
    gholds = 5u;
    gholdmax = 20u;
    galign = 0;
    gxp = 0u;
    gore = 5u;
    gorg = 0u;
    gequ = 0u;
    gday = 0u;
    gsec = 1u;
    gwarpsel = 0u;
    gcmdopen = 0u;
    gcmdsel = 0u;
    gportsel = 0u;
    gportcom = 0u;
    gporthag = 0u;
    gportmsg = 0u;
    gportfrom = 0u;
    gdept = 0u;
    gdocksel = 0u;
    gdockmsg = 0u;
    ghubonce = 0u;
    ghubug = 0u;
    ghubfence = 0u;
    gbank = 0u;
    gprobes = 0u;
    gbeacons = 0u;
    gtorp = 0u;
    gcomm = 0u;
    gphotons = 0u;
    gbankday = 0u;
    gcitadel = 0u;
    gcolore = 0u;
    gcolorg = 0u;
    gcolequ = 0u;
    gpftrs = 0u;
    gpsh = 0u;
    gqsec = 0u;
    gqatm = 0u;
    gfuel = 0u;
    gcolship = 0u;
    gadet = 1u;
    gcolsec = 0u;
    gcolsel = 0u;
    gplsel = 0u;
    gplmsg = 0u;
    gmsgmode = 0u;
    gm1 = "";
    gm2 = "";
    gm3 = "";
    gm4 = "";
    gm1pal = PAL_WHITE;
    gdemo = 0u;
    gi = 0u;
    while (gi < 64u) {
        gvisited[gi] = 0u;
        gi++;
    }
    sram_sync();
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 6u; gdx = 10u; gdstr = "SECTOR 001"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 9u;  gdx = 6u; gdstr = "TRADER"; draw_text();
    gdy = 11u; gdx = 6u; gdstr = "SHIP";   draw_text();
    gdpal = PAL_CYAN;
    gdy = 9u;  gdx = 14u; gdstr = gname; draw_text();
    gdy = 11u; gdx = 14u; gdstr = gship; draw_text();
    gdpal = PAL_GRAY;
    gdy = 15u; gdx = 6u;  gdstr = "CLEARANCE GRANTED";  draw_text();
    gdy = 16u; gdx = 8u;  gdstr = "A:ENTER SECTOR 1";   draw_text();
    gdy = 24u; gdx = 12u; gdstr = "B:BACK"; draw_text();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_LAUNCH;
    gframe = 0u;
}
/* ---- Milestone 3: sector view (TradeWars 2002 main loop) ---- */
static void univ_size(void) {
    if (gopt1 == 0u) { gtmp = 500u; }
    else if (gopt1 == 1u) { gtmp = 1000u; }
    else { gtmp = 2000u; }
}
static void sec_seed(void) {
    ghash = gsec;
    ghash ^= (uint16_t)(ghash << 7);
    ghash ^= (uint16_t)(ghash >> 5);
    ghash ^= (uint16_t)(ghash << 3);
}
static void sec_next(void) {
    ghash ^= (uint16_t)(ghash << 7);
    ghash ^= (uint16_t)(ghash >> 5);
    ghash ^= (uint16_t)(ghash << 3);
    ghash += 0x3D65u;
}
static void gen_sector(void) {
    univ_size();
    guniv = gtmp;
    if (gsec == 1u) {
        gwcount = 4u;
        gwarps[0] = 2u;
        gwarps[1] = 3u;
        gwarps[2] = 4u;
        gwarps[3] = 5u;
        gwarps[4] = 0u;
        gwarps[5] = 0u;
        gport = 1u;
        gportcls = 8u;
        gplanet = 1u;
        gplevel = 5u;
        gftrs = 0u;
        return;
    }
    sec_seed();
    gwcount = (uint8_t)(2u + ((ghash >> 12) & 3u));
    if (gwcount > 4u) gwcount = 4u;
    gi = 0u;
    while (gi < gwcount) {
        sec_next();
        gtmp = ghash;
        while (gtmp >= guniv) gtmp -= guniv;
        gwarps[gi] = (uint16_t)(gtmp + 1u);
        if (gwarps[gi] == gsec) {
            gwarps[gi]++;
            if (gwarps[gi] > guniv) gwarps[gi] = 1u;
        }
        gi++;
    }
    sec_next();
    if ((ghash & 15u) < 6u) { gport = 1u; } else { gport = 0u; }
    gportcls = (uint8_t)((ghash >> 5) & 7u);
    sec_next();
    if (((ghash >> 4) & 15u) < 5u) { gplanet = 1u; } else { gplanet = 0u; }
    gplevel = (uint8_t)(1u + ((ghash >> 9) & 3u));
    sec_next();
    if (((ghash >> 3) & 15u) < 3u) { gftrs = (uint8_t)(1u + ((ghash >> 7) & 15u)); }
    else { gftrs = 0u; }
}
static void mark_visited(void) {
    gtmp = (uint16_t)((gsec - 1u) & 511u);
    gk = (uint8_t)(gtmp & 7u);
    gvisited[gtmp >> 3] |= bitmask[gk];
}
static void is_visited(void) {
    gtmp = (uint16_t)((gsec - 1u) & 511u);
    gk = (uint8_t)(gtmp & 7u);
    if (gvisited[gtmp >> 3] & bitmask[gk]) { gj = 1u; }
    else { gj = 0u; }
}
static void neb_name(void) {
    gtmp = (uint16_t)((gsec >> 2) & 7u);
    if (gtmp >= 5u) gtmp -= 5u;
    if (gtmp == 0u) { gdstr = "UNCHARTED"; }
    else if (gtmp == 1u) { gdstr = "SOL"; }
    else if (gtmp == 2u) { gdstr = "TARTERUS"; }
    else if (gtmp == 3u) { gdstr = "ORION DEEP"; }
    else { gdstr = "VEGA DRIFT"; }
}
static void port_name(void) {
    gtmp = (uint16_t)(gsec & 7u);
    if (gtmp == 0u) { gdstr = "HUYGENS"; }
    else if (gtmp == 1u) { gdstr = "STARDOCK"; }
    else if (gtmp == 2u) { gdstr = "XPORT"; }
    else if (gtmp == 3u) { gdstr = "RIGEL"; }
    else if (gtmp == 4u) { gdstr = "VEGA"; }
    else if (gtmp == 5u) { gdstr = "ORION"; }
    else if (gtmp == 6u) { gdstr = "KEPLER"; }
    else { gdstr = "TERRA"; }
}
static void cls_str(void) {
    if (gportcls == 0u) { gdstr = "BBS"; }
    else if (gportcls == 1u) { gdstr = "BSB"; }
    else if (gportcls == 2u) { gdstr = "SBB"; }
    else if (gportcls == 3u) { gdstr = "SSB"; }
    else if (gportcls == 4u) { gdstr = "SBS"; }
    else if (gportcls == 5u) { gdstr = "BSS"; }
    else if (gportcls == 6u) { gdstr = "SSS"; }
    else if (gportcls == 7u) { gdstr = "BBB"; }
    else { gdstr = "S"; }
}
static void fmt_num(void) {
    gdiv = 10000u;
    gstarted = 0u;
    gk = 0u;
    while (gdiv > 0u) {
        gdigit = 0u;
        while (gn >= gdiv) {
            gn -= gdiv;
            gdigit++;
        }
        if (gdigit > 0u || gdiv == 1u || gstarted) {
            gnum[gk] = (char)(48u + gdigit);
            gk++;
            gstarted = 1u;
        }
        if (gdiv == 10000u) { gdiv = 1000u; }
        else if (gdiv == 1000u) { gdiv = 100u; }
        else if (gdiv == 100u) { gdiv = 10u; }
        else if (gdiv == 10u) { gdiv = 1u; }
        else { gdiv = 0u; }
    }
    gnum[gk] = 0;
}
static void draw_num(void) {
    fmt_num();
    gdstr = gnum;
    draw_text();
}
static void draw_sec_head(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_CYAN;
    gdstr = "SECTOR "; draw_text();
    gn = gsec; draw_num();
    gdstr = ":"; draw_text();
    neb_name(); draw_text();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
    gdx = 0u; gdy = 2u; gdpal = PAL_WHITE;
    gdstr = "WARPS:"; draw_text();
    gi = 0u;
    while (gi < gwcount) {
        if (gdx >= 24u) { gdy++; gdx = 7u; }
        if (gi == gwarpsel) { gdpal = PAL_CYAN; gdstr = ">"; }
        else { gdpal = PAL_WHITE; gdstr = " "; }
        draw_text();
        gsav = gsec;
        gsec = gwarps[gi];
        is_visited();
        gsec = gsav;
        if (gj == 0u) { gdpal = PAL_RED; }
        else if (gi == gwarpsel) { gdpal = PAL_CYAN; }
        else { gdpal = PAL_WHITE; }
        gn = gwarps[gi];
        draw_num();
        gi++;
    }
    gdx = 0u; gdy = 4u; gdpal = PAL_GRAY;
    gdstr = "PORT:"; draw_text();
    if (gport) {
        gdpal = PAL_YEL; port_name(); draw_text();
        gdpal = PAL_WHITE; gdstr = " CLS "; draw_text();
        cls_str(); draw_text();
    } else {
        gdpal = PAL_GRAY; gdstr = "NONE"; draw_text();
    }
    gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
    gdstr = "PLANET:"; draw_text();
    if (gplanet) {
        gdpal = PAL_YEL; port_name(); draw_text();
        gdpal = PAL_WHITE; gdstr = " L"; draw_text();
        gn = gplevel; draw_num();
    } else {
        gdpal = PAL_GRAY; gdstr = "NONE"; draw_text();
    }
    gdx = 0u; gdy = 6u; gdpal = PAL_GRAY;
    gdstr = "FTRS:"; draw_text();
    if (gftrs) {
        gdpal = PAL_RED; gn = gftrs; draw_num();
        gdstr = " HOSTILE"; draw_text();
    } else {
        gdpal = PAL_GRAY; gdstr = "CLEAR"; draw_text();
    }
    gdy = 7u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_msgs(void) {
    if (gmsgmode == 1u) {
        gdx = 0u; gdy = 8u; gdpal = PAL_CYAN;
        gdstr = "WARP COMPLETE"; draw_text();
        gdx = 0u; gdy = 9u; gdpal = PAL_WHITE;
        gdstr = "SECTOR "; draw_text();
        gn = gsec; draw_num();
        gdx = 0u; gdy = 10u; gdpal = PAL_WHITE;
        gdstr = "TURNS LEFT: "; draw_text();
        gn = gturns; draw_num();
        return;
    }
    if (gmsgmode == 2u) {
        gdx = 0u; gdy = 8u; gdpal = PAL_CYAN;
        gdstr = "DENSITY SCAN:"; draw_text();
        gi = 0u;
        gdy = 9u;
        while (gi < gwcount) {
            gdx = 0u; gdpal = PAL_WHITE;
            gdstr = "SEC "; draw_text();
            gn = gwarps[gi]; draw_num();
            gdstr = ":"; draw_text();
            ghash = gwarps[gi];
            ghash ^= (uint16_t)(ghash << 7);
            ghash ^= (uint16_t)(ghash >> 5);
            gtmp = (uint16_t)((ghash >> 3) & 127u);
            if (gtmp >= 100u) gtmp -= 100u;
            gn = gtmp; draw_num();
            if (gtmp > 85u) {
                gdpal = PAL_RED; gdstr = " HAZ"; draw_text();
            }
            gdy++;
            gi++;
        }
        return;
    }
    if (gmsgmode == 3u) {
        gdx = 0u; gdy = 8u; gdpal = PAL_CYAN;
        gdstr = "HOLO-SCAN:"; draw_text();
        gdx = 0u; gdy = 9u; gdpal = PAL_WHITE;
        gdstr = "PORT "; draw_text();
        if (gport) {
            port_name(); draw_text();
            gdstr = " "; draw_text();
            cls_str(); draw_text();
        } else {
            gdstr = "NONE"; draw_text();
        }
        gdx = 0u; gdy = 10u; gdpal = PAL_WHITE;
        gdstr = "PLANET "; draw_text();
        if (gplanet) {
            port_name(); draw_text();
            gdstr = " L"; draw_text();
            gn = gplevel; draw_num();
        } else {
            gdstr = "NONE"; draw_text();
        }
        gdx = 0u; gdy = 11u;
        if (gftrs) {
            gdpal = PAL_RED;
            gdstr = "FTRS "; draw_text();
            gn = gftrs; draw_num();
            gdstr = " HOSTILE"; draw_text();
        } else {
            gdpal = PAL_GRAY; gdstr = "NO HOSTILES"; draw_text();
        }
        return;
    }
    if (gmsgmode == 4u) {
        gdx = 0u; gdy = 8u; gdpal = PAL_CYAN;
        gdstr = "COURSE:SEC 1 STARDOCK"; draw_text();
        if (gsec == 1u) { gtmp = 0u; }
        else { gtmp = (uint16_t)(1u + (gsec & 3u)); }
        gdx = 0u; gdy = 9u; gdpal = PAL_WHITE;
        gdstr = "HOPS "; draw_text();
        gn = gtmp; draw_num();
        gn = gtmp;
        gn += gtmp;
        gn += gtmp;
        gdx = 0u; gdy = 10u; gdpal = PAL_WHITE;
        gdstr = "FUEL "; draw_text();
        draw_num();
        gdstr = " TURNS"; draw_text();
        return;
    }
    gdx = 0u; gdy = 8u; gdpal = gm1pal;
    gdstr = gm1; draw_text();
    gdx = 0u; gdy = 9u; gdpal = PAL_WHITE;
    gdstr = gm2; draw_text();
    gdx = 0u; gdy = 10u; gdpal = PAL_WHITE;
    gdstr = gm3; draw_text();
    gdx = 0u; gdy = 11u; gdpal = PAL_WHITE;
    gdstr = gm4; draw_text();
}
static void draw_status(void) {
    gdx = 0u; gdy = 13u; gdpal = PAL_GRAY;
    gdstr = "CR "; draw_text();
    gdpal = PAL_WHITE; gn = gcredits; draw_num();
    gdpal = PAL_GRAY; gdstr = " TR "; draw_text();
    gdpal = PAL_WHITE; gn = gturns; draw_num();
    gdx = 0u; gdy = 14u; gdpal = PAL_GRAY;
    gdstr = "FTR "; draw_text();
    gdpal = PAL_WHITE; gn = gfighters; draw_num();
    gdpal = PAL_GRAY; gdstr = " SH "; draw_text();
    gdpal = PAL_WHITE; gn = gshields; draw_num();
    gdpal = PAL_GRAY; gdstr = " HL "; draw_text();
    gdpal = PAL_WHITE; gn = gholds; draw_num();
    gdstr = "/"; draw_text();
    gn = gholdmax; draw_num();
    gdx = 0u; gdy = 15u; gdpal = PAL_GRAY;
    gdstr = "AL "; draw_text();
    gdpal = PAL_WHITE;
    if (galign < 0) {
        gdstr = "-"; draw_text();
        gn = (uint16_t)(0 - galign);
    } else {
        gn = (uint16_t)galign;
    }
    draw_num();
    gdpal = PAL_GRAY; gdstr = " XP "; draw_text();
    gdpal = PAL_WHITE; gn = gxp; draw_num();
    gdy = 16u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_cmdarea(void) {
    if (gcmdopen) {
        gdx = 0u; gdy = 17u; gdpal = PAL_CYAN;
        gdstr = "COMMANDS:"; draw_text();
        gi = 0u;
        while (gi < 8u) {
            gdx = 4u; gdy = (uint8_t)(18u + gi);
            if (gi == gcmdsel) {
                gdpal = PAL_CYAN; gdstr = ">"; draw_text();
                gdpal = PAL_YEL;
            } else {
                gdpal = PAL_WHITE; gdstr = " "; draw_text();
                gdpal = PAL_WHITE;
            }
            gdx = 6u;
            if (gi == 0u) { gdstr = "D REDISPLAY"; }
            else if (gi == 1u) { gdstr = "H HOLO-SCAN"; }
            else if (gi == 2u) { gdstr = "S DENSITY"; }
            else if (gi == 3u) { gdstr = "C COURSE PLOT"; }
            else if (gi == 4u) { gdstr = "P PORT"; }
            else if (gi == 5u) { gdstr = "L LAND"; }
            else if (gi == 6u) { gdstr = "A ATTACK"; }
            else { gdstr = "Q QUIT"; }
            draw_text();
            gi++;
        }
        return;
    }
    gdpal = PAL_GRAY;
    gdy = 18u; gdx = 2u; gdstr = "A/START:WARP SECTOR"; draw_text();
    gdy = 19u; gdx = 2u; gdstr = "D-PAD:SELECT WARP"; draw_text();
    gdy = 20u; gdx = 2u; gdstr = "X:COMMANDS Y:SCAN"; draw_text();
    gdy = 21u; gdx = 2u; gdstr = "B:MENU SEL:HELP"; draw_text();
}
static void draw_secprompt(void) {
    gdx = 0u; gdy = 25u; gdpal = PAL_WHITE;
    gdstr = "CMD:["; draw_text();
    gn = gsec; draw_num();
    gdstr = "] (?=HELP)? :"; draw_text();
    gdx = 0u; gdy = 26u; gdpal = PAL_WHITE;
    gdstr = "WARP>"; draw_text();
    if (gwcount > 0u) {
        gn = gwarps[gwarpsel]; draw_num();
    }
    gdstr = " A:GO X:CMDS"; draw_text();
}
static void show_sector(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    gen_sector();
    mark_visited();
    if (gplanet) {
        if (gcolsec != gsec) {
            planet_fresh();
        }
    }
    if (gwarpsel >= gwcount) gwarpsel = 0u;
    draw_sec_head();
    draw_msgs();
    draw_status();
    draw_cmdarea();
    draw_secprompt();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    if (gdemo) { gstate = ST_ATTRACT; }
    else { gstate = ST_SECTOR; }
    gframe = 0u;
}
static void show_attract(void) {
    gdemo = 1u;
    gsec = 1u;
    gwarpsel = 0u;
    gcmdopen = 0u;
    gmsgmode = 0u;
    gm1 = "DEMO: STARDOCK SECTOR";
    gm1pal = PAL_YEL;
    gm2 = "A TRADEWARS TRIBUTE";
    gm3 = "";
    gm4 = "";
    show_sector();
}
static void do_warp(void) {
    if (gturns == 0u) {
        gmsgmode = 0u;
        gm1 = "NO TURNS LEFT";
        gm1pal = PAL_RED;
        gm2 = "REST AT CITADEL (SOON)";
        gm3 = "";
        gm4 = "";
        gcmdopen = 0u;
        show_sector();
        return;
    }
    gturns--;
    gday++;
    if (gplanet) {
        gcolsec = gsec;
        if (gcitadel > 0u) {
            gfuel += (uint16_t)(gcolore >> 3);
            if (gfuel > 9999u) gfuel = 9999u;
            gpftrs += gcitadel;
            if (gpftrs > 9999u) gpftrs = 9999u;
        }
    }
    gsec = gwarps[gwarpsel];
    gwarpsel = 0u;
    gcmdopen = 0u;
    gmsgmode = 1u;
    sram_sync();
    show_sector();
}
static void exec_cmd(void) {
    gcmdopen = 0u;
    if (gcmdsel == 0u) {
        gmsgmode = 0u;
        gm1 = ""; gm2 = ""; gm3 = ""; gm4 = "";
        show_sector();
    } else if (gcmdsel == 1u) {
        gmsgmode = 3u;
        show_sector();
    } else if (gcmdsel == 2u) {
        gmsgmode = 2u;
        show_sector();
    } else if (gcmdsel == 3u) {
        gmsgmode = 4u;
        show_sector();
    } else if (gcmdsel == 4u) {
        gcmdopen = 0u;
        if (gsec == 1u) {
            dock_hub();
        } else if (gport) {
            gportfrom = 0u;
            port_dock();
        } else {
            gmsgmode = 0u;
            gm1 = "NO PORT IN SECTOR";
            gm1pal = PAL_RED;
            gm2 = "FIND BBS/BSB/SBB PORTS";
            gm3 = "";
            gm4 = "";
            show_sector();
        }
    } else if (gcmdsel == 5u) {
        gcmdopen = 0u;
        if (gplanet) {
            gplsel = 0u;
            gplmsg = 0u;
            gm1 = "";
            gm2 = "";
            gm3 = "";
            gm4 = "";
            show_planet();
        } else if (gtorp > 0u) {
            planet_genesis();
        } else {
            gmsgmode = 0u;
            gm1 = "NO PLANET HERE";
            gm1pal = PAL_RED;
            gm2 = "NEED GENESIS TORP (5000)";
            gm3 = "";
            gm4 = "";
            show_sector();
        }
    } else if (gcmdsel == 6u) {
        gcmdopen = 0u;
        if (gftrs > 0u) {
            glastsec = gsec;
            gen_enemy();
            gfightsel = 0u;
            gfightover = 0u;
            gm1 = "ENGAGING ENEMY!";
            gm1pal = PAL_RED;
            gm2 = "PREPARE FOR BATTLE";
            gm3 = "";
            gm4 = "";
            show_fight();
        } else {
            gmsgmode = 0u;
            gm1 = "NO HOSTILES HERE";
            gm1pal = PAL_GRAY;
            gm2 = "SECTOR IS CLEAR";
            gm3 = "";
            gm4 = "";
            show_sector();
        }
    } else {
        show_menu();
    }
}
static void tick_title(void) {
    wait_vblank();
    gframe++;
    if ((gframe & 31u) == 0u) {
        if (gshow) { gshow = 0u; }
        else { gshow = 1u; }
        draw_prompt();
    }
    read_pads();
    if (gj_new & (KEY_START | KEY_A)) {
        gsel = 0u;
        show_menu();
        return;
    }
    if (gframe >= 600u) {
        show_attract();
        return;
    }
}
static void tick_attract(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (KEY_A | KEY_B | KEY_START)) {
        gdemo = 0u;
        show_title();
        return;
    }
    if (gframe >= 600u) {
        gdemo = 0u;
        show_title();
        return;
    }
}
static void tick_launch(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (KEY_A | KEY_START)) {
        gmsgmode = 0u;
        gm1 = "";
        gm2 = "";
        gm3 = "";
        gm4 = "";
        gm1pal = PAL_WHITE;
        show_sector();
        return;
    }
    if (gj_new & KEY_B) {
        show_sum();
        return;
    }
}
static void tick_loadret(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (KEY_A | KEY_START)) {
        show_sector();
        return;
    }
    if (gj_new & KEY_B) {
        show_menu();
        return;
    }
}
static void tick_sector(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gcmdopen) {
        if (gj_dir & KEY_UP) {
            if (gcmdsel == 0u) { gcmdsel = 7u; }
            else { gcmdsel--; }
            show_sector();
            return;
        }
        if (gj_dir & KEY_DOWN) {
            gcmdsel++;
            if (gcmdsel >= 8u) gcmdsel = 0u;
            show_sector();
            return;
        }
        if (gj_new & (KEY_A | KEY_START)) {
            exec_cmd();
            return;
        }
        if (gj_new & (KEY_B | KEY_X)) {
            gcmdopen = 0u;
            show_sector();
            return;
        }
        return;
    }
    if (gj_dir & (KEY_UP | KEY_LEFT)) {
        if (gwarpsel == 0u) { gwarpsel = (uint8_t)(gwcount - 1u); }
        else { gwarpsel--; }
        show_sector();
        return;
    }
    if (gj_dir & (KEY_DOWN | KEY_RIGHT)) {
        gwarpsel++;
        if (gwarpsel >= gwcount) gwarpsel = 0u;
        show_sector();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        do_warp();
        return;
    }
    if (gj_new & KEY_X) {
        gcmdopen = 1u;
        gcmdsel = 0u;
        show_sector();
        return;
    }
    if (gj_new & KEY_Y) {
        gmsgmode = 2u;
        show_sector();
        return;
    }
    if (gj_new & KEY_B) {
        show_menu();
        return;
    }
    if (gj_new & KEY_SELECT) {
        gmsgmode = 0u;
        gm1 = "D REDISPLAY H HOLO S SCAN";
        gm1pal = PAL_WHITE;
        gm2 = "C COURSE P PORT L LAND";
        gm3 = "A ATTACK Q QUIT TO MENU";
        gm4 = "";
        show_sector();
        return;
    }
}
/* ---- Milestone 4: starport trading (TW2002 port loop) ----
 * Port classes trade Fuel Ore / Organics / Equipment. Per-commodity
 * side letter: S = port sells (you BUY), B = port buys (you SELL).
 * Docking costs 1 turn. Haggle shifts prices once per visit (+5 XP).
 * Steal takes port stock (evil path: alignment hit, bust risk).
 */
static void port_side(void) {
    if (gportcom == 0u) { gj = (uint8_t)((clsides[gportcls] >> 2) & 1u); }
    else if (gportcom == 1u) { gj = (uint8_t)((clsides[gportcls] >> 1) & 1u); }
    else { gj = (uint8_t)(clsides[gportcls] & 1u); }
}
static void com_name(void) {
    if (gportcom == 0u) { gdstr = "FUEL ORE"; }
    else if (gportcom == 1u) { gdstr = "ORGANICS"; }
    else { gdstr = "EQUIPMENT"; }
}
static void cargo_have(void) {
    if (gportcom == 0u) { gn = gore; }
    else if (gportcom == 1u) { gn = gorg; }
    else { gn = gequ; }
}
static void cargo_add(void) {
    if (gportcom == 0u) { gore++; }
    else if (gportcom == 1u) { gorg++; }
    else { gequ++; }
    gholds++;
}
static void cargo_sub(void) {
    if (gportcom == 0u) { gore--; }
    else if (gportcom == 1u) { gorg--; }
    else { gequ--; }
    gholds--;
}
static void port_price(void) {
    gn = gprc[gportcom];
    if (gporthag == 0u) return;
    port_side();
    if (gj == 0u) {
        gn -= (uint16_t)(gn >> 3);
        if (gn == 0u) gn = 1u;
    } else {
        gn += (uint16_t)(gn >> 3);
    }
}
static void port_genrow(void) {
    sec_next();
    if (gportcom == 0u) {
        gprc[0] = (uint8_t)(18u + (ghash & 15u));
        gtmp = (uint16_t)(20u + ((ghash >> 6) & 63u));
    } else if (gportcom == 1u) {
        gprc[1] = (uint8_t)(10u + (ghash & 7u));
        gtmp = (uint16_t)(20u + ((ghash >> 6) & 63u));
    } else {
        gprc[2] = (uint8_t)(45u + (ghash & 31u));
        gtmp = (uint16_t)(20u + ((ghash >> 6) & 63u));
    }
    if (gday >= 40u) { gtmp += 40u; }
    else { gtmp += gday; }
    if (gtmp > 99u) { gtmp = 99u; }
    gstock[gportcom] = (uint8_t)gtmp;
}
static void draw_porthead(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_GRAY;
    gdstr = "PORT "; draw_text();
    gdpal = PAL_YEL; port_name(); draw_text();
    gdpal = PAL_GRAY; gdstr = " CLS "; draw_text();
    gdpal = PAL_WHITE; cls_str(); draw_text();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_portrows(void) {
    gi = 0u;
    while (gi < 3u) {
        gsav = gportcom;
        gportcom = gi;
        port_side();
        gtmp = gj;
        if (gi == 0u) { gdstr = "FUEL ORE"; }
        else if (gi == 1u) { gdstr = "ORGANICS"; }
        else { gdstr = "EQUIPMENT"; }
        gdx = 0u; gdy = (uint8_t)(2u + gi);
        if (gi == gsav) { gdpal = PAL_YEL; }
        else { gdpal = PAL_GRAY; }
        draw_text();
        gdx = 11u;
        if (gtmp == 0u) { gdpal = PAL_CYAN; gdstr = "S"; }
        else { gdpal = PAL_YEL; gdstr = "B"; }
        draw_text();
        gdx = 13u; gdpal = PAL_WHITE;
        gn = gprc[gi]; draw_num();
        if (gtmp == 0u) {
            gdstr = " STK"; draw_text();
            gn = gstock[gi]; draw_num();
        }
        gportcom = (uint8_t)gsav;
        gi++;
    }
}
static void draw_portstatus(void) {
    gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
    gdstr = "YOU F"; draw_text();
    gdpal = PAL_WHITE; gn = gore; draw_num();
    gdpal = PAL_GRAY; gdstr = " O"; draw_text();
    gdpal = PAL_WHITE; gn = gorg; draw_num();
    gdpal = PAL_GRAY; gdstr = " E"; draw_text();
    gdpal = PAL_WHITE; gn = gequ; draw_num();
    gdpal = PAL_GRAY; gdstr = " CR "; draw_text();
    gdpal = PAL_WHITE; gn = gcredits; draw_num();
    gdy = 6u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_portmsgs(void) {
    if (gportmsg == 1u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_WHITE;
        gdstr = "BOUGHT 1 "; draw_text();
        com_name(); draw_text();
        gdx = 0u; gdy = 8u;
        gdstr = "PAID "; draw_text();
        port_price(); draw_num();
        gdstr = " CR"; draw_text();
        gdx = 0u; gdy = 9u; gdpal = PAL_GRAY;
        gdstr = "HOLDS "; draw_text();
        gdpal = PAL_WHITE; gn = gholds; draw_num();
        gdstr = "/"; draw_text();
        gn = gholdmax; draw_num();
        return;
    }
    if (gportmsg == 2u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_WHITE;
        gdstr = "SOLD 1 "; draw_text();
        com_name(); draw_text();
        gdx = 0u; gdy = 8u;
        gdstr = "GAINED "; draw_text();
        port_price(); draw_num();
        gdstr = " CR XP +1"; draw_text();
        return;
    }
    if (gportmsg == 3u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_YEL;
        gdstr = "HAGGLE ACCEPTED"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "PRICES SHIFT IN FAVOR"; draw_text();
        gdx = 0u; gdy = 9u; gdpal = PAL_GRAY;
        gdstr = "XP +5 TRADE BONUS"; draw_text();
        return;
    }
    if (gportmsg == 4u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_YEL;
        gdstr = "STOLE "; draw_text();
        gn = gportn; draw_num();
        gdstr = " "; draw_text();
        com_name(); draw_text();
        gdstr = "!"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_RED;
        gdstr = "ALIGN -5 XP +10"; draw_text();
        return;
    }
    if (gportmsg == 5u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_RED;
        gdstr = "BUSTED BY PORT AUTHORITY!"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "FINE 500 CR ALIGN -20"; draw_text();
        return;
    }
    gdx = 0u; gdy = 7u; gdpal = gm1pal;
    gdstr = gm1; draw_text();
    gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
    gdstr = gm2; draw_text();
}
static void draw_portopts(void) {
    gi = 0u;
    while (gi < 5u) {
        gdx = 2u; gdy = (uint8_t)(12u + gi);
        if (gi == gportsel) {
            gdpal = PAL_CYAN; gdstr = ">"; draw_text();
            gdpal = PAL_YEL;
        } else {
            gdpal = PAL_WHITE; gdstr = " "; draw_text();
            gdpal = PAL_WHITE;
        }
        gdx = 4u;
        if (gi == 0u) { gdstr = "B BUY CARGO"; }
        else if (gi == 1u) { gdstr = "S SELL CARGO"; }
        else if (gi == 2u) { gdstr = "H HAGGLE"; }
        else if (gi == 3u) { gdstr = "R STEAL"; }
        else { gdstr = "L LEAVE PORT"; }
        draw_text();
        gi++;
    }
}
static void draw_portfoot(void) {
    gdpal = PAL_GRAY;
    gdy = 18u; gdx = 0u; gdstr = "COM:"; draw_text();
    gdpal = PAL_WHITE; com_name(); draw_text();
    gdpal = PAL_GRAY; gdstr = " L/R SELECT"; draw_text();
    gdy = 19u; gdx = 0u; gdstr = "A:DO IT B:UNDOCK"; draw_text();
}
static void show_port(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_porthead();
    draw_portrows();
    draw_portstatus();
    draw_portmsgs();
    draw_portopts();
    draw_portfoot();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_PORT;
    gframe = 0u;
}
static void port_dock(void) {
    if (gturns == 0u) {
        gmsgmode = 0u;
        gm1 = "NEED 1 TURN TO DOCK";
        gm1pal = PAL_RED;
        gm2 = "WARP AROUND FIRST";
        gm3 = "";
        gm4 = "";
        gcmdopen = 0u;
        show_sector();
        return;
    }
    gturns--;
    gportcom = 0u;
    port_genrow();
    gportcom = 1u;
    port_genrow();
    gportcom = 2u;
    port_genrow();
    gportcom = 0u;
    gportsel = 0u;
    gporthag = 0u;
    gportmsg = 0u;
    gm1 = "WELCOME, TRADER";
    gm1pal = PAL_WHITE;
    gm2 = "ONE TURN DEDUCTED";
    gcmdopen = 0u;
    sram_sync();
    show_port();
}
static void port_buy(void) {
    port_side();
    if (gj == 1u) {
        gportmsg = 0u;
        gm1 = "PORT WON'T SELL THAT";
        gm1pal = PAL_RED;
        gm2 = "CHECK B/S CLASS LETTERS";
        show_port();
        return;
    }
    if (gstock[gportcom] == 0u) {
        gportmsg = 0u;
        gm1 = "OUT OF STOCK";
        gm1pal = PAL_RED;
        gm2 = "TRY ANOTHER PORT";
        show_port();
        return;
    }
    port_price();
    gsav = gn;
    if (gcredits < gsav) {
        gportmsg = 0u;
        gm1 = "NOT ENOUGH CREDITS";
        gm1pal = PAL_RED;
        gm2 = "SELL CARGO FIRST";
        show_port();
        return;
    }
    if (gholds >= gholdmax) {
        gportmsg = 0u;
        gm1 = "HOLDS FULL";
        gm1pal = PAL_RED;
        gm2 = "SELL OR UPGRADE (SOON)";
        show_port();
        return;
    }
    gcredits -= gsav;
    cargo_add();
    gstock[gportcom]--;
    gxp++;
    gportmsg = 1u;
    sram_sync();
    show_port();
}
static void port_sell(void) {
    port_side();
    if (gj == 0u) {
        gportmsg = 0u;
        gm1 = "PORT WON'T BUY THAT";
        gm1pal = PAL_RED;
        gm2 = "CHECK B/S CLASS LETTERS";
        show_port();
        return;
    }
    cargo_have();
    if (gn == 0u) {
        gportmsg = 0u;
        gm1 = "NONE ABOARD TO SELL";
        gm1pal = PAL_RED;
        gm2 = "YOUR HOLDS ARE EMPTY";
        show_port();
        return;
    }
    port_price();
    gsav = gn;
    gcredits += gsav;
    cargo_sub();
    gxp++;
    gportmsg = 2u;
    sram_sync();
    show_port();
}
static void port_haggle(void) {
    if (gporthag) {
        gportmsg = 0u;
        gm1 = "ALREADY HAGGLED";
        gm1pal = PAL_RED;
        gm2 = "ONE HAGGLE PER VISIT";
        show_port();
        return;
    }
    gporthag = 1u;
    gxp += 5u;
    gportmsg = 3u;
    sram_sync();
    show_port();
}
static void port_steal(void) {
    port_side();
    if (gj == 1u) {
        gportmsg = 0u;
        gm1 = "NOTHING TO STEAL";
        gm1pal = PAL_RED;
        gm2 = "PORT KEEPS NO STOCK";
        show_port();
        return;
    }
    if (gstock[gportcom] == 0u) {
        gportmsg = 0u;
        gm1 = "NOTHING TO STEAL";
        gm1pal = PAL_RED;
        gm2 = "STOCKROOM IS EMPTY";
        show_port();
        return;
    }
    if (gholds >= gholdmax) {
        gportmsg = 0u;
        gm1 = "HOLDS FULL";
        gm1pal = PAL_RED;
        gm2 = "SELL OR UPGRADE (SOON)";
        show_port();
        return;
    }
    gn = gstock[gportcom];
    if (gn > 2u) gn = 2u;
    gtmp = (uint16_t)(gholdmax - gholds);
    if (gn > gtmp) gn = gtmp;
    gportn = (uint8_t)gn;
    cargo_add();
    if (gportn >= 2u) { cargo_add(); }
    gstock[gportcom] -= gportn;
    gtmp = (uint16_t)((gday + gsec) & 3u);
    if (gtmp == 3u) {
        if (gcredits >= 500u) { gcredits -= 500u; }
        else { gcredits = 0u; }
        galign -= 20;
        gxp++;
        gportmsg = 5u;
    } else {
        galign -= 5;
        gxp += 10u;
        gportmsg = 4u;
    }
    sram_sync();
    show_port();
}
static void port_leave(void) {
    gcmdopen = 0u;
    sram_sync();
    if (gportfrom == 1u) {
        gdockmsg = 0u;
        gm1 = "BACK AT CONCOURSE";
        gm1pal = PAL_YEL;
        gm2 = "PICK A DEPARTMENT";
        show_dock();
        return;
    }
    gmsgmode = 0u;
    gm1 = "UNDOCKED: FLY SAFE, TRADER";
    gm1pal = PAL_WHITE;
    gm2 = "";
    gm3 = "";
    gm4 = "";
    show_sector();
}
static void tick_port(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & KEY_UP) {
        if (gportsel == 0u) { gportsel = 4u; }
        else { gportsel--; }
        show_port();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gportsel++;
        if (gportsel >= 5u) gportsel = 0u;
        show_port();
        return;
    }
    if (gj_dir & KEY_LEFT) {
        if (gportcom == 0u) { gportcom = 2u; }
        else { gportcom--; }
        show_port();
        return;
    }
    if (gj_dir & KEY_RIGHT) {
        gportcom++;
        if (gportcom >= 3u) gportcom = 0u;
        show_port();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        if (gportsel == 0u) { port_buy(); }
        else if (gportsel == 1u) { port_sell(); }
        else if (gportsel == 2u) { port_haggle(); }
        else if (gportsel == 3u) { port_steal(); }
        else { port_leave(); }
        return;
    }
    if (gj_new & KEY_B) {
        port_leave();
        return;
    }
}
/* ---- Milestone 5: Stardock hub (sector 1 Class 9 station) ----
 * Hub departments: trading post (commodity port), shipyard (Class 0:
 * holds/fighters/shields), hardware (probes/beacons/genesis), bank
 * (deposit/withdraw/ledger + interest), police (commission/bounty),
 * underground (evil contracts), tavern (ale/gossip/library).
 */
static void cap_credits(void) {
    if (gcredits > 60000u) gcredits = 60000u;
}
static void dock_count(void) {
    if (gdept == 0u) { gtmp = 8u; }
    else if (gdept == 2u) { gtmp = 5u; }
    else { gtmp = 4u; }
}
static void draw_dockhead(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_CYAN;
    gdstr = "STARDOCK SECTOR 1"; draw_text();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
    gdx = 0u; gdy = 2u; gdpal = PAL_GRAY;
    gdstr = "CR "; draw_text();
    gdpal = PAL_WHITE; gn = gcredits; draw_num();
    gdpal = PAL_GRAY; gdstr = " TR "; draw_text();
    gdpal = PAL_WHITE; gn = gturns; draw_num();
    gdx = 0u; gdy = 3u; gdpal = PAL_GRAY;
    gdstr = "BANK "; draw_text();
    gdpal = PAL_WHITE; gn = gbank; draw_num();
    gdpal = PAL_GRAY; gdstr = " FTRS "; draw_text();
    gdpal = PAL_WHITE; gn = gfighters; draw_num();
    gdy = 4u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_dockmsgs(void) {
    if (gdockmsg == 1u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "HOLDS +5 MAX "; draw_text();
        gn = gholdmax; draw_num();
        return;
    }
    if (gdockmsg == 2u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "+5 FIGHTERS TOT "; draw_text();
        gn = gfighters; draw_num();
        return;
    }
    if (gdockmsg == 3u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "+5 SHIELDS TOT "; draw_text();
        gn = gshields; draw_num();
        return;
    }
    if (gdockmsg == 4u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "E-PROBE ABOARD TOT "; draw_text();
        gn = gprobes; draw_num();
        return;
    }
    if (gdockmsg == 5u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "BEACON ABOARD TOT "; draw_text();
        gn = gbeacons; draw_num();
        return;
    }
    if (gdockmsg == 7u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "DEPOSITED BAL "; draw_text();
        gn = gbank; draw_num();
        return;
    }
    if (gdockmsg == 8u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "WITHDREW BAL "; draw_text();
        gn = gbank; draw_num();
        return;
    }
    if (gdockmsg == 20u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "PHOTON ABOARD TOT "; draw_text();
        gn = gphotons; draw_num();
        return;
    }
    if (gdockmsg == 9u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
        gdstr = "BAL "; draw_text();
        gdpal = PAL_WHITE; gn = gbank; draw_num();
        gdpal = PAL_GRAY; gdstr = " INT 3PCT/DAY"; draw_text();
        return;
    }
    if (gdockmsg == 10u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_YEL;
        gdstr = "FEDERATION COMMISSION"; draw_text();
        gdx = 0u; gdy = 6u; gdpal = PAL_WHITE;
        gdstr = "ISS ACCESS (MILESTONE 7)"; draw_text();
        return;
    }
    if (gdockmsg == 11u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "BOUNTY PAID "; draw_text();
        gn = gtmp; draw_num();
        gdstr = " CR XP +5"; draw_text();
        return;
    }
    if (gdockmsg == 12u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
        gdstr = "ALIGN "; draw_text();
        gdpal = PAL_WHITE;
        if (galign < 0) {
            gdstr = "-"; draw_text();
            gn = (uint16_t)(0 - galign);
        } else {
            gn = (uint16_t)galign;
        }
        draw_num();
        gdpal = PAL_GRAY; gdstr = " XP "; draw_text();
        gdpal = PAL_WHITE; gn = gxp; draw_num();
        return;
    }
    if (gdockmsg == 13u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_YEL;
        gdstr = "CONTRACT DONE XP +25"; draw_text();
        gdx = 0u; gdy = 6u; gdpal = PAL_RED;
        gdstr = "THE UG REMEMBERS YOU"; draw_text();
        return;
    }
    if (gdockmsg == 14u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "FENCED HOT GOODS +150"; draw_text();
        return;
    }
    if (gdockmsg == 15u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "YOU LAY LOW A WHILE"; draw_text();
        return;
    }
    if (gdockmsg == 16u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_WHITE;
        gdstr = "ALE WARMS THE CREW XP+1"; draw_text();
        return;
    }
    if (gdockmsg == 17u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_YEL;
        gtmp = (uint16_t)((gday + gsec) & 3u);
        if (gtmp == 0u) { gdstr = "GRIMY: FIGS WIN WARS"; }
        else if (gtmp == 1u) { gdstr = "TRICRON: ORE IS KING"; }
        else if (gtmp == 2u) { gdstr = "DOC: PROBES SAVE LIVES"; }
        else { gdstr = "ALIENS BEYOND SECTOR 5"; }
        draw_text();
        gtmp = (uint16_t)((gday + gsec) & 7u);
        if (gtmp == 0u) {
            gdx = 0u; gdy = 6u; gdpal = PAL_RED;
            gdstr = "MUGGED IN ALLEY -100 CR"; draw_text();
            if (gcredits >= 100u) { gcredits -= 100u; }
            else { gcredits = 0u; }
        }
        return;
    }
    if (gdockmsg == 18u) {
        gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
        gdstr = "ISS: NEED COMMISSION"; draw_text();
        gdx = 0u; gdy = 6u; gdpal = PAL_GRAY;
        gdstr = "FREIGHTER 60HL CRUISER 20HL"; draw_text();
        gdx = 0u; gdy = 7u; gdpal = PAL_GRAY;
        gdstr = "CORVETTE 85HL ESCORT 12HL"; draw_text();
        return;
    }
    gdx = 0u; gdy = 5u; gdpal = gm1pal;
    gdstr = gm1; draw_text();
    gdx = 0u; gdy = 6u; gdpal = PAL_WHITE;
    gdstr = gm2; draw_text();
}
static void draw_dockopts(void) {
    gi = 0u;
    dock_count();
    gj = (uint8_t)gtmp;
    while (gi < gj) {
        gdx = 2u; gdy = (uint8_t)(9u + gi);
        if (gi == gdocksel) {
            gdpal = PAL_CYAN; gdstr = ">"; draw_text();
            gdpal = PAL_YEL;
        } else {
            gdpal = PAL_WHITE; gdstr = " "; draw_text();
            gdpal = PAL_WHITE;
        }
        gdx = 4u;
        if (gdept == 0u) {
            if (gi == 0u) { gdstr = "TRADING POST"; }
            else if (gi == 1u) { gdstr = "SHIPYARD"; }
            else if (gi == 2u) { gdstr = "HARDWARE"; }
            else if (gi == 3u) { gdstr = "GALACTIC BANK"; }
            else if (gi == 4u) { gdstr = "FED POLICE"; }
            else if (gi == 5u) { gdstr = "UNDERGROUND"; }
            else if (gi == 6u) { gdstr = "TAVERN"; }
            else { gdstr = "LEAVE STARDOCK"; }
        } else if (gdept == 1u) {
            if (gi == 0u) { gdstr = "BUY HOLDS 5000"; }
            else if (gi == 1u) { gdstr = "BUY FIGHTERS 500"; }
            else if (gi == 2u) { gdstr = "BUY SHIELDS 1000"; }
            else { gdstr = "BACK"; }
        } else if (gdept == 2u) {
            if (gi == 0u) { gdstr = "BUY PROBE 500"; }
            else if (gi == 1u) { gdstr = "BUY BEACON 100"; }
            else if (gi == 2u) { gdstr = "BUY GENESIS 5000"; }
            else if (gi == 3u) { gdstr = "BUY PHOTON 500"; }
            else { gdstr = "BACK"; }
        } else if (gdept == 3u) {
            if (gi == 0u) { gdstr = "DEPOSIT 1000"; }
            else if (gi == 1u) { gdstr = "WITHDRAW 1000"; }
            else if (gi == 2u) { gdstr = "LEDGER"; }
            else { gdstr = "BACK"; }
        } else if (gdept == 4u) {
            if (gi == 0u) { gdstr = "COMMISSION"; }
            else if (gi == 1u) { gdstr = "BOUNTY"; }
            else if (gi == 2u) { gdstr = "RECORD"; }
            else { gdstr = "BACK"; }
        } else if (gdept == 5u) {
            if (gi == 0u) { gdstr = "SEE BOSS"; }
            else if (gi == 1u) { gdstr = "FENCE 150"; }
            else if (gi == 2u) { gdstr = "LAY LOW"; }
            else { gdstr = "BACK"; }
        } else {
            if (gi == 0u) { gdstr = "ALE 10"; }
            else if (gi == 1u) { gdstr = "GOSSIP"; }
            else if (gi == 2u) { gdstr = "LIBRARY"; }
            else { gdstr = "BACK"; }
        }
        draw_text();
        gi++;
    }
}
static void draw_dockfoot(void) {
    gdpal = PAL_GRAY;
    gdy = 18u; gdx = 0u;
    if (gdept == 0u) { gdstr = "A:ENTER B:SECTOR"; }
    else { gdstr = "A:DO B:HUB"; }
    draw_text();
    gdy = 19u; gdx = 0u;
    if (gdept == 0u) { gdstr = "TRADING POST=PORT MARKET"; }
    else if (gdept == 1u) { gdstr = "CLASS 0 OUTFITTER"; }
    else if (gdept == 2u) { gdstr = "STELLAR HARDWARE"; }
    else if (gdept == 3u) { gdstr = "3PCT DAILY NOMINAL"; }
    else if (gdept == 4u) { gdstr = "SERVE THE FEDERATION"; }
    else if (gdept == 5u) { gdstr = "EVIL ONLY BEYOND HERE"; }
    else { gdstr = "GRIMY TRADER KNOWS ALL"; }
    draw_text();
}
static void show_dock(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_dockhead();
    draw_dockmsgs();
    draw_dockopts();
    draw_dockfoot();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_DOCK;
    gframe = 0u;
}
static void dock_hub(void) {
    gtmp = (uint16_t)(gday - gbankday);
    gi = 0u;
    while (gtmp > 0u) {
        gbank += (uint16_t)(gbank >> 5);
        if (gbank > 60000u) gbank = 60000u;
        gtmp--;
        gi++;
        if (gi >= 32u) { gtmp = 0u; }
    }
    gbankday = gday;
    gdept = 0u;
    gdocksel = 0u;
    gdockmsg = 0u;
    ghubonce = 0u;
    ghubug = 0u;
    ghubfence = 0u;
    gm1 = "WELCOME TO STARDOCK";
    gm1pal = PAL_YEL;
    gm2 = "FEDERATION PROTECTED";
    gcmdopen = 0u;
    sram_sync();
    show_dock();
}
static void dock_buyholds(void) {
    if (gholdmax > 95u) {
        gdockmsg = 0u;
        gm1 = "HOLDS MAXED OUT";
        gm1pal = PAL_RED;
        gm2 = "NO ROOM TO EXPAND";
        show_dock();
        return;
    }
    if (gcredits < 5000u) {
        gdockmsg = 0u;
        gm1 = "NEED 5000 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "TRADE AT THE PORT FIRST";
        show_dock();
        return;
    }
    gcredits -= 5000u;
    gholdmax += 5u;
    gdockmsg = 1u;
    sram_sync();
    show_dock();
}
static void dock_buyftrs(void) {
    if (gcredits < 500u) {
        gdockmsg = 0u;
        gm1 = "NEED 500 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "FIGHTERS COST MONEY";
        show_dock();
        return;
    }
    gcredits -= 500u;
    gfighters += 5u;
    gdockmsg = 2u;
    sram_sync();
    show_dock();
}
static void dock_buyshields(void) {
    if (gcredits < 1000u) {
        gdockmsg = 0u;
        gm1 = "NEED 1000 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "SHIELDS COST MONEY";
        show_dock();
        return;
    }
    gcredits -= 1000u;
    gshields += 5u;
    gdockmsg = 3u;
    sram_sync();
    show_dock();
}
static void dock_buyprobe(void) {
    if (gprobes >= 99u) {
        gdockmsg = 0u;
        gm1 = "PROBE RACKS FULL";
        gm1pal = PAL_RED;
        gm2 = "LAUNCH SOME FIRST (M6)";
        show_dock();
        return;
    }
    if (gcredits < 500u) {
        gdockmsg = 0u;
        gm1 = "NEED 500 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "PROBES COST MONEY";
        show_dock();
        return;
    }
    gcredits -= 500u;
    gprobes++;
    gdockmsg = 4u;
    sram_sync();
    show_dock();
}
static void dock_buybeacon(void) {
    if (gbeacons >= 99u) {
        gdockmsg = 0u;
        gm1 = "BEACON RACKS FULL";
        gm1pal = PAL_RED;
        gm2 = "DEPLOY SOME FIRST (M6)";
        show_dock();
        return;
    }
    if (gcredits < 100u) {
        gdockmsg = 0u;
        gm1 = "NEED 100 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "BEACONS COST MONEY";
        show_dock();
        return;
    }
    gcredits -= 100u;
    gbeacons++;
    gdockmsg = 5u;
    sram_sync();
    show_dock();
}
static void dock_buygenesis(void) {
    if (gtorp >= 5u) {
        gdockmsg = 0u;
        gm1 = "TORP MAGAZINE FULL";
        gm1pal = PAL_RED;
        gm2 = "FIRE SOME FIRST (M6)";
        show_dock();
        return;
    }
    if (gcredits < 5000u) {
        gdockmsg = 0u;
        gm1 = "NEED 5000 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "GENESIS AINT CHEAP";
        show_dock();
        return;
    }
    gcredits -= 5000u;
    gtorp++;
    gdockmsg = 6u;
    gm1 = "GENESIS TORP SECURED";
    gm1pal = PAL_YEL;
    gm2 = "USE ON EMPTY SECTOR (M6)";
    show_dock();
}
static void dock_buyphoton(void) {
    if (gphotons >= 3u) {
        gdockmsg = 0u;
        gm1 = "PHOTON RACKS FULL";
        gm1pal = PAL_RED;
        gm2 = "MAX 3 ABOARD";
        show_dock();
        return;
    }
    if (gcredits < 500u) {
        gdockmsg = 0u;
        gm1 = "NEED 500 CREDITS";
        gm1pal = PAL_RED;
        gm2 = "PHOTON MISSILE PRICE";
        show_dock();
        return;
    }
    gcredits -= 500u;
    gphotons++;
    gdockmsg = 20u;
    sram_sync();
    show_dock();
}
static void dock_deposit(void) {
    if (gcredits < 1000u) {
        gdockmsg = 0u;
        gm1 = "NEED 1000 TO DEPOSIT";
        gm1pal = PAL_RED;
        gm2 = "COME BACK RICHER";
        show_dock();
        return;
    }
    gcredits -= 1000u;
    gbank += 1000u;
    if (gbank > 60000u) gbank = 60000u;
    gdockmsg = 7u;
    sram_sync();
    show_dock();
}
static void dock_withdraw(void) {
    if (gbank < 1000u) {
        gdockmsg = 0u;
        gm1 = "BALANCE TOO LOW";
        gm1pal = PAL_RED;
        gm2 = "DEPOSIT FIRST";
        show_dock();
        return;
    }
    gbank -= 1000u;
    gcredits += 1000u;
    cap_credits();
    gdockmsg = 8u;
    sram_sync();
    show_dock();
}
static void dock_commission(void) {
    if (galign < 500) {
        gdockmsg = 0u;
        gm1 = "NEED 500 ALIGN FOR ISS";
        gm1pal = PAL_RED;
        gm2 = "SERVE, THEN RETURN";
        show_dock();
        return;
    }
    gcomm = 1u;
    gdockmsg = 10u;
    sram_sync();
    show_dock();
}
static void dock_bounty(void) {
    if (galign <= 0) {
        gdockmsg = 0u;
        gm1 = "NO FED STANDING";
        gm1pal = PAL_RED;
        gm2 = "GOOD DEEDS PAY (ALIGN+)";
        show_dock();
        return;
    }
    if (ghubonce) {
        gdockmsg = 0u;
        gm1 = "ALREADY CLAIMED";
        gm1pal = PAL_RED;
        gm2 = "ONE BOUNTY PER VISIT";
        show_dock();
        return;
    }
    ghubonce = 1u;
    gtmp = (uint16_t)(galign + galign);
    gcredits += gtmp;
    cap_credits();
    gxp += 5u;
    gdockmsg = 11u;
    sram_sync();
    show_dock();
}
static void dock_boss(void) {
    if (galign > -100) {
        gdockmsg = 0u;
        gm1 = "NEED EVIL ALIGN -100";
        gm1pal = PAL_RED;
        gm2 = "STEAL AND ROB FIRST";
        show_dock();
        return;
    }
    if (ghubug) {
        gdockmsg = 0u;
        gm1 = "NO MORE WORK TONIGHT";
        gm1pal = PAL_RED;
        gm2 = "COME BACK NEXT VISIT";
        show_dock();
        return;
    }
    ghubug = 1u;
    gxp += 25u;
    gdockmsg = 13u;
    sram_sync();
    show_dock();
}
static void dock_fence(void) {
    if (ghubfence) {
        gdockmsg = 0u;
        gm1 = "FENCE IS DRY";
        gm1pal = PAL_RED;
        gm2 = "ONE SALE PER VISIT";
        show_dock();
        return;
    }
    ghubfence = 1u;
    gcredits += 150u;
    cap_credits();
    gdockmsg = 14u;
    sram_sync();
    show_dock();
}
static void dock_laylow(void) {
    if (galign < 0) {
        galign += 5;
        if (galign > 0) galign = 0;
    }
    gdockmsg = 15u;
    sram_sync();
    show_dock();
}
static void dock_ale(void) {
    if (gcredits < 10u) {
        gdockmsg = 0u;
        gm1 = "NOT EVEN BAR MONEY";
        gm1pal = PAL_RED;
        gm2 = "10 CREDITS FOR ALE";
        show_dock();
        return;
    }
    gcredits -= 10u;
    gxp++;
    gdockmsg = 16u;
    sram_sync();
    show_dock();
}
static void dock_gossip(void) {
    gdockmsg = 17u;
    show_dock();
}
static void dock_library(void) {
    gdockmsg = 18u;
    show_dock();
}
static void dock_back(void) {
    gdocksel = gdept;
    gdept = 0u;
    gdockmsg = 0u;
    gm1 = "STARDOCK CONCOURSE";
    gm1pal = PAL_YEL;
    gm2 = "PICK A DEPARTMENT";
    show_dock();
}
static void dock_leave(void) {
    gmsgmode = 0u;
    gm1 = "LEFT STARDOCK: FLY SAFE";
    gm1pal = PAL_WHITE;
    gm2 = "";
    gm3 = "";
    gm4 = "";
    gcmdopen = 0u;
    sram_sync();
    show_sector();
}
static void dock_exec(void) {
    if (gdept == 0u) {
        if (gdocksel == 0u) {
            gportfrom = 1u;
            port_dock();
        } else if (gdocksel == 7u) {
            dock_leave();
        } else {
            gdept = (uint8_t)(gdocksel);
            gdocksel = 0u;
            gdockmsg = 0u;
            gm1 = "";
            gm2 = "";
            show_dock();
        }
        return;
    }
    if (gdept == 2u) {
        if (gdocksel == 4u) {
            dock_back();
            return;
        }
        if (gdocksel == 0u) { dock_buyprobe(); }
        else if (gdocksel == 1u) { dock_buybeacon(); }
        else if (gdocksel == 2u) { dock_buygenesis(); }
        else { dock_buyphoton(); }
        return;
    }
    if (gdocksel == 3u) {
        dock_back();
        return;
    }
    if (gdept == 1u) {
        if (gdocksel == 0u) { dock_buyholds(); }
        else if (gdocksel == 1u) { dock_buyftrs(); }
        else { dock_buyshields(); }
    } else if (gdept == 3u) {
        if (gdocksel == 0u) { dock_deposit(); }
        else if (gdocksel == 1u) { dock_withdraw(); }
        else {
            gdockmsg = 9u;
            show_dock();
        }
    } else if (gdept == 4u) {
        if (gdocksel == 0u) { dock_commission(); }
        else if (gdocksel == 1u) { dock_bounty(); }
        else {
            gdockmsg = 12u;
            show_dock();
        }
    } else if (gdept == 5u) {
        if (gdocksel == 0u) { dock_boss(); }
        else if (gdocksel == 1u) { dock_fence(); }
        else { dock_laylow(); }
    } else {
        if (gdocksel == 0u) { dock_ale(); }
        else if (gdocksel == 1u) { dock_gossip(); }
        else { dock_library(); }
    }
}
static void tick_dock(void) {
    wait_vblank();
    gframe++;
    read_pads();
    dock_count();
    if (gj_dir & KEY_UP) {
        if (gdocksel == 0u) { gdocksel = (uint8_t)(gtmp - 1u); }
        else { gdocksel--; }
        show_dock();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gdocksel++;
        if (gdocksel >= gtmp) gdocksel = 0u;
        show_dock();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        dock_exec();
        return;
    }
    if (gj_new & KEY_B) {
        if (gdept == 0u) {
            dock_leave();
        } else {
            dock_back();
        }
        return;
    }
}
/* ---- Milestone 6: planet management (citadels, colonists, cannon) ----
 * Single-colony model: planet record attaches to its sector via
 * gcolsec; entering a sector with a planet restores it when the
 * sectors match, else starts fresh (sector 1 seeds an abandoned
 * colony). Warp production ticks fuel/fighters for claimed worlds.
 */
static void planet_fresh(void) {
    gcitadel = 0u;
    gcolore = 0u;
    gcolorg = 0u;
    gcolequ = 0u;
    gpftrs = 0u;
    gpsh = 0u;
    gqsec = 0u;
    gqatm = 0u;
    gfuel = 0u;
    if (gsec == 1u) {
        gcolore = 120u;
        gcolorg = 80u;
        gcolequ = 60u;
        gfuel = 500u;
    }
    gcolsec = gsec;
}
static void col_group_name(void) {
    if (gcolsel == 0u) { gdstr = "ORE"; }
    else if (gcolsel == 1u) { gdstr = "ORG"; }
    else { gdstr = "EQU"; }
}
static void col_have(void) {
    if (gcolsel == 0u) { gn = gcolore; }
    else if (gcolsel == 1u) { gn = gcolorg; }
    else { gn = gcolequ; }
}
static void col_take(void) {
    if (gcolsel == 0u) { gcolore -= gtmp; }
    else if (gcolsel == 1u) { gcolorg -= gtmp; }
    else { gcolequ -= gtmp; }
    gcolship += gtmp;
    gholds += gtmp;
}
static void col_give(void) {
    if (gcolsel == 0u) { gcolore += gtmp; }
    else if (gcolsel == 1u) { gcolorg += gtmp; }
    else { gcolequ += gtmp; }
    gcolship -= gtmp;
    gholds -= gtmp;
}
static void draw_planhead(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_GRAY;
    gdstr = "PLANET "; draw_text();
    gdpal = PAL_YEL; port_name(); draw_text();
    gdpal = PAL_GRAY; gdstr = " LV "; draw_text();
    gdpal = PAL_WHITE; gn = gplevel; draw_num();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
    gdx = 0u; gdy = 2u; gdpal = PAL_GRAY;
    gdstr = "CIT "; draw_text();
    gdpal = PAL_WHITE;
    if (gcitadel == 0u) { gdstr = "NONE"; }
    else { gn = gcitadel; draw_num(); }
    if (gcitadel > 0u) {
        gdpal = PAL_GRAY; gdstr = " LV"; draw_text();
        gdpal = PAL_WHITE; gn = gplevel; draw_num();
    }
    gdx = 0u; gdy = 3u; gdpal = PAL_GRAY;
    gdstr = "COL O"; draw_text();
    gdpal = PAL_WHITE; gn = gcolore; draw_num();
    gdpal = PAL_GRAY; gdstr = " G"; draw_text();
    gdpal = PAL_WHITE; gn = gcolorg; draw_num();
    gdpal = PAL_GRAY; gdstr = " E"; draw_text();
    gdpal = PAL_WHITE; gn = gcolequ; draw_num();
    gdx = 0u; gdy = 4u; gdpal = PAL_GRAY;
    gdstr = "FTR "; draw_text();
    gdpal = PAL_WHITE; gn = gpftrs; draw_num();
    gdpal = PAL_GRAY; gdstr = " FUEL "; draw_text();
    gdpal = PAL_WHITE; gn = gfuel; draw_num();
    gdpal = PAL_GRAY; gdstr = " SH "; draw_text();
    gdpal = PAL_WHITE; gn = gpsh; draw_num();
    gdx = 0u; gdy = 5u; gdpal = PAL_GRAY;
    gdstr = "QSR "; draw_text();
    gdpal = PAL_WHITE; gn = gqsec; draw_num();
    gdstr = "/"; draw_text();
    gn = gqatm; draw_num();
    gdpal = PAL_GRAY; gdstr = " MRL 30 TW "; draw_text();
    gdpal = PAL_WHITE;
    if (gcitadel >= 4u) { gn = gplevel; }
    else { gn = 0u; }
    draw_num();
    gdy = 6u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_planmsgs(void) {
    if (gplmsg == 1u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_YEL;
        gdstr = "CLAIMED! CITADEL LV "; draw_text();
        gn = gcitadel; draw_num();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "XP +25 BUILD IT UP"; draw_text();
        return;
    }
    if (gplmsg == 3u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_WHITE;
        gdstr = "DEPLOYED FTRS TOT "; draw_text();
        gn = gpftrs; draw_num();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "SH TOT "; draw_text();
        gn = gpsh; draw_num();
        gdstr = " SHIP FTR "; draw_text();
        gn = gfighters; draw_num();
        return;
    }
    if (gplmsg == 4u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_WHITE;
        gdstr = "LOADED 10 "; draw_text();
        col_group_name(); draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "SHIP COLS "; draw_text();
        gn = gcolship; draw_num();
        gdstr = " HL "; draw_text();
        gn = gholds; draw_num();
        gdstr = "/"; draw_text();
        gn = gholdmax; draw_num();
        return;
    }
    if (gplmsg == 5u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_WHITE;
        gdstr = "UNLOADED 10 "; draw_text();
        col_group_name(); draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_GRAY;
        gdstr = "COLONY GROWS STRONGER"; draw_text();
        return;
    }
    if (gplmsg == 6u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_RED;
        gdstr = "JETTISONED ALIGN -10"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_GRAY;
        gdstr = "THE VOID KEEPS THEM"; draw_text();
        return;
    }
    if (gplmsg == 7u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_YEL;
        gdstr = "QUASAR SET "; draw_text();
        gn = gqsec; draw_num();
        gdstr = "PCT"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "FUEL STOCK "; draw_text();
        gn = gfuel; draw_num();
        return;
    }
    gdx = 0u; gdy = 7u; gdpal = gm1pal;
    gdstr = gm1; draw_text();
    gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
    gdstr = gm2; draw_text();
}
static void draw_planopts(void) {
    gi = 0u;
    while (gi < 8u) {
        gdx = 2u; gdy = (uint8_t)(12u + gi);
        if (gi == gplsel) {
            gdpal = PAL_CYAN; gdstr = ">"; draw_text();
            gdpal = PAL_YEL;
        } else {
            gdpal = PAL_WHITE; gdstr = " "; draw_text();
            gdpal = PAL_WHITE;
        }
        gdx = 4u;
        if (gi == 0u) {
            if (gcitadel == 0u) { gdstr = "CLAIM PLANET"; }
            else if (gcitadel >= 5u) { gdstr = "CITADEL MAX"; }
            else { gdstr = "UPGRADE CITADEL"; }
        }
        else if (gi == 1u) { gdstr = "DEPLOY FTRS"; }
        else if (gi == 2u) { gdstr = "LOAD COLS"; }
        else if (gi == 3u) { gdstr = "UNLOAD COLS"; }
        else if (gi == 4u) { gdstr = "JETTISON"; }
        else if (gi == 5u) { gdstr = "QUASAR SET"; }
        else if (gi == 6u) { gdstr = "DETONATE"; }
        else { gdstr = "LEAVE PLANET"; }
        draw_text();
        gi++;
    }
}
static void draw_planfoot(void) {
    gdpal = PAL_GRAY;
    gdy = 21u; gdx = 0u; gdstr = "GRP:"; draw_text();
    gdpal = PAL_WHITE; col_group_name(); draw_text();
    gdpal = PAL_GRAY; gdstr = " L/R SHIP "; draw_text();
    gdpal = PAL_WHITE; gn = gcolship; draw_num();
    gdstr = "/"; draw_text();
    gn = gholdmax; draw_num();
    gdy = 22u; gdx = 0u; gdstr = "A:DO IT B:SECTOR"; draw_text();
}
static void show_planet(void) {
    if (gplanet == 0u) {
        show_sector();
        return;
    }
    REG_INIDISP = 0x80u;
    clear_map();
    draw_planhead();
    draw_planmsgs();
    draw_planopts();
    draw_planfoot();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_PLANET;
    gframe = 0u;
}
static void plan_claim(void) {
    if (gcitadel >= 5u) {
        gplmsg = 0u;
        gm1 = "CITADEL MAXED";
        gm1pal = PAL_RED;
        gm2 = "L5 IS THE PINNACLE";
        show_planet();
        return;
    }
    if (gcitadel == 0u) {
        if (gturns < 5u) {
            gplmsg = 0u;
            gm1 = "NEED 5 TURNS TO CLAIM";
            gm1pal = PAL_RED;
            gm2 = "COME BACK LATER";
            show_planet();
            return;
        }
        gturns -= 5u;
        gcitadel = 1u;
        gxp += 25u;
        gplmsg = 1u;
        sram_sync();
        show_planet();
        return;
    }
    gtmp = citcost[gcitadel + 1u];
    if (gcredits < gtmp) {
        gplmsg = 0u;
        gm1 = "NOT ENOUGH CREDITS";
        gm1pal = PAL_RED;
        gm2 = "TRADE UP FIRST";
        show_planet();
        return;
    }
    gsav = gcolore + gcolorg + gcolequ;
    gtmp = 50u;
    gi = 0u;
    while (gi < gcitadel) {
        gtmp += 50u;
        gi++;
    }
    if (gsav < gtmp) {
        gplmsg = 0u;
        gm1 = "NEED MORE COLONISTS";
        gm1pal = PAL_RED;
        gm2 = "TRANSPORT SETTLERS IN";
        show_planet();
        return;
    }
    if (gturns == 0u) {
        gplmsg = 0u;
        gm1 = "NEED 1 TURN";
        gm1pal = PAL_RED;
        gm2 = "TO RAISE CITADEL";
        show_planet();
        return;
    }
    gcredits -= citcost[gcitadel + 1u];
    gcitadel++;
    gturns--;
    gplmsg = 1u;
    sram_sync();
    show_planet();
}
static void plan_deploy(void) {
    gtmp = 10u;
    if (gfighters < gtmp) gtmp = gfighters;
    gsav = 5u;
    if (gshields < gsav) gsav = gshields;
    if (gtmp == 0u && gsav == 0u) {
        gplmsg = 0u;
        gm1 = "NO SHIP FORCES";
        gm1pal = PAL_RED;
        gm2 = "BUY FTRS AT STARDOCK";
        show_planet();
        return;
    }
    gfighters -= gtmp;
    gpftrs += gtmp;
    if (gpftrs > 9999u) gpftrs = 9999u;
    gshields -= gsav;
    gpsh += gsav;
    if (gpsh > 5000u) gpsh = 5000u;
    gplmsg = 3u;
    sram_sync();
    show_planet();
}
static void plan_load(void) {
    col_have();
    gsav = (uint16_t)(gholdmax - gholds);
    if (gn < 10u || gsav < 10u) {
        gplmsg = 0u;
        if (gholds >= gholdmax) {
            gm1 = "HOLDS FULL";
        } else {
            gm1 = "NEED 10 COLONISTS";
        }
        gm1pal = PAL_RED;
        gm2 = "TRY ANOTHER GROUP";
        show_planet();
        return;
    }
    gtmp = 10u;
    col_take();
    gplmsg = 4u;
    sram_sync();
    show_planet();
}
static void plan_unload(void) {
    if (gcolship < 10u) {
        gplmsg = 0u;
        gm1 = "NEED 10 ABOARD";
        gm1pal = PAL_RED;
        gm2 = "LOAD FIRST";
        show_planet();
        return;
    }
    gtmp = 10u;
    col_give();
    gplmsg = 5u;
    sram_sync();
    show_planet();
}
static void plan_jettison(void) {
    if (gcolship == 0u) {
        gplmsg = 0u;
        gm1 = "HOLDS EMPTY";
        gm1pal = PAL_RED;
        gm2 = "NOTHING TO JETTISON";
        show_planet();
        return;
    }
    gholds -= gcolship;
    gcolship = 0u;
    galign -= 10;
    if (galign < -999) galign = -999;
    gplmsg = 6u;
    sram_sync();
    show_planet();
}
static void plan_quasar(void) {
    if (gfuel < 100u) {
        gplmsg = 0u;
        gm1 = "NEED 100 FUEL ORE";
        gm1pal = PAL_RED;
        gm2 = "PLANET STOCK EMPTY";
        show_planet();
        return;
    }
    gfuel -= 100u;
    gqsec += 20u;
    if (gqsec > 100u) gqsec = 0u;
    gqatm = gqsec;
    gplmsg = 7u;
    sram_sync();
    show_planet();
}
static void plan_detonate(void) {
    if (gadet == 0u) {
        gplmsg = 0u;
        gm1 = "NO DETONATORS LEFT";
        gm1pal = PAL_RED;
        gm2 = "THEY ARE ONE-SHOT";
        show_planet();
        return;
    }
    if (gturns == 0u) {
        gplmsg = 0u;
        gm1 = "NEED 1 TURN";
        gm1pal = PAL_RED;
        gm2 = "TO ARM THE DEVICE";
        show_planet();
        return;
    }
    gadet--;
    gturns--;
    gplanet = 0u;
    gcitadel = 0u;
    gcolore = 0u;
    gcolorg = 0u;
    gcolequ = 0u;
    gpftrs = 0u;
    gpsh = 0u;
    gqsec = 0u;
    gqatm = 0u;
    gfuel = 0u;
    gcolsec = 0u;
    gxp += 50u;
    galign -= 50;
    if (galign < -999) galign = -999;
    gmsgmode = 0u;
    gm1 = "PLANET DESTROYED +50XP";
    gm1pal = PAL_RED;
    gm2 = "ALIGN -50 THE FEDS NOTICE";
    gm3 = "";
    gm4 = "";
    gcmdopen = 0u;
    sram_sync();
    show_sector();
}
static void planet_genesis(void) {
    if (gturns == 0u) {
        gmsgmode = 0u;
        gm1 = "NEED 1 TURN";
        gm1pal = PAL_RED;
        gm2 = "TO LAUNCH TORPEDO";
        gm3 = "";
        gm4 = "";
        gcmdopen = 0u;
        show_sector();
        return;
    }
    gtorp--;
    gturns--;
    sec_next();
    gplanet = 1u;
    gplevel = (uint8_t)(1u + ((ghash >> 4) & 3u));
    gcitadel = 0u;
    gcolore = 0u;
    gcolorg = 0u;
    gcolequ = 0u;
    gpftrs = 0u;
    gpsh = 0u;
    gqsec = 0u;
    gqatm = 0u;
    gfuel = 0u;
    gcolsec = gsec;
    gxp += 25u;
    gmsgmode = 0u;
    gm1 = "GENESIS SUCCESS +25XP";
    gm1pal = PAL_YEL;
    gm2 = "A NEW WORLD AWAITS LAND";
    gm3 = "";
    gm4 = "";
    gcmdopen = 0u;
    sram_sync();
    show_sector();
}
static void plan_leave(void) {
    sram_sync();
    show_sector();
}
static void tick_planet(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & KEY_UP) {
        if (gplsel == 0u) { gplsel = 7u; }
        else { gplsel--; }
        show_planet();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gplsel++;
        if (gplsel >= 8u) gplsel = 0u;
        show_planet();
        return;
    }
    if (gj_dir & KEY_LEFT) {
        if (gcolsel == 0u) { gcolsel = 2u; }
        else { gcolsel--; }
        show_planet();
        return;
    }
    if (gj_dir & KEY_RIGHT) {
        gcolsel++;
        if (gcolsel >= 3u) gcolsel = 0u;
        show_planet();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        if (gplsel == 0u) { plan_claim(); }
        else if (gplsel == 1u) { plan_deploy(); }
        else if (gplsel == 2u) { plan_load(); }
        else if (gplsel == 3u) { plan_unload(); }
        else if (gplsel == 4u) { plan_jettison(); }
        else if (gplsel == 5u) { plan_quasar(); }
        else if (gplsel == 6u) { plan_detonate(); }
        else { plan_leave(); }
        return;
    }
    if (gj_new & KEY_B) {
        plan_leave();
        return;
    }
}
/* ---- Milestone 7: Combat System (TradeWars 2002 ship combat) ----
 * Enemy = the sector's fighter force (gftrs * 3, reinforced when
 * engaged) in a random ship class with class odds.
 * Combat math: (enemy_ftrs * enemy_odds) / your_odds = min fighters
 * to win. Photon missile strips shields and blinds defenses (odds
 * halved). Corbomite detonates mutually. Escape pod survives a
 * destroyed ship (buy a new one at the shipyard).
 */
static void ship_name(void) {
    if (gencls == 0u) { gdstr = "MER CRU"; }
    else if (gencls == 1u) { gdstr = "SCO MAR"; }
    else if (gencls == 2u) { gdstr = "MIS FRI"; }
    else if (gencls == 3u) { gdstr = "COR BAT"; }
    else if (gencls == 4u) { gdstr = "COR FLA"; }
    else if (gencls == 5u) { gdstr = "COL TRA"; }
    else if (gencls == 6u) { gdstr = "CAR TRA"; }
    else if (gencls == 7u) { gdstr = "MER FRE"; }
    else if (gencls == 8u) { gdstr = "IMP STA"; }
    else if (gencls == 9u) { gdstr = "HAV GUN"; }
    else if (gencls == 10u) { gdstr = "STA MAS"; }
    else if (gencls == 11u) { gdstr = "CON STE"; }
    else if (gencls == 12u) { gdstr = "TKH ORI"; }
    else if (gencls == 13u) { gdstr = "THO SEN"; }
    else { gdstr = "TAU MUL"; }
}
static void gen_enemy(void) {
    sec_seed();
    gencls = (uint8_t)((ghash >> 8) & 15u);
    if (gencls >= 15u) gencls = 14u;
    genftrs = (uint16_t)((uint16_t)gftrs * 3u);
    gensh = (uint16_t)((ghash & 15u) * 10u);
    gencorb = (uint8_t)((ghash >> 12) & 3u);
    gblind = 0u;
}
static void fight_odds(void) {
    gtmp = (uint16_t)shipoff[gencls];
    gn = (uint16_t)shipdef[gencls];
    gsav = (uint16_t)((genftrs * gtmp) / (gn == 0u ? 1u : gn));
    if (gblind) {
        gsav = (uint16_t)(gsav >> 1);
    }
    if (gsav < 1u) gsav = 1u;
    if (gsav > 9999u) gsav = 9999u;
}
static void draw_fighthead(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_RED;
    gdstr = "COMBAT: "; draw_text();
    gdpal = PAL_YEL; ship_name(); draw_text();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
    gdx = 0u; gdy = 2u; gdpal = PAL_GRAY;
    gdstr = "ENEMY FTRS "; draw_text();
    gdpal = PAL_WHITE; gn = genftrs; draw_num();
    gdpal = PAL_GRAY; gdstr = " SH "; draw_text();
    gdpal = PAL_WHITE; gn = gensh; draw_num();
    gdx = 0u; gdy = 3u; gdpal = PAL_GRAY;
    gdstr = "ODDS "; draw_text();
    gdpal = PAL_WHITE; gn = (uint16_t)shipoff[gencls]; draw_num();
    gdstr = "/"; draw_text();
    gn = (uint16_t)shipdef[gencls]; draw_num();
    gdx = 0u; gdy = 4u; gdpal = PAL_GRAY;
    gdstr = "YOUR FTRS "; draw_text();
    gdpal = PAL_WHITE; gn = gfighters; draw_num();
    gdpal = PAL_GRAY; gdstr = " SH "; draw_text();
    gdpal = PAL_WHITE; gn = gshields; draw_num();
    gdx = 0u; gdy = 5u; gdpal = PAL_BLUE;
    gdstr = rulerow; draw_text();
}
static void draw_fightmsgs(void) {
    if (gfightover == 1u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_YEL;
        gdstr = "ENEMY DESTROYED!"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        gdstr = "BOUNTY "; draw_text(); gn = gbounty; draw_num();
        gdstr = " CR XP +"; draw_text(); gn = (gbounty / 100u) + 5u; draw_num();
        return;
    }
    if (gfightover == 2u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_RED;
        gdstr = "YOU WERE DESTROYED!"; draw_text();
        gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
        if (gphotons > 0u) {
            gdstr = "PHOTON SAVES POD!"; draw_text();
        } else if (gencorb) {
            gdstr = "CORBOMITE RETALIATES!"; draw_text();
        } else {
            gdstr = "ESCAPE POD LAUNCHED"; draw_text();
        }
        return;
    }
    if (gfightover == 3u) {
        gdx = 0u; gdy = 7u; gdpal = PAL_CYAN;
        gdstr = "ESCAPED TO SECTOR "; draw_text();
        gn = glastsec; draw_num();
        return;
    }
    gdx = 0u; gdy = 7u; gdpal = gm1pal;
    gdstr = gm1; draw_text();
    gdx = 0u; gdy = 8u; gdpal = PAL_WHITE;
    gdstr = gm2; draw_text();
    gdx = 0u; gdy = 9u; gdpal = PAL_WHITE;
    gdstr = gm3; draw_text();
    gdx = 0u; gdy = 10u; gdpal = PAL_WHITE;
    gdstr = gm4; draw_text();
}
static void draw_fightopts(void) {
    gi = 0u;
    while (gi < 5u) {
        gdx = 2u; gdy = (uint8_t)(12u + gi);
        if (gi == gfightsel) {
            gdpal = PAL_CYAN; gdstr = ">"; draw_text();
            gdpal = PAL_YEL;
        } else {
            gdpal = PAL_WHITE; gdstr = " "; draw_text();
            gdpal = PAL_WHITE;
        }
        gdx = 4u;
        if (gi == 0u) { gdstr = "A ATTACK"; }
        else if (gi == 1u) { gdstr = "P PHOTON"; }
        else if (gi == 2u) { gdstr = "C CORBOMITE"; }
        else if (gi == 3u) { gdstr = "E ESCAPE"; }
        else { gdstr = "R RETREAT"; }
        draw_text();
        gi++;
    }
}
static void draw_fightfoot(void) {
    gdpal = PAL_GRAY;
    gdy = 18u; gdx = 0u; gdstr = "MIN TO WIN "; draw_text();
    fight_odds();
    gdpal = PAL_WHITE; gn = gsav; draw_num();
    gdy = 19u; gdx = 0u; gdstr = "A:DO IT B:SECTOR"; draw_text();
}
static void show_fight(void) {
    REG_INIDISP = 0x80u;
    clear_map();
    draw_fighthead();
    draw_fightmsgs();
    draw_fightopts();
    draw_fightfoot();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_FIGHT;
    gframe = 0u;
}
static void fight_attack(void) {
    fight_odds();
    if (gfighters >= gsav) {
        gfighters -= gsav;
        genftrs = 0u;
        gfightover = 1u;
        gbounty = (uint16_t)((uint16_t)gencls * 100u + 500u);
        gcredits += gbounty;
        gxp += (uint16_t)(gbounty / 100u) + 5u;
        gm1 = "VICTORY!";
        gm1pal = PAL_YEL;
        gm2 = "ENEMY VAPORIZED";
        gm3 = "";
        gm4 = "";
        sram_sync();
    } else {
        gfighters = 0u;
        gfightover = 2u;
        if (gphotons > 0u) {
            gphotons--;
            gm1 = "PHOTON USED!";
            gm2 = "POD ESCAPES, SHIP LOST";
            gm3 = "";
            gm4 = "";
        } else if (gencorb) {
            gencorb--;
            galign -= 10;
            if (galign < -999) galign = -999;
            gm1 = "CORBOMITE FIRES!";
            gm2 = "SHIP LOST ALIGN -10";
            gm3 = "";
            gm4 = "";
        } else {
            gm1 = "ESCAPE POD...";
            gm2 = "YOU SURVIVE, SHIP LOST";
            gm3 = "";
            gm4 = "";
        }
        sram_sync();
    }
    show_fight();
}
static void fight_photon(void) {
    if (gphotons == 0u) {
        gm1 = "NO PHOTONS";
        gm1pal = PAL_RED;
        gm2 = "BUY AT STARDOCK HARDWARE";
        gm3 = "";
        gm4 = "";
        show_fight();
        return;
    }
    gphotons--;
    gensh = 0u;
    gblind = 1u;
    gm1 = "PHOTON LAUNCHED!";
    gm1pal = PAL_YEL;
    gm2 = "ENEMY SHIELDS DOWN";
    gm3 = "DEFENSES BLINDED";
    gm4 = "";
    sram_sync();
    show_fight();
}
static void fight_corbomite(void) {
    if (gencorb == 0u) {
        gm1 = "NO CORBOMITE";
        gm1pal = PAL_RED;
        gm2 = "ENEMY HAS NONE";
        gm3 = "";
        gm4 = "";
        show_fight();
        return;
    }
    gencorb--;
    gfighters = (uint16_t)(gfighters / 2u);
    genftrs = (uint16_t)(genftrs / 2u);
    gm1 = "CORBOMITE DETONATED!";
    gm1pal = PAL_YEL;
    gm2 = "BOTH SIDES HALVED";
    gm3 = "";
    gm4 = "";
    show_fight();
}
static void fight_escape(void) {
    if (gfighters == 0u) {
        gm1 = "NO FIGHTERS LEFT";
        gm1pal = PAL_RED;
        gm2 = "CANNOT ESCAPE";
        gm3 = "";
        gm4 = "";
        show_fight();
        return;
    }
    gfighters = (uint16_t)(gfighters / 2u);
    gturns--;
    gsec = gwarps[0];
    if (gsec == 0u || gsec > guniv) gsec = 1u;
    glastsec = gsec;
    gfightover = 3u;
    gwarpsel = 0u;
    gcmdopen = 0u;
    sram_sync();
    show_fight();
}
static void fight_retreat(void) {
    gturns--;
    gfightover = 0u;
    gcmdopen = 0u;
    show_sector();
}
static void tick_fight(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gfightover) {
        if (gj_new & (KEY_A | KEY_START | KEY_B)) {
            if (gfightover == 1u) {
                gfightover = 0u;
                gcmdopen = 0u;
                show_sector();
            } else if (gfightover == 2u) {
                if (gphotons > 0u || gencorb) {
                    gfightover = 0u;
                    gcmdopen = 0u;
                    show_sector();
                } else {
                    show_menu();
                }
            } else {
                gfightover = 0u;
                gcmdopen = 0u;
                show_sector();
            }
        }
        return;
    }
    if (gj_dir & KEY_UP) {
        if (gfightsel == 0u) { gfightsel = 4u; }
        else { gfightsel--; }
        show_fight();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gfightsel++;
        if (gfightsel >= 5u) gfightsel = 0u;
        show_fight();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        if (gfightsel == 0u) { fight_attack(); }
        else if (gfightsel == 1u) { fight_photon(); }
        else if (gfightsel == 2u) { fight_corbomite(); }
        else if (gfightsel == 3u) { fight_escape(); }
        else { fight_retreat(); }
        return;
    }
    if (gj_new & KEY_B) {
        fight_retreat();
        return;
    }
}
static void tick_menu(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & KEY_UP) {
        if (gsel == 0u) { gsel = 3u; }
        else { gsel--; }
        show_menu();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gsel++;
        if (gsel >= 4u) gsel = 0u;
        show_menu();
        return;
    }
    if (gj_new & (KEY_A | KEY_START)) {
        if (gsel == 0u) {
            gentry = 0u;
            gslot = 0u;
            show_name();
        } else if (gsel == 1u) {
            show_continue();
        } else if (gsel == 2u) {
            gosel = 0u;
            show_options();
        } else {
            show_credits();
        }
        return;
    }
    if (gj_new & KEY_B) {
        show_title();
        return;
    }
}
static void tick_simple_back(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (KEY_A | KEY_B | KEY_START)) {
        show_menu();
        return;
    }
}
static void tick_options(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & KEY_UP) {
        if (gosel == 0u) { gosel = 3u; }
        else { gosel--; }
        show_options_keep();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        gosel++;
        if (gosel >= 4u) gosel = 0u;
        show_options_keep();
        return;
    }
    if (gj_dir & KEY_LEFT) {
        opt_dec();
        show_options_keep();
        return;
    }
    if (gj_dir & KEY_RIGHT) {
        opt_inc();
        show_options_keep();
        return;
    }
    if (gj_new & KEY_B) {
        show_menu();
        return;
    }
}
static void tick_name(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & KEY_UP) {
        name_index();
        gci++;
        if (gci >= 37u) gci = 0u;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
    if (gj_dir & KEY_DOWN) {
        name_index();
        if (gci == 0u) { gci = 37u; }
        gci--;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
    if (gj_dir & KEY_LEFT) {
        if (gslot > 0u) gslot--;
        show_name();
        return;
    }
    if (gj_dir & KEY_RIGHT) {
        if (gslot < 7u) gslot++;
        show_name();
        return;
    }
    if (gj_new & KEY_A) {
        if (gslot < 7u) {
            gslot++;
            show_name();
        } else {
            name_confirm();
        }
        return;
    }
    if (gj_new & KEY_START) {
        name_confirm();
        return;
    }
    if (gj_new & KEY_B) {
        if (gentry == 0u) {
            show_menu();
        } else {
            gentry = 0u;
            gslot = 0u;
            show_name();
        }
        return;
    }
    /* The only hold-repeat in the game: holding Up/Down on the name
     * screens scrolls letters. gframe counts frames since the last
     * redraw (every step redraws and zeroes it), so the first repeat
     * lands 25 frames after the press and the rest every 8 frames.
     * Releasing keys stops it; every other screen is edge-only. */
    if ((gj_pad & KEY_UP) && gframe >= 25u && ((gframe & 7u) == 1u)) {
        name_index();
        gci++;
        if (gci >= 37u) gci = 0u;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
    if ((gj_pad & KEY_DOWN) && gframe >= 25u && ((gframe & 7u) == 1u)) {
        name_index();
        if (gci == 0u) { gci = 37u; }
        gci--;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
}
static void tick_sum(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (KEY_A | KEY_START)) {
        show_launch();
        return;
    }
    if (gj_new & KEY_B) {
        gentry = 1u;
        gslot = 0u;
        show_name();
        return;
    }
}
int main(void) {
    REG_INIDISP = 0x80u;
    REG_BGMODE = 0x01u;
    REG_BG1SC = 0x68u;
    REG_BG12NBA = 0x03u;
    REG_VMAIN = 0x80u;
    REG_BG1HOFS = 0; REG_BG1HOFS = 0;
    REG_BG1VOFS = 0; REG_BG1VOFS = 0;
    REG_NMITIMEN = 0x81u;
    load_palettes();
    load_font();
    gopt0 = 1u;
    gopt1 = 1u;
    gopt2 = 1u;
    gopt3 = 1u;
    gselfdrive = 0u;
    gdemo = 0u;
    gcmdopen = 0u;
    gsi = 0u;
    gj_held = 0u;
    gr = 0u;
    while (gr < 8u) {
        gname[gr] = 32u;
        gship[gr] = 32u;
        gr++;
    }
    gname[8] = 0;
    gship[8] = 0;
    gch[1] = 0;
    gj_prev = 0u;
    show_title();
    while (1) {
        tick_once();
    }
}
static void tick_once(void) {
        if (gstate == ST_TITLE) {
            tick_title();
        } else if (gstate == ST_MENU) {
            tick_menu();
        } else if (gstate == ST_OPTIONS) {
            tick_options();
        } else if (gstate == ST_NEWNAME) {
            tick_name();
        } else if (gstate == ST_NEWSHIP) {
            tick_name();
        } else if (gstate == ST_NEWSUM) {
            tick_sum();
        } else if (gstate == ST_LAUNCH) {
            tick_launch();
        } else if (gstate == ST_LOADRET) {
            tick_loadret();
        } else if (gstate == ST_SECTOR) {
            tick_sector();
        } else if (gstate == ST_PORT) {
            tick_port();
        } else if (gstate == ST_DOCK) {
            tick_dock();
        } else if (gstate == ST_PLANET) {
            tick_planet();
        } else if (gstate == ST_FIGHT) {
            tick_fight();
        } else if (gstate == ST_ATTRACT) {
            tick_attract();
        } else {
            tick_simple_back();
        }
}
