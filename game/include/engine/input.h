#ifndef SM_INPUT_H
#define SM_INPUT_H

#include <snes.h>

void smInputUpdate(void);
u16 smInputHeld(u16 button);
u16 smInputPressed(u16 button);

#endif
