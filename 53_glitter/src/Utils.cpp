#include "Utils.h"

int32_t cabs(int32_t a)
{
    return (a > 0) ? a : -a;
}

// taken from goldfish
// If a knob foes near either limit, or the middle,
// clamp it to that limit (or to the middle)
int16_t virtualDetentedKnob(int16_t val)
{
    if (val > 4079)
    {
        val = 4095;
    }
    else if (val < 16)
    {
        val = 0;
    }

    if (cabs(val - 2048) < 16)
    {
        val = 2048;
    }

    return val;
}
