#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/* Returns floor(log2(n)). n must be a power of 2 and > 0. */
int log2_int(int n);

/*
 * Decoded address fields for one cache level.
 * Given a 64-bit byte address and the cache geometry, the address splits as:
 *
 *   [ tag | index | offset ]
 *
 * where:
 *   offset      = low  offset_bits  bits (byte within block)
 *   index       = next index_bits   bits (which set)
 *   tag         = remaining high    bits (distinguishes blocks in same set)
 */
typedef struct {
    uint64_t offset;
    uint64_t index;
    uint64_t tag;
    uint64_t block_address; /* address >> offset_bits */
} DecodedAddr;

/*
 * Decode a byte address given:
 *   offset_bits  = log2(block_size)
 *   index_bits   = log2(num_sets)
 */
DecodedAddr decode_address(uint64_t address, int offset_bits, int index_bits);

#endif /* UTILS_H */
