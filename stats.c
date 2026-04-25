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
                         const char **names,
                         const int  *latencies,
                         const double *miss_rates,
                         int num_levels,
                         int mem_latency) {
    /*
     * Dynamic AMAT: start from main memory and work backward:
     *   amat = mem_latency
     *   for i = last..first:
     *     amat = latency[i] + miss_rate[i] * amat
     */
    double amat = (double)mem_latency;
    for (int i = num_levels - 1; i >= 0; i--)
        amat = (double)latencies[i] + miss_rates[i] * amat;

    printf("=== Overall ===\n");
    printf("  Total Accesses:  %8llu\n", (unsigned long long)total_accesses);
    printf("  Memory Accesses: %8llu\n", (unsigned long long)memory_accesses);
    printf("  AMAT (approx):   %.2f cycles\n", amat);
    printf("\n");

    /* Latency table */
    printf("  Latencies:");
    for (int i = 0; i < num_levels; i++)
        printf("  %s=%d", names[i], latencies[i]);
    printf("  Mem=%d cycles\n", mem_latency);

    /* Formula string, e.g. "L1(1) + mr_L1*(L3(40) + mr_L3*Mem(100))" */
    printf("  AMAT formula:  ");
    for (int i = 0; i < num_levels; i++)
        printf("%s(%d) + mr_%s*(", names[i], latencies[i], names[i]);
    printf("Mem(%d)", mem_latency);
    for (int i = 0; i < num_levels; i++) printf(")");
    printf("\n");
}
