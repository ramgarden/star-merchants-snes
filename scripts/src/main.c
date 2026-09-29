/* Minimal main.c - direct SNES register access */
/* Types defined manually to avoid devkitsnes stdint.h conflicts */
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

/* SNES register definitions */
#define REG_BG0CNT (*(volatile uint16_t *)0x4000000)
#define REG_DISPLAY_CONTROL (*(volatile uint16_t *)0x4000000)

/* BG0 control bits */
#define BG0_ENABLE (1 << 8)
#define BG0_SIZE_8x8 (0 << 6)
#define BG0_SIZE_16x16 (1 << 6)
#define BG0_SIZE_32x32 (2 << 6)
#define BG0_SIZE_64x32 (3 << 6)
#define BG_8BPP (1 << 15)
#define BG_16COLOR (0 << 13)

/* VRAM addresses for text mode */
#define VRAM_TILE 0x06000000
#define VRAM_MAP 0x06000000

int main() {
    /* Initialize BG0 for text mode */
    REG_BG0CNT = BG0_ENABLE | BG0_SIZE_8x8 | BG_8BPP | BG_16COLOR;

    /* Loop forever */
    while (1) {
        /* Wait for VBlank */
        while (*(volatile uint16_t *)0x4000006 & 0x01) {}
        while (!(*(volatile uint16_t *)0x4000006 & 0x01)) {}
    }
    return 0;
}