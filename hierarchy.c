#include <stdio.h>
#include <stdlib.h>
#include "hierarchy.h"

/*
 * Insert `address` into levels[level_idx], propagating any dirty eviction
 * to the next lower level (or counting a memory access if we fall off L3).
 *
 * Used only for writeback propagation — demand-access stats are counted
 * separately in hierarchy_access().
 */
static void writeback_to(CacheLevel **levels, int level_idx,
                         uint64_t address, ReplacementPolicy policy,
                         uint64_t ts, uint64_t *mem_accesses) {
    if (level_idx > 2) {
        /* Dirty eviction from L3 — write to main memory */
        (*mem_accesses)++;
        return;
    }
    CacheLevel *lvl = levels[level_idx];
    uint64_t wb_addr = 0;
    /* cache_insert updates evictions + dirty_writebacks on the level */
    int dirty = cache_insert(lvl, address, policy, ts, &wb_addr);
    if (dirty)
        writeback_to(levels, level_idx + 1, wb_addr, policy, ts, mem_accesses);
}

/* ── Public API ───────────────────────────────────────────────────────── */

CacheHierarchy *hierarchy_init(const SimConfig *cfg) {
    CacheHierarchy *h = malloc(sizeof(CacheHierarchy));
    if (!h) { perror("malloc"); exit(EXIT_FAILURE); }

    h->L1 = cache_level_init("L1", cfg->l1_size, cfg->block_size, cfg->l1_assoc);
    h->L2 = cache_level_init("L2", cfg->l2_size, cfg->block_size, cfg->l2_assoc);
    h->L3 = cache_level_init("L3", cfg->l3_size, cfg->block_size, cfg->l3_assoc);
    h->timestamp       = 0;
    h->memory_accesses = 0;
    return h;
}

void hierarchy_access(CacheHierarchy *hier, char op, uint64_t address,
                      ReplacementPolicy policy) {
    hier->timestamp++;
    uint64_t ts = hier->timestamp;

    CacheLevel *levels[3] = { hier->L1, hier->L2, hier->L3 };
    int missed[3] = { 0, 0, 0 };
    int found_at  = -1;  /* index of level that hit, -1 = memory */

    /* ── Lookup phase: L1 → L2 → L3 ── */
    for (int i = 0; i < 3; i++) {
        CacheLevel *lvl = levels[i];
        DecodedAddr d;

        lvl->stats.accesses++;
        if (op == 'R') lvl->stats.reads++;
        else           lvl->stats.writes++;

        int way = cache_lookup(lvl, address, &d);

        if (way >= 0) {
            /* HIT at this level */
            lvl->stats.hits++;
            cache_update_lru(lvl, address, way, ts);
            if (op == 'W')
                cache_mark_dirty(lvl, address, way);
            found_at = i;
            break;
        } else {
            /* MISS */
            lvl->stats.misses++;
            missed[i] = 1;
        }
    }

    /* ── Fill phase ── */
    if (found_at == -1) {
        /* All three levels missed — fetch from main memory */
        hier->memory_accesses++;
    }

    /*
     * Fill every level that missed, from the lowest (closest to memory) up
     * to L1.  We go high-index → low-index so that a dirty eviction from L1
     * propagates into an L2 that has already been filled (making room for the
     * evicted block if L2 was also full).
     *
     * fill_start: highest missed level we need to fill.
     *   - All miss  (found_at == -1): fill L3(2), L2(1), L1(0)  → fill_start = 2
     *   - L2 hit    (found_at ==  1): fill L1(0)                → fill_start = 0
     *   - L1 hit    (found_at ==  0): nothing to fill            → fill_start = -1
     */
    int fill_start = (found_at == -1) ? 2 : found_at - 1;

    for (int i = fill_start; i >= 0; i--) {
        CacheLevel *lvl = levels[i];
        uint64_t wb_addr = 0;

        /* Insert the fetched block; cache_insert handles eviction/dirty stats */
        int dirty = cache_insert(lvl, address, policy, ts, &wb_addr);

        if (dirty) {
            /* Dirty line evicted from levels[i] — write it to levels[i+1] */
            writeback_to(levels, i + 1, wb_addr, policy, ts,
                         &hier->memory_accesses);
        }

        /* Write-allocate: after inserting into L1, mark the line dirty */
        if (op == 'W' && i == 0) {
            DecodedAddr d;
            int way = cache_lookup(lvl, address, &d);
            if (way >= 0)
                cache_mark_dirty(lvl, address, way);
        }
    }

    /* ── Per-access output line ── */
    printf("%c 0x%08llx -> ", op, (unsigned long long)address);
    int sep = 0;
    for (int i = 0; i < 3; i++) {
        if (missed[i]) {
            printf("%s%s MISS", sep ? ", " : "", levels[i]->name);
            sep = 1;
        } else if (found_at == i) {
            printf("%s%s HIT", sep ? ", " : "", levels[i]->name);
            break;
        }
    }
    if (found_at == -1)
        printf(", MEMORY");
    printf("\n");
}

void hierarchy_free(CacheHierarchy *hier) {
    if (!hier) return;
    cache_free(hier->L1);
    cache_free(hier->L2);
    cache_free(hier->L3);
    free(hier);
}
