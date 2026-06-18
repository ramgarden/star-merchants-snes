/*---------------------------------------------------------------------------------


    Simple console 'hello world' demo
    -- alekmaul


---------------------------------------------------------------------------------*/
#include <snes.h>

extern char tilfont, palfont;

//---------------------------------------------------------------------------------
int main(void)
{
    // Initialize text console with our font
    consoleSetTextMapPtr(0x6800);
    consoleSetTextGfxPtr(0x3000);
    consoleSetTextOffset(0x0100);
    consoleInitText(0, 16 * 2, &tilfont, &palfont);

    // Init background
    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    // Now Put in 16 color mode and disable Bgs except current
    setMode(BG_MODE1, 0);
    bgSetDisable(1);
    bgSetDisable(2);

    // Draw title
    consoleDrawText(8, 8, "STAR MERCHANTS");
    consoleDrawText(7, 12, "PROJECT VIBEBREW");
    consoleDrawText(10, 16, "BUILD 0001");
    consoleDrawText(8, 22, "RAMGARDEN 2026");

    // Wait for nothing :P
    setScreenOn();

    while (1)
    {
        WaitForVBlank();
    }
    return 0;
}