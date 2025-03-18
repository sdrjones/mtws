#include "Utils.h"
// random number generator
int32_t rnd()
{
    static uint32_t lcg_seed = 1;
    lcg_seed = 1664525 * lcg_seed + 1013904223;
    return lcg_seed >> 16;
}

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
