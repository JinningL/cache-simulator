#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "cache.h"
#include "hierarchy.h"
#include "trace.h"
#include "stats.h"

int main(int argc, char *argv[]) {
    /* ── 1. Parse CLI arguments ── */
    SimConfig cfg;
    parse_args(argc, argv, &cfg);
    print_config(&cfg);

    /* ── 2. Build the cache hierarchy ── */
    CacheHierarchy *hier = hierarchy_init(&cfg);

    /* ── 3. Open the trace file ── */
    TraceReader *reader = trace_open(cfg.trace_file);
    if (!reader) return EXIT_FAILURE;

    /* ── 4. Simulate each trace entry ── */
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

    /* ── 5. Print statistics ── */
    printf("\n");
    print_level_stats("L1", &hier->L1->stats);
    print_level_stats("L2", &hier->L2->stats);
    print_level_stats("L3", &hier->L3->stats);

    /* Compute per-level miss rates for AMAT */
    double mr_L1 = hier->L1->stats.accesses
                   ? (double)hier->L1->stats.misses / hier->L1->stats.accesses
                   : 0.0;
    double mr_L2 = hier->L2->stats.accesses
                   ? (double)hier->L2->stats.misses / hier->L2->stats.accesses
                   : 0.0;
    double mr_L3 = hier->L3->stats.accesses
                   ? (double)hier->L3->stats.misses / hier->L3->stats.accesses
                   : 0.0;

    print_overall_stats(total_accesses, hier->memory_accesses,
                        mr_L1, mr_L2, mr_L3);

    /* ── 6. Clean up ── */
    hierarchy_free(hier);
    return EXIT_SUCCESS;
}
