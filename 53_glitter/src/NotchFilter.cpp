#include "NotchFilter.h"

NotchFilter::NotchFilter(NotchFilter::NotchQ q)
    : x1(0),
      x2(0),
      y1(0),
      y2(0)
{
    switch (q)
    {
    case Q10:
        ooa0 = 7801;
        a2oa0 = 7411;
        break;

    case Q100:
        ooa0 = 8192;
        a2oa0 = 8092;
        break;

    default:
        break;
    }
}

int32_t NotchFilter::ProcessSample(int32_t x0)
{
    x0 <<= 2;

    int32_t y0 = (ooa0 * (x0 + x2) - a2oa0 * y2) >> 14;
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    return y0 >> 2;
}
