#ifndef BITPACK_H
#define BITPACK_H

#include <inttypes.h>
#include <stdbool.h>

typedef uint64_t bitpack_el;
typedef struct _bitpack BitPack;
struct _bitpack {
    bitpack_el* _data;
    int _numEls;
    int bitsCapacity;
};

// return the previous value at bit
void bitPackSet(BitPack bp, int bit, bool newValue);

// return the value at bit
bool bitPackGet(BitPack bp, int bit);

#endif
