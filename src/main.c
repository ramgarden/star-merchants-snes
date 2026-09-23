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
#define PB_UP 0x0008u
#define PB_DOWN 0x0004u
#define PB_LEFT 0x0002u
#define PB_RIGHT 0x0001u
#define PB_START 0x0010u
#define PB_SEL 0x0020u
#define PB_Y 0x0040u
#define PB_B 0x0080u
#define PB_R 0x1000u
#define PB_L 0x2000u
#define PB_X 0x4000u
#define PB_A 0x8000u
#define SCN 92u
#define SCN2 106u
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
void sram_wr(void);
void sram_rd(void);
const uint8_t palc1[16] = {
    0xFF, 0x7F, 0x10, 0x42, 0x1F, 0x00, 0x0C, 0x00,
    0x00, 0x7C, 0x00, 0x2C, 0xE0, 0x7F, 0xFF, 0x03,
};
const uint8_t mrows[4] = { 11u, 13u, 15u, 17u };
const char charset[38] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
const uint8_t bitmask[8] = { 1u, 2u, 4u, 8u, 16u, 32u, 64u, 128u };
const uint8_t clsides[9] = { 6u, 5u, 3u, 1u, 2u, 4u, 0u, 7u, 4u };
const uint8_t scf[92] = {
    3u, 4u, 9u, 10u, 15u, 16u, 21u, 22u, 27u, 28u,
    40u, 41u, 46u, 47u, 52u, 53u, 58u, 59u, 64u, 65u,
    70u, 71u, 76u, 77u, 82u, 83u, 88u, 89u, 94u, 95u,
    100u, 101u, 106u, 107u, 112u, 113u, 116u, 117u, 120u, 121u,
    124u, 125u, 128u, 129u, 132u, 133u, 136u, 137u, 140u, 141u,
    148u, 149u, 152u, 153u, 160u, 161u, 164u, 165u, 168u, 169u,
    176u, 177u, 180u, 181u, 184u, 185u, 188u, 189u, 196u, 197u,
    200u, 201u, 208u, 209u, 212u, 213u, 216u, 217u, 220u, 221u,
    224u, 225u, 228u, 229u, 232u, 233u, 236u, 237u, 240u, 241u,
    244u, 245u,
};
const uint8_t scp[92] = {
    1u, 0u, 3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u,
    7u, 0u, 2u, 0u, 2u, 0u, 6u, 0u, 7u, 0u,
    2u, 0u, 6u, 0u, 1u, 0u, 1u, 0u, 6u, 0u,
    3u, 0u, 2u, 0u, 6u, 0u, 8u, 0u, 3u, 0u,
    3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u, 6u, 0u,
    3u, 0u, 6u, 0u, 2u, 0u, 5u, 0u, 6u, 0u,
    3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u, 2u, 0u,
    6u, 0u, 3u, 0u, 3u, 0u, 6u, 0u, 2u, 0u,
    6u, 0u, 3u, 0u, 6u, 0u, 7u, 0u, 3u, 0u,
    6u, 0u,
};
const uint8_t scf2[106] = {
    3u, 4u, 9u, 10u, 15u, 16u, 21u, 22u, 27u, 28u,
    40u, 41u, 46u, 47u, 52u, 53u, 58u, 59u, 64u, 65u,
    70u, 71u, 76u, 77u, 82u, 83u, 88u, 89u, 94u, 95u,
    100u, 101u, 106u, 107u, 112u, 113u, 116u, 117u, 120u, 121u,
    124u, 125u, 128u, 129u, 132u, 133u, 136u, 137u, 140u, 141u,
    144u, 145u, 148u, 149u, 152u, 153u, 156u, 157u, 160u, 161u,
    164u, 165u, 168u, 169u, 172u, 173u, 176u, 177u, 180u, 181u,
    184u, 185u, 188u, 189u, 192u, 193u, 196u, 197u, 200u, 201u,
    204u, 205u, 208u, 209u, 212u, 213u, 216u, 217u, 220u, 221u,
    224u, 225u, 228u, 229u, 232u, 233u, 236u, 237u, 240u, 241u,
    244u, 245u, 248u, 249u, 252u, 253u,
};
const uint8_t scp2[106] = {
    1u, 0u, 3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u,
    7u, 0u, 2u, 0u, 2u, 0u, 6u, 0u, 7u, 0u,
    2u, 0u, 6u, 0u, 1u, 0u, 1u, 0u, 6u, 0u,
    3u, 0u, 2u, 0u, 6u, 0u, 8u, 0u, 3u, 0u,
    3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u, 3u, 0u,
    6u, 0u, 6u, 0u, 2u, 0u, 6u, 0u, 3u, 0u,
    6u, 0u, 6u, 0u, 2u, 0u, 6u, 0u, 3u, 0u,
    6u, 0u, 6u, 0u, 3u, 0u, 6u, 0u, 3u, 0u,
    6u, 0u, 3u, 0u, 6u, 0u, 6u, 0u, 3u, 0u,
    6u, 0u, 3u, 0u, 6u, 0u, 3u, 0u, 6u, 0u,
    3u, 0u, 6u, 0u, 6u, 0u,
};
static void show_sum(void);
static void show_sector(void);
static void show_menu(void);
static void show_loadret(void);
static void show_dock(void);
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
    for (gw = 0; gw < 1536u; gw++) {
        REG_VMDATAL = font_pic[gw * 2u];
        REG_VMDATAH = font_pic[gw * 2u + 1u];
    }
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
static void script_pads(void) {
    gsclk = (uint8_t)(gsfr >> 4);
    if (gselfdrive == 2u) {
        while (gsi < SCN2) {
            if (gsclk < scf2[gsi]) break;
            gact = scp2[gsi];
            if (gact == 0u) { gj_held = 0u; }
            else if (gact == 1u) { gj_held = PB_START; }
            else if (gact == 2u) { gj_held = PB_UP; }
            else if (gact == 3u) { gj_held = PB_DOWN; }
            else if (gact == 4u) { gj_held = PB_LEFT; }
            else if (gact == 5u) { gj_held = PB_RIGHT; }
            else if (gact == 6u) { gj_held = PB_A; }
            else if (gact == 7u) { gj_held = PB_B; }
            else if (gact == 8u) { gj_held = PB_X; }
            else { gj_held = PB_Y; }
            gsi++;
        }
        gj_pad = gj_held;
        gj_new = (uint16_t)(gj_pad & (uint16_t)(gj_pad ^ gj_prev));
        gj_prev = gj_pad;
        gj_dir = gj_new;
        return;
    }
    while (gsi < SCN) {
        if (gsclk < scf[gsi]) break;
        gact = scp[gsi];
        if (gact == 0u) { gj_held = 0u; }
        else if (gact == 1u) { gj_held = PB_START; }
        else if (gact == 2u) { gj_held = PB_UP; }
        else if (gact == 3u) { gj_held = PB_DOWN; }
        else if (gact == 4u) { gj_held = PB_LEFT; }
        else if (gact == 5u) { gj_held = PB_RIGHT; }
        else if (gact == 6u) { gj_held = PB_A; }
        else if (gact == 7u) { gj_held = PB_B; }
        else if (gact == 8u) { gj_held = PB_X; }
        else { gj_held = PB_Y; }
        gsi++;
    }
    gj_pad = gj_held;
    gj_new = (uint16_t)(gj_pad & (uint16_t)(gj_pad ^ gj_prev));
    gj_prev = gj_pad;
    gj_dir = gj_new;
}
static void read_pads(void) {
    if (gselfdrive) {
        script_pads();
        return;
    }
    while (!(REG_HVBJOY & 0x01u)) {
    }
    while (REG_HVBJOY & 0x01u) {
    }
    gj_pad = (uint16_t)REG_JOY1L;
    gj_pad |= (uint16_t)((uint16_t)REG_JOY1H << 8);
    gj_new = (uint16_t)(gj_pad & (uint16_t)(gj_pad ^ gj_prev));
    gj_prev = gj_pad;
    gj_dir = gj_new;
    if ((gframe & 15u) == 0u) {
        gj_dir |= (uint16_t)(gj_pad & 0x000Fu);
    }
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
    gdy = 11u; gdx = 0u; gdpal = PAL_BLUE; gdstr = "--------------------------------"; draw_text();
    gdy = 13u; gdx = 0u; gdpal = PAL_BLUE; gdstr = "--------------------------------"; draw_text();
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
    gdy = 7u;  gdx = 0u; gdpal = PAL_BLUE; gdstr = "--------------------------------"; draw_text();
    gdy = 21u; gdx = 0u; gdpal = PAL_BLUE; gdstr = "--------------------------------"; draw_text();
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
/* ---- SRAM save/load ($70:0000+, 45 bytes, sum checksum) ---- */
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
    gsram_d = 3u; sram_put();
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
    if (gsram_d != 3u) return;
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
    gbankday = 0u;
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
    gdstr = "--------------------------------"; draw_text();
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
    gdstr = "--------------------------------"; draw_text();
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
    gdstr = "--------------------------------"; draw_text();
}
static void draw_cmdarea(void) {
    if (gcmdopen) {
        gdx = 0u; gdy = 17u; gdpal = PAL_CYAN;
        gdstr = "COMMANDS:"; draw_text();
        gi = 0u;
        while (gi < 6u) {
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
    if (gj_pad & PB_SEL) {
        gdx = 0u; gdy = 26u; gdpal = PAL_GRAY;
        gdstr = "PAD "; draw_text();
        gdpal = PAL_WHITE;
        gn = (uint16_t)(gj_pad & 255u); draw_num();
        gdstr = " "; draw_text();
        gn = (uint16_t)(gj_pad >> 8); draw_num();
    } else if ((gframe & 31u) == 0u) {
        gdx = 0u; gdy = 26u; gdpal = PAL_GRAY;
        gdstr = "         "; draw_text();
        if (gshow) { gshow = 0u; }
        else { gshow = 1u; }
        draw_prompt();
    }
    if (gj_new & (PB_START | PB_A)) {
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
    if (gj_new & (PB_A | PB_B | PB_START)) {
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
    if (gj_new & (PB_A | PB_START)) {
        gmsgmode = 0u;
        gm1 = "";
        gm2 = "";
        gm3 = "";
        gm4 = "";
        gm1pal = PAL_WHITE;
        show_sector();
        return;
    }
    if (gj_new & PB_B) {
        show_sum();
        return;
    }
}
static void tick_loadret(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (PB_A | PB_START)) {
        show_sector();
        return;
    }
    if (gj_new & PB_B) {
        show_menu();
        return;
    }
}
static void tick_sector(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gcmdopen) {
        if (gj_dir & PB_UP) {
            if (gcmdsel == 0u) { gcmdsel = 5u; }
            else { gcmdsel--; }
            show_sector();
            return;
        }
        if (gj_dir & PB_DOWN) {
            gcmdsel++;
            if (gcmdsel >= 6u) gcmdsel = 0u;
            show_sector();
            return;
        }
        if (gj_new & (PB_A | PB_START)) {
            exec_cmd();
            return;
        }
        if (gj_new & (PB_B | PB_X)) {
            gcmdopen = 0u;
            show_sector();
            return;
        }
        return;
    }
    if (gj_dir & (PB_UP | PB_LEFT)) {
        if (gwarpsel == 0u) { gwarpsel = (uint8_t)(gwcount - 1u); }
        else { gwarpsel--; }
        show_sector();
        return;
    }
    if (gj_dir & (PB_DOWN | PB_RIGHT)) {
        gwarpsel++;
        if (gwarpsel >= gwcount) gwarpsel = 0u;
        show_sector();
        return;
    }
    if (gj_new & (PB_A | PB_START)) {
        do_warp();
        return;
    }
    if (gj_new & PB_X) {
        gcmdopen = 1u;
        gcmdsel = 0u;
        show_sector();
        return;
    }
    if (gj_new & PB_Y) {
        gmsgmode = 2u;
        show_sector();
        return;
    }
    if (gj_new & PB_B) {
        show_menu();
        return;
    }
    if (gj_new & PB_SEL) {
        gmsgmode = 0u;
        gm1 = "D REDISPLAY H HOLO S SCAN";
        gm1pal = PAL_WHITE;
        gm2 = "C COURSE P PORT Q QUIT";
        gm3 = "";
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
    gdstr = "--------------------------------"; draw_text();
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
    gdstr = "--------------------------------"; draw_text();
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
    if (gj_dir & PB_UP) {
        if (gportsel == 0u) { gportsel = 4u; }
        else { gportsel--; }
        show_port();
        return;
    }
    if (gj_dir & PB_DOWN) {
        gportsel++;
        if (gportsel >= 5u) gportsel = 0u;
        show_port();
        return;
    }
    if (gj_dir & PB_LEFT) {
        if (gportcom == 0u) { gportcom = 2u; }
        else { gportcom--; }
        show_port();
        return;
    }
    if (gj_dir & PB_RIGHT) {
        gportcom++;
        if (gportcom >= 3u) gportcom = 0u;
        show_port();
        return;
    }
    if (gj_new & (PB_A | PB_START)) {
        if (gportsel == 0u) { port_buy(); }
        else if (gportsel == 1u) { port_sell(); }
        else if (gportsel == 2u) { port_haggle(); }
        else if (gportsel == 3u) { port_steal(); }
        else { port_leave(); }
        return;
    }
    if (gj_new & PB_B) {
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
    else { gtmp = 4u; }
}
static void draw_dockhead(void) {
    gdx = 0u; gdy = 0u; gdpal = PAL_CYAN;
    gdstr = "STARDOCK SECTOR 1"; draw_text();
    gdy = 1u; gdx = 0u; gdpal = PAL_BLUE;
    gdstr = "--------------------------------"; draw_text();
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
    gdstr = "--------------------------------"; draw_text();
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
    if (gdocksel == 3u) {
        dock_back();
        return;
    }
    if (gdept == 1u) {
        if (gdocksel == 0u) { dock_buyholds(); }
        else if (gdocksel == 1u) { dock_buyftrs(); }
        else { dock_buyshields(); }
    } else if (gdept == 2u) {
        if (gdocksel == 0u) { dock_buyprobe(); }
        else if (gdocksel == 1u) { dock_buybeacon(); }
        else { dock_buygenesis(); }
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
    if (gj_dir & PB_UP) {
        if (gdocksel == 0u) { gdocksel = (uint8_t)(gtmp - 1u); }
        else { gdocksel--; }
        show_dock();
        return;
    }
    if (gj_dir & PB_DOWN) {
        gdocksel++;
        if (gdocksel >= gtmp) gdocksel = 0u;
        show_dock();
        return;
    }
    if (gj_new & (PB_A | PB_START)) {
        dock_exec();
        return;
    }
    if (gj_new & PB_B) {
        if (gdept == 0u) {
            dock_leave();
        } else {
            dock_back();
        }
        return;
    }
}
static void tick_menu(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & PB_UP) {
        if (gsel == 0u) { gsel = 3u; }
        else { gsel--; }
        show_menu();
        return;
    }
    if (gj_dir & PB_DOWN) {
        gsel++;
        if (gsel >= 4u) gsel = 0u;
        show_menu();
        return;
    }
    if (gj_new & (PB_A | PB_START)) {
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
    if (gj_new & PB_B) {
        show_title();
        return;
    }
}
static void tick_simple_back(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (PB_A | PB_B | PB_START)) {
        show_menu();
        return;
    }
}
static void tick_options(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & PB_UP) {
        if (gosel == 0u) { gosel = 3u; }
        else { gosel--; }
        show_options_keep();
        return;
    }
    if (gj_dir & PB_DOWN) {
        gosel++;
        if (gosel >= 4u) gosel = 0u;
        show_options_keep();
        return;
    }
    if (gj_dir & PB_LEFT) {
        opt_dec();
        show_options_keep();
        return;
    }
    if (gj_dir & PB_RIGHT) {
        opt_inc();
        show_options_keep();
        return;
    }
    if (gj_new & PB_B) {
        show_menu();
        return;
    }
}
static void tick_name(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_dir & PB_UP) {
        name_index();
        gci++;
        if (gci >= 37u) gci = 0u;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
    if (gj_dir & PB_DOWN) {
        name_index();
        if (gci == 0u) { gci = 37u; }
        gci--;
        gbuf[gslot] = charset[gci];
        show_name();
        return;
    }
    if (gj_dir & PB_LEFT) {
        if (gslot > 0u) gslot--;
        show_name();
        return;
    }
    if (gj_dir & PB_RIGHT) {
        if (gslot < 7u) gslot++;
        show_name();
        return;
    }
    if (gj_new & PB_A) {
        if (gslot < 7u) {
            gslot++;
            show_name();
        } else {
            name_confirm();
        }
        return;
    }
    if (gj_new & PB_START) {
        name_confirm();
        return;
    }
    if (gj_new & PB_B) {
        if (gentry == 0u) {
            show_menu();
        } else {
            gentry = 0u;
            gslot = 0u;
            show_name();
        }
        return;
    }
}
static void tick_sum(void) {
    wait_vblank();
    gframe++;
    read_pads();
    if (gj_new & (PB_A | PB_START)) {
        show_launch();
        return;
    }
    if (gj_new & PB_B) {
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
        } else if (gstate == ST_ATTRACT) {
            tick_attract();
        } else {
            tick_simple_back();
        }
}
