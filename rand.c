#include <limits.h>
#include <stdlib.h>
#include "rand.h"

unsigned long xorshift(unsigned long* y)
{
    *y ^= (*y << 13);
    *y ^= (*y >> 17);
    return (*y ^= (*y << 15));
}

float xorshiftf(unsigned long* y)
{
    return xorshift(y) / (float)ULONG_MAX;
}
