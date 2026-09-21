/* Star Merchants - title screen
 *
 * VRAM layout (word addresses):
 *   $3000-$35FF  BG1 font tiles (96 x 4bpp 8x8 glyphs, ASCII 32..127)
 *   $6800-$6BFF  BG1 tilemap (32x32)
 *
 * BG1's character base is word $3000 (BG12NBA = $03), so the tile number
 * for ASCII character c is simply (c - 32). Glyph pixels use color index 1.
 */

typedef signed char int8_t;
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
#define REG_TS       (*(volatile uint8_t*)0x212D)
#define REG_NMITIMEN (*(volatile uint8_t*)0x4200)
#define REG_HVBJOY   (*(volatile uint8_t*)0x4212)
#define REG_JOY1L    (*(volatile uint8_t*)0x4218)

/* VRAM word addresses */
#define VRAM_FONT_WORDS 0x3000u
#define VRAM_MAP_WORDS  0x6800u

#define MAP_W 32u

extern const uint8_t font_pic[3072];

static void wait_vblank(void) {
    while (REG_HVBJOY & 0x80) {}
    while (!(REG_HVBJOY & 0x80)) {}
}

static void vram_set(uint16_t words) {
    REG_VMADDL = (uint8_t)(words & 0xFFu);
    REG_VMADDH = (uint8_t)(words >> 8);
}

static void load_font(void) {
    uint16_t w;
    vram_set(VRAM_FONT_WORDS);
    for (w = 0; w < 1536u; w++) {
        REG_VMDATAL = font_pic[w * 2u];
        REG_VMDATAH = font_pic[w * 2u + 1u];
    }
}

static void load_palette(void) {
    REG_CGADD = 0;
    REG_CGDATA = 0x00; REG_CGDATA = 0x00;   /* color 0: black backdrop */
    REG_CGDATA = 0xFF; REG_CGDATA = 0x7F;   /* color 1: white glyphs */
}

static void map_write(uint8_t x, uint8_t y, uint16_t tile) {
    vram_set(VRAM_MAP_WORDS + (uint16_t)y * MAP_W + x);
    REG_VMDATAL = (uint8_t)(tile & 0xFFu);
    REG_VMDATAH = (uint8_t)(tile >> 8);
}

static void draw_text(uint8_t x, uint8_t y, const char *s) {
    while (*s) {
        uint8_t c = (uint8_t)*s++;
        if (c < 32 || c > 127) c = 32;
        if (x >= MAP_W) break;
        map_write(x++, y, (uint16_t)(c - 32u));
    }
}

static void clear_map(void) {
    uint16_t n;
    vram_set(VRAM_MAP_WORDS);
    for (n = 0; n < 1024u; n++) {
        REG_VMDATAL = 0;    /* tile 0 = blank space glyph */
        REG_VMDATAH = 0;
    }
}

int main(void) {
    uint16_t frame = 0;
    uint8_t prompt_on = 1;

    /* Force blank while setting up video */
    REG_INIDISP = 0x80;

    /* Mode 1: BG1 4bpp, tilemap at VRAM words $6800 (32x32), tiles at $3000 */
    REG_BGMODE = 0x01;
    REG_BG1SC = 0x68;
    REG_BG12NBA = 0x03;

    /* VRAM word step 1, increment after the high byte is written */
    REG_VMAIN = 0x80;

    /* No scrolling */
    REG_BG1HOFS = 0; REG_BG1HOFS = 0;
    REG_BG1VOFS = 0; REG_BG1VOFS = 0;

    /* NMI + joypad auto-read */
    REG_NMITIMEN = 0x81;

    load_font();
    load_palette();
    clear_map();
    draw_text(8, 10, "STAR MERCHANTS");
    draw_text(6, 14, "PRESS START");

    /* Black backdrop is already set in palette, enable screen */
    REG_TM = 0x01;
    REG_INIDISP = 0x0F;

    while (1) {
        wait_vblank();
        frame++;

        /* Blink prompt */
        if ((frame & 31u) == 0) {
            prompt_on = (uint8_t)!prompt_on;
            draw_text(6, 14, prompt_on ? "PRESS START" : "                    ");
        }

        /* START is bit 3 of JOY1L */
        if (REG_JOY1L & 0x08) break;
    }

    /* Game starts here */
    while (1) {
        wait_vblank();
    }
}
