/*---------------------------------------------------------------------------------
    Star Merchants - Minimal ANSI Text Mode Test (cc65)
    Direct SNES hardware programming for text-mode BG layer
    Builds with PVSnesLib + ca65/ld65 toolchain
---------------------------------------------------------------------------------*/
/* Types - define manually to avoid PVSnesLib conflicts */
#ifndef __int8_t_defined
typedef signed char int8_t;
#endif
#ifndef __int16_t_defined
typedef signed short int16_t;
#endif
#ifndef __uint8_t_defined
typedef unsigned char uint8_t;
#endif
#ifndef __uint16_t_defined
typedef unsigned short uint16_t;
#endif

#define REG_DISPCTL       (*(volatile uint8_t*)0x2100)
#define REG_BGMODE        (*(volatile uint8_t*)0x2105)
#define REG_BG1SC         (*(volatile uint8_t*)0x2107)
#define REG_BG12NBA       (*(volatile uint8_t*)0x210B)
#define REG_VMAIN         (*(volatile uint8_t*)0x2115)
#define REG_VMADDL        (*(volatile uint8_t*)0x2116)
#define REG_VMADDH        (*(volatile uint8_t*)0x2117)
#define REG_VMDATAL       (*(volatile uint8_t*)0x2118)
#define REG_VMDATAH       (*(volatile uint8_t*)0x2119)
#define REG_CGADD         (*(volatile uint8_t*)0x2121)
#define REG_CGDATA        (*(volatile uint8_t*)0x2122)
#define REG_TM            (*(volatile uint8_t*)0x212C)
#define REG_INIDISP       (*(volatile uint8_t*)0x2100)
#define REG_NMITIMEN      (*(volatile uint8_t*)0x4200)
#define REG_HDMAEN        (*(volatile uint8_t*)0x420C)
#define REG_JOY1L         (*(volatile uint8_t*)0x4218)
#define REG_JOY1H         (*(volatile uint8_t*)0x4219)

/* VRAM Addresses */
#define VRAM_BG1_TILEMAP  0x6800
#define VRAM_BG1_TILES    0x3000
#define CGRAM_PALETTE     0x00

/* Joypad bits */
#define KEY_A      0x80
#define KEY_B      0x40
#define KEY_X      0x20
#define KEY_Y      0x10
#define KEY_L      0x08
#define KEY_R      0x04
#define KEY_START  0x02
#define KEY_SELECT 0x01
#define KEY_UP     0x08
#define KEY_DOWN   0x04
#define KEY_LEFT   0x02
#define KEY_RIGHT  0x01

/* Minimal CP437 font - 8x8, 256 chars, 2bpp (4 colors) */
extern const uint8_t font_cp437_8x8_2bpp[256 * 16];

/* Palette: 0=transparent, 1=white, 2=gray, 3=dark gray */
const uint16_t palette_text[16] = {
    0x0000,  /* 0: transparent */
    0x7FFF,  /* 1: white */
    0x4210,  /* 2: gray */
    0x2108,  /* 3: dark gray */
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
};

/* Text buffer (32x28 visible area) */
#define TEXT_W 32
#define TEXT_H 28
char text_buffer[TEXT_W * TEXT_H];
uint8_t cursor_x = 0, cursor_y = 0;

/* Wait for VBlank */
void wait_vblank(void) {
    while (!(*(volatile uint8_t*)0x4210 & 0x80)) {}
    (void)*(volatile uint8_t*)0x4210;
}

/* Read joypad */
uint16_t read_joypad(void) {
    return ((uint16_t)*(volatile uint8_t*)0x4219 << 8) | *(volatile uint8_t*)0x4218;
}

/* Initialize SNES for text mode */
void init_text_mode(void) {
    /* Force blank */
    REG_INIDISP = 0x80;
    
    /* Set BG Mode 1: BG1=4bpp, BG2=4bpp, BG3=2bpp */
    REG_BGMODE = 0x01;
    
    /* BG1 tilemap at $6800, 32x32 */
    REG_BG1SC = (VRAM_BG1_TILEMAP >> 8) & 0xFC;
    
    /* BG1&2 tile data at $3000 (word address = $1800) */
    REG_BG12NBA = ((VRAM_BG1_TILES >> 9) & 0x70) | ((VRAM_BG1_TILES >> 9) & 0x07);
    
    /* VRAM increment mode: increment by 1, word access */
    REG_VMAIN = 0x80;
    
    /* Enable BG1 on main screen */
    REG_TM = 0x01;
    
    /* Enable NMI */
    REG_NMITIMEN = 0x81;
    
    /* Load font to VRAM */
    REG_VMADDL = (VRAM_BG1_TILES) & 0xFF;
    REG_VMADDH = (VRAM_BG1_TILES) >> 8;
    {
        int i;
        for (i = 0; i < 256 * 16; i++) {
            REG_VMDATAL = font_cp437_8x8_2bpp[i];
        }
    }
    
    /* Load palette */
    REG_CGADD = CGRAM_PALETTE * 16;
    {
        int i;
        for (i = 0; i < 16; i++) {
            REG_CGDATA = palette_text[i] & 0xFF;
            REG_CGDATA = palette_text[i] >> 8;
        }
    }
    
    /* Clear tilemap */
    REG_VMADDL = VRAM_BG1_TILEMAP & 0xFF;
    REG_VMADDH = VRAM_BG1_TILEMAP >> 8;
    {
        int i;
        for (i = 0; i < 32 * 32; i++) {
            REG_VMDATAL = 0;
            REG_VMDATAH = 0;
        }
    }
    
    /* Clear text buffer */
    {
        int i;
        for (i = 0; i < TEXT_W * TEXT_H; i++) {
            text_buffer[i] = ' ';
        }
    }
    cursor_x = 0; cursor_y = 0;
    
    /* Turn on screen */
    REG_INIDISP = 0x0F;
}

