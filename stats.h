#ifndef STATS_H
#define STATS_H

#include <stdint.h>

/* Per-cache-level counters accumulated during simulation. */
typedef struct {
    uint64_t accesses;        /* total lookups at this level */
    uint64_t reads;
    uint64_t writes;
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;       /* lines displaced to make room */
    uint64_t dirty_writebacks;/* dirty evictions written to next level */
} Stats;

/* Latency constants (cycles) used for AMAT approximation. */
#define LATENCY_L1  1
#define LATENCY_L2  10
#define LATENCY_L3  40
#define LATENCY_MEM 100

/*
 * Print a formatted stats block for one cache level.
 * level_name is e.g. "L1", "L2", "L3".
 */
void print_level_stats(const char *level_name, const Stats *s);

/*
 * Print the overall simulation summary.
 * total_accesses : number of trace entries processed
 * memory_accesses: L3 miss count (went to main memory)
 * miss_rate_L1/L2/L3: precomputed miss rates [0.0, 1.0]
 */
void print_overall_stats(uint64_t total_accesses,
                         uint64_t memory_accesses,
                         double miss_rate_L1,
                         double miss_rate_L2,
                         double miss_rate_L3);

#endif /* STATS_H */
