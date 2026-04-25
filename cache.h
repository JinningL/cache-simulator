#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include "config.h"
#include "stats.h"
#include "utils.h"

/* ── Data structures ───────────────────────────────────────────────────── */

/*
 * One cache line (a.k.a. cache block slot).
 *
 * tag         : identifies which memory block occupies this slot
 * valid       : 1 if this line holds real data, 0 if empty/invalid
 * dirty       : 1 if the line was written since it was loaded (write-back)
 * last_used   : global timestamp of the last access — used by LRU eviction
 * inserted_at : global timestamp when line was first loaded — used by FIFO
 */
struct CacheLine {
    uint64_t tag;
    int      valid;
    int      dirty;
    uint64_t last_used;
    uint64_t inserted_at;
};
typedef struct CacheLine CacheLine;

/*
 * One cache set: an array of `associativity` lines that all map to the
 * same index (i.e. compete for the same slot in the cache).
 */
typedef struct {
    CacheLine *lines; /* array of length == associativity */
} CacheSet;

/*
 * One level of the cache hierarchy (L1, L2, or L3).
 *
 * Geometry:
 *   num_lines  = cache_size / block_size
 *   num_sets   = num_lines  / associativity
 *   offset_bits= log2(block_size)
 *   index_bits = log2(num_sets)
 */
typedef struct {
    char  name[8];       /* "L1", "L2", or "L3" */
    int   cache_size;
    int   block_size;
    int   associativity;
    int   num_lines;
    int   num_sets;
    int   offset_bits;
    int   index_bits;
    CacheSet *sets;      /* array of num_sets sets */
    Stats stats;
} CacheLevel;

/* ── Functions ─────────────────────────────────────────────────────────── */

/*
 * Allocate and initialise a CacheLevel.
 * Returns a pointer to the new level (caller must call cache_free later).
 */
CacheLevel *cache_level_init(const char *name, int cache_size,
                             int block_size, int associativity);

/*
 * Look up `address` in `level`.
 * Returns the way index (0..assoc-1) of the matching valid line on hit,
 * or -1 on miss.
 * Also fills `*out` with the decoded address fields.
 */
int cache_lookup(CacheLevel *level, uint64_t address, DecodedAddr *out);

/*
 * Insert a block for `address` into `level` using the given policy/timestamp.
 *
 * If the target set is full the victim is selected by find_victim().
 * If the victim is dirty its reconstructed block address is written into
 * *writeback_addr and the function returns 1 (dirty eviction occurred).
 * Otherwise returns 0.
 *
 * The caller is responsible for acting on a dirty writeback (e.g. writing
 * the block down to the next cache level or main memory).
 */
int cache_insert(CacheLevel *level, uint64_t address,
                 ReplacementPolicy policy, uint64_t timestamp,
                 uint64_t *writeback_addr);

/*
 * Mark the line at `way` in the set for `address` as dirty.
 * Call this after a write hit.
 */
void cache_mark_dirty(CacheLevel *level, uint64_t address, int way);

/*
 * Update last_used on a hit so LRU stays accurate.
 */
void cache_update_lru(CacheLevel *level, uint64_t address,
                      int way, uint64_t timestamp);

/* Free all memory owned by `level` and the level struct itself. */
void cache_free(CacheLevel *level);

#endif /* CACHE_H */
