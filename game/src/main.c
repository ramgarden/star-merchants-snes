#include <snes.h>
#include "engine/console.h"
#include "engine/draw.h"
#include "engine/input.h"
#include "version.h"

#define SCREEN_BOX_X 0
#define SCREEN_BOX_Y 0
#define SCREEN_BOX_WIDTH 32
#define SCREEN_BOX_HEIGHT 13

#define MENU_START 0
#define MENU_OPTIONS 1

typedef enum
{
    SCREEN_TITLE,
    SCREEN_MENU
} smScreen;

static void smDrawTitleScreen(void)
{
    smDrawBox(SCREEN_BOX_X, SCREEN_BOX_Y, SCREEN_BOX_WIDTH, SCREEN_BOX_HEIGHT);
    smPrintCentered(2, SM_GAME_NAME);
    smPrintCentered(4, SM_PROJECT_NAME);
    smPrintCentered(6, "BUILD " SM_BUILD);
    smPrintCentered(9, "PRESS START");
}

static void smDrawMenuScreen(u8 selected)
{
    smDrawBox(SCREEN_BOX_X, SCREEN_BOX_Y, SCREEN_BOX_WIDTH, SCREEN_BOX_HEIGHT);
    smPrintCentered(2, SM_GAME_NAME);
    smPrintCentered(4, "MAIN MENU");
    smPrintCentered(7, selected == MENU_START ? "> START GAME" : "  START GAME");
    smPrintCentered(9, selected == MENU_OPTIONS ? "> OPTIONS" : "  OPTIONS");
    smPrintCentered(11, "START SELECTS");
}

int main(void)
{
    smScreen screen = SCREEN_TITLE;
    u8 selected = MENU_START;

    smConsoleInit();
    smDrawTitleScreen();

    while (1)
    {
        WaitForVBlank();
        smInputUpdate();

        if (screen == SCREEN_TITLE)
        {
            if (smInputPressed(KEY_START))
            {
                screen = SCREEN_MENU;
                smDrawMenuScreen(selected);
            }
        }
        else
        {
            if (smInputPressed(KEY_UP) || smInputPressed(KEY_DOWN))
            {
                selected = (selected == MENU_START) ? MENU_OPTIONS : MENU_START;
                smDrawMenuScreen(selected);
            }

            if (smInputPressed(KEY_START))
            {
                if (selected == MENU_START)
                {
                    smPrintCentered(11, "STARTING...");
                }
                else
                {
                    smPrintCentered(11, "OPTIONS TBD");
                }
            }
        }
    }

    return 0;
}
