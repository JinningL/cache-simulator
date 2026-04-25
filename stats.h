#ifndef STATS_H
#define STATS_H

#include <stdint.h>

/* Per-cache-level counters accumulated during simulation. */
typedef struct {
    uint64_t accesses;
    uint64_t reads;
    uint64_t writes;
    uint64_t hits;
    uint64_t misses;
    uint64_t evictions;
    uint64_t dirty_writebacks;
} Stats;

/* Print a formatted stats block for one cache level. */
void print_level_stats(const char *level_name, const Stats *s);

/*
 * Print the overall summary including a dynamic AMAT calculation.
 *
 * names[]      : level name strings, e.g. {"L1","L3"}
 * latencies[]  : per-level latency in cycles
 * miss_rates[] : per-level miss rates in [0.0, 1.0]
 * num_levels   : length of the three arrays above
 * mem_latency  : main memory latency in cycles
 */
void print_overall_stats(uint64_t total_accesses,
                         uint64_t memory_accesses,
                         const char **names,
                         const int  *latencies,
                         const double *miss_rates,
                         int num_levels,
                         int mem_latency);

#endif /* STATS_H */
