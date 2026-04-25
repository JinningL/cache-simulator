#ifndef HIERARCHY_H
#define HIERARCHY_H

#include <stdint.h>
#include "cache.h"
#include "config.h"

/*
 * A dynamic cache hierarchy.
 *
 * levels[]    : active cache levels in order (nearest → farthest from CPU).
 * num_levels  : number of active levels (1–3).
 * mem_latency : latency to main memory in cycles.
 * timestamp   : global access counter for LRU/FIFO.
 */
typedef struct {
    CacheLevel **levels;
    int          num_levels;
    int          mem_latency;
    uint64_t     timestamp;
    uint64_t     memory_accesses;
} CacheHierarchy;

/*
 * Build the hierarchy from cfg.
 * Only levels with enabled=1 are included.
 */
CacheHierarchy *hierarchy_init(const SimConfig *cfg);

/*
 * Process one memory access (op = 'R' or 'W').
 * Prints a one-line result and updates stats.
 */
void hierarchy_access(CacheHierarchy *hier, char op, uint64_t address,
                      ReplacementPolicy policy);

/* Free all levels and the hierarchy struct. */
void hierarchy_free(CacheHierarchy *hier);

#endif /* HIERARCHY_H */
