#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options] <trace_file>\n\n"
        "Level options (at least one level must be enabled):\n"
        "  --l1-size <B>  --l1-assoc <n>  [--l1-latency <cyc>]  [--no-l1]\n"
        "  --l2-size <B>  --l2-assoc <n>  [--l2-latency <cyc>]  [--no-l2]\n"
        "  --l3-size <B>  --l3-assoc <n>  [--l3-latency <cyc>]  [--no-l3]\n\n"
        "Shared options:\n"
        "  --block    <B>           block (cache line) size in bytes\n"
        "  --policy   LRU|FIFO\n"
        "  --mem-latency <cyc>      main memory latency (default 100)\n\n"
        "Example (L1+L3 only, custom latencies):\n"
        "  %s --l1-size 32768 --l1-assoc 4 --l1-latency 2\n"
        "     --no-l2\n"
        "     --l3-size 8388608 --l3-assoc 16 --l3-latency 40\n"
        "     --block 64 --policy LRU --mem-latency 200 trace.txt\n",
        prog, prog);
    exit(EXIT_FAILURE);
}

void parse_args(int argc, char *argv[], SimConfig *cfg) {
    /* Defaults */
    cfg->l1_enabled = 1; cfg->l1_size = 0; cfg->l1_assoc = 0; cfg->l1_latency = 1;
    cfg->l2_enabled = 1; cfg->l2_size = 0; cfg->l2_assoc = 0; cfg->l2_latency = 10;
    cfg->l3_enabled = 1; cfg->l3_size = 0; cfg->l3_assoc = 0; cfg->l3_latency = 40;
    cfg->block_size   = 0;
    cfg->mem_latency  = 100;
    cfg->policy       = POLICY_LRU;
    cfg->trace_file[0]= '\0';

    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "--l1-size"))     cfg->l1_size     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l1-assoc"))    cfg->l1_assoc    = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l1-latency"))  cfg->l1_latency  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--no-l1"))       cfg->l1_enabled  = 0;
        else if (!strcmp(argv[i], "--l2-size"))     cfg->l2_size     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l2-assoc"))    cfg->l2_assoc    = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l2-latency"))  cfg->l2_latency  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--no-l2"))       cfg->l2_enabled  = 0;
        else if (!strcmp(argv[i], "--l3-size"))     cfg->l3_size     = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l3-assoc"))    cfg->l3_assoc    = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--l3-latency"))  cfg->l3_latency  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--no-l3"))       cfg->l3_enabled  = 0;
        else if (!strcmp(argv[i], "--block"))       cfg->block_size  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--mem-latency")) cfg->mem_latency = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--policy")) {
            i++;
            if      (!strcmp(argv[i], "LRU"))  cfg->policy = POLICY_LRU;
            else if (!strcmp(argv[i], "FIFO")) cfg->policy = POLICY_FIFO;
            else { fprintf(stderr, "Unknown policy: %s\n", argv[i]); usage(argv[0]); }
        }
        else {
            strncpy(cfg->trace_file, argv[i], sizeof(cfg->trace_file) - 1);
        }
    }

    /* At least one level must be enabled */
    if (!cfg->l1_enabled && !cfg->l2_enabled && !cfg->l3_enabled) {
        fprintf(stderr, "Error: all cache levels are disabled.\n");
        usage(argv[0]);
    }

    /* Enabled levels must have size and assoc */
    int ok = 1;
    if (cfg->l1_enabled && (!cfg->l1_size || !cfg->l1_assoc)) {
        fprintf(stderr, "Error: L1 is enabled but --l1-size / --l1-assoc are missing.\n");
        ok = 0;
    }
    if (cfg->l2_enabled && (!cfg->l2_size || !cfg->l2_assoc)) {
        fprintf(stderr, "Error: L2 is enabled but --l2-size / --l2-assoc are missing.\n");
        ok = 0;
    }
    if (cfg->l3_enabled && (!cfg->l3_size || !cfg->l3_assoc)) {
        fprintf(stderr, "Error: L3 is enabled but --l3-size / --l3-assoc are missing.\n");
        ok = 0;
    }
    if (!cfg->block_size) {
        fprintf(stderr, "Error: --block is required.\n");
        ok = 0;
    }
    if (cfg->trace_file[0] == '\0') {
        fprintf(stderr, "Error: trace file path is required.\n");
        ok = 0;
    }
    if (!ok) usage(argv[0]);
}

void print_config(const SimConfig *cfg) {
    printf("=== Configuration ===\n");
    if (cfg->l1_enabled)
        printf("  L1: %d bytes, %d-way, latency=%d cyc\n",
               cfg->l1_size, cfg->l1_assoc, cfg->l1_latency);
    else
        printf("  L1: disabled\n");
    if (cfg->l2_enabled)
        printf("  L2: %d bytes, %d-way, latency=%d cyc\n",
               cfg->l2_size, cfg->l2_assoc, cfg->l2_latency);
    else
        printf("  L2: disabled\n");
    if (cfg->l3_enabled)
        printf("  L3: %d bytes, %d-way, latency=%d cyc\n",
               cfg->l3_size, cfg->l3_assoc, cfg->l3_latency);
    else
        printf("  L3: disabled\n");
    printf("  Block size:  %d bytes\n", cfg->block_size);
    printf("  Mem latency: %d cycles\n", cfg->mem_latency);
    printf("  Policy: %s\n", cfg->policy == POLICY_LRU ? "LRU" : "FIFO");
    printf("  Trace: %s\n\n", cfg->trace_file);
}
