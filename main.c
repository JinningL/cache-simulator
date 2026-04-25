#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "cache.h"
#include "hierarchy.h"
#include "trace.h"
#include "stats.h"

int main(int argc, char *argv[]) {
    SimConfig cfg;
    parse_args(argc, argv, &cfg);
    print_config(&cfg);

    CacheHierarchy *hier = hierarchy_init(&cfg);

    TraceReader *reader = trace_open(cfg.trace_file);
    if (!reader) return EXIT_FAILURE;

    printf("=== Access Log ===\n");
    TraceEntry entry;
    uint64_t total_accesses = 0;
    int rc;

    while ((rc = trace_next(reader, &entry)) == 1) {
        total_accesses++;
        hierarchy_access(hier, entry.op, entry.addr, cfg.policy);
    }

    if (rc == -1) {
        fprintf(stderr, "Simulation aborted due to trace parse error.\n");
        trace_close(reader);
        hierarchy_free(hier);
        return EXIT_FAILURE;
    }

    trace_close(reader);

    printf("\n");

    /* Print per-level stats and build arrays for AMAT */
    const char *names[3];
    int    latencies[3];
    double miss_rates[3];

    for (int i = 0; i < hier->num_levels; i++) {
        CacheLevel *lvl = hier->levels[i];
        print_level_stats(lvl->name, &lvl->stats);
        names[i]      = lvl->name;
        latencies[i]  = lvl->latency;
        miss_rates[i] = lvl->stats.accesses
                        ? (double)lvl->stats.misses / lvl->stats.accesses
                        : 0.0;
    }

    print_overall_stats(total_accesses, hier->memory_accesses,
                        names, latencies, miss_rates,
                        hier->num_levels, hier->mem_latency);

    hierarchy_free(hier);
    return EXIT_SUCCESS;
}
