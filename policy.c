#include "policy.h"
#include "cache.h"  /* full definition of CacheLine */

int find_victim(CacheLine *lines, int assoc, ReplacementPolicy policy) {
    /* Prefer an invalid (empty) line — no real eviction needed. */
    for (int i = 0; i < assoc; i++) {
        if (!lines[i].valid)
            return i;
    }

    /* All lines are valid; pick based on policy. */
    int    victim = 0;
    uint64_t best;

    if (policy == POLICY_LRU) {
        best = lines[0].last_used;
        for (int i = 1; i < assoc; i++) {
            if (lines[i].last_used < best) {
                best   = lines[i].last_used;
                victim = i;
            }
        }
    } else { /* FIFO */
        best = lines[0].inserted_at;
        for (int i = 1; i < assoc; i++) {
            if (lines[i].inserted_at < best) {
                best   = lines[i].inserted_at;
                victim = i;
            }
        }
    }

    return victim;
}
