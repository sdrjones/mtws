#pragma once

#define COMPUTERCARD_NOIMPL
#include "ComputerCard.h"

// Simple 12KHz Notchfilter - thanks to Chris Johnson

class NotchFilter
{
public:
    enum NotchQ
    {
        Q10,
        Q100
    };

    // contructor
    NotchFilter(NotchQ q);

    // process sample x0, return y0
    //int32_t ProcessSample(int32_t x0);
    int32_t __not_in_flash_func(ProcessSample)(int32_t x0);

private:
    // state
    int32_t x1;
    int32_t x2;
    int32_t y1;
    int32_t y2;

    // coeffs
    int32_t ooa0 = 8192;
    int32_t a2oa0 = 8092;
};
