#ifndef SM_CONSOLE_H
#define SM_CONSOLE_H

#include <snes.h>

void smConsoleInit(void);
void smConsoleClear(void);
void smPrint(u16 x,u16 y,const char *text);
void smPrintCentered(u16 y,const char *text);

#endif
