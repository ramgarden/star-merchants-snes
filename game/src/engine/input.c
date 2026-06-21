#include "engine/input.h"

static u16 previous = 0;
static u16 current = 0;

void smInputUpdate(void)
{
    previous = current;
    current = padsCurrent(0);
}

u16 smInputHeld(u16 button)
{
    return (current & button) != 0;
}

u16 smInputPressed(u16 button)
{
    return (current & button) != 0 && (previous & button) == 0;
}
