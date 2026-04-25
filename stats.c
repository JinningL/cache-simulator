#include <stdio.h>
#include "stats.h"

void print_level_stats(const char *level_name, const Stats *s) {
    double hit_rate  = s->accesses ? (double)s->hits   / s->accesses * 100.0 : 0.0;
    double miss_rate = s->accesses ? (double)s->misses / s->accesses * 100.0 : 0.0;

    printf("=== %s Cache ===\n", level_name);
    printf("  Accesses:        %8llu\n", (unsigned long long)s->accesses);
    printf("  Reads:           %8llu   Writes:           %8llu\n",
           (unsigned long long)s->reads, (unsigned long long)s->writes);
    printf("  Hits:            %8llu   Misses:           %8llu\n",
           (unsigned long long)s->hits,  (unsigned long long)s->misses);
    printf("  Evictions:       %8llu   Dirty Writebacks: %8llu\n",
           (unsigned long long)s->evictions, (unsigned long long)s->dirty_writebacks);
    printf("  Hit Rate:        %7.2f%%  Miss Rate:        %7.2f%%\n",
           hit_rate, miss_rate);
    printf("\n");
}

void print_overall_stats(uint64_t total_accesses,
                         uint64_t memory_accesses,
                         double miss_rate_L1,
                         double miss_rate_L2,
                         double miss_rate_L3) {
    /*
     * Approximate AMAT using the hierarchical miss-penalty formula:
     *   AMAT = L1_lat
     *        + miss_L1 * (L2_lat
     *        + miss_L2 * (L3_lat
     *        + miss_L3 * mem_lat))
     */
    double amat = LATENCY_L1
                + miss_rate_L1 * (LATENCY_L2
                + miss_rate_L2 * (LATENCY_L3
                + miss_rate_L3 * LATENCY_MEM));

    printf("=== Overall ===\n");
    printf("  Total Accesses:  %8llu\n", (unsigned long long)total_accesses);
    printf("  Memory Accesses: %8llu\n", (unsigned long long)memory_accesses);
    printf("  AMAT (approx):   %.2f cycles\n", amat);
    printf("\n");
    printf("  Latencies used: L1=%d, L2=%d, L3=%d, Mem=%d cycles\n",
           LATENCY_L1, LATENCY_L2, LATENCY_L3, LATENCY_MEM);
    printf("  Formula: L1 + mr_L1*(L2 + mr_L2*(L3 + mr_L3*Mem))\n");
}