/* Put character at cursor position */
void put_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= TEXT_H) cursor_y = TEXT_H - 1;
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }
    
    if (cursor_x >= TEXT_W) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= TEXT_H) cursor_y = TEXT_H - 1;
    }
    
    text_buffer[cursor_y * TEXT_W + cursor_x] = c;
    cursor_x++;
}

/* Print string */
void print_str(const char* s) {
    while (*s) put_char(*s++);
}

/* Print string at position */
void print_at(uint8_t x, uint8_t y, const char* s) {
    cursor_x = x; cursor_y = y;
    print_str(s);
}

/* Render text buffer to VRAM tilemap */
void render_text(void) {
    REG_VMADDL = VRAM_BG1_TILEMAP & 0xFF;
    REG_VMADDH = VRAM_BG1_TILEMAP >> 8;
    
    {
        int y;
        for (y = 0; y < TEXT_H; y++) {
            {
                int x;
                for (x = 0; x < TEXT_W; x++) {
                    uint8_t c = text_buffer[y * TEXT_W + x];
                    REG_VMDATAL = c;      /* Tile index (low byte) */
                    REG_VMDATAH = 0x00;   /* Tile index high byte + palette/priority */
                }
            }
            /* Skip remaining tiles in 32-wide row */
            {
                int x;
                for (x = TEXT_W; x < 32; x++) {
                    REG_VMDATAL = 0;
                    REG_VMDATAH = 0;
                }
            }
        }
    }
}

/* Draw a box using CP437 box-drawing chars */
void draw_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    /* Top edge */
    cursor_x = x; cursor_y = y;
    put_char(0xC9);  /* ┌ */
    {
        int i;
        for (i = 1; i < w - 1; i++) put_char(0xCD);  /* ─ */
    }
    put_char(0xBB);  /* ┐ */
    
    /* Sides */
    {
        int row;
        for (row = 1; row < h - 1; row++) {
            cursor_x = x; cursor_y = y + row;
            put_char(0xBA);  /* │ */
            cursor_x = x + w - 1;
            put_char(0xBA);  /* │ */
        }
    }
    
    /* Bottom edge */
    cursor_x = x; cursor_y = y + h - 1;
    put_char(0xC8);  /* └ */
    {
        int i;
        for (i = 1; i < w - 1; i++) put_char(0xCD);  /* ─ */
    }
    put_char(0xBC);  /* ┘ */
}

/* Title screen with animated text */
void title_screen(void) {
    /* Initialize text mode */
    init_text_mode();
    
    /* Display title */
    cursor_x = 0; cursor_y = 0;
    print_str("  STAR MERCHANTS  ");
    print_str("\n");
    print_str("  Spiritual successor to TradeWars 2002");
    print_str("\n");
    print_str("  Press START to play");
    print_str("\n");
    print_str("  (C) 2026 GPL-3.0");
    
    render_text();
    
    /* Wait for START */
    {
        uint16_t joy;
        while (1) {
            wait_vblank();
            joy = read_joypad();
            if (joy & KEY_START) break;
        }
    }
}

/* Main */
int main(void) {
    uint16_t joy;
    
    /* Title Screen */
    title_screen();
    
    /* Draw title screen */
    draw_box(4, 3, 24, 14);
    print_at(8, 5, "STAR MERCHANTS");
    print_at(6, 7, "Spiritual successor to");
    print_at(9, 8, "TradeWars 2002");
    print_at(10, 10, "GPL-3.0 License");
    print_at(7, 12, "Press START to begin");
    print_at(7, 14, "(C) 2026");
    
    render_text();
    
    /* Main loop */
    while (1) {
        wait_vblank();
        joy = read_joypad();
        if (joy & KEY_START) break;
    }
    
    return 0;
}