#pragma once
#include <sys/types.h>

#define find_first_bit(bmap)            find_next_bit(bmap, 0)
#define find_first_bit_zero(bmap)       find_next_bit_zero(bmap, 0)

#define find_next_bit(bmap, offset) ({          \
    unsigned int pos = offset;                  \
    while (!((bmap >> pos) & 0x1)) {            \
        pos++;                                  \
    }                                           \
    pos;                                        \
})

#define find_next_bit_zero(bmap, offset) ({     \
    unsigned int pos = offset;                  \
    while (((bmap >> pos) & 0x1)) {             \
        pos++;                                  \
    }                                           \
    pos;                                        \
})
