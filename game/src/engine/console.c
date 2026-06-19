#include "engine/console.h"

extern char tilfont, palfont;

void smConsoleInit(void)
{
    consoleSetTextMapPtr(0x6800);
    consoleSetTextGfxPtr(0x3000);
    consoleSetTextOffset(0x0100);

    consoleInitText(0, 16 * 2, &tilfont, &palfont);

    bgSetGfxPtr(0, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_32x32);

    setMode(BG_MODE1, 0);

    bgSetDisable(1);
    bgSetDisable(2);

    setScreenOn();
}

void smConsoleClear(void)
{
    // Clears the internal text layer buffers entirely by re-initializing it
    consoleInit();
}

void smPrint(u16 x, u16 y, const char *text)
{
    consoleDrawText(x, y, text);
}

void smPrintCentered(u16 y, const char *text)
{
    int len = 0;

    while (text[len] != '\0')
        len++;

    consoleDrawText((32 - len) / 2, y, text);
}