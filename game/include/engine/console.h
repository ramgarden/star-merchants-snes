#ifndef SM_CONSOLE_H
#define SM_CONSOLE_H

#include <snes.h>

/*
 * Initializes the Star Merchants text console.
 */
void smConsoleInit(void);

/*
 * Clears the console.
 */
void smConsoleClear(void);

/*
 * Prints text at the specified position.
 */
void smPrint(u16 x, u16 y, const char *text);

/*
 * Prints centered text on a row.
 */
void smPrintCentered(u16 y, const char *text);

#endif