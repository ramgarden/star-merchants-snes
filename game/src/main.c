#include <snes.h>

#include "engine/console.h"
#include "engine/draw.h"
#include "version.h"

int main(void)
{
    smConsoleInit();
    // smConsoleClear();

    smDrawBox(0, 0, 32, 9);

    smPrintCentered(2, SM_GAME_NAME);
    smPrintCentered(4, SM_PROJECT_NAME);
    smPrintCentered(6, "BUILD " SM_BUILD);

    while (1)
    {
        WaitForVBlank();
    }

    return 0;
}