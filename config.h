#ifndef CONFIG_H
#define CONFIG_H

typedef enum {
    POLICY_LRU,
    POLICY_FIFO
} ReplacementPolicy;

/*
 * All parameters parsed from the command line.
 *
 * Per-level: enabled=1 means the level participates in the hierarchy.
 * A disabled level is skipped entirely; its size/assoc are ignored.
 *
 * Latencies (cycles) are used for AMAT approximation.
 * Defaults: L1=1, L2=10, L3=40, mem=100.
 */
typedef struct {
    int l1_enabled, l1_size, l1_assoc, l1_latency;
    int l2_enabled, l2_size, l2_assoc, l2_latency;
    int l3_enabled, l3_size, l3_assoc, l3_latency;
    int block_size;
    int mem_latency;
    ReplacementPolicy policy;
    char trace_file[512];
} SimConfig;

void parse_args(int argc, char *argv[], SimConfig *cfg);
void print_config(const SimConfig *cfg);

#endif /* CONFIG_H */
