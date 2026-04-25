#ifndef CONFIG_H
#define CONFIG_H

/* Replacement policies supported by the simulator. */
typedef enum {
    POLICY_LRU,   /* Least Recently Used */
    POLICY_FIFO   /* First In, First Out  */
} ReplacementPolicy;

/* All parameters parsed from the command line. */
typedef struct {
    int l1_size;   /* L1 cache size in bytes */
    int l1_assoc;  /* L1 associativity (ways per set) */
    int l2_size;
    int l2_assoc;
    int l3_size;
    int l3_assoc;
    int block_size;              /* block (cache line) size in bytes */
    ReplacementPolicy policy;    /* LRU or FIFO */
    char trace_file[512];        /* path to the trace file */
} SimConfig;

/*
 * Parse command-line arguments into cfg.
 * Exits with a usage message on error.
 */
void parse_args(int argc, char *argv[], SimConfig *cfg);

/* Print the parsed configuration to stdout. */
void print_config(const SimConfig *cfg);

#endif /* CONFIG_H */
