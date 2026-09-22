/* Star Merchants - ANSI-style title screen (TradeWars 2002 homage).
 *
 * Renders on BG1 (Mode 1, 4bpp): font tiles at VRAM words $3000,
 * tilemap 32x32 at VRAM words $6800. Tile for ASCII c is (c-32).
 * Per-tile palettes (tilemap high byte = palette << 2) give the
 * multi-color ANSI look from a 2-color-per-tile font.
 *
 * CODING CONSTRAINTS (verified 2026-09-22, see devlog):
 * 1. cc65 stack-frame locals and stack-passed args HANG on this
 *    toolchain/setup. This file uses ONLY globals + parameterless
 *    functions + inline register writes. No C locals, no C args, ever.
 * 2. CGRAM power-on state is NOT black: init ALL 128 BG colors, not
 *    just the ones used, or uninitialized slots show garbage colors.
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

#define VRAM_FONT_WORDS 0x3000u
#define VRAM_MAP_WORDS  0x6800u
#define MAP_W 32u

/* palettes: white, gray, red, darkred, blue, dimblue, cyan, yellow */
#define PAL_WHITE 0u
#define PAL_GRAY  1u
#define PAL_RED   2u
#define PAL_DRED  3u
#define PAL_BLUE  4u
#define PAL_DBLUE 5u
#define PAL_CYAN  6u

extern const uint8_t font_pic[3072];

/* ---- globals (the ONLY state allowed in this file) ---- */
uint16_t gw;            /* generic loop counter */
uint16_t gseed;         /* starfield state */
uint16_t gframe;        /* vblank frame counter */
uint8_t gshow;          /* prompt visible flag */
const char *gdstr;      /* draw-string pointer */
uint8_t gdx;            /* draw x */
uint8_t gdy;            /* draw y */
uint8_t gdpal;          /* draw palette */
uint8_t gc;             /* scratch char */
uint8_t gp;             /* palette loop */
uint8_t gr;             /* inner loop */

/* c1 color (lo,hi) per palette 0..7 */
const uint8_t palc1[16] = {
    0xFF, 0x7F,   /* pal0: white */
    0x10, 0x42,   /* pal1: gray */
    0x1F, 0x00,   /* pal2: red */
    0x0C, 0x00,   /* pal3: dark red */
    0x00, 0x7C,   /* pal4: blue */
    0x00, 0x2C,   /* pal5: dim blue */
    0xE0, 0x7F,   /* pal6: cyan */
    0xFF, 0x03,   /* pal7: yellow */
};

/* ---- parameterless helpers ---- */

static void load_palettes(void) {
    /* init ALL 128 BG colors (8 pals x 16): c0=black, c1=theme color,
     * c2..c15=black. Uninitialized CGRAM shows power-on garbage! */
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

/* draw gdstr at (gdx,gdy) in palette gdpal; advances gdx */
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
}

static void wait_joy_settle(void) {
    while (REG_HVBJOY & 0x01u) {
    }
}

/* ---- scene pieces ---- */

static void draw_stars(void) {
    gseed = 1234u;
    for (gw = 0; gw < 70u; gw++) {
        gseed += 997u;
        gdx = (uint8_t)(gseed & 31u);
        gdy = (uint8_t)((gseed >> 5) & 31u);
        if (gdy >= 28u) continue;
        gc = (uint8_t)((gseed >> 10) & 3u);
        if (gc == 0u) {
            gdstr = ".";
            gdpal = PAL_GRAY;
        } else if (gc == 1u) {
            gdstr = "*";
            gdpal = PAL_WHITE;
        } else if (gc == 2u) {
            gdstr = "+";
            gdpal = PAL_BLUE;
        } else {
            gdstr = ".";
            gdpal = PAL_DBLUE;
        }
        draw_text();
    }
}

static void draw_planet(void) {
    /* disc rows 1-8, cols 21-31; blue body, dimblue rim, cyan limb */
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
    /* shadow pass (gray / dark red), offset +1,+1 */
    gdx = 13u; gdy = 5u; gdpal = PAL_GRAY; gdstr = "S T A R";           draw_text();
    gdx = 8u;  gdy = 7u; gdpal = PAL_GRAY; gdstr = "M E R C H A N T S"; draw_text();
    gdx = 13u; gdy = 9u; gdpal = PAL_DRED; gdstr = "2 0 2 6";           draw_text();
    /* face pass (white / red) */
    gdx = 12u; gdy = 4u; gdpal = PAL_WHITE; gdstr = "S T A R";           draw_text();
    gdx = 7u;  gdy = 6u;  gdpal = PAL_WHITE; gdstr = "M E R C H A N T S"; draw_text();
    gdx = 12u; gdy = 8u;  gdpal = PAL_RED;   gdstr = "2 0 2 6";           draw_text();
}

static void draw_ship(void) {
    /* gray freighter lower right; cyan windows; white engine glints */
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
    gdy = 23u; gdx = 6u; gdstr = "A TRADEWARS TRIBUTE"; draw_text();
    gdy = 24u; gdx = 8u; gdstr = "(C) 2026 GPL-3.0";    draw_text();
}

static void draw_prompt(void) {
    gdpal = PAL_WHITE;
    gdy = 26u; gdx = 10u;
    if (gshow) {
        gdstr = "PRESS START";
    } else {
        gdstr = "           ";
    }
    draw_text();
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
    clear_map();
    draw_stars();
    draw_planet();
    draw_lines();
    draw_title();
    draw_ship();
    draw_footer();

    REG_TM = 0x01u;
    REG_INIDISP = 0x0Fu;

    gframe = 0u;
    gshow = 1u;
    draw_prompt();

    while (1) {
        wait_vblank();
        gframe++;
        if ((gframe & 31u) == 0u) {
            if (gshow) {
                gshow = 0u;
            } else {
                gshow = 1u;
            }
            draw_prompt();
        }
        wait_joy_settle();
        if (REG_JOY1L & 0x10u) break;   /* START = bit 4 */
    }

    while (1) {
        wait_vblank();
    }
}
