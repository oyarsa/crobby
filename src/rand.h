#ifndef MYRAND_H
#define MYRAND_H

#include <limits.h>

#define MYRAND_MAX ULONG_MAX

static inline unsigned long myrand(unsigned long* y)
{
    *y ^= (*y << 13);
    *y ^= (*y >> 17);
    return (*y ^= (*y << 5));
}

#endif
