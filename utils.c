#include "utils.h"

int log2_int(int n) {
    int bits = 0;
    while (n > 1) {
        n >>= 1;
        bits++;
    }
    return bits;
}

DecodedAddr decode_address(uint64_t address, int offset_bits, int index_bits) {
    DecodedAddr d;
    uint64_t offset_mask = ((uint64_t)1 << offset_bits) - 1;
    uint64_t index_mask  = ((uint64_t)1 << index_bits)  - 1;

    d.offset        = address & offset_mask;
    d.block_address = address >> offset_bits;
    d.index         = d.block_address & index_mask;
    d.tag           = d.block_address >> index_bits;
    return d;
}
