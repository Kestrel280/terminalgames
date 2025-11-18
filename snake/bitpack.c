#include <stdio.h>
#include "bitpack.h"

void bitPackSet(BitPack bp, int bit, bool newValue) {
    if (bit >= (sizeof(bitpack_el) * bp._numEls)) return;
    int idx = bit / sizeof(bitpack_el);
    int subbit = bit % sizeof(bitpack_el);
    bitpack_el mask = ((bitpack_el)1) << subbit;
    if (newValue) bp._data[idx] = bp._data[idx] | mask;
    else bp._data[idx] = bp._data[idx] & ~mask;
    return;
}

bool bitPackGet(BitPack bp, int bit) {
    if (bit >= (sizeof(bitpack_el) * bp._numEls)) return false;
    int idx = bit / sizeof(bitpack_el);
    int subbit = bit % sizeof(bitpack_el);
    return (((bitpack_el)1 << subbit) & bp._data[idx]) != (bitpack_el)0;
}
