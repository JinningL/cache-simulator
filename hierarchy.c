#include <stdio.h>
#include <stdlib.h>
#include "hierarchy.h"

/*
 * Write `address` into levels[level_idx], propagating any dirty eviction
 * deeper.  Falls off to main memory when level_idx >= num_levels.
 * This only tracks evictions/dirty-writebacks; demand-access stats
 * (accesses, reads, writes, hits, misses) are counted in hierarchy_access.
 */
static void writeback_to(CacheLevel **levels, int num_levels, int level_idx,
                         uint64_t address, ReplacementPolicy policy,
                         uint64_t ts, uint64_t *mem_accesses) {
    if (level_idx >= num_levels) {
        (*mem_accesses)++;  /* dirty eviction from the last level → memory */
        return;
    }
    uint64_t wb_addr = 0;
    int dirty = cache_insert(levels[level_idx], address, policy, ts, &wb_addr);
    if (dirty)
        writeback_to(levels, num_levels, level_idx + 1, wb_addr, policy, ts, mem_accesses);
}

/* ── Public API ── */

CacheHierarchy *hierarchy_init(const SimConfig *cfg) {
    CacheHierarchy *h = malloc(sizeof(CacheHierarchy));
    if (!h) { perror("malloc"); exit(EXIT_FAILURE); }

    h->levels     = malloc(sizeof(CacheLevel *) * 3);
    if (!h->levels) { perror("malloc"); exit(EXIT_FAILURE); }

    h->num_levels     = 0;
    h->mem_latency    = cfg->mem_latency;
    h->timestamp      = 0;
    h->memory_accesses= 0;

    /* Append only enabled levels, preserving L1 < L2 < L3 order */
    if (cfg->l1_enabled)
        h->levels[h->num_levels++] = cache_level_init(
            "L1", cfg->l1_size, cfg->block_size, cfg->l1_assoc, cfg->l1_latency);
    if (cfg->l2_enabled)
        h->levels[h->num_levels++] = cache_level_init(
            "L2", cfg->l2_size, cfg->block_size, cfg->l2_assoc, cfg->l2_latency);
    if (cfg->l3_enabled)
        h->levels[h->num_levels++] = cache_level_init(
            "L3", cfg->l3_size, cfg->block_size, cfg->l3_assoc, cfg->l3_latency);

    return h;
}

void hierarchy_access(CacheHierarchy *hier, char op, uint64_t address,
                      ReplacementPolicy policy) {
    hier->timestamp++;
    uint64_t ts = hier->timestamp;
    int n = hier->num_levels;

    /* Track which levels missed (max 3 levels supported) */
    int missed[3] = {0, 0, 0};
    int found_at  = -1;

    /* ── Lookup phase: check levels in order ── */
    for (int i = 0; i < n; i++) {
        CacheLevel *lvl = hier->levels[i];
        DecodedAddr d;

        lvl->stats.accesses++;
        if (op == 'R') lvl->stats.reads++;
        else           lvl->stats.writes++;

        int way = cache_lookup(lvl, address, &d);

        if (way >= 0) {
            lvl->stats.hits++;
            cache_update_lru(lvl, address, way, ts);
            if (op == 'W')
                cache_mark_dirty(lvl, address, way);
            found_at = i;
            break;
        } else {
            lvl->stats.misses++;
            missed[i] = 1;
        }
    }

    /* All levels missed → fetch from main memory */
    if (found_at == -1)
        hier->memory_accesses++;

    /*
     * Fill phase: insert block into every missed level, from the
     * lowest missed (farthest from CPU) up to the first level.
     * Going high-index → low-index means a dirty eviction from L1
     * propagates into L2 that has already been prepared.
     *
     *   fill_start = n-1          (all missed, go from last level up)
     *   fill_start = found_at-1   (hit at found_at, fill levels above it)
     *   fill_start = -1           (L1 hit, nothing to fill)
     */
    int fill_start = (found_at == -1) ? n - 1 : found_at - 1;

    for (int i = fill_start; i >= 0; i--) {
        uint64_t wb_addr = 0;
        int dirty = cache_insert(hier->levels[i], address, policy, ts, &wb_addr);
        if (dirty)
            writeback_to(hier->levels, n, i + 1, wb_addr, policy, ts,
                         &hier->memory_accesses);

        /* Write-allocate: freshly inserted L1 line is immediately dirty */
        if (op == 'W' && i == 0) {
            DecodedAddr d;
            int way = cache_lookup(hier->levels[0], address, &d);
            if (way >= 0)
                cache_mark_dirty(hier->levels[0], address, way);
        }
    }

    /* ── Per-access output line ── */
    printf("%c 0x%08llx -> ", op, (unsigned long long)address);
    int sep = 0;
    for (int i = 0; i < n; i++) {
        if (missed[i]) {
            printf("%s%s MISS", sep ? ", " : "", hier->levels[i]->name);
            sep = 1;
        } else if (found_at == i) {
            printf("%s%s HIT", sep ? ", " : "", hier->levels[i]->name);
            break;
        }
    }
    if (found_at == -1)
        printf(", MEMORY");
    printf("\n");
}

void hierarchy_free(CacheHierarchy *hier) {
    if (!hier) return;
    for (int i = 0; i < hier->num_levels; i++)
        cache_free(hier->levels[i]);
    free(hier->levels);
    free(hier);
}
