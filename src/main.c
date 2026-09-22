/* Star Merchants - title + menu engine (Milestones 1-2).
 *
 * Title: ANSI homage (TradeWars 2002 style) on BG1 (Mode 1, 4bpp):
 * font tiles at VRAM words $3000, tilemap 32x32 at VRAM words $6800.
 * Tile for ASCII c is (c-32); tilemap high byte is palette << 2.
 * Menu: state machine TITLE->MENU->(NEW GAME wizard/CONTINUE/OPTIONS/
 * CREDITS)->summary->launch stub (Milestone 3 hook).
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
#define ST_TITLE 0u
#define ST_MENU 1u
#define ST_CREDITS 2u
#define ST_CONTINUE 3u
#define ST_OPTIONS 4u
#define ST_NEWNAME 5u
#define ST_NEWSHIP 6u
#define ST_NEWSUM 7u
#define ST_LAUNCH 8u
#define PB_UP 0x0008u
#define PB_DOWN 0x0004u
#define PB_LEFT 0x0002u
#define PB_RIGHT 0x0001u
#define PB_START 0x0010u
#define PB_B 0x0080u
#define PB_A 0x8000u
#define SCN 30u
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
const uint8_t palc1[16] = {
    0xFF, 0x7F, 0x10, 0x42, 0x1F, 0x00, 0x0C, 0x00,
    0x00, 0x7C, 0x00, 0x2C, 0xE0, 0x7F, 0xFF, 0x03,
};
const uint8_t mrows[4] = { 11u, 13u, 15u, 17u };
const char charset[38] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
const uint8_t scf[30] = {
    3u, 4u, 9u, 10u, 15u, 16u, 21u, 22u, 27u, 28u,
    40u, 41u, 46u, 47u, 52u, 53u, 58u, 59u, 64u, 65u,
    70u, 71u, 76u, 77u, 82u, 83u, 88u, 89u, 0u, 0u,
};
const uint8_t scp[30] = {
    1u, 0u, 3u, 0u, 3u, 0u, 3u, 0u, 6u, 0u,
    7u, 0u, 2u, 0u, 2u, 0u, 6u, 0u, 7u, 0u,
    2u, 0u, 6u, 0u, 1u, 0u, 1u, 0u, 0u, 0u,
};
static void show_sum(void);
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
        else { gj_held = PB_B; }
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
    REG_INIDISP = 0x80u;
    clear_map();
    draw_menulines();
    gdpal = PAL_WHITE;
    gdy = 6u; gdx = 12u; gdstr = "CONTINUE"; draw_text();
    gdpal = PAL_RED;
    gdy = 12u; gdx = 7u; gdstr = "NO SAVED GAME FOUND"; draw_text();
    gdpal = PAL_GRAY;
    gdy = 14u; gdx = 6u; gdstr = "SRAM SUPPORT COMING"; draw_text();
    gdy = 15u; gdx = 9u;  gdstr = "IN A LATER BUILD";    draw_text();
    draw_hint_back();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_CONTINUE;
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
    gdy = 15u; gdx = 9u;  gdstr = "MILESTONE 3:";     draw_text();
    gdy = 16u; gdx = 7u;  gdstr = "SECTOR VIEW SOON"; draw_text();
    draw_hint_back();
    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;
    gstate = ST_LAUNCH;
    gframe = 0u;
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
    if (gj_new & PB_START) {
        gsel = 0u;
        show_menu();
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
        } else {
            tick_simple_back();
        }
    }
}
