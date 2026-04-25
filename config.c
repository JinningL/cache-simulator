#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s\n"
        "  --l1-size  <bytes>  --l1-assoc <n>\n"
        "  --l2-size  <bytes>  --l2-assoc <n>\n"
        "  --l3-size  <bytes>  --l3-assoc <n>\n"
        "  --block    <bytes>\n"
        "  --policy   LRU|FIFO\n"
        "  <trace_file>\n\n"
        "Example:\n"
        "  %s --l1-size 32768 --l1-assoc 4 \\\n"
        "     --l2-size 262144 --l2-assoc 8 \\\n"
        "     --l3-size 8388608 --l3-assoc 16 \\\n"
        "     --block 64 --policy LRU trace.txt\n",
        prog, prog);
    exit(EXIT_FAILURE);
}

void parse_args(int argc, char *argv[], SimConfig *cfg) {
    /* Set defaults so we can detect missing required args */
    cfg->l1_size   = 0; cfg->l1_assoc = 0;
    cfg->l2_size   = 0; cfg->l2_assoc = 0;
    cfg->l3_size   = 0; cfg->l3_assoc = 0;
    cfg->block_size = 0;
    cfg->policy    = POLICY_LRU;
    cfg->trace_file[0] = '\0';

    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "--l1-size")  == 0) cfg->l1_size   = atoi(argv[++i]);
        else if (strcmp(argv[i], "--l1-assoc") == 0) cfg->l1_assoc  = atoi(argv[++i]);
        else if (strcmp(argv[i], "--l2-size")  == 0) cfg->l2_size   = atoi(argv[++i]);
        else if (strcmp(argv[i], "--l2-assoc") == 0) cfg->l2_assoc  = atoi(argv[++i]);
        else if (strcmp(argv[i], "--l3-size")  == 0) cfg->l3_size   = atoi(argv[++i]);
        else if (strcmp(argv[i], "--l3-assoc") == 0) cfg->l3_assoc  = atoi(argv[++i]);
        else if (strcmp(argv[i], "--block")    == 0) cfg->block_size = atoi(argv[++i]);
        else if (strcmp(argv[i], "--policy")   == 0) {
            i++;
            if      (strcmp(argv[i], "LRU")  == 0) cfg->policy = POLICY_LRU;
            else if (strcmp(argv[i], "FIFO") == 0) cfg->policy = POLICY_FIFO;
            else { fprintf(stderr, "Unknown policy: %s\n", argv[i]); usage(argv[0]); }
        }
        else {
            /* Positional argument: trace file path */
            strncpy(cfg->trace_file, argv[i], sizeof(cfg->trace_file) - 1);
        }
    }

    /* Validate required fields */
    if (!cfg->l1_size || !cfg->l1_assoc ||
        !cfg->l2_size || !cfg->l2_assoc ||
        !cfg->l3_size || !cfg->l3_assoc ||
        !cfg->block_size || cfg->trace_file[0] == '\0') {
        fprintf(stderr, "Error: missing required arguments.\n");
        usage(argv[0]);
    }
}

void print_config(const SimConfig *cfg) {
    printf("=== Configuration ===\n");
    printf("  L1: %d bytes, %d-way\n", cfg->l1_size, cfg->l1_assoc);
    printf("  L2: %d bytes, %d-way\n", cfg->l2_size, cfg->l2_assoc);
    printf("  L3: %d bytes, %d-way\n", cfg->l3_size, cfg->l3_assoc);
    printf("  Block size: %d bytes\n", cfg->block_size);
    printf("  Policy: %s\n", cfg->policy == POLICY_LRU ? "LRU" : "FIFO");
    printf("  Trace: %s\n\n", cfg->trace_file);
}
