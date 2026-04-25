#ifndef HIERARCHY_H
#define HIERARCHY_H

#include <stdint.h>
#include "cache.h"
#include "config.h"

/*
 * The three-level cache hierarchy plus a global timestamp counter.
 *
 * The timestamp is incremented once per hierarchy_access() call and is
 * passed into cache_insert() / cache_update_lru() so that LRU and FIFO
 * can compare relative order of accesses and insertions.
 */
typedef struct {
    CacheLevel *L1;
    CacheLevel *L2;
    CacheLevel *L3;
    uint64_t    timestamp;
    uint64_t    memory_accesses; /* L3 misses that went to main memory */
} CacheHierarchy;

/*
 * Allocate and wire up the three levels based on the parsed config.
 * Returns a heap-allocated CacheHierarchy (caller must call hierarchy_free).
 */
CacheHierarchy *hierarchy_init(const SimConfig *cfg);

/*
 * Process one memory access (op = 'R' or 'W', address = byte address).
 * Prints a one-line result for this access and updates all stats.
 */
void hierarchy_access(CacheHierarchy *hier, char op, uint64_t address,
                      ReplacementPolicy policy);

/* Free all levels and the hierarchy struct itself. */
void hierarchy_free(CacheHierarchy *hier);

#endif /* HIERARCHY_H */
